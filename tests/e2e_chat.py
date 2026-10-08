#!/usr/bin/env python3
"""Exercise real User/Room servers and two CLI clients over loopback."""
import argparse
import os
from pathlib import Path
import re
import signal
import socket
import subprocess
import threading
import time

ANSI = re.compile(r"\x1b\[[0-9;]*m")
TIMEOUT = 20


class Process:
    def __init__(self, name, argv, log_dir):
        self.name = name
        self.output = ""
        self.condition = threading.Condition()
        self.log = (log_dir / (name + ".log")).open("wb")
        self.process = subprocess.Popen(
            argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, start_new_session=True)
        self.reader = threading.Thread(target=self.drain, daemon=True)
        self.reader.start()

    def drain(self):
        try:
            while True:
                chunk = os.read(self.process.stdout.fileno(), 4096)
                if not chunk:
                    break
                self.log.write(chunk)
                self.log.flush()
                with self.condition:
                    self.output += chunk.decode("utf-8", errors="replace")
                    self.condition.notify_all()
        finally:
            with self.condition:
                self.condition.notify_all()

    def mark(self):
        with self.condition:
            return len(ANSI.sub("", self.output))

    def send(self, command):
        mark = self.mark()
        self.process.stdin.write((command + "\n").encode())
        self.process.stdin.flush()
        return mark

    def expect(self, pattern, start=0):
        deadline = time.monotonic() + TIMEOUT
        with self.condition:
            while True:
                output = ANSI.sub("", self.output)
                match = re.search(pattern, output[start:])
                if match:
                    return match
                if self.process.poll() is not None and not self.reader.is_alive():
                    raise RuntimeError(
                        f"{self.name} exited before {pattern!r}:\n{output}")
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError(
                        f"{self.name} did not print {pattern!r}:\n{output}")
                self.condition.wait(min(remaining, 0.2))

    def close(self):
        if self.process.poll() is None:
            os.killpg(self.process.pid, signal.SIGTERM)
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(self.process.pid, signal.SIGKILL)
                self.process.wait(timeout=5)
        self.reader.join(timeout=5)
        self.log.close()


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
        print(f"PASS: {name} logged in", flush=True)
        return client

    def chat(sender, receiver, message):
        received_after = receiver.mark()
        mark = sender.send("2")
        sender.expect("type message to send", mark)
        sender.send(message)
        receiver.expect(
            re.escape(f"({sender.name}):{message}"), received_after)
        print(f"PASS: {sender.name} -> {receiver.name}: {message}", flush=True)

    try:
        # Refuse to test against an unrelated service already using this port.
        with socket.socket() as probe:
            probe.settimeout(0.2)
            if probe.connect_ex(("127.0.0.1", 8080)) == 0:
                raise RuntimeError("port 8080 is already occupied")

        room = launch("room", build / "rsp-svr/room/Room")
        user = launch("user", build / "rsp-svr/user/User")
        deadline = time.monotonic() + TIMEOUT
        while True:
            if room.process.poll() is not None or user.process.poll() is not None:
                raise RuntimeError("a server exited during startup; see logs")
            with socket.socket() as probe:
                probe.settimeout(0.2)
                if probe.connect_ex(("127.0.0.1", 8080)) == 0:
                    break
            if time.monotonic() >= deadline:
                raise TimeoutError("User server did not listen on port 8080")
            time.sleep(0.1)

        alice = login("e2e_alice")
        mark = alice.send("2")
        created = alice.expect(r"created room #\s*(\d+)\s*, and joined", mark)
        room_id = created.group(1)
        alice.expect("send message", mark)
        print(f"PASS: alice created and joined room {room_id}", flush=True)

        bob = login("e2e_bob")
        mark = bob.send("3")
        bob.expect("put room id", mark)
        mark = bob.send(room_id)
        bob.expect(r"joined room #\s*" + room_id, mark)
        bob.expect("send message", mark)
        print(f"PASS: bob joined room {room_id}", flush=True)

        chat(alice, bob, "hello-from-alice")
        chat(bob, alice, "hello-from-bob")

        for client in (bob, alice):
            mark = client.send("3")
            client.expect(r"res_leave_room received: success\?:\s*(1|true)", mark)
            client.expect("create_room", mark)
            print(f"PASS: {client.name} left room and returned to lobby", flush=True)

        for client in (bob, alice):
            mark = client.send("1")
            client.expect(r"success to logout, bye bye:\s*" + client.name, mark)
            if client.process.wait(timeout=TIMEOUT) != 0:
                raise RuntimeError(f"{client.name} exited with an error")
            client.reader.join(timeout=5)
            if re.search(r"invalid message received|failed to parse|"
                         r"failed to deserialize|failed to fwd",
                         ANSI.sub("", client.output)):
                raise RuntimeError(f"{client.name} reported a protocol error")

        if room.process.poll() is not None or user.process.poll() is not None:
            raise RuntimeError("a server exited during the scenario")
        print("PASS: two-client chat and leave-room E2E", flush=True)
    finally:
        for process in reversed(processes):
            process.close()


if __name__ == "__main__":
    main()
