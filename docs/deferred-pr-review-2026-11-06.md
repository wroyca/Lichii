# Deferred PR and local-branch review — 2026-11-06

Follow-up to [the initial review](performance-pr-review-2026-11-06.md), based on fork `nx2` at `536412161a873a2254011395204997edb1c5cdc8`. Integration branch: `perf/review-deferred-prs`. The original checkout's unfinished merge remains untouched.

## Integrated

| Source | Reviewed head | Result |
| --- | --- | --- |
| [#299](https://github.com/zortos293/Lichii/pull/299) | `ad6963c42030` | Extend the embedded Ocarina Dynamic FPS routine from a 50 ms to a 100 ms frame-time clamp. Update the embedded-byte expectation. Four optional Unicorn tests execute the actual AArch64 routine at 60–10 fps, below the clamp, at startup, and against the old routine. |
| [#311](https://github.com/zortos293/Lichii/pull/311) | `34f9bf4bd68e` | The updated head fixes the earlier review blocker: clear coverage is checked in both JoinImages and IsFullClear, including cached targets. |
| [#149](https://github.com/zortos293/Lichii/pull/149) | `e8918edb08b5` | Most behavior was already present. Preserve merge provenance and add the missing RED/SURED data-register shadow detection. Tests check that lowering reads the guest shadows. |
| Local `0c0b320e64d522c7bbb1f337f96f39b46246ad55` | cherry-picked with `-x` as `a543cc669f` | Recover SHF register-form shadowing and uniform-bank alignment. The overlapping alignment fix is subsequently consolidated with #241. |
| [#251](https://github.com/zortos293/Lichii/pull/251) | `98ae0a44a130` | Ocarina-only wall-clock movie catch-up and opt-in diagnostics. Repair decode-time dropping so even an entirely late batch retains its newest picture and can satisfy a client's output fence. Track take/release timings per image and preserve SDL sink names so the stream counters work. Four session-helper tests cover late batches, EOS, pause/reset and interleaved image releases. |
| [#241](https://github.com/zortos293/Lichii/pull/241) | `d25d06b588fe` | Request an aligned uniform-bank retry from the failing lowering pass; support SHF and LOP3 constant-buffer/uniform operand forms. Remove the redundant error-string retry from the recovered local commit. Preserve both shadow-copy and lowered-instruction checks. Add LOP3 coverage verifying B is not rewritten as a guest register. |

## Deferred review dispositions

These decisions use current heads, diffs, review discussions and comparison with the integration base. They distinguish code-review blockers from changes requiring hardware/game validation; they are not claims of benchmark results.

| PR | Disposition |
| --- | --- |
| #300 | Deferred compute pipelines can skip dispatches; reported missing video remains a behavior tradeoff requiring AMD/game validation. |
| #298 | Draft LSFG feature; no production integration. |
| #292 | The enlarged compute-buffer capacity checks Vulkan's storage range but does not cap OpenGL's storage-block limit; 128 MiB implementations remain a concern. |
| #282 | Closed; bypassing anti-aliasing changes image quality. |
| #278 | Throttle override parses invalid/empty strings as zero; the nanosecond multiplication can overflow signed integers. Needs validation and timing tests. |
| #270 | Present-rate limiter feature; no demonstrated throughput improvement. Kept separate. |
| #267 | Multi-commit branch includes considerably more than configuration reporting; needs individual changes extracted and reviewed. |
| #259 | Experimental preflush has reported flicker; experiment switches cannot ship to nx2 under repository policy. |
| #253 | The current follow-up fixes the old 100 ms jitter-clear review issue. DLSS runtime/SDK wiring and image quality still require their own validation. |
| #243 | Queued graphics work can write a report page before the backend sets its GPU-written bit; the proposed fast read can race that work. Page classification does not establish exclusive ownership. |
| #235 | Experimental split-safe-read switch excluded by repository policy. |
| #232 | Windows soft-affinity change; author A/B reports no FPS gain and this review has no Windows hybrid-CPU validation. |
| #231 | Depends on draft #179 and imports a large experimental history. |
| #223 | Broad hardware-specific launch branch, not a small standalone fix. |
| #219 | Draft CI/AV1 changes. |
| #216 | Older custom delta hook overlaps the current embedded Dynamic FPS implementation. Do not stack competing timing hooks. |
| #192 | Mesh serialization is already present. The remaining guarded-native serialization change needs a guard/round-trip test and a rebase preserving current pipeline ownership/budget handling. |
| #179 | Draft split-thread branch. |
| #166 | Broad mixed changes and experiments; no whole-branch merge. |
| #161 | Broad collection with substantial overlap and retracted performance claims; no whole-branch merge. |
| #147 | Superseded by newer DownloadRangesMemory handling, including mapped GPU-to-host visibility, exact dirty ranges and trap tracking. Avoid restoring the older implementation's copy-alignment/trap issues. |
| #143 | Intel fallback includes fixed-address sampling and heuristic format/gamma handling; cannot justify enabling this automatically. |
| #114 | Draft clock boost needs dependency and game validation; reported black rendering without its companion change. |

## Older local branches

Compared `oot/native-tess-fix`, `perf/oot-combined` and related worktrees/remote history against the integration base, including 34 William Roy commits. Only `0c0b320e64` was recovered directly in this round.

- Already represented by newer code: `8eff9e4e32` mapped GPU-to-host barriers; `1a815d01b4` command-list copy/page generation; `f5e180b538` integer multiply folding; `a1848acfe5` bounded occlusion-query banks; older shader cache/worker changes. Do not restore their old cache versions or ownership models.
- Explicitly reverted: `843f657b5e` untracked global atomics. Do not resurrect it.
- Experimental flags or known regressions: `837d31c10f` bank re-upload gate, `6458db20b6` preflush expiry, and older MME split/drain experiments. Do not merge these switches.
- Need targeted coherence/concurrency tests: `00ba7a941b` physical-backing range checks, `b92da88d1b` dirty-epoch skipping, `708d3b8bcc` batched CPU protection, `3b6900611a` High-accuracy command-copy reads.
- `5593ae4c4d` eager mapping/retained backing trades page faults for retained RAM and needs memory-pressure measurements. Older pipeline-age policy changes have conflicting follow-ups.
- `40f34fd803` immediate High-accuracy fences would weaken synchronization. `0957a6c75c` fabricated RomFS replacement is not an established correctness fix.
- Local `c182ab4f19` query flushing has a stronger upstream candidate in #215. A simple flush plus WaitConsumed is insufficient at Low GPU accuracy; review the dedicated pending-query synchronization path before porting either.

## Recovered investigation notes

Found `finding.md` in `stash@{0}^3` (stash description: debug tooling and finding.md). Extracted it unchanged to `/home/wroy/Projects/Lichii-pr-review/finding.md` for the user, without applying or popping the stash.

The latest observations supersede earlier hypotheses in that document: the near-leaf compute path leaves newly encountered near trees without an LOD, and forcing its gate produces some near-leaf work but still leaves missing geometry. The notes suggest initialization/timing plus a task/mesh-path problem, not a proven translation fix. The reverted atomics change is explicitly ruled out there. No forced IR values or temporary debug instrumentation were imported.

## Validation

Linux Clang/Ninja Release build of tests, Qt and SDL frontends, plus updater tests. Focused movie-clock/uniform-vote/RED/SURED/SHF checks passed 11 cases and 4,818 assertions before #241. The optional Unicorn routine suite passed all four cases. Final integrated frontend/test builds passed. CTest passed both executables: 780 main cases (69,714,729 assertions) and 8 updater cases (99 assertions), 788 C++ cases total. The new LOP3 case passed 18 assertions. `git diff --check` passed.

No live emulator was started or stopped. No game-level movie A/V, visual correctness or FPS measurement was performed; Windows, Intel and AMD-specific results were not reproduced.
