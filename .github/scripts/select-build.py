"""Select CMake targets, including reverse and runtime dependencies."""
import json
import os
import subprocess

COMPONENTS = ("Proto", "Libs", "Client", "User", "Room", "RoomContents")
DEPENDENTS = {
    "Proto": {"Libs", "RoomContents"},
    "Libs": {"Client", "User", "Room"},
    "RoomContents": {"Room"},  # Loaded dynamically by Room.
    "Room": {"RoomContents"},  # Include the plugin needed at runtime.
}
PREFIXES = {
    "proto/": {"Proto"}, "gen-proto/": {"Proto"},
    "rsp-libs/": {"Libs"}, "rsp-cli/": {"Client"},
    "rsp-svr/user/": {"User"}, "rsp-svr/room/": {"Room"},
    "rsp-svr/room_contents/": {"RoomContents"},
    "rsp-svr/rci/": {"Room", "RoomContents"},
}
TESTS = {"Libs": "LibsTest", "Client": "ClientTest", "Room": "RoomTest"}


def select(paths, full=False):
    affected = set()
    for path in paths:
        if path.endswith((".md", ".rst")) or path.startswith(
            ("docs/", ".github/assets/")
        ) or path in {"LICENSE", "NOTICE"}:
            continue
        for prefix, components in PREFIXES.items():
            if path.startswith(prefix):
                affected.update(components)
                break
        else:
            # Unknown paths, build configuration, CI and E2E changes: full build.
            affected.update(COMPONENTS)
    changed = bool(affected)
    if changed and full:
        affected.update(COMPONENTS)
    pending = list(affected)
    while pending:
        for dependent in DEPENDENTS.get(pending.pop(), set()):
            if dependent not in affected:
                affected.add(dependent)
                pending.append(dependent)
    targets = [target for target in COMPONENTS if target in affected]
    targets += [test for component, test in TESTS.items() if component in affected]
    return {
        "changed": str(changed).lower(),
        "targets": " ".join(targets),
        "libs": str("Libs" in affected).lower(),
        "client": str("Client" in affected).lower(),
        "room": str("Room" in affected).lower(),
        "e2e": str({"Client", "User", "Room", "RoomContents"} <= affected).lower(),
    }


def git(*args):
    return subprocess.check_output(["git", *args])


def changed_paths(event, event_name):
    if event_name == "pull_request":
        base = event["pull_request"]["base"]["sha"]
        head = event["pull_request"]["head"]["sha"]
        base = git("merge-base", base, head).decode().strip()
    elif event_name == "push":
        base, head = event["before"], event["after"]
        if base == "0" * 40:
            return ["CMakeLists.txt"]
        if subprocess.run(
            ["git", "cat-file", "-e", base + "^{commit}"],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        ).returncode:
            return ["CMakeLists.txt"]
    else:
        return ["CMakeLists.txt"]
    # Renames count in both old and new components.
    return [
        path.decode("utf-8", errors="surrogateescape")
        for path in git("diff", "--no-renames", "--name-only", "-z", base, head).split(b"\0")
        if path
    ]


if __name__ == "__main__":
    with open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8") as event_file:
        event = json.load(event_file)
    event_name = os.environ["GITHUB_EVENT_NAME"]
    result = select(changed_paths(event, event_name), full=event_name != "pull_request")
    with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
        for key, value in result.items():
            print(f"{key}={value}", file=output)
    print(json.dumps(result, indent=2))

