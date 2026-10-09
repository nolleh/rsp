#!/usr/bin/env python3
"""Verify autonomous User timeouts using real clients with Room unavailable."""
import argparse
from pathlib import Path
import re
import socket
import time

from e2e_chat import Process, TIMEOUT


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=Path("build"))
    parser.add_argument("--log-dir", type=Path, default=Path("build/e2e-logs"))
    args = parser.parse_args()
    build = args.build_dir.resolve()
    args.log_dir.mkdir(parents=True, exist_ok=True)
    processes = []

    def launch(name, executable, *arguments):
        process = Process(name, [str(executable), *arguments], args.log_dir)
        processes.append(process)
        return process

    def login(name):
        client = launch(name, build / "rsp-cli/Client", "127.0.0.1")
        client.expect("type user name to login")
        mark = client.send(name)
        client.expect(r"success to login:\s*" + re.escape(name), mark)
        client.expect("create_room", mark)
        return client

    try:
        for port in (8080, 5559):
            with socket.socket() as probe:
                probe.settimeout(0.2)
                if probe.connect_ex(("127.0.0.1", port)) == 0:
                    raise RuntimeError(f"port {port} is already occupied")

        user = launch("timeout_user", build / "rsp-svr/user/User")
        deadline = time.monotonic() + TIMEOUT
        while True:
            if user.process.poll() is not None:
                raise RuntimeError("User exited during startup; see logs")
            with socket.socket() as probe:
                probe.settimeout(0.2)
                if probe.connect_ex(("127.0.0.1", 8080)) == 0:
                    break
            if time.monotonic() >= deadline:
                raise TimeoutError("User did not listen on port 8080")
            time.sleep(0.1)

        alice = login("timeout_alice")
        bob = login("timeout_bob")
        started = time.monotonic()
        alice_mark = alice.send("2")
        # Queued before expiration: must run when the timeout finishes creation.
        alice.send("1")
        bob_mark = bob.send("3")
        bob.expect("put room id", bob_mark)
        bob_mark = bob.send("1234")

        # No further input is sent to either client until timeout responses arrive.
        alice.expect("unable to create room", alice_mark)
        alice.expect("create_room", alice_mark)
        alice.expect(r"success to logout, bye bye:\s*timeout_alice", alice_mark)
        if alice.process.wait(timeout=TIMEOUT) != 0:
            raise RuntimeError("alice exited with an error")
        bob.expect("unable to join room", bob_mark)
        bob.expect("create_room", bob_mark)
        elapsed = time.monotonic() - started
        if not 4.5 <= elapsed <= 12:
            raise RuntimeError(f"unexpected timeout duration: {elapsed:.2f}s")

        mark = bob.send("1")
        bob.expect(r"success to logout, bye bye:\s*timeout_bob", mark)
        if bob.process.wait(timeout=TIMEOUT) != 0:
            raise RuntimeError("bob exited with an error")
        if user.process.poll() is not None:
            raise RuntimeError("User exited during timeout handling")
        print(f"PASS: two independent timeouts and queued logout in {elapsed:.2f}s",
              flush=True)
    finally:
        for process in reversed(processes):
            process.close()


if __name__ == "__main__":
    main()
