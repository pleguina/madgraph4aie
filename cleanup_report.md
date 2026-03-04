# Repository Cleanup Report

**Date**: 2026-03-04
**Repository**: `/home/pelayo/work/vitis_workspace`
**Original size**: ~143 GB
**Cleaned size**: ~21 MB (source + docs + platform metadata)
**Quarantined**: ~46 GB (in `_quarantine/`)

---

## 1. How This Repo Builds End-to-End

```
1. AIE Graph Compile     5k_impl/ → cmake + aiecompiler → libadf.a
2. HLS Kernel Compile    MM2S_pkt_gen/ → v++ -c --mode hls → mm2s_pkt_gen.xo
                         S2MM_pkt_gen/ → v++ -c --mode hls → s2mm_pkt_parser.xo
3. System Link           system_project/build_link.sh → v++ -l → binary_container_1.xclbin
4. Host Compile          system_project/build_host.sh → aarch64 g++ → host.exe
5. Package               system_project/package_sd.sh → v++ -p → sd_card.img
6. Deploy                system_project/prepare_sd_card.sh or update_ssh.sh → VCK190 board
```

**Evidence of build types**:
- AIE: `aiecompiler.cfg`, `vitis-comp.json`, `ggttg_graph_*.h`, `CMakeLists.txt`
- HLS: `hls_config.cfg`, `mm2s_pkt_gen.cpp`, `s2mm_pkt_parser.cpp`
- Host: `CMakeLists.txt`, `host.cpp`, XRT usage
- Platform: `platform/export/platform/`, `hw.xsa` (quarantined)
- System: `vitis-sys.json`, `system_link.cfg`, `package.cfg`, build scripts

---

## 2. What Was Moved to `_quarantine/`

### `_quarantine/build_artifacts/` (46 GB)

| Path | Size | Category |
|------|------|----------|
| `5k_impl_build/` | ~11 GB | AIE build outputs (libadf.a, Work/, vcd files, simulator outputs) |
| `host_app_build/` | ~12 MB | Host app CMake build |
| `system_project_build/` | ~35 GB | System link/package outputs (xclbin, sd_card.img, Vivado runs, .ipcache) |

**Rationale**: All regenerable via build scripts. Contains multi-GB VCD waveforms, Vivado checkpoint files, and SD card images.

### `_quarantine/deploy_images/` (124 MB)

| Path | Contents |
|------|----------|
| `deploy_20260211_155307/` | BOOT.BIN, host.exe, data/ |
| `deploy_20260211_181403/` | BOOT.BIN, host.exe, data/ |
| `deploy_20260219_175216/` | BOOT.BIN, xclbin, Image, host.exe, boot.scr, data/ |
| `deploy_20260223_145623/` | BOOT.BIN, xclbin, Image, host.exe, boot.scr, data/ |

**Rationale**: Timestamped deploy snapshots. Regenerable via `system_project/package_sd.sh`.

### `_quarantine/logs/` (67 MB)

| Path | Size | Description |
|------|------|-------------|
| `logs/` (directory) | 67 MB | Vitis builder logs (`builder_*.py`), IDE verbose log (60 MB) |
| `x86simulator_baseline.log` | 58 MB | x86 simulator output |
| `host_app_results.txt` | small | Host app run results |
| `me2_out_0_baseline.txt` | small | Baseline ME² output |
| `system_project_logs/` | small | System project Vitis logs |
| `package.log` | small | Package step log |
| `MM2S xcd.log`, `S2MM xcd.log` | small | HLS compile logs |

### `_quarantine/hls_outputs/` (56 MB)

| Path | Description |
|------|-------------|
| `MM2S_hls/`, `MM2S_logs/`, `MM2S_reports/` | MM2S HLS synthesis, C-sim, implementation outputs |
| `S2MM_hls/`, `S2MM_logs/`, `S2MM_reports/` | S2MM HLS synthesis, C-sim, implementation outputs |
| `mm2s_pkt_gen.xo` | Compiled HLS kernel object |
| `s2mm_pkt_parser.xo` | Compiled HLS kernel object |
| `*.hlscompile_summary`, `*.hlsrun_*_summary` | HLS run summaries |

**Rationale**: All regenerable from source via `v++ -c --mode hls`.

### `_quarantine/large_data/` (262 MB)

