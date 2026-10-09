"""Regression coverage for changed-path selection and Git event ranges."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock

SPEC = importlib.util.spec_from_file_location(
    "select_build", Path(__file__).resolve().parents[1] / ".github/scripts/select-build.py"
)
selector = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(selector)


class SelectionTests(unittest.TestCase):
    def test_documentation_does_not_build_on_pr_or_push(self):
        for full in (False, True):
            result = selector.select(
                ["README.md", "docs/setup.md", ".github/assets/rsp-banner.jpg", "LICENSE"],
                full=full,
            )
            self.assertEqual(result["changed"], "false")
            self.assertEqual(result["targets"], "")
            self.assertEqual(result["e2e"], "false")

    def test_leaf_modules(self):
        cases = {
            "rsp-cli/src/client_session.cpp": {"Client", "ClientTest"},
            "rsp-svr/user/src/main.cpp": {"User"},
            "rsp-svr/room/src/room/room.cpp": {
                "Room", "RoomTest", "RoomShutdownTest", "RoomContents",
            },
            "rsp-svr/room_contents/src/so.cpp": {
                "RoomContents", "Room", "RoomTest", "RoomShutdownTest",
            },
            "rsp-svr/rci/include/interface.hpp": {
                "RoomContents", "Room", "RoomTest", "RoomShutdownTest",
            },
        }
        for path, expected in cases.items():
            with self.subTest(path=path):
                result = selector.select([path])
                self.assertEqual(set(result["targets"].split()), expected)
                self.assertEqual(result["e2e"], "false")

    def test_shared_library_reaches_all_runtime_consumers(self):
        result = selector.select(["rsp-libs/include/rsplib/session.hpp"])
        self.assertEqual(set(result["targets"].split()), {
            "Libs", "Client", "User", "Room", "RoomContents",
            "LibsTest", "ClientTest", "RoomTest", "RoomShutdownTest",
        })
        self.assertEqual(result["e2e"], "true")

    def test_protocol_reaches_all_components(self):
        for path in ("proto/common/ping.proto", "gen-proto/CMakeLists.txt"):
            self.assertEqual(set(selector.select([path])["targets"].split()),
                             set(selector.COMPONENTS) | set(selector.TEST_TARGETS))

    def test_unknown_config_ci_and_integration_tests_build_all(self):
        for path in ("CMakeLists.txt", ".github/workflows/cmake-multi-platform.yml",
                     ".github/scripts/select-build.py", "tests/e2e_chat.py",
                     "protoc.sh", "new-module/src/new.cpp"):
            with self.subTest(path=path):
                self.assertEqual(set(selector.select([path])["targets"].split()),
                                 set(selector.COMPONENTS) | set(selector.TEST_TARGETS))

    def test_main_code_push_runs_full_regression(self):
        result = selector.select(["rsp-cli/src/main.cpp"], full=True)
        self.assertEqual(set(result["targets"].split()),
                         set(selector.COMPONENTS) | set(selector.TEST_TARGETS))
        self.assertEqual(result["e2e"], "true")

    def test_combined_changes(self):
        result = selector.select(["README.md", "rsp-cli/test/state_login_test.cpp",
                                  "rsp-svr/user/src/main.cpp"])
        self.assertEqual(set(result["targets"].split()), {"Client", "ClientTest", "User"})


class GitRangeTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.original_cwd = os.getcwd()
        os.chdir(self.directory.name)
        self.addCleanup(self.directory.cleanup)
        self.addCleanup(os.chdir, self.original_cwd)
        self.run_git("init", "-q")
        self.run_git("config", "user.email", "ci@example.invalid")
        self.run_git("config", "user.name", "CI Test")
        self.write("README.md")
        self.base = self.commit()

    def run_git(self, *args):
        return subprocess.check_output(["git", *args], stderr=subprocess.DEVNULL).decode().strip()

    def write(self, path):
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text("test\n", encoding="utf-8")

    def commit(self):
        self.run_git("add", "-A")
        self.run_git("commit", "-qm", "test")
        return self.run_git("rev-parse", "HEAD")

    def test_pr_merge_base_excludes_unrelated_base_branch_changes(self):
        self.run_git("checkout", "-qb", "feature")
        self.write("rsp-cli/src/example.cpp")
        head = self.commit()
        self.run_git("checkout", "-q", "-")
        self.write("rsp-svr/user/src/unrelated.cpp")
        base = self.commit()
        event = {"pull_request": {"base": {"sha": base}, "head": {"sha": head}}}
        self.assertEqual(selector.changed_paths(event, "pull_request"),
                         ["rsp-cli/src/example.cpp"])

    def test_deleted_file_is_detected(self):
        self.write("rsp-svr/user/src/deleted.cpp")
        base = self.commit()
        Path("rsp-svr/user/src/deleted.cpp").unlink()
        head = self.commit()
        self.assertEqual(selector.changed_paths({"before": base, "after": head}, "push"),
                         ["rsp-svr/user/src/deleted.cpp"])

    def test_cross_module_rename_counts_both_paths(self):
        self.write("rsp-cli/src/example.cpp")
        base = self.commit()
        Path("rsp-svr/user/src").mkdir(parents=True)
        Path("rsp-cli/src/example.cpp").rename("rsp-svr/user/src/example.cpp")
        head = self.commit()
        paths = selector.changed_paths({"before": base, "after": head}, "push")
        self.assertEqual(set(paths), {"rsp-cli/src/example.cpp", "rsp-svr/user/src/example.cpp"})

    def test_new_branch_and_missing_previous_tip_use_full_build(self):
        head = self.run_git("rev-parse", "HEAD")
        for base in ("0" * 40, "f" * 40):
            paths = selector.changed_paths({"before": base, "after": head}, "push")
            self.assertEqual(selector.select(paths)["e2e"], "true")

    def test_deleted_branch_skips_git_and_builds(self):
        events = (
            {"before": self.base, "after": "0" * 40, "deleted": True},
            {"before": self.base, "after": "0" * 40},
            {"before": "f" * 40, "after": "0" * 40},
            {"before": "0" * 40, "after": "0" * 40},
            {"before": self.base, "after": self.base, "deleted": True},
        )
        for event in events:
            with self.subTest(event=event):
                with mock.patch.object(selector, "git") as git_mock:
                    with mock.patch.object(selector.subprocess, "run") as run_mock:
                        paths = selector.changed_paths(event, "push")
                git_mock.assert_not_called()
                run_mock.assert_not_called()
                self.assertEqual(paths, [])
                result = selector.select(paths, full=True)
                self.assertEqual(result["targets"], "")
                for flag in ("changed", "libs", "client", "room", "e2e"):
                    self.assertEqual(result[flag], "false")

    def test_deleted_push_cli_emits_successful_no_build_outputs(self):
        event_file = Path("event.json").resolve()
        output_file = Path("outputs.txt").resolve()
        event_file.write_text(json.dumps({
            "before": self.base, "after": "0" * 40, "deleted": True,
            "ref": "refs/heads/feat/ci",
        }), encoding="utf-8")
        result = subprocess.run(
            ["python3", str(Path(selector.__file__).resolve())],
            env={**os.environ, "GITHUB_EVENT_PATH": str(event_file),
                 "GITHUB_EVENT_NAME": "push", "GITHUB_OUTPUT": str(output_file)},
            capture_output=True, text=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output_file.read_text().splitlines(), [
            "changed=false", "targets=", "libs=false", "client=false",
            "room=false", "e2e=false",
        ])

    def test_whitespace_paths_are_preserved(self):
        self.write(" file with spaces.cpp")
        head = self.commit()
        self.assertEqual(selector.changed_paths({"before": self.base, "after": head}, "push"),
                         [" file with spaces.cpp"])


if __name__ == "__main__":
    unittest.main()

