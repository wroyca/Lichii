# Open performance PR review — 2026-11-06

Target fork: https://github.com/wroyca/Lichii (`nx2`). Source: https://github.com/zortos293/Lichii (`nx2` at `a7a281757da390ce45070e772784e0333831bb25`).

The target started at `724aede4ea51fae8224cdd02549cdbdf7ceecd25`, an ancestor of the source. This integration includes the source branch updates as well as the selected open PR heads. The original `/home/wroy/Projects/Lichii` checkout has an unrelated unfinished merge and was left untouched. Worktree: `/home/wroy/Projects/Lichii-pr-review`.

## Scope and evidence

Inventoried all 74 open PRs. Inspected diffs for 22 performance candidates (#174, #175, #176, #178, #192, #197, #206, #232, #235, #243, #250, #259, #264, #268, #278, #288, #296, #300, #311, #161, #147, #231), plus test-support PRs #167/#246, relevant review discussions and production call sites. Full production-code review concentrated on the selected changes. Remaining entries below are triage decisions from scope, PR descriptions and base-code comparison, not claims of a full code review. No author-reported FPS measurements were reproduced here.

## Selected changes

| PR | Reviewed head | Decision |
| --- | --- | --- |
| [#174](https://github.com/zortos293/Lichii/pull/174) | `7221023ed4ad` | Unroll software HMMA within each K8 group, preserving K8 rounding. |
| [#178](https://github.com/zortos293/Lichii/pull/178) | `994b99c35b86` | Skip redundant split-front instance-loop methods; includes batch/replay follow-ups. |
| [#197](https://github.com/zortos293/Lichii/pull/197) | `237ab472633d` | Avoid repeatedly rethrowing cached native-compute rejections; retain OpenGL failure reporting. |
| [#250](https://github.com/zortos293/Lichii/pull/250) | `2e5d6d6ebd27` | Skip cache invalidations only on pages with no registered objects; buffer invalidation remains unconditional. |
| [#264](https://github.com/zortos293/Lichii/pull/264) | `9fdc5a59c0d2` | Arm only newly unarmed read-trap pages; forget the entire texture-served flush area. |
| [#268](https://github.com/zortos293/Lichii/pull/268) | `4262a993e32a` | Scale the texture-cache critical cap with the driver budget on larger GPUs. |
| [#288](https://github.com/zortos293/Lichii/pull/288) | `e06761986714` | Skip uploads and older alias copies overwritten by a complete compatible image copy. |
| [#296](https://github.com/zortos293/Lichii/pull/296) | `245d750a9714` | Scan contiguous ASTC input in place and hoist block-invariant decoder calculations. |

## Remaining PR triage

“Already present” means the relevant behavior exists in the integration base, even when the original PR head is not an ancestor. PRs #176 and #206 were merged for provenance; their remaining code changes were respectively a comment and none.

| PR | Disposition | Reason |
| --- | --- | --- |
| [#311](https://github.com/zortos293/Lichii/pull/311) | Deferred | Partial correctness follow-up: review still identifies surface-clip coverage missing from IsFullClear for existing cached targets. Do not call this a complete clear-coverage fix. |
| [#310](https://github.com/zortos293/Lichii/pull/310) | Outside selected performance scope | dynarmic: wait for the code space commit instead of emitting past it |
| [#309](https://github.com/zortos293/Lichii/pull/309) | Outside selected performance scope | vk_query_cache: log when the bank-bound reset drops a samples count |
| [#303](https://github.com/zortos293/Lichii/pull/303) | Outside selected performance scope | qt: add opt-in desktop compatibility reports |
| [#301](https://github.com/zortos293/Lichii/pull/301) | Outside selected performance scope | vulkan: add EDEN_NX2_PIPELINE_BUILD_TRACE to name shaders a driver crashes on |
| [#300](https://github.com/zortos293/Lichii/pull/300) | Deferred | Skips compute dispatches while deferred pipelines build, with reported skipped video. Needs explicit acceptance and driver/game testing. |
| [#299](https://github.com/zortos293/Lichii/pull/299) | Deferred | Game timing/mod behavior, not a general emulator throughput optimization. |
| [#298](https://github.com/zortos293/Lichii/pull/298) | Deferred draft | Not ready for production integration; may contain experimental behavior. |
| [#292](https://github.com/zortos293/Lichii/pull/292) | Deferred | Changes compute pointer binding capacity; needs game-specific correctness validation. |
| [#287](https://github.com/zortos293/Lichii/pull/287) | Outside selected performance scope | frontend: load Linux AppImage switch files and package defaults |
| [#282](https://github.com/zortos293/Lichii/pull/282) | Deferred | Bypasses the game anti-aliasing network and changes image quality. |
| [#279](https://github.com/zortos293/Lichii/pull/279) | Outside selected performance scope | shader: shadow high registers of IMAD with a uniform multiplier |
| [#278](https://github.com/zortos293/Lichii/pull/278) | Deferred | Optional storage-throttle override; changes timing and has not been checked against games that needed the original throttle. |
| [#270](https://github.com/zortos293/Lichii/pull/270) | Deferred | Present-rate control feature; keep separate from the performance-only integration. |
| [#267](https://github.com/zortos293/Lichii/pull/267) | Deferred | Configuration/performance report, not an independently established code optimization. |
| [#259](https://github.com/zortos293/Lichii/pull/259) | Deferred | Experimental preflush switch has reported flicker; repository policy excludes experimental switches from nx2. |
| [#255](https://github.com/zortos293/Lichii/pull/255) | Outside selected performance scope | Implement Switch 2 microphone capture and Controls configuration |
| [#253](https://github.com/zortos293/Lichii/pull/253) | Deferred | DLSS replacement is a separate feature with SDK/runtime and image-quality validation requirements. |
| [#251](https://github.com/zortos293/Lichii/pull/251) | Deferred | Video/audio timing behavior; needs dedicated playback validation. |
| [#246](https://github.com/zortos293/Lichii/pull/246) | Included test support | Match unsigned result types in assertions; no runtime behavior change. |
| [#245](https://github.com/zortos293/Lichii/pull/245) | Outside selected performance scope | video_core: read bindless handle words from the host-aligned storage base |
| [#243](https://github.com/zortos293/Lichii/pull/243) | Deferred | Substantial synchronization rewrite of macro DMA/report reads; needs dedicated dependency/readback integration testing before merge. |
| [#241](https://github.com/zortos293/Lichii/pull/241) | Outside selected performance scope | shader: retry native lowering with an aligned high uniform base before rejecting |
| [#235](https://github.com/zortos293/Lichii/pull/235) | Deferred | Adds an experimental split-safe-read switch; excluded by the repository toggle policy. |
| [#232](https://github.com/zortos293/Lichii/pull/232) | Deferred | Windows-specific scheduling change; no Windows/hybrid-CPU validation in this review. |
| [#231](https://github.com/zortos293/Lichii/pull/231) | Deferred | Depends on draft #179 and imports its large experimental history. |
| [#230](https://github.com/zortos293/Lichii/pull/230) | Outside selected performance scope | vulkan: enable float16 on the device for host shaders so LSFG frame generation loads |
| [#228](https://github.com/zortos293/Lichii/pull/228) | Outside selected performance scope | dist: describe EDEN_NX2_FRAMES_AHEAD as the code reads it |
| [#223](https://github.com/zortos293/Lichii/pull/223) | Deferred | Broad hardware-specific launch branch; #300 is its separately scoped compute change. |
| [#221](https://github.com/zortos293/Lichii/pull/221) | Outside selected performance scope | file_sys: ignore IPSwitch lines that are not patch records |
| [#219](https://github.com/zortos293/Lichii/pull/219) | Deferred draft | Not ready for production integration; may contain experimental behavior. |
| [#216](https://github.com/zortos293/Lichii/pull/216) | Deferred | Game timing behavior; keep separate from the performance-only integration. |
| [#215](https://github.com/zortos293/Lichii/pull/215) | Outside selected performance scope | video_core: wait for pending query results before macro DMA reads (Ocarina white flashes) |
| [#214](https://github.com/zortos293/Lichii/pull/214) | Already present / overlap | The base already contains pipeline-cache prebuild support; do not import the older branch wholesale. |
| [#213](https://github.com/zortos293/Lichii/pull/213) | Outside selected performance scope | input: HD Rumble and DualSense haptics through RenHDrum |
| [#206](https://github.com/zortos293/Lichii/pull/206) | Already present / overlap | The corrected u32-word compare-exchange retry loops are already in the base; the merge adds no code. |
| [#204](https://github.com/zortos293/Lichii/pull/204) | Outside selected performance scope | video_core: flush the query cache before macro DMA reads under the GPU split |
| [#203](https://github.com/zortos293/Lichii/pull/203) | Outside selected performance scope | file_sys, config: apply LayeredFS mods to NUP titles; assert nested array index against the innermost array |
| [#199](https://github.com/zortos293/Lichii/pull/199) | Outside selected performance scope | common: write each console log line in one call on Windows |
| [#195](https://github.com/zortos293/Lichii/pull/195) | Outside selected performance scope | ci: decode AV1 in the MinGW build with MSYS2's FFmpeg |
| [#192](https://github.com/zortos293/Lichii/pull/192) | Deferred | Mesh serialization/cache version is already present in the base; removing guarded-program serialization restrictions needs independent correctness review. |
| [#189](https://github.com/zortos293/Lichii/pull/189) | Outside selected performance scope | shader: shadow the gradient and global atomic registers of TXD and ATOMG |
| [#186](https://github.com/zortos293/Lichii/pull/186) | Outside selected performance scope | common: name threads on MinGW builds too |
| [#185](https://github.com/zortos293/Lichii/pull/185) | Outside selected performance scope | texture_cache: land deferred clears before non-pass image writes |
| [#182](https://github.com/zortos293/Lichii/pull/182) | Outside selected performance scope | debug server: frame_times for guest present intervals |
| [#180](https://github.com/zortos293/Lichii/pull/180) | Outside selected performance scope | feat: add native HDR presentation |
| [#179](https://github.com/zortos293/Lichii/pull/179) | Deferred draft | Not ready for production integration; may contain experimental behavior. |
| [#177](https://github.com/zortos293/Lichii/pull/177) | Outside selected performance scope | fix(yuzu): support wayland for performance overlay |
| [#176](https://github.com/zortos293/Lichii/pull/176) | Already present / overlap | The direct uniform-bank upload is already in the base; the merge adds only its explanatory comment. |
| [#175](https://github.com/zortos293/Lichii/pull/175) | Already present / overlap | The base has a storage-binding cache keyed by mapping serial and alignment, plus map-time continuous runs. Do not replace it with the older range-cache implementation. |
| [#173](https://github.com/zortos293/Lichii/pull/173) | Outside selected performance scope | fix(shader): keep depth-compare images float when read int |
| [#172](https://github.com/zortos293/Lichii/pull/172) | Outside selected performance scope | core: log the host clock calibration at boot |
| [#170](https://github.com/zortos293/Lichii/pull/170) | Outside selected performance scope | audio: don't pad a sysmix block with silence while the host has audio queued |
| [#168](https://github.com/zortos293/Lichii/pull/168) | Outside selected performance scope | audio: account retried and padded sysmix frames and log both clocks |
| [#167](https://github.com/zortos293/Lichii/pull/167) | Included test support | Compare only defined visible NV12 pixels, while retaining full/limited-range difference checks. |
| [#166](https://github.com/zortos293/Lichii/pull/166) | Deferred | Mixed overlapping fixes and experimental switches; do not merge the whole branch. |
| [#164](https://github.com/zortos293/Lichii/pull/164) | Outside selected performance scope | build: compile with clang-cl on Windows (NOMINMAX and FARPROC casts) |
| [#161](https://github.com/zortos293/Lichii/pull/161) | Deferred | Large mixed timing/JIT/synchronization patch overlaps promoted #184/#194 changes; original speedup claims were corrected by the author. |
| [#160](https://github.com/zortos293/Lichii/pull/160) | Outside selected performance scope | loader: fail the NSO load when ZBIC decompression fails (#95) |
| [#159](https://github.com/zortos293/Lichii/pull/159) | Outside selected performance scope | file_sys: fall back to the loaded RomFS for program index 0 |
| [#149](https://github.com/zortos293/Lichii/pull/149) | Deferred | Mixed overlapping split/FSR3/shader fixes; assess remaining independent changes separately. |
| [#147](https://github.com/zortos293/Lichii/pull/147) | Deferred | Base has newer CPU readback/trap changes; needs a narrow rebased comparison, not a whole old-branch merge. |
| [#143](https://github.com/zortos293/Lichii/pull/143) | Deferred | Driver-specific tensor fallback changes rendered output; requires Intel gameplay and image comparisons. |
| [#114](https://github.com/zortos293/Lichii/pull/114) | Deferred draft | Not ready for production integration; may contain experimental behavior. |
| [#28](https://github.com/zortos293/Lichii/pull/28) | Outside selected performance scope | Windows: select the AV1 FFmpeg package, and document MSVC build pitfalls |
| [#26](https://github.com/zortos293/Lichii/pull/26) | Outside selected performance scope | Nup from build 168 |

## Validation

Passed: Clang 22/Ninja Release build of Qt frontend (`yuzu` / `bin/lichii`), CLI (`yuzu-cmd` / `bin/eden-cli`), `tests`, and `update_checker_tests`, with Vulkan and OpenGL enabled and `ENABLE_WERROR=OFF`. Existing warnings remain.

The main executable passed **773 test cases / 69,539,381 assertions** in 16.35 seconds, including read-trap, page-filter, instance-loop and software-matrix cases. CTest initially could not run the updater test because the explicit target list omitted its executable; after building that target, `ctest --test-dir build --rerun-failed --output-on-failure --no-tests=error` passed it. Both registered test executables passed.

All six emitted software HMMA SPIR-V modules (K8/K16, repeated call sites and 64-lane subgroups) passed `spirv-val --target-env vulkan1.2`. `git diff --check` passed.

Passed: ASTC decoder compiles with glslang for both Vulkan and OpenGL, and both generated modules pass spirv-val. The production host-visibility probe passes all eight coherent mapped readback cases on NVIDIA GeForce RTX 3070 Ti, with zero Vulkan validation errors. ThreadWorker Python tests: 8 passed. Optional Python/Vulkan probe dependencies were supplied with an isolated uv environment; SPIR-V tools were extracted under /tmp without installing system packages.

Added integration regression tests for page-filter boundary/fallback behavior and split-front instance-loop transitions through multi-method batches and shadow replay.

No game was started, stopped or controlled. No new gameplay FPS or cross-driver visual correctness claim is made. Windows-specific behavior remains untested locally. The separate Eglantine upstream has diverged; this task uses the Lichii source branch and does not indiscriminately merge Eglantine history.
