---
name: Pull Request
about: Submit a pull request to improve DIT-OS
---

## Description
Please include a summary of the change and why it is needed. Also include any relevant issue numbers.

## Related Issue(s)
- Closes # (replace with issue number)

## Type of change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Refactoring (code style, performance improvements, etc.)
- [ ] Documentation update
- [ ] Build/CI changes

## Checklist
- [ ] Clean build passes (`rm -rf bootloader/build kernel/build userland/build && make -C kernel && make -C bootloader all`)
- [ ] `scripts/qemu-test.sh` passes; new behaviour has a self-test or qemu-test step
- [ ] `scripts/cppcheck.sh` passes with no new suppressions (or each one is justified below)
- [ ] Security audit done (`.agents/skills/security-audit/SKILL.md`), including the known-CVE pass; report below
- [ ] Formatted with clang-format 20.1.8; shell scripts pass shellcheck
- [ ] `CHANGELOG.md`, `TODO.md`, `ISSUES.md` and agent rules updated where affected

## Test results
<!-- Paste the relevant qemu-test.sh / cppcheck.sh output. -->

## Security audit
<!-- Report format: .agents/skills/security-audit/SKILL.md section 6. Write "docs only" for documentation-only PRs. -->
