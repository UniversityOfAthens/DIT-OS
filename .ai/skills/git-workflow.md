# Git Workflow and Commit Standards

## Branch Strategy

```
main          ← stable, always builds and boots
  ├── dev     ← integration branch for features
  │   ├── feature/my-feature
  │   ├── fix/my-bugfix
  │   └── refactor/my-refactor
  └── hotfix/urgent-fix
```

### Branch Naming

| Prefix | Usage | Example |
|--------|-------|---------|
| `feature/` | New functionality | `feature/smp-scheduler` |
| `fix/` | Bug fix | `fix/page-fault-boot` |
| `refactor/` | Code restructuring | `refactor/syscall-handler` |
| `docs/` | Documentation only | `docs/memory-map` |
| `test/` | Adding tests | `test/kernel-panic` |
| `hotfix/` | Urgent fix on main | `hotfix/mbr-signature` |

## Commit Messages

### Format

```
<component>: <short summary>

<optional body>

<optional footer>
```

### Rules

1. **Subject line**: max 72 characters
2. **Component prefix**: use the subsystem name
3. **Imperative mood**: "add feature" not "added feature"
4. **No period** at end of subject line
5. **Blank line** between subject and body

### Valid Component Prefixes

```
bootloader:     Bootloader (stage1, stage2, loader)
kernel:         Core kernel
kernel/mm:      Memory management
kernel/sched:   Scheduler
kernel/dev:     Device drivers
kernel/fs:      Filesystem
kernel/syscall: Syscall layer
userland:       Userland code
libc:           C library
shell:          Shell binary
build:          Build system / Makefiles
ci:             CI/CD workflows
nix:            Nix flake
docs:           Documentation
scripts:        Helper scripts
```

### Examples

```
kernel/mm: implement page frame allocator

Adds physical memory manager with bitmap allocation.
Supports allocation of individual pages and contiguous
regions up to 256 pages.

Closes #42
```

```
fix: correct MBR signature placement

The signature at offset 510-511 was being overwritten by
padding. Now uses explicit dw 0xAA55 after times padding.
```

```
refactor: extract serial driver from panic handler

Moves serial I/O functions to kernel/dev/serial.cpp
so they can be reused by other subsystems.
```

### Bad Examples

```
fixed stuff              ← no component, vague
Updated the kernel.      ← past tense, has period
WIP                      ← not descriptive
bug fix                  ← no component, not imperative
```

## Commit Granularity

- **One logical change per commit**
- Do not mix refactoring with feature additions
- Do not mix formatting changes with logic changes
- If a commit touches more than 10 files, consider splitting

## Staging and Review

### Before Committing

1. Run `make clean && make all` - full clean build
2. Run `make run` - verify it boots
3. Run `clang-format` on changed files
4. Review `git diff` for unintended changes

### Pre-Commit Checklist

- [ ] Builds cleanly with `-Werror`
- [ ] Boots in QEMU (if applicable)
- [ ] No debug prints left in code
- [ ] No commented-out code blocks (use git to restore)
- [ ] Header guards are correct
- [ ] No trailing whitespace
- [ ] File ends with newline

## Pull Requests

### PR Title

Follow commit message convention:

```
<component>: <short summary>
```

### PR Description Template

```markdown
## Summary
What this PR does, 1-3 bullet points.

## Changes
- File/module list with brief description of changes

## Testing
- How it was tested (QEMU boot, unit test, manual)
- What was verified (boots, feature works, no regressions)

## Related Issues
Closes #N
```

### PR Rules

1. **No force push** to `main` or `dev`
2. **At least one review** before merge (self-review for solo contributors)
3. **Squash merge** for feature branches (clean history)
4. **Merge commit** for `dev` -> `main` (preserves integration points)

## Tagging and Releases

Use semantic versioning for milestones:

```
v0.1.0   - first bootable build
v0.2.0   - basic kernel with GDT/IDT
v0.3.0   - memory management (PMM + VMM)
...
v1.0.0   - stable multi-user system
```

### Tag Format

```bash
git tag -a v0.1.0 -m "First bootable kernel with serial output"
```

## Undoing Changes

### Revert (safe, creates new commit)

```bash
git revert <commit-hash>
```

### Amend (only on local/unpushed commits)

```bash
git commit --amend --no-edit   # fix last commit message
git commit --amend -m "new message"
```

### Never on Shared Branches

- Never `git rebase -i` on `main` or `dev`
- Never `git push --force` to `main` or `dev`
- Never `git commit --amend` on pushed commits

## .gitignore Rules

Already configured at project root. Key patterns:

```
# Build artifacts
*.o
*.elf
*.bin
*.img
build/
bootloader/build/

# Editor
.vscode/
.idea/
*.swp
*~

# OS
.DS_Store
Thumbs.db
```

## Absolute Rules

1. **Never commit broken code** - must build and boot
2. **Never commit secrets** - no passwords, keys, tokens
3. **Never commit binary files** - build generates them
4. **Never commit IDE-specific files** - use .gitignore
5. **Always run `make clean && make all`** before committing
