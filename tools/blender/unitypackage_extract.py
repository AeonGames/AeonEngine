# Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Rebuild the asset tree inside a Unity .unitypackage.

A .unitypackage is a gzipped tar of one directory per asset, named after the
asset's Unity GUID rather than the file. Each holds the original file bytes as
``asset``, the project relative path as ``pathname``, and Unity's import
settings as ``asset.meta``. Unpacking with tar alone therefore yields a pile of
hashes; the names live in the ``pathname`` files.

This is what turns the Synty SIDEKICK packages into the .fbx tree the rest of
the pipeline expects, so sidekick_batch.py and sidekick_anim.py can be pointed
straight at a directory:

    python3 tools/blender/unitypackage_extract.py SIDEKICK_*.unitypackage -o extracted

Plain Python; it does not need Blender.
"""

import argparse
import os
import shutil
import sys
import tarfile


def _destination_for(root, relative):
    """Resolve an archived path under root, refusing to escape it."""
    root = os.path.abspath(root)
    target = os.path.abspath(os.path.join(root, relative))
    if target != root and not target.startswith(root + os.sep):
        raise ValueError("entry would be written outside the destination: {}".format(relative))
    return target


def extract(package, destination):
    written = 0
    with tarfile.open(package, "r:gz") as archive:
        members = archive.getmembers()
        # The GUID directory is the only link between a name and its bytes.
        names = {}
        for member in members:
            if not member.isfile() or os.path.basename(member.name) != "pathname":
                continue
            handle = archive.extractfile(member)
            if handle is None:
                continue
            # These carry no trailing newline, so reading them as a stream of
            # lines is what keeps every path from running into the next.
            text = handle.read().decode("utf-8", "replace").splitlines()
            if text:
                names[member.name.split("/")[0]] = text[0].strip()

        for member in members:
            if not member.isfile() or os.path.basename(member.name) != "asset":
                continue
            # Folder entries carry a pathname but no asset, and are skipped by
            # virtue of never reaching this loop.
            relative = names.get(member.name.split("/")[0])
            if not relative:
                continue
            target = _destination_for(destination, relative)
            os.makedirs(os.path.dirname(target), exist_ok=True)
            handle = archive.extractfile(member)
            if handle is None:
                continue
            with open(target, "wb") as out:
                shutil.copyfileobj(handle, out)
            written += 1
    return written


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("packages", nargs="+", help="one or more .unitypackage files")
    parser.add_argument("-o", "--out", required=True, help="directory to rebuild the tree in")
    arguments = parser.parse_args()

    total = 0
    for package in arguments.packages:
        count = extract(package, arguments.out)
        total += count
        print("{}: {} assets".format(os.path.basename(package), count))
    print("wrote {} assets to {}".format(total, arguments.out))
    return 0 if total else 1


if __name__ == "__main__":
    sys.exit(main())