| Path | Size | Description |
|------|------|-------------|
| `hw.xsa` | 65 MB | VCK190 platform hardware XSA |
| `hw_emu.xsa` | 65 MB | VCK190 platform HW emulation XSA |
| `export_hw.xsa` | 65 MB | Platform export copy of hw.xsa |
| `export_hw_emu.xsa` | 65 MB | Platform export copy of hw_emu.xsa |
| `psp_in_0.txt` | ~1 MB | 20,000-line PSP input dataset |
| `psp_in_1000.txt` | ~250 KB | 5,000-line PSP dataset |
| `psp_in_0_rowformat.txt` | ~250 KB | Row-format PSP dataset |
| `hw_presynth.pdi` | small | Pre-synthesis PDI |
| `plm.elf` | small | PLM firmware |

**XSA Policy**: Platform XSAs are required but too large for Git (65 MB each). Users should obtain from Vivado export or Xilinx downloads. Documented in root README.

### `_quarantine/ide/` (1.1 MB)

| Path | Description |
|------|-------------|
| `5k_impl_ide/`, `host_app_ide/`, `system_project_ide/` | Vitis IDE project metadata |
| `platform_ide/`, `daisy_chain_impl_ide/`, `versal_test_ide/` | More IDE metadata |
| `5k_impl_cache/`, `MM2S_cache/`, `S2MM_cache/` | clangd index caches |
| `MM2S_Xil/`, `S2MM_Xil/`, `platform_sdt_Xil/` | Xilinx temp directories |
| `vscode/`, `wsdata/` | VS Code and workspace data |

### `_quarantine/duplicates/` (128 KB)

| Path | Description |
|------|-------------|
| `paper_mcfpgas.drawio (1).xml` | Duplicate drawio diagram |
| `paper_mcfpgas.drawio (2).xml` | Duplicate drawio diagram |
| `Paper_MC_v2_root_copy/` | Root-level copy of paper already in `paper/Paper_MC_Acceleration_Hardware_v2/` |

### `_quarantine/docs_drafts/` (32 KB)

| Path | Description |
|------|-------------|
| `BUILD_NAMING_CONSISTENCY_CHECK.md` | One-time build script audit (low value) |
| `HOST_RAMBO_CHANGES.md` | Changelog note for host app RAMBO changes |
| `mem_analysis_5k.txt` | Memory analysis notes (renamed from `mem analysys 5k.txt`) |

### `_quarantine/misc/` (1.6 MB)

| Path | Description |
|------|-------------|
| `processing_core_baseline.cpp` | Baseline single-kernel reference (historical) |
| `wavedrom.py` | WaveDrom timing diagram generator |
| `diagram_untitled.drawio` | Untitled diagram (renamed from Spanish) |
| `cascade_k4_filtered.txt` | Cascade event filter output |
| `.lock` | Vitis workspace lock file |
| `*_compile_commands.json` (6 files) | CMake-generated compile databases |
| `aie_cascade_paper.zip` | Zipped paper (sources already in `paper/`) |
| `sn-article.pdf`, `user-manual.pdf` | Springer template PDFs |

### `_quarantine/secrets/` (empty)

No secrets or credentials were found. See Security Scan below.

---

## 3. What Was Kept and Why

### Source Code (KEPT)

| Component | Key Files | Rationale |
|-----------|-----------|-----------|
| `5k_impl/src/` | 11 C++/H files | Core AIE kernels (K1-K4) and graph definitions |
| `5k_impl/helas/` | 8 header files | HELAS physics function library |
| `5k_impl/scripts/` | 10 Python scripts | Analysis, validation, data generation |
| `5k_impl/data/` | 14 small text files | Sample PSP inputs and expected outputs for testing |
| `5k_impl/alternatives/` | 2 graph headers | Alternative architecture explorations |
| `5k_impl/hslice_experimental/` | 6 source files | Helicity-parallel experimental kernels |
| `5k_impl/archive/` | 5 source files | Historical kernel versions for reference |
| `host_app/src/` | 12 source files | Host application, PSP generator, MadGraph params |
| `host_app/data/` | 16 small files | Test data (<1 KB each: .bin PSP inputs + .txt expected ME²) |
| `host_app/Cards/` | 1 file | Physics parameter card |
| `MM2S_pkt_gen/src/` | 3 files | HLS packet generator source + test |
| `S2MM_pkt_gen/src/` | 3 files | HLS packet parser source + test |
| `daisy_chain_impl/src/` | 7 files | Alternative daisy-chain architecture |
| `system_project/*.sh` | 10 scripts | Build automation scripts |
| `paper/` | ~25 files | LaTeX paper sources, figures, bibliography |

### Build Configuration (KEPT)

