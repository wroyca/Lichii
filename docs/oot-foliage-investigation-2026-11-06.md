# Ocarina nearby foliage investigation — 2026-11-06

Based on `finding.md`, the saved shader/capture artifacts, and GitHub reports. Integration starts at fork `nx2` `80bcae6c1f337651c208f2f67425916c1476dc4d`. The original checkout's unfinished merge and stash remain untouched; no live emulator was launched, stopped or modified.

## Concrete missing instruction and port

[seedds/nxnative#1462](https://github.com/seedds/nxnative/pull/1462) restores six foliage compute libraries by adding Ampere `ATOMG.CAS` (`0x3a9`). It documents the exact instruction from `FoliageShader.sharcb`, key `13002657609092b4`, offset `0x8c00`:

```
000073a9 00000008 001ee109 000ea400
```

Replaying this instruction followed by EXIT on our base failed with `unsupported native opcode` at offset zero. Existing [Lichii#140](https://github.com/zortos293/Lichii/pull/140) supplies Maxwell `ATOM.CAS`, IR and backend support, but not this Ampere opcode. None of the 1,237 saved native runtime captures contain `0x3a9`; those captures cannot establish whether the missing instruction is encountered in the affected scene.

Commit `9bf220ed7e` adapts #1462's 32-bit CAS behavior to our Ampere-to-Maxwell frontend:

- Assemble the comparator and replacement in adjacent lowering scratch registers for Maxwell CAS.
- Return the old memory word, and optionally set the native success predicate.
- Preserve semantics when the destination aliases the address, comparator or replacement, and when the output predicate is also the instruction guard.
- Include every register operand in high-register shadowing.
- Accept the demonstrated register-addressed, extended, strong GPU U32/S32 forms; reject unsupported sizes, uniform addressing and offsets outside the Maxwell encoding.

This is an instruction-support fix, not a demonstrated nearby-foliage gameplay fix. The nxnative PR demonstrates additional library coverage and bounded Mac gameplay; its shader keys, translator and Metal renderer are different from ours. Its other packaging, binary and Mac test changes were not imported.

## Corrections to the recovered notes

The saved artifacts are under `/tmp/claude-1000/-home-wroy-Projects-eden-nx2/d2581eb7-0bd3-4891-97d2-884501c2a969/scratchpad`.

- `a2c7af4889370200` is a mesh shader, not a task shader: its IR writes primitive indices with `IsbeWriteMapU8`. It and `12950c978b9762fb` differ in several constants and metadata offsets; they are not interchangeable near/far copies.
- In forced-gate capture `dbg/capture-1793985114-001`, draw 545 launches two groups. Its input storage buffer `b9e64263217f1a56` has header `(2, 0, 0, 32)`. The 32 output records at byte `307264 + 64*i` describe positions about 73–88 units from the captured camera, not the nearby 10–24 unit trees.
- That draw emits 64 primitives, exactly the expected two triangles for each of its 32 records, and generates 200 fragments. This does not demonstrate the mesh stage dropping nearby trees.
- Compute `e2bf0c44a4b53d06` writes the same GPU address (`0x2277272000`, storage binding 6) that draw 545 reads (storage binding 0). Forcing one distance-related IR value does not force all earlier and later gates or prove that the detailed nearby representation should be emitted by this path.

These observations favor investigating the handoff from distant foliage representations to detailed geometry. They do not establish a load-time ordering defect, a mesh-culling defect, or that the game's distance condition should be overridden. The original `finding.md` is preserved separately, without replacing its historical observations.

## GitHub leads and limits

- [Lichii issue #201](https://github.com/zortos293/Lichii/issues/201) closely matches the symptom: bushes, shrubbery and grass lose foliage when approached in Hyrule Castle. Follow-up reports say it persists in build 83. The thread supplies reproduction screenshots, not a confirmed implementation fix.
- [Lichii issue #101](https://github.com/zortos293/Lichii/issues/101) discusses missing vegetation, zero-count draws, shadow-map evidence, texture handles and query freshness. Several comments are hypotheses; literal zero draw parameters alone do not prove stale query reads.
- [Lichii#204](https://github.com/zortos293/Lichii/pull/204) removes the split-thread gate from pending-query flushing. [#215](https://github.com/zortos293/Lichii/pull/215) adds a real wait because below High accuracy an ordinary fence callback can run immediately. #215 reports camera-motion flashes fixed, not a matched demonstration of the distance-dependent foliage problem. Neither was merged in this round.
- [Lichii#242](https://github.com/zortos293/Lichii/pull/242) fixes erroneous compile-time texture-type guesses for dynamic handles; its reported visual result concerns reflections. [#245](https://github.com/zortos293/Lichii/pull/245) fixes handle reads relative to storage-buffer alignment; the author reports no visual change on NVIDIA. Neither is established as the cause here.

A fresh unforced reproduction on the updated fork remains necessary. Compare the same plants from far and near, trace the detailed draw's shader hashes, counts, handles and query inputs, and use one targeted frame capture if needed. Existing render tracing reports skipped pipelines/configuration failures; successful draw captures alone omit those failures. Avoid restoring the temporary IR overrides from the stash.

## Requested pipeline retention change

Merged [Lichii#313](https://github.com/zortos293/Lichii/pull/313), head `e62622de3577cbad3ec888282a45337ca3b3f446`, as `d7c45031a2`. It preloads all cached Switch 2 graphics pipelines and disables their age-based eviction. This preserves current owned pipeline-worker code and MoltenVK handling. The documented tradeoffs remain: retained pipelines have no new memory bound, boot can take longer, and a pipeline that fails at boot can remain failed for that session. No new-game or driver-specific performance claim is made here.

## Validation

- Exact #1462 instruction: rejected before, lowers successfully after (five words including EXIT).
- Four new C++ cases: CAS old-value/store/success semantics over guard and alias combinations; explicit unsupported-mode rejection; high-register shadowing; unused-result atomic retention through optimization and SPIR-V emission. 75,287 assertions with optional SPIR-V export enabled.
- Qt frontend, SDL frontend, tests, updater tests and offline shader compiler all built with Clang/Ninja Release.
- Full CTest: 784 main cases / 69,647,799 assertions and 8 updater cases / 99 assertions; 792 cases total, all passed.
- All 1,237 prior runtime native captures still lower successfully to byte-identical words after the new opcode is added.
- Emitted CAS SPIR-V passes `spirv-val --target-env vulkan1.1`.
- Standalone compute dispatch on RTX 3070 Ti: `CAS(word, 0, 42)` changes 0 to 42 and retains 7, 42 and `0xffffffff`, with neighbor sentinels unchanged and zero Vulkan validation errors. This tests the emitted storage atomic; CPU/frontend tests separately check the native operand and predicate lowering.
- No game-level foliage, A/V or FPS validation was performed.
