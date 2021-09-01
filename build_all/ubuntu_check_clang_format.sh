#!/bin/bash
clang-format --version
find . -type d \( -path deps -o -path docs \) -prune -o \( -iname '*.h' -o -iname '*.c' \) -exec clang-format -i {} \;
git status --untracked-files=no --porcelain
if [[ `git status --untracked-files=no --porcelain` ]]; then
  echo Some files were not formatted correctly according to the .clang-format file.
  echo Please run clang-format to fix the issue by using this bash command at the root of the repo:
  echo "find . \( -iname '*.h' -o -iname '*.c' \) -exec clang-format -i {} \;"
  exit 1
fi
echo Success, all files are formatted correctly according to the .clang-format file.
exit 0

