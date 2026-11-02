---
name: handoff
description: Keep handoff.md current during any multi-step DIT-OS task so a fresh agent can resume after a crash, usage limit or closed session. Read it first when it exists.
---

# Handoff file

Sessions end without warning. `handoff.md` in the repository root is how the
next agent (or you, after a restart) picks up the work.

## Rules

1. **Start of session:** if `handoff.md` exists, read it before anything
   else. Check its claims against `git status`, `git diff` and the files it
   names; the code is the truth if they disagree. Then continue from "Next".
2. **Multi-step task:** create `handoff.md` before the first edit.
3. **Update after every meaningful step:** a sub-task finished, a decision
   made, a finding, a failed approach, and **before** anything long-running
   (a build, a QEMU run, a big refactor). Rewrite in place; do not append a
   log.
4. **Keep it short** (aim for under 60 lines). Facts, paths and commands; no
   narrative.
5. **Finish:** set `Status: COMPLETE` at the top and leave the verification
   results in it.
6. **Never commit it.** It is in `.gitignore`. Only commit it if the user asks.
7. Never put secrets, tokens or personal data in it.

## Template

```markdown
# Handoff — <task in a few words>

**Status: IN PROGRESS** (or COMPLETE / BLOCKED: <why>)

## Goal
<one or two sentences: what the user asked for and what "done" means>

## Done
- <step> — <file(s)>; verified by <command/result>

## In progress
- <file>: <what is half done, exact state>

## Next
1. <next concrete step>
2. ...

## Decisions and gotchas
- <decision> — <why>
- <thing that wasted time / must not be repeated>

## Verify
- `rm -rf bootloader/build kernel/build userland/build && make -C kernel && make -C bootloader all`
- `scripts/qemu-test.sh`
- `scripts/cppcheck.sh`
- Security audit: <done / pending, with report location>
```
