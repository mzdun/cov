#!/usr/bin/env python3
# Copyright (c) 2023 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import argparse
import os
import subprocess
import sys

__dir__ = os.path.dirname(__file__)

parser = argparse.ArgumentParser()
parser.add_argument("--bin", required=True, metavar="BINDIR")
parser.add_argument("--tests", required=True, metavar="DIR")
parser.add_argument("--version", metavar="SEMVER")
parser.add_argument(
    "--run",
    metavar="TEST[,...]",
    action="append",
    default=[],
)
args = parser.parse_args()

ext = ".exe" if sys.platform == "win32" else ""

if not os.path.isdir(args.bin) and os.path.isdir(f"./build/{args.bin}"):
    args.bin = f"./build/{args.bin}"

if not os.path.isdir(f"{__dir__}/{args.tests}") and os.path.isdir(
    f"{__dir__}/main-set/{args.tests}"
):
    args.tests = f"main-set/{args.tests}"

if args.version is None:
    tools = os.path.join(os.path.dirname(os.path.dirname(__dir__)), "tools")
    print(tools)
    sys.path.append(tools)
    from github.cmake import get_version

    project = get_version()
    args.version = project.ver()

new_args = [
    (f"--{key}", value)
    for key, value in {
        "target": f"{args.bin}/bin/cov{ext}",
        "tests": f"{__dir__}/{args.tests}",
        "data-dir": f"{__dir__}/data",
        "version": args.version,
        "install": f"{__dir__}/copy",
        "install-with": f"{args.bin}/elsewhere/libexec/cov/cov-echo{ext}",
    }.items()
]

new_args.extend([("--run", run) for run in args.run])

sys.exit(
    subprocess.run(
        [
            sys.executable,
            f"{__dir__}/test_driver.py",
            *(item for arg in new_args for item in arg),
        ],
        shell=False,
    ).returncode
)