| File | Rationale |
|------|-----------|
| `*/CMakeLists.txt` | CMake build definitions |
| `*/vitis-comp.json` | Vitis component configuration |
| `*/aiecompiler.cfg` | AIE compiler settings |
| `*/hls_config.cfg` | HLS settings |
| `system_project/vitis-sys.json` | System project definition |
| `system_project/hw_link/system_link.cfg` | v++ link connectivity |
| `system_project/package/package.cfg` | SD card packaging config |
| `host_app/UserConfig.cmake` | CMake user config |
| `host_app/Makefile` | Hand-written Makefile |

### Platform (KEPT — metadata only)

| Path | Rationale |
|------|-----------|
| `platform/vitis-comp.json` | Platform component config |
| `platform/hw/sdt/` | Device tree sources and bindings (text files) |
| `platform/export/` | Platform export metadata (xpfm, spfm, sdt) |
| `platform/resources/` | Boot resources (BIF, QEMU args, DTB, boot loaders) |

**Note**: XSA files (65 MB each) quarantined. Platform binary files (.elf, .cdo, .pdi) will be excluded by `.gitignore`.

---

## 4. What Was Refactored

### Documentation Reorganization

37 markdown documents were classified and moved from scattered locations (root, `5k_impl/`, `5k_impl/docs/`) into a structured `docs/` hierarchy:

| Category | Count | Source Locations |
|----------|-------|-----------------|
| `docs/architecture/` | 4 docs + 1 drawio | `5k_impl/docs/`, `5k_impl/` root |
| `docs/build/` | 3 docs | Root |
| `docs/deploy/` | 3 docs | Root |
| `docs/validation/` | 4 docs | `5k_impl/docs/`, `5k_impl/`, root |
| `docs/benchmarking/` | 3 docs | Root, `5k_impl/` |
| `docs/troubleshooting/` | 7 docs | Root, `5k_impl/` |
| `docs/analysis/` | 9 docs | `5k_impl/`, `5k_impl/docs/`, root |
| `docs/reference/` | 4 docs | `5k_impl/` |

Created `docs/README.md` as documentation index.

### Filename Normalization

| Original | New | Reason |
|----------|-----|--------|
| `5k_impl/Diagrama sin título.drawio` | `_quarantine/misc/diagram_untitled.drawio` | Non-ASCII + spaces |
| `5k_impl/docs/mem analysys 5k.txt` | `_quarantine/docs_drafts/mem_analysis_5k.txt` | Spaces + typo |
| `5k_impl/paper_mcfpgas.drawio (1).xml` | Quarantined as duplicate | Parenthetical suffix |
| `5k_impl/paper_mcfpgas.drawio (2).xml` | Quarantined as duplicate | Parenthetical suffix |

### .gitignore

Replaced the minimal default `.gitignore` with a comprehensive tailored version covering:
- Vitis/Vivado/AIE build outputs
- HLS outputs
- CMake generated files
- IDE artifacts
- Deploy bundles
- Platform binaries
- Python bytecode
- LaTeX build outputs

Removed redundant sub-directory `.gitignore` files (7 files) now covered by root.

### Root README.md

Completely rewritten to reflect the 80-pipeline architecture:
- Project description and key results
- Repository map
- Quick start build instructions (5 steps)
- Platform XSA policy documentation
- Links to organized `docs/` structure

---

## 5. Security Scan Results

### Files Scanned
- All `*.md`, `*.py`, `*.sh`, `*.json`, `*.cfg`, `*.cpp`, `*.h`, `*.cmake` files
- Searched for: key files (`.pem`, `.key`, `.p12`, `.pfx`, `.netrc`, `.pypirc`, SSH keys)
- Searched for patterns: `API_KEY`, `TOKEN`, `SECRET`, `AWS_`, `OPENAI_`, `HF_`, `GH_`, `GITLAB_`, `bearer`, `password`

### Findings

| File | Match | Assessment |
|------|-------|------------|
| `5k_impl/src/kernels_4k_token.h` | `TOKEN` in filename/guards | **False positive** — "token" refers to data token concept, not auth token |
| `DEPLOYMENT_READY.md` | `Password: root` | **Low risk** — default PetaLinux root password (well-known default) |
| `5k_impl/FOUND_CASCADE_ISSUE.md` | `TOKEN` in context | **False positive** — cascade token transfer discussion |
| `5k_impl/src/kernel_k4_vvvv_color.cpp` | `TOKEN` in context | **False positive** — data token processing |
| `system_project/update_ssh.sh` | Hardcoded IP `156.35.89.171`, user `petalinux` | **Note**: Internal network IP + default PetaLinux user. Not a secret but reveals internal network topology |

