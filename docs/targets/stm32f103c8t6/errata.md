# Device restrictions and errata

Check the errata for the exact silicon before relying on a peripheral, timing
feature, analog specification, low-power mode, or debug behavior. A compatible
clone can have different restrictions from the ST device.

## How to use errata

1. Identify the manufacturer and device revision.
2. Obtain the matching errata document.
3. Search for every peripheral used by the application.
4. Record affected revisions and required workarounds.
5. Add a validation test when the workaround affects firmware behavior.

Do not treat an errata document for `STM32F103x8/xB` as proof about a CKS32
device. Use the CKS documentation for a CKS32 part and record any unresolved
difference in the target profile.

## Current project boundary

This repository does not yet carry a revision-specific errata assessment. The
target profile therefore identifies source documents and known board constraints,
but it does not claim that all ST and clone behaviors are interchangeable.

Until the installed silicon is identified, use conservative assumptions:

- Verify Flash programming and readback.
- Do not exceed datasheet voltage or current limits.
- Keep SWD connected during development.
- Check clock and timer behavior on real hardware.
- Recheck peripheral behavior when moving between ST and clone devices.

The [source-document guide](../../workflow/source-documents.md) explains how to
choose the authoritative document.
