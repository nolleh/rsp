# Change-aware CMake CI

The existing build workflow calls `detect-build-changes.yml` before installing
dependencies. Documentation-only PRs and pushes keep the required
`build (ubuntu-latest, Release, gcc)` check but skip dependency installation,
CMake configuration, compilation and C++/E2E tests. The lightweight detector
and its Python regression tests still run.

## Pull request selection

| Changed path | Selected components |
| --- | --- |
| `rsp-cli/` | Client |
| `rsp-svr/user/` | User |
| `rsp-svr/room/` | Room, RoomContents |
| `rsp-svr/room_contents/` | RoomContents, Room |
| `rsp-svr/rci/` | Room, RoomContents |
| `rsp-libs/` | Libs and all runtime consumers |
| `proto/`, `gen-proto/` | All components |
| Root build files, CI, integration tests, unknown paths | All components |

Corresponding LibsTest, ClientTest and RoomTest targets are included when their
component is affected. CMake builds the selected targets' compile/link
dependencies automatically. Room and RoomContents are selected together because
Room loads the plugin dynamically, an edge absent from CMake's link graph.
Shared protocol changes conservatively validate every consumer rather than
trying to infer dependencies from individual protobuf imports.

PRs run the unit suites for affected components. Both existing E2E scenarios
run when the selection contains Client, User, Room and RoomContents, including
shared-library, protocol and CI changes. A leaf-module PR skips E2E to avoid
building unrelated executable fixtures. After a code change is pushed to main
(or the existing feat/ci push branch), the full build, unit suites and both
E2E scenarios run. Documentation-only pushes skip this full build too.

## Diff safety

PR detection uses the merge base of the event's base and head commits.
Push detection compares the event's before and after commits; new branches or
an unavailable previous tip use a full build. Deletions and moves across modules
count both old and new paths. Unknown paths default to a full build.

Branch-deletion pushes have no new commit to build. An explicit `deleted` flag
or an all-zero `after` SHA returns an empty selection before any Git commands,
so dependency installation, compilation and C++/E2E tests are skipped normally.

Markdown/reStructuredText, `docs/`, `.github/assets/`, LICENSE and NOTICE are
documentation-only. Do not store build inputs in those directories. Extend
the selector and its tests when introducing a new module or dependency edge.

No workflow-level path filter is used, so required checks are not left pending
on documentation changes. Detector failures fail the existing required build
job. The existing job name and matrix remain unchanged.

Run detector regressions locally:

```sh
python3 -m unittest discover -s tests -p 'test_select_build.py' -v
```

