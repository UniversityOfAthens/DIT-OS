---
name: git-workflow
description: DIT-OS branches, commit message format, commit granularity, PR requirements and safe git operations for agents. Load before committing, branching or opening a PR.
---

# Git workflow

## Agents

- Commit, push or open a PR **only when the user asks**.
- Commits are authored by the user only: no `Co-Authored-By` AI trailer, no
  "Generated with ..." line.
- Never commit `handoff.md`, build output (`build/`, `*.bin`, `*.img`,
  `*.o`), secrets or IDE files.
- **Before any command that discards changes** (`git checkout -- <path>`,
  `git restore`, `git reset --hard`, `git clean`, `git stash drop`): run
  `git status` and `git diff`, and limit the command to the exact files you
  changed yourself. Other people (or other sessions) may have uncommitted
  work in the same tree. If unsure, ask.
- Review `git diff` before every commit for unintended changes.

## Branches

```
main      stable: always builds and boots
develop   integration branch
  feature/<name>  fix/<name>  refactor/<name>  docs/<name>  test/<name>
hotfix/<name>     from main, for urgent fixes
```

- Branch from `develop`. Never commit straight to `main`.
- Never force-push, rebase or rewrite history on `main` or `develop`.
  `--amend` only on unpushed local commits.
- Undo published commits with `git revert`.

## Commit messages

```
<component>: <imperative summary, ≤ 72 chars, no period>

<optional body: one or two lines on why>
```

Components: `bootloader`, `kernel`, `kernel/mm`, `kernel/sched`,
`kernel/dev`, `kernel/fs`, `kernel/syscall`, `userland`, `libc`, `shell`,
`build`, `ci`, `nix`, `scripts`, `docs`, `agents` (rules and skills).

Examples:

```
kernel/syscall: reject wrapping buffers in sys_write
docs: describe the disk layout check
agents: add cppcheck gate to the definition of done
```

## Granularity

- One logical change per commit; it builds and boots on its own.
- Do not mix formatting and logic, or refactors and features.
- Touching more than ~10 files? Consider splitting.

## Before committing

1. Verify sequence passes (`testing` skill): clean build, `qemu-test.sh`,
   `cppcheck.sh`.
2. Security audit done (`security-audit` skill).
3. clang-format on changed C/C++; shellcheck on changed scripts.
4. `CHANGELOG.md` `[Unreleased]` updated for user-visible changes.

## Pull requests

- Title in commit format. Body: what and why, test results, and the
  security-audit report.
- Target `develop` (hotfixes: `main`). At least one review. Squash-merge
  feature branches; merge commit for `develop` → `main`.
