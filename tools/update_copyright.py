#!/usr/bin/env python3

# Copyright (C) 2025 Rodrigo Jose Hernandez Cordoba
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

"""
Update copyright year lists.

Behavior:
- By default, only processes files that Git shows as changed (staged or unstaged
    modifications/additions per "git diff --name-only" and "git diff --name-only --cached").
- Use --all to scan all files under the provided paths (or repo root when omitted).

Rules to update years:
- Find lines like: "Copyright (C) 2017-2019,2021,2024 Name".
- If the current year is already present, do nothing.
- Else, examine the last entry (after the last comma):
    * If it's a range that ends in last year (current_year - 1), replace the end
        with the current year (e.g., 2000-2024 -> 2000-2025).
    * Otherwise, append the current year after a comma (e.g., 2000,2024 -> 2000,2024,2025).

Notes:
- Matches optional (C) marker in various spacings and case.
- Preserves everything after the years block (e.g., author name) untouched.
- Defaults to repo root as the base when no paths are given.
"""
from __future__ import annotations

import argparse
import datetime as _dt
import os
import re
import sys
import subprocess
from pathlib import Path
from typing import Iterable, Tuple


CURRENT_YEAR = _dt.date.today().year


# Regex to match a copyright line with years list.
# Examples matched:
#   Copyright (C) 2017-2019,2021,2024 John Doe
#   Copyright (c) 2017, 2019-2021 Jane Doe
#   Copyright 2012-2014, 2016-2017,2019 Foo
COPYRIGHT_YEARS_RE = re.compile(
    r"(Copyright\s*(?:\(\s*[cC]\s*\)\s*)?)"
    r"(?P<years>"  # group of one or more year entries separated by commas
    r"(?:\d{4}(?:\s*-\s*\d{4})?)"
    r"(?:\s*,\s*(?:\d{4}(?:\s*-\s*\d{4})?))*"
    r")"
    r"(?P<after>\s+[^\n]*)",
    re.IGNORECASE,
)


def _parse_entry(entry: str) -> Tuple[int, int, str | None]:
    """Return (start, end, sep) for an entry.

    sep preserves the original hyphen and surrounding spaces when it's a range,
    otherwise None.
    """
    entry = entry.strip()
    m = re.match(r"^(\d{4})(\s*-\s*)(\d{4})$", entry)
    if m:
        start = int(m.group(1))
        sep = m.group(2)
        end = int(m.group(3))
        return start, end, sep
    # single year
    y = int(entry)
    return y, y, None


def _covered_years(entries: Iterable[str]) -> set[int]:
    covered: set[int] = set()
    for e in entries:
        s, e2, _ = _parse_entry(e)
        if s <= e2:
            covered.update(range(s, e2 + 1))
        else:
            # Malformed range; still include both endpoints
            covered.add(s)
            covered.add(e2)
    return covered


def update_years_str(years_str: str, current_year: int = CURRENT_YEAR) -> str:
    """Return the updated years string or the original if no change needed."""
    # Split by commas; keep tokens without empty entries
    tokens = [t.strip() for t in years_str.split(',') if t.strip()]
    if not tokens:
        return years_str

    covered = _covered_years(tokens)
    if current_year in covered:
        return years_str

    last = tokens[-1]
    start, end, sep = _parse_entry(last)

    if end == current_year - 1 and sep is not None:
        # Extend the existing last range end to current year, preserve original separator spacing
        new_last = f"{start}{sep}{current_year}"
        tokens[-1] = new_last
        return ",".join(tokens)

    # Otherwise, append as a new year token
    tokens.append(str(current_year))
    return ",".join(tokens)


def update_content(content: str, current_year: int = CURRENT_YEAR) -> Tuple[str, bool]:
    changed = False

    def _repl(m: re.Match) -> str:
        nonlocal changed
        head = m.group(1)
        years = m.group('years')
        after = m.group('after') or ''
        new_years = update_years_str(years, current_year)
        if new_years != years:
            changed = True
        return f"{head}{new_years}{after}"

    new_content = COPYRIGHT_YEARS_RE.sub(_repl, content)
    return new_content, changed