**Verdict**: No actual secrets found. No files quarantined to `_quarantine/secrets/`.

**Recommendation**: Before publishing, review `system_project/update_ssh.sh` — consider replacing the hardcoded IP with a variable or placeholder (`VCK190_IP`).

---

## 6. Suspected Important Items Not Validated

| Item | Concern | Action Needed |
|------|---------|---------------|
| `platform/resources/xrt/boot/bl31.elf` | ARM Trusted Firmware binary — cannot verify license | **TODO**: Confirm redistribution rights for boot binaries |
| `platform/resources/xrt/boot/u-boot.elf` | U-Boot binary | **TODO**: Same as above |
| `platform/resources/xrt/boot/system.dtb` | Device tree blob | Likely fine (generated from DTS in platform) |
| `paper/aie_cascade_paper/figures/*.pdf` | Figure PDFs ~small | Verify these are your own figures |
| `paper/springer_template/sn-jnl.cls` | Springer LaTeX class | Standard template — check Springer's redistribution terms |
| `host_app/src/Parameters_sm.cc` | MadGraph5 generated code | Check MadGraph5/GPL license compatibility |
| `host_app/src/read_slha.cc` | SLHA reader | Check origin and license |

---

## 7. Human TODOs

### Critical (Before First Push)

- [ ] **License**: Add a `LICENSE` file. The project combines Xilinx platform files, MadGraph5-generated code, and original work — choose compatible license (e.g., MIT for original code, note GPL dependencies from MadGraph5)
- [ ] **Platform XSA decision**: Either (a) use Git LFS for the 4 × 65 MB XSA files, or (b) keep excluded and document fetch instructions
- [ ] **update_ssh.sh**: Replace hardcoded IP `156.35.89.171` with environment variable or config file
- [ ] **MadGraph5 attribution**: `Parameters_sm.cc`, `read_slha.cc` appear MadGraph5-generated — verify license and add attribution

### Recommended

- [ ] **Git LFS**: If PDFs in `paper/aie_cascade_paper/figures/` will grow, consider `.gitattributes` with LFS rules for `*.pdf`
- [ ] **Test data**: Verify that sample data in `5k_impl/data/` and `host_app/data/` is sufficient to run a basic validation
- [ ] **CI/CD**: Consider adding a `.github/workflows/` or equivalent for basic build verification (at least x86sim)
- [ ] **versal_test/**: Decide whether to keep this (appears to be an empty/template Vitis system project) or quarantine it

### Nice to Have

- [ ] Clean up `platform/` to remove duplicate SDT content between `platform/hw/sdt/` and `platform/export/platform/hw/sdt/`
- [ ] Consider merging `5k_impl/README.md` content into `docs/` to avoid staleness
- [ ] Archive `daisy_chain_impl/` and `5k_impl/hslice_experimental/` if no longer actively developed

---

## 8. Build Path Validation

All source file references in build configs verified present. Known broken paths (all expected — they reference build outputs):

| Config File | Broken Path | Resolution |
|-------------|-------------|------------|
| `system_project/build_link.sh` | `5k_impl/build/hw/libadf.a` | Regenerated by AIE compile step |
| `system_project/build_link.sh` | `MM2S_pkt_gen/MM2S_pkt_gen/mm2s_pkt_gen.xo` | Regenerated by HLS compile. Note: output dir `MM2S_pkt_gen/MM2S_pkt_gen/` is created by v++ |
| `system_project/build_link.sh` | `S2MM_pkt_gen/S2MM_pkt_gen/s2mm_pkt_parser.xo` | Same as above |
| `platform/vitis-comp.json` | `platform/hw/hw.xsa` | Must be obtained externally (see Platform XSA section in README) |

---

## 9. Size Comparison

| Category | Before | After | Reduction |
|----------|--------|-------|-----------|
| **Git-tracked content** | ~143 GB | ~21 MB | 99.99% |
| Build artifacts | ~46 GB | 0 | Quarantined |
| Deploy images | ~124 MB | 0 | Quarantined |
| Logs | ~67 MB | 0 | Quarantined |
| Platform XSAs | ~262 MB | 0 | Quarantined |
| HLS outputs | ~56 MB | 0 | Quarantined |
| IDE/cache | ~1.1 MB | 0 | Quarantined |
| Documentation | Scattered (37 files) | Organized in `docs/` (8 categories) | Restructured |
