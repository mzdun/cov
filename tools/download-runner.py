#!/usr/bin/env python
# Copyright (c) 2023 Marcin Zdun
# This code is licensed under MIT license (see LICENSE for details)

import hashlib
import io
import os
import sys
from typing import Dict

import requests

TOOL_DIR = "build/downloads"
TOOL_NAME = "json-runner"
if os.name == "nt":
    ARCHIVE_EXT = "zip"
    ARCHIVE_ARCH = "windows-x86_64"
    EXEC_EXT = ".exe"
else:
    from flow.lib.uname import uname

    ARCHIVE_EXT = "tar.gz"
    ARCHIVE_ARCH = "-".join(uname())
    EXEC_EXT = ""


def _hash(filename: str) -> str:
    sha = hashlib.sha256()
    with open(filename, "rb") as data:
        for block in iter(lambda: data.read(io.DEFAULT_BUFFER_SIZE), b""):
            sha.update(block)
    return sha.hexdigest()


def _download(url: str, filename: str, store: bool = True):
    response = requests.get(url, allow_redirects=True)
    if response.status_code // 100 != 2:
        print(
            f"{url}: download failed: {response.status_code}, {response.reason}",
            file=sys.stderr,
        )
        return None
    if not store:
        return response.content

    with open(filename, "wb") as file:
        file.write(response.content)
    return True


if os.name == "nt":
    import zipfile

    def _extract(path: str, entry: str, target_path: str):
        with zipfile.ZipFile(path) as zip:
            try:
                info = zip.getinfo(entry)
            except KeyError:
                print(
                    f"{path}: extract failed: {entry} not found",
                    file=sys.stderr,
                )
                return False

            entry_data = zip.open(info)
            try:
                with open(target_path, "wb") as binary_dest:
                    for block in iter(
                        lambda: entry_data.read(io.DEFAULT_BUFFER_SIZE), b""
                    ):
                        binary_dest.write(block)
            finally:
                entry_data.close()
        return True

else:
    import tarfile

    def _extract(path: str, entry: str, target_path: str):
        with tarfile.open(path) as tar:
            try:
                info = tar.getmember(entry)
            except KeyError:
                print(
                    f"{path}: extract failed: {entry} not found",
                    file=sys.stderr,
                )
                return False
            entry_data = tar.extractfile(info)
            try:
                with open(target_path, "wb") as binary_dest:
                    for block in iter(
                        lambda: entry_data.read(io.DEFAULT_BUFFER_SIZE), b""
                    ):
                        binary_dest.write(block)
            finally:
                entry_data.close()

        if not info.issym():
            try:
                os.chmod(target_path, info.mode)
            except OSError as e:
                print(
                    f"{target_path}: could not change mode: {e}",
                    file=sys.stderr,
                )
                return False
            try:
                os.utime(target_path, (info.mtime, info.mtime))
            except OSError as e:
                print(
                    f"{target_path}: could not change modification time: {e}",
                    file=sys.stderr,
                )
                return False

        return True


def lts_for(arch: str):
    triplet = arch.split('-')
    if triplet[0] == 'ubuntu':
        ver = triplet[1].split('.')
        if ver[1] == '10': ver[1] = '04'
        maj = int(ver[0])
        if maj % 2 == 1: maj -= 1
        triplet[1] = f"{maj}.{'.'.join(ver[1:])}"
        arch = '-'.join(triplet)
    
    return arch


def download_tools(version: str):
    sha_url = f"https://github.com/mzdun/{TOOL_NAME}/releases/download/v{version}/sha256sum.txt"

    os.makedirs(TOOL_DIR, exist_ok=True)

    print(f"[DL] {sha_url}", file=sys.stderr)
    sha256sum_text = _download(sha_url, f"{TOOL_DIR}/sha256sum.txt", False)
    if not sha256sum_text:
        return False
    sha256sum: Dict[str, str] = {}

    for hash, filename in [
        line.split(" ", 1)
        for line in sha256sum_text.decode("UTF-8").split("\n")
        if line != ""
    ]:
        if filename[:1] != "*":
            continue
        filename = filename[1:]
        sha256sum[filename] = hash

    package_name = f"{TOOL_NAME}-{version}-{ARCHIVE_ARCH}"
    orig_package_name = package_name
    expected_hash = sha256sum.get(f"{package_name}.{ARCHIVE_EXT}")
    if expected_hash is None:
        package_name = f"{TOOL_NAME}-{version}-{lts_for(ARCHIVE_ARCH)}"
        expected_hash = sha256sum.get(f"{package_name}.{ARCHIVE_EXT}")
    if expected_hash is None:
        print(
            f"{sha_url}: error: cannot locate sha256sum for {orig_package_name}.{ARCHIVE_EXT}",
            file=sys.stderr,
        )
        return False

    arch_url = f"https://github.com/mzdun/{TOOL_NAME}/releases/download/v{version}/{package_name}.{ARCHIVE_EXT}"
    path = f"{TOOL_DIR}/{package_name}.{ARCHIVE_EXT}"

    needs_download = True
    if os.path.isfile(path):
        needs_download = _hash(path) != expected_hash

    if not needs_download:
        print(f"[USE] {path}", file=sys.stderr)

    if needs_download:
        print(f"[DL] {arch_url}", file=sys.stderr)
        if not _download(arch_url, path):
            return False
        actual_hash = _hash(path)
        if actual_hash != expected_hash:
            os.remove(path)
            print(
                f"{arch_url}: download failed: unexpected sh256sum:\n expecting {expected_hash}\n received {actual_hash}",
                file=sys.stderr,
            )
            return False

    short_version = ".".join(version.split(".")[:2])
    print(
        f"[EXTRACT] {package_name}/share/{TOOL_NAME}-{short_version}/schema.json",
        file=sys.stderr,
    )
    if not _extract(
        path,
        f"{package_name}/share/{TOOL_NAME}-{short_version}/schema.json",
        f"{TOOL_DIR}/{TOOL_NAME}-schema.json",
    ):
        return False

    print(f"[EXTRACT] {package_name}/bin/{TOOL_NAME}{EXEC_EXT}", file=sys.stderr)
    if not _extract(
        path,
        f"{package_name}/bin/{TOOL_NAME}{EXEC_EXT}",
        f"{TOOL_DIR}/{TOOL_NAME}{EXEC_EXT}",
    ):
        return False

    return True


if __name__ == "__main__" and not download_tools(
    "0.3.0" if len(sys.argv) < 2 else sys.argv[1]
):
    sys.exit(1)