EXCLUDE_DIRS = {
    '.git', '.hg', '.svn',
    'build', 'dist', 'out',
    'bin', 'lib',
    'CMakeFiles',
    '__pycache__',
    '.vscode', '.idea',
    'node_modules',
    # Repo-specific build output folders
    'gcc64',
}


def iter_files(paths: Iterable[Path]) -> Iterable[Path]:
    for p in paths:
        if p.is_file():
            yield p
        elif p.is_dir():
            for root, dirs, files in os.walk(p):
                # prune excluded dirs in-place
                dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS and not d.startswith('.')]
                for name in files:
                    fp = Path(root) / name
                    # Skip obvious binary files by extension
                    if fp.suffix.lower() in {'.png', '.jpg', '.jpeg', '.gif', '.bmp', '.tga', '.dds', '.bin', '.so', '.dll', '.dylib', '.a', '.o', '.obj', '.exe', '.pdf', '.zip', '.gz', '.7z', '.xz'}:
                        continue
                    yield fp


def git_repo_root(start: Path) -> Path | None:
    try:
        out = subprocess.check_output(["git", "rev-parse", "--show-toplevel"], cwd=start)
        return Path(out.decode().strip())
    except Exception:
        return None


def git_changed_files(repo_root: Path) -> set[Path]:
    """Return absolute Paths for files changed according to Git.

    Includes staged and unstaged changes (diff and diff --cached). Untracked files are
    not included.
    """
    changed: set[Path] = set()
    cmds = [
        ["git", "diff", "--name-only"],
        ["git", "diff", "--name-only", "--cached"],
    ]
    for cmd in cmds:
        try:
            out = subprocess.check_output(cmd, cwd=repo_root)
        except subprocess.CalledProcessError:
            continue
        for line in out.decode("utf-8", "ignore").splitlines():
            line = line.strip()
            if not line:
                continue
            # Normalize to absolute path
            changed.add((repo_root / line).resolve())
    return changed


def process_file(path: Path, write: bool = True, encoding: str = 'utf-8') -> bool:
    try:
        text = path.read_text(encoding=encoding, errors='ignore')
    except Exception:
        return False

    new_text, changed = update_content(text)
    if changed and write:
        try:
            path.write_text(new_text, encoding=encoding)
        except Exception as e:
            print(f"Failed to write {path}: {e}", file=sys.stderr)
            return False
    return changed


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('paths', nargs='*', type=Path, help='Files or directories to process. Defaults to repo root if none provided.')
    parser.add_argument('--dry-run', action='store_true', help='Do not write changes; just report would-be modifications.')
    parser.add_argument('--all', action='store_true', help='Process all files under the given paths (or repo root), not just Git-changed ones.')
    args = parser.parse_args(argv)

    # Determine roots
    if args.paths:
        roots = [p.resolve() for p in args.paths]
    else:
        roots = [Path(__file__).resolve().parent.parent]

    # Determine target files
    targets: Iterable[Path]
    if args.all:
        targets = iter_files(roots)
    else:
        repo_root = git_repo_root(roots[0]) or roots[0]
        changed = git_changed_files(repo_root)
        if not changed:
            # Nothing to do
            if args.dry_run:
                print("[Dry run] No Git-changed files found.")
            else:
                print("No Git-changed files found.")
            return 0

        # Filter to provided roots
        filtered: list[Path] = []
        for fp in changed:
            for root in roots:
                try:
                    fp.relative_to(root)
                    filtered.append(fp)
                    break
                except ValueError:
                    continue
        targets = filtered

    changed_count = 0
    scanned_count = 0
    for fp in targets:
        scanned_count += 1
        did_change = process_file(fp, write=(not args.dry_run))
        if did_change:
            changed_count += 1
            print(f"Updated: {fp}")

    if args.dry_run:
        print(f"[Dry run] Scanned {scanned_count} files; {changed_count} would be updated.")
    else:
        print(f"Scanned {scanned_count} files; {changed_count} updated.")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
