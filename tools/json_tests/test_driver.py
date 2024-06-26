#!/usr/bin/env python3
# Copyright (c) 2022 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import argparse
import concurrent.futures
import os
import shutil
import subprocess
import sys
import tempfile
from typing import List, Optional, Tuple

from driver.commands import HANDLERS
from driver.test import Env, Test
from driver.testbed import Counters, task

if os.name == "nt":
    sys.stdout.reconfigure(encoding="utf-8")  # type: ignore

RUN_LINEAR = os.environ.get("RUN_LINEAR", 0) != 0

parser = argparse.ArgumentParser()
parser.add_argument("--target", required=True, metavar="EXE")
parser.add_argument("--tests", required=True, metavar="DIR")
parser.add_argument("--data-dir", required=True, metavar="DIR")
parser.add_argument("--version", required=True, metavar="SEMVER")
parser.add_argument("--install", metavar="DIR")
parser.add_argument(
    "--install-with",
    metavar="TOOL",
    type=lambda s: s.split(";"),
    action="append",
    default=[],
)
parser.add_argument(
    "--run",
    metavar="TEST[,...]",
    type=lambda s: [int(x) for x in s.split(",")],
    action="append",
    default=[],
)


def _run_with(test: Test, *args: str):
    cwd = None if test.linear else test.cwd
    subprocess.run(args, shell=False, cwd=cwd)


def _git(test: Test, args: List[str]):
    _run_with(test, "git", *args)


def _cov(target: str):
    def __(test: "Test", aditional: List[str]):
        _run_with(test, target, *aditional)

    return __


def _detach(test: Test, args):
    dirname = test.path(args[0])
    with open(os.path.join(dirname, "HEAD")) as f:
        link = f.readline().strip()
    print(link)
    if link[:5] != "ref: ":
        return
    link = link[5:]
    with open(os.path.join(dirname, link)) as f:
        link = f.readline().strip()
    with open(os.path.join(dirname, "HEAD"), "w") as f:
        print(link, file=f)


def _git_config(name, value):
    subprocess.run(["git", "config", "--global", name, value], capture_output=True)


def _ensure_identity_exists():
    proc = subprocess.run(["git", "config", "user.name"], capture_output=True)
    if proc.stdout != b"":
        return
    _git_config("user.email", "test_runner@example.com")
    _git_config("user.name", "Test Runner")
    _git_config("init.defaultBranch", "main")


def _enum_tests(args: argparse.Namespace):
    testsuite: List[str] = []
    for root, _, files in os.walk(args.tests):
        for filename in files:
            testsuite.append(os.path.join(root, filename))
    return testsuite


def _make_env(args: argparse.Namespace, counter_total: int):
    target = args.target
    tempdir = os.path.abspath(
        os.path.join(tempfile.gettempdir(), "test-driver")
    ).replace("\\", "/")
    tempdir_alt = None
    data_dir_alt = None

    if os.name == "nt":
        tempdir = os.path.realpath(tempdir).replace("\\", "/")

    if os.sep != "/":
        tempdir_alt = tempdir.replace("/", os.sep)
        data_dir_alt = args.data_dir.replace("/", os.sep)

    length = counter_total
    digits = 1
    while length > 9:
        digits += 1
        length = length // 10

    _HANDLERS = {
        **HANDLERS,
        "git": (0, _git),
        "detach": (1, _detach),
    }

    return Env(
        target=target,
        build_dir=os.path.dirname(os.path.dirname(target)),
        data_dir=args.data_dir,
        inst_dir=os.path.join(os.path.dirname(args.data_dir), "copy", "bin"),
        tempdir=tempdir,
        version=args.version,
        counter_digits=digits,
        counter_total=counter_total,
        handlers=_HANDLERS,
        data_dir_alt=data_dir_alt,
        tempdir_alt=tempdir_alt,
    )


def _append(key: str, value: str):
    vals = os.environ.get(key, "").split(os.pathsep)
    vals.append(value)
    os.environ[key] = os.pathsep.join(val for val in vals if val != "")


