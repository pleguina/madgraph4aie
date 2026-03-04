# System Project Build Scripts

Complete build automation for the 80-Pipeline Packet-Switched Architecture on VCK190.

## Prerequisites

Before running these scripts, ensure you have:

1. ✅ **AIE graph compiled**: `5k_impl/build/hw/libadf.a`
2. ✅ **PL kernels compiled**:
   - `MM2S_pkt_gen/MM2S_pkt_gen/mm2s_pkt_gen.xo`
   - `S2MM_pkt_gen/S2MM_pkt_gen/s2mm_pkt_parser.xo`
3. ✅ **Vitis 2024.1** installed
4. ✅ **Common Image** installed at `/opt/xilinx/platform/xilinx-versal-common-v2024.1/`

## Build Scripts

### Option 1: Complete Build (Recommended)

Build everything in one go:

```bash
cd /home/pelayo/work/vitis_workspace/system_project
./build_all.sh
```

This runs all three steps:
1. System linking (AIE + PL kernels) → `binary_container_1.xsa`
2. Host application compilation → `host.exe`
3. SD card packaging → `sd_card/` directory

**Estimated time**: 1-2 hours (mostly system linking)

### Option 2: Step-by-Step Build

Run individual steps:

#### Step 1: System Link
```bash
./build_link.sh
```
- Links AIE graph with PL kernels
- Creates hardware binary (`.xsa`)
- **Time**: 1-2 hours

#### Step 2: Build Host Application
```bash
./build_host.sh
```
- Cross-compiles `host.cpp` for ARM
- Creates `host.exe` for VCK190
- **Time**: < 1 minute

#### Step 3: Package SD Card
```bash
./package_sd.sh
```
- Creates bootable SD card image
- Packages BOOT.BIN, kernel, xclbin, host app
- **Time**: 5-10 minutes

### Option 3: Selective Build

Skip certain steps:

```bash
# Only link (if you already have host + package)
./build_all.sh --link-only

# Only build host (if you already have .xsa)
./build_all.sh --host-only

# Only package (if you already have .xsa and host.exe)
./build_all.sh --package-only

# Skip linking (useful for quick iterations)
./build_all.sh --no-link
```

## Output Files

### After System Link
```
build/hw/
├── binary_container_1.xsa        # Hardware specification archive
├── logs/                         # Build logs
└── reports/                      # Timing, utilization reports
```

### After Host Build
```
host_app/build/
└── host.exe                      # ARM executable
```

### After Packaging
```
build/hw/package/sd_card/
├── BOOT.BIN                      # Boot loader
├── Image                         # Linux kernel
├── boot.scr                      # U-Boot script
├── binary_container_1.xclbin     # Hardware binary (bitstream + AIE config)
└── rootfs.ext4                   # Root filesystem

deploy_YYYYMMDD_HHMMSS/           # Timestamped deployment package
├── BOOT.BIN
├── binary_container_1.xclbin
├── Image
├── host.exe
└── data/                         # PSP input data
```

## Configuration Files

### `hw_link/system_link.cfg`

System link configuration defining:
- **Kernel instantiation**: 8 instances of MM2S and S2MM
- **Stream connections**: PL kernels ↔ AIE PLIOs
- **Memory mapping**: DDR bank assignments
- **Clock frequencies**: 312.5 MHz for PL kernels
- **Implementation strategy**: Performance-optimized

Key settings:
```ini
# 8 trunks × 10 pipelines = 80 total pipelines
nk=mm2s_pkt_gen:8
nk=s2mm_pkt_parser:8

# Stream connections
stream_connect=mm2s_0.axis_out:ai_engine_0.psp_in_0
...
stream_connect=ai_engine_0.me2_out_0:s2mm_0.axis_in
...

# DDR bank distribution for bandwidth
sp=mm2s_0.psp_buffer:DDR[0]
...
```

## Troubleshooting

### System Link Fails

**Issue**: Timing violations

**Solution**:
```bash
# Check timing reports
cat build/hw/reports/timing_*.rpt

# Lower clock frequency in system_link.cfg
# Change from 312.5 MHz to 250 MHz:
freqHz=250000000:mm2s_0.psp_buffer
```

**Issue**: Resource overflow (too many kernels)

**Solution**: Design fits 8 trunks × 10 pipelines. If issues occur, check:
```bash
cat build/hw/reports/link.summary
# Look for:
# - AIE tile utilization (should be ~80/400 tiles)
# - PL LUT utilization (should be < 70%)
```

### Host Build Fails

**Issue**: Cross-compiler not found

**Solution**:
```bash
# Verify Common Image installation
ls /opt/xilinx/platform/xilinx-versal-common-v2024.1/

# Reinstall if needed
cd /path/to/common_image_download/
./sdk.sh -d /opt/xilinx/platform/xilinx-versal-common-v2024.1
```

**Issue**: XRT headers not found

**Solution**:
```bash
# Verify sysroot
ls /opt/xilinx/platform/xilinx-versal-common-v2024.1/sysroots/cortexa72-cortexa53-amd-linux/usr/include/xrt/
```

### Packaging Fails

**Issue**: Missing input files

**Solution**:
```bash
# Verify all inputs exist
ls build/hw/binary_container_1.xsa
ls build/hw/host.exe
ls 5k_impl/build/hw/libadf.a

# Rebuild missing components
./build_link.sh   # if .xsa missing
./build_host.sh   # if host.exe missing
```

## Viewing Reports

### Timing Reports
```bash
cat build/hw/reports/timing_route.rpt
```

### Resource Utilization
```bash
cat build/hw/reports/link.summary
```

### AIE Compilation Log
```bash
cat 5k_impl/build/hw/AIECompiler.log
```

### System Link Log
```bash
cat build/hw/logs/link.log
```

## Next Steps After Building

See [VCK190_DEPLOYMENT_GUIDE.md](../VCK190_DEPLOYMENT_GUIDE.md) for:
- Preparing SD card
- Booting VCK190
- Running the application
- Debugging on hardware

## Quick Reference

| Command | Purpose | Time | Output |
|---------|---------|------|--------|
| `./build_all.sh` | Complete build | 1-2h | SD card image |
| `./build_link.sh` | Link PL + AIE | 1-2h | `.xsa` file |
| `./build_host.sh` | Build host app | <1m | `host.exe` |
| `./package_sd.sh` | Create SD image | 5-10m | `sd_card/` |

## Architecture Summary

**Process**: g g → t t̄ g (gluon-gluon → top pair + gluon)

**Hardware**:
- 8 trunks × 10 pipelines = **80 parallel pipelines**
- Each pipeline: K1 → K2a → K2b → K3 → K4 (5 kernels)
- Total: **400 AIE tiles**
- Packet-switched routing via `pktsplit<10>` / `pktmerge<10>`

**Data Flow**:
```
DDR → MM2S[8] → AIE_PLIO[8] → pktsplit[8] → 80_pipelines → pktmerge[8] → AIE_PLIO[8] → S2MM[8] → DDR
```

**Performance Target**:
- 80 PSPs processed simultaneously
- ~1 µs per PSP
- ~1 million PSP/s throughput