def _install(install: Optional[str], install_with: List[str], env: Env):
    if install is None:
        return

    root_dir = os.path.dirname(os.path.dirname(env.target))
    filters_target = os.path.join(install, "additional-filters")
    filters_source = os.path.join(os.path.dirname(install), "test-filters")

    _append("COV_FILTER_PATH", filters_target)
    _append("COV_PATH", os.path.join(os.path.abspath(root_dir), "dont-copy"))

    os.makedirs(os.path.join(install, "bin"), exist_ok=True)
    os.makedirs(os.path.join(install, "libexec", "cov"), exist_ok=True)
    os.makedirs(filters_target, exist_ok=True)

    shutil.copy2(env.target, os.path.join(install, "bin"))
    if os.path.exists(os.path.join(root_dir, "libexec")):
        shutil.copytree(
            os.path.join(root_dir, "libexec"),
            os.path.join(install, "libexec"),
            dirs_exist_ok=True,
        )
    if os.path.exists(os.path.join(root_dir, "share")):
        shutil.copytree(
            os.path.join(root_dir, "share"),
            os.path.join(install, "share"),
            dirs_exist_ok=True,
        )

    for _, dirs, files in os.walk(filters_source):
        dirs[:] = []
        for filename in files:
            name, ext = os.path.splitext(filename)
            if ext != ".py":
                continue
            shutil.copy2(
                os.path.join(filters_source, filename),
                os.path.join(filters_target, name),
            )

    for module in install_with:
        shutil.copy2(module, os.path.join(install, "libexec", "cov"))

    env.target = os.path.join(install, "bin", os.path.basename(env.target))
    env.handlers["cov"] = (0, _cov(env.target))


def _load_tests(testsuite: List[str], run: List[str]):
    counter = 0
    independent_tests: List[Tuple[Test, int]] = []
    linear_tests: List[Tuple[Test, int]] = []

    for filename in sorted(testsuite):
        counter += 1
        if counter not in run:
            continue

        try:
            test = Test.load(filename, counter)
            if not test.ok:
                continue
        except Exception:
            print(">>>>>>", filename, file=sys.stderr)
            raise

        if test.linear:
            linear_tests.append((test, counter))
        else:
            independent_tests.append((test, counter))

    return independent_tests, linear_tests


def __main__():
    args = parser.parse_args()
    args.install_with = [item for groups in args.install_with for item in groups]
    args.run = [item for groups in args.run for item in groups]
    args.data_dir = os.path.abspath(args.data_dir).replace("\\", "/")

    _ensure_identity_exists()
    testsuite = _enum_tests(args)
    env = _make_env(args, len(testsuite))
    print("target: ", env.target, env.version)
    if env.data_dir_alt is None:
        print("data:   ", env.data_dir)
    else:
        print("data:   ", env.data_dir, env.data_dir_alt)
    print("tests:  ", args.tests)
    if env.tempdir_alt is None:
        print("$TEMP:  ", env.tempdir)
    else:
        print("$TEMP:  ", env.tempdir, env.tempdir_alt)

    os.makedirs(env.tempdir, exist_ok=True)
    _install(args.install, args.install_with, env)

    run = args.run

    if not len(run):
        run = list(range(1, len(testsuite) + 1))
    else:
        print("running:", ", ".join(str(x) for x in run))

    independent_tests, linear_tests = _load_tests(testsuite, run)

    counters = Counters()

    if RUN_LINEAR:
        for test, counter in independent_tests:
            outcome, test_id, message, tempdir = task(env, test, counter)
            counters.report(outcome, test_id, message)
            shutil.rmtree(tempdir, ignore_errors=True)
    else:
        hw_concurrency = os.cpu_count()
        try:
            thread_count = int(os.environ.get("POOL_SIZE"))
        except TypeError:
            thread_count = min(61, max(1, hw_concurrency * 2))
        with concurrent.futures.ThreadPoolExecutor(thread_count) as executor:
            futures = []
            for test, counter in independent_tests:
                futures.append(executor.submit(task, env, test, counter))

            for future in concurrent.futures.as_completed(futures):
                outcome, test_id, message, tempdir = future.result()
                counters.report(outcome, test_id, message)
                shutil.rmtree(tempdir, ignore_errors=True)

    for test, counter in linear_tests:
        outcome, test_id, message, tempdir = task(env, test, counter)
        counters.report(outcome, test_id, message)
        shutil.rmtree(tempdir, ignore_errors=True)

    if args.install is not None:
        shutil.rmtree(args.install)
    shutil.rmtree("build/.testing", ignore_errors=True)

    if not counters.summary(len(independent_tests) + len(linear_tests)):
        sys.exit(1)


if __name__ == "__main__":
    __main__()
