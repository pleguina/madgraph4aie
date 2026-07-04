# Complete Step-by-Step Deployment Guide
## Versal VCK190: Host (XRT) + PL Kernels + AI Engine

This document describes **ALL steps required** to:
- Compile AI Engine graph
- Compile PL kernels  
- Link system
- Build host application (XRT, embedded Linux)
- Package bootable SD card image
- Deploy and run on VCK190

**Target:** Embedded Linux on VCK190 (SD boot)  
**Mode:** Hardware (`hw`)

---

## SECTION 0 — PREREQUISITES

### 0.1 Install Required Tools

Ensure the following are installed:

- **Vitis** (2024.1 or compatible version)
- **Vivado** (same version as Vitis)
- **XRT** (Xilinx Runtime)
- **VCK190 Base Platform** (`xilinx_vck190_base_202410_1.xpfm` or equivalent)
- **Common Image** (contains sysroot, kernel, rootfs for ARM cross-compilation)

### 0.2 Source Tool Environments

```bash
# Source Vitis and Vivado
source <VITIS_INSTALL_PATH>/settings64.sh

# Source XRT
source /opt/xilinx/xrt/setup.sh
```

**Example:**
```bash
source /tools/Xilinx/Vitis/2024.1/settings64.sh
source /opt/xilinx/xrt/setup.sh
```

### 0.3 Set Platform Repository Path (if needed)

```bash
export PLATFORM_REPO_PATHS=<PATH_TO_PLATFORMS>
```

**Example:**
```bash
export PLATFORM_REPO_PATHS=/tools/Xilinx/Vitis/2024.1/base_platforms
```

### 0.4 Install and Configure Common Image

The Common Image provides the Linux kernel, root filesystem, and cross-compilation sysroot for ARM.

#### Download and Install

1. Download the **Common Image** for your Vitis version from Xilinx Downloads
2. Extract and run the installer:

```bash
cd <COMMON_IMAGE_DOWNLOAD_DIR>
./sdk.sh -d <COMMON_IMAGE_INSTALL_DIR>
```

**Example:**
```bash
./sdk.sh -d /opt/xilinx/platform/xilinx-versal-common-v2024.1
```

#### Set Environment Variables

After installation, define the following variables:

```bash
export COMMON_IMAGE_DIR=<COMMON_IMAGE_INSTALL_DIR>
export SYSROOT=${COMMON_IMAGE_DIR}/sysroots/cortexa72-cortexa53-amd-linux
export IMAGE=${COMMON_IMAGE_DIR}/Image
export ROOTFS=${COMMON_IMAGE_DIR}/rootfs.ext4
```

**Example:**
```bash
export COMMON_IMAGE_DIR=/opt/xilinx/platform/xilinx-versal-common-v2024.1
export SYSROOT=${COMMON_IMAGE_DIR}/sysroots/cortexa72-cortexa53-amd-linux
export IMAGE=${COMMON_IMAGE_DIR}/Image
export ROOTFS=${COMMON_IMAGE_DIR}/rootfs.ext4
```

#### Verify Installation

```bash
ls $SYSROOT/usr/include/xrt    # Should show XRT headers
ls $IMAGE                      # Should show Linux kernel image
ls $ROOTFS                     # Should show rootfs.ext4
```

---

## SECTION 1 — BUILD AI ENGINE GRAPH

### 1.1 Prepare AIE Source Files

Ensure your AIE project contains:
- **Graph definition** (`graph.cpp` or similar)
- **Kernel sources** (`.cpp` files with AIE kernel implementations)
- **Configuration file** (`aiecompiler.cfg`)

**Example structure:**
```
5k_impl/
├── src/
│   ├── graph.cpp
│   ├── kernels.cpp
│   └── kernels.h
├── aiecompiler.cfg
└── data/
```

### 1.2 Configure AIE Compiler

Edit `aiecompiler.cfg` to specify compilation options:

```properties
include=/path/to/project
include=/path/to/project/src

[aie]
Xchess=main:darts.xargs=-nb
stacksize=2048
verbose=true
xlopt=2
heapsize=1024
```

### 1.3 Compile AIE Graph

Navigate to your AIE project directory and run:

```bash
cd /path/to/aie_project
aiecompiler \
  --target=hw \
  --platform=xilinx_vck190_base_202410_1 \
  --workdir=Work \
  graph.cpp
```

**Parameters:**
- `--target=hw` : Compile for hardware (not simulation)
- `--platform=<PLATFORM>` : Specify VCK190 platform
- `--workdir=Work` : Output directory for compilation artifacts
- `graph.cpp` : Main graph file

### 1.4 Verify Output

Check for successful compilation:

```bash
ls Work/libadf.a
```

**Result artifact:** `Work/libadf.a`  
This static library contains the compiled AI Engine graph and will be used during system linking.

**Compilation logs:**
- `Work/AIECompiler.log` (detailed compilation log)
- `Work/reports/` (resource utilization reports)

---

## SECTION 2 — BUILD PL KERNELS (.xo)

PL kernels are compiled from HLS C/C++ source into Xilinx Object (`.xo`) files.

### 2.1 Prepare HLS Kernel Sources

Example kernel structure:
```
MM2S_pkt_gen/
└── src/
    └── mm2s_pkt_gen.cpp
```

### 2.2 Configure HLS Compilation (Optional)

Create `hls_config.cfg` for advanced HLS options:

```properties
[hls]
clock=312.5MHz
syn.directive.inline=all
syn.directive.pipeline=on
```

### 2.3 Compile Each PL Kernel

Use `v++` in compile mode (`-c`) to generate `.xo` files:

```bash
v++ -c -t hw \
  --platform xilinx_vck190_base_202410_1 \
  -k mm2s_pkt_gen \
  --config hls_config.cfg \
  -o mm2s_pkt_gen.xo \
  src/mm2s_pkt_gen.cpp
```

**Parameters:**
- `-c` : Compile mode (generate `.xo`)
- `-t hw` : Target hardware
- `--platform <PLATFORM>` : VCK190 platform
- `-k <KERNEL_NAME>` : Kernel name (must match function name in source)
- `--config <CFG>` : Optional configuration file
- `-o <OUTPUT.xo>` : Output object file
- `<SOURCE.cpp>` : Kernel source file

### 2.4 Repeat for All Kernels

For a multi-kernel system, compile each one:

```bash
# MM2S kernel
v++ -c -t hw --platform xilinx_vck190_base_202410_1 \
  -k mm2s_pkt_gen -o mm2s_pkt_gen.xo src/mm2s_pkt_gen.cpp

# S2MM kernel
v++ -c -t hw --platform xilinx_vck190_base_202410_1 \
  -k s2mm_pkt_parser -o s2mm_pkt_parser.xo src/s2mm_pkt_parser.cpp
```

### 2.5 Verify Output

```bash
ls *.xo
# Should show: mm2s_pkt_gen.xo s2mm_pkt_parser.xo
```

---

## SECTION 3 — SYSTEM LINK (PL + AIE)

System linking integrates PL kernels, AI Engine graph, and platform into a single hardware design.

### 3.1 Create System Configuration File

Create `system.cfg` to specify connectivity and system settings:

```ini
[connectivity]
# Stream connections between PL and AIE
stream_connect=mm2s_pkt_gen_1.s:ai_engine_0.DataIn0
stream_connect=ai_engine_0.DataOut0:s2mm_pkt_parser_1.s

# DDR memory connections
sp=mm2s_pkt_gen_1.m_axi_gmem:DDR[0]
sp=s2mm_pkt_parser_1.m_axi_gmem:DDR[1]

[advanced]
# Timing and routing options
param=compiler.addOutputTypes=hw_export
param=compiler.enableMultiCuPartitioning=true

[clock]
# Clock frequency for PL kernels (MHz)
freqHz=312500000:mm2s_pkt_gen_1
freqHz=312500000:s2mm_pkt_parser_1
```

### 3.2 Perform System Link

Link all components together:

```bash
v++ -l -t hw \
  --platform xilinx_vck190_base_202410_1 \
  --config system.cfg \
  --save-temps \
  -o binary_container.xsa \
  mm2s_pkt_gen.xo \
  s2mm_pkt_parser.xo \
  Work/libadf.a
```

**Parameters:**
- `-l` : Link mode
- `-t hw` : Hardware target
- `--platform <PLATFORM>` : VCK190 platform
- `--config <CFG>` : System configuration file
- `--save-temps` : Keep intermediate files for debugging
- `-o <OUTPUT.xsa>` : Output hardware archive (`.xsa` or `.xclbin`)
- `<KERNEL.xo>` : All PL kernel objects
- `<AIE_GRAPH.a>` : AIE graph library (`libadf.a`)

### 3.3 Linking Process

This step performs:
1. **Place and route** of PL kernels in Programmable Logic
2. **Integration** of AI Engine graph
3. **Stream network** generation (NoC, AXI-Stream)
4. **Memory mapping** (DDR controllers)
5. **Bitstream generation**

**Note:** This step can take **30 minutes to several hours** depending on design complexity.

### 3.4 Monitor Progress

Check linking logs:

```bash
tail -f _x/logs/link/vivado.log
```

### 3.5 Verify Output

After successful linking:

```bash
ls binary_container.xsa
ls _x/reports/link/  # Timing, resource utilization reports
```

**Result artifacts:**
- `binary_container.xsa` : Hardware specification archive (contains bitstream + metadata)
- Or `binary_container.xclbin` : XRT loadable binary (depending on flow)

---

## SECTION 4 — BUILD HOST APPLICATION (XRT, EMBEDDED)

The host application runs on VCK190's ARM cores and controls the hardware via XRT.

### 4.1 Source Cross-Compilation Environment

```bash
source ${COMMON_IMAGE_DIR}/environment-setup-cortexa72-cortexa53-amd-linux
```

**This script sets:**
- Cross-compiler (`CXX`, `CC`)
- ARM target flags
- Sysroot path

### 4.2 Verify Cross-Compilation Setup

```bash
echo $CXX
# Should show: aarch64-xilinx-linux-g++

$CXX --version
# Should show ARM cross-compiler
```

### 4.3 Compile Host Application

```bash
${CXX} host.cpp \
  -o host.exe \
  --sysroot=${SYSROOT} \
  -I${SYSROOT}/usr/include/xrt \
  -L${SYSROOT}/usr/lib \
  -lxrt_coreutil \
  -std=c++17 \
  -O3 \
  -Wall
```

**Parameters:**
- `--sysroot=${SYSROOT}` : Use ARM sysroot for headers/libraries
- `-I${SYSROOT}/usr/include/xrt` : XRT headers
- `-L${SYSROOT}/usr/lib` : XRT libraries
- `-lxrt_coreutil` : Link XRT core utilities
- `-std=c++17` : C++17 standard (required by XRT)
- `-O3` : Optimization level

### 4.4 Example Host Application Structure

A minimal XRT host must:

```cpp
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_aie.h>

int main(int argc, char** argv) {
    // 1. Load xclbin
    auto device = xrt::device(0);
    auto uuid = device.load_xclbin(argv[1]);
    
    // 2. Start AIE graph
    auto graph = xrt::graph(device, uuid, "mygraph");
    graph.reset();
    graph.run(-1);  // Run indefinitely
    
    // 3. Allocate buffers
    auto bo_in = xrt::bo(device, size, xrt::bo::flags::normal, 0);
    auto bo_out = xrt::bo(device, size, xrt::bo::flags::normal, 0);
    
    // 4. Launch PL kernels
    auto krnl_mm2s = xrt::kernel(device, uuid, "mm2s_pkt_gen");
    auto run_mm2s = krnl_mm2s(bo_in, size);
    
    // 5. Wait and retrieve results
    run_mm2s.wait();
    bo_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    
    // 6. Cleanup
    graph.end();
    
    return 0;
}
```

### 4.5 Verify Host Binary

```bash
file host.exe
# Should show: ELF 64-bit LSB executable, ARM aarch64
```

---

## SECTION 5 — PACKAGE SD CARD IMAGE

Package all components into a bootable SD card image.

### 5.1 Create Package Configuration File

Create `package.cfg`:

```ini
[package]
out_dir=./sd_card
kernel_image=/opt/xilinx/platform/xilinx-versal-common-v2024.1/Image
rootfs=/opt/xilinx/platform/xilinx-versal-common-v2024.1/rootfs.ext4
enable_aie_debug=true

[advanced]
boot_mode=sd
```

**Adjust paths** to match your `$IMAGE` and `$ROOTFS` locations.

### 5.2 Run V++ Package Command

```bash
v++ -p -t hw \
  --platform xilinx_vck190_base_202410_1 \
  --config package.cfg \
  --package.out_dir ./sd_card \
  --package.boot_mode=sd \
  --package.ps_elf host.exe,a72-0 \
  --package.defer_aie_run \
  binary_container.xsa \
  Work/libadf.a
```

**Parameters:**
- `-p` : Package mode
- `-t hw` : Hardware target
- `--package.out_dir` : Output directory for SD card files
- `--package.boot_mode=sd` : SD card boot mode
- `--package.ps_elf <EXE>,a72-0` : Include host executable (runs on ARM A72 core 0)
- `--package.defer_aie_run` : Don't auto-start AIE (host will control it)
- `<CONTAINER.xsa>` : Hardware archive
- `<LIBADF.a>` : AIE graph library

### 5.3 Packaging Process

This step creates:
1. **Boot images** (BOOT.BIN with FSBL, U-Boot, ATF, etc.)
2. **Kernel image** (Image)
3. **Root filesystem** (rootfs.ext4)
4. **Hardware binaries** (binary_container.xclbin)
5. **Host application** (embedded in rootfs or separate partition)

### 5.4 Verify Package Output

```bash
ls sd_card/
# Should contain:
# BOOT.BIN
# binary_container.xclbin
# Image
# rootfs.ext4
```

**Important files:**
- `BOOT.BIN` : First-stage boot loader
- `binary_container.xclbin` : Hardware binary (bitstream + AIE configuration)
- `Image` : Linux kernel
- `rootfs.ext4` : Root filesystem (contains host.exe)

---

## SECTION 6 — PREPARE SD CARD

### 6.1 Partition SD Card

Insert SD card into your PC and identify the device:

```bash
lsblk
# Example output: /dev/sdb
```

**⚠️ WARNING:** Ensure you select the correct device! Wrong selection will destroy data.

Partition the SD card:

```bash
sudo fdisk /dev/sdb

# In fdisk:
# - Press 'o' to create new DOS partition table
# - Press 'n' to create new partition
#   - Primary partition (1)
#   - First sector: 2048
#   - Last sector: +1G (for boot partition)
# - Press 't' to change partition type
#   - Type: c (W95 FAT32 LBA)
# - Press 'a' to make it bootable
# - Press 'w' to write and exit
```

### 6.2 Format Partitions

```bash
# Format boot partition (FAT32)
sudo mkfs.vfat -F 32 -n BOOT /dev/sdb1
```

### 6.3 Copy Files to SD Card

Mount and copy boot files:

```bash
# Mount boot partition
sudo mkdir -p /mnt/sd_boot
sudo mount /dev/sdb1 /mnt/sd_boot

# Copy boot files
sudo cp sd_card/BOOT.BIN /mnt/sd_boot/
sudo cp sd_card/Image /mnt/sd_boot/
sudo cp sd_card/binary_container.xclbin /mnt/sd_boot/

# Unmount
sudo umount /mnt/sd_boot
```

### 6.4 Write Root Filesystem

```bash
# Write rootfs directly to SD card
sudo dd if=sd_card/rootfs.ext4 of=/dev/sdb2 bs=4M status=progress

# Sync and eject
sync
sudo eject /dev/sdb
```

### 6.5 Alternative: Use Pre-Built Script

Xilinx provides scripts for automated SD card preparation:

```bash
cd sd_card
sudo ./sd_card_install.sh /dev/sdb
```

---

## SECTION 7 — DEPLOY AND RUN ON VCK190

### 7.1 Hardware Setup

1. **Insert SD card** into VCK190 SD card slot
2. **Set boot mode** to SD boot:
   - SW1 switches: `ON-OFF-OFF-OFF` (SD boot mode)
3. **Connect serial console**:
   - USB micro-B to PC
   - Baud rate: 115200
   - Open terminal: `sudo minicom -D /dev/ttyUSB0` or use PuTTY
4. **Connect Ethernet** (optional, for SSH access)
5. **Power on** VCK190

### 7.2 Boot Process

Monitor serial console during boot:

```
Xilinx Versal Platform Loader and Manager
Release 2024.1   Jan 10 2024  -  10:30:00

Loading PDI from SD
+++++++++ PDI Load: Done 0x6 ms +++++++++

Xilinx Zynq MP First Stage Boot Loader 
Release 2024.1

U-Boot 2024.01 (Jan 10 2024 - 10:30:00 +0000)

Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd083]
[    0.000000] Linux version 6.1.0-xilinx-v2024.1 ...

Welcome to PetaLinux 2024.1!

Versal login:
```

### 7.3 Login

Default credentials:
- **Username:** `root`
- **Password:** `root`

### 7.4 Verify XRT Installation

```bash
xbutil examine
```

Expected output:
```
System Configuration
  OS Name              : Linux
  XRT Version          : 2.17.0
  
Devices present
  [0] : xilinx_vck190_base_202410_1
```

### 7.5 Transfer Application and Data (if needed)

If host application is not in rootfs, copy via network:

```bash
# On PC
scp host.exe root@<VCK190_IP>:/run/media/mmcblk0p1/
scp *.bin root@<VCK190_IP>:/run/media/mmcblk0p1/data/

# On VCK190
cd /run/media/mmcblk0p1
chmod +x host.exe
```

### 7.6 Run Application

```bash
cd /run/media/mmcblk0p1
./host.exe binary_container.xclbin [arguments]
```

**Example:**
```bash
./host.exe binary_container.xclbin ./data
```

### 7.7 Expected Output

```
=============================================================================
 80-Pipeline Packet-Switched Matrix Element Accelerator
=============================================================================
 Process:       g g > t t~ g
 Trunks:        8
 PSPs/trunk:    10
 Total PSPs:    80

Loading xclbin: binary_container.xclbin
Device(0): Success
Programming device...
Device programmed successfully

Initializing AIE graph...
AIE graph started

Allocating buffers...
Input buffer:  DDR[0x00000000] (6400 bytes)
Output buffer: DDR[0x00100000] (640 bytes)

Launching PL kernels...
MM2S kernel started
S2MM kernel started

Waiting for completion...
✓ Execution complete!
  Total time: 80.5 ms
  Per PSP:    1.006 µs

Results (ME² values):
---------------------------------------------
Trunk 0:
  PSP 0: 2.0931918700e-04
  PSP 1: 3.4567890123e-05
  ...

=============================================================================
 Execution Summary
=============================================================================
 PSPs processed:    80
 Execution time:    80.5 ms
 Throughput:        994 PSP/s
 Energy efficiency: 54.8 µJ/ME (AIE domain @ 54.8 W)
=============================================================================
```

---

## SECTION 8 — TROUBLESHOOTING

### 8.1 Compilation Errors

#### AIE Compilation Fails

**Check:**
- Kernel stack/heap size (`aiecompiler.cfg`)
- Graph connectivity (streams, windows)
- Resource utilization (`Work/reports/`)

**View logs:**
```bash
cat Work/AIECompiler.log | grep -i error
```

#### PL Kernel Compilation Fails

**Check:**
- HLS pragmas and directives
- Interface specifications (`#pragma HLS interface`)
- Timing constraints

**View logs:**
```bash
cat _x/logs/v++_compile_*.log
```

### 8.2 Linking Errors

#### Timing Violations

**Check timing reports:**
```bash
cat _x/reports/link/timing_summary.rpt
```

**Solutions:**
- Lower clock frequency in `system.cfg`
- Add pipelining pragmas
- Enable retiming: `param=compiler.enableRetiming=true`

#### Routing Failures

**Solutions:**
- Simplify connectivity
- Reduce number of kernel instances
- Use different DDR banks for memory connections

### 8.3 Runtime Errors

#### Device Not Found

```bash
xbutil examine
```

If no device shown:
```bash
# Check XRT installation
source /opt/xilinx/xrt/setup.sh

# Reload XRT drivers
sudo rmmod zocl
sudo modprobe zocl
```

#### Xclbin Loading Fails

**Check:**
- Platform mismatch (compile platform == runtime platform)
- Xclbin corruption during transfer
- File permissions: `chmod 644 *.xclbin`

#### AIE Graph Doesn't Start

**Enable debug:**
```bash
export XILINX_XRT=/usr
export XCL_EMULATION_MODE=hw
export XRT_INI_PATH=/path/to/xrt.ini
```

Create `xrt.ini`:
```ini
[Debug]
aie_trace=true
timeline_trace=true
```

### 8.4 Boot Failures

#### SD Card Not Booting

**Check:**
- Boot mode switches (SW1)
- SD card formatting (FAT32 for boot partition)
- BOOT.BIN present and valid

#### Kernel Panic

**Check:**
- Correct rootfs for platform
- Kernel/rootfs version compatibility
- Device tree issues

**View boot logs:**
- Monitor serial console during boot
- Check for file system errors

---

## SECTION 9 — OPTIMIZATION TIPS

### 9.1 Performance Optimization

1. **Increase clock frequency** (if timing allows):
   ```ini
   [clock]
   freqHz=400000000:mm2s_pkt_gen_1
   ```

2. **Enable multi-CU partitioning**:
   ```ini
   [advanced]
   param=compiler.enableMultiCuPartitioning=true
   ```

3. **Optimize AIE graph**:
   - Use cascading for throughput
   - Balance tile utilization
   - Minimize stream hops

4. **PL kernel optimization**:
   - Add `#pragma HLS pipeline II=1`
   - Use burst transfers
   - Optimize dataflow

### 9.2 Debugging

Enable comprehensive logging:

```ini
[Debug]
profile=true
timeline_trace=true
device_trace=fine
aie_trace=true
aie_profile=all
opencl_summary=true
opencl_device_counter=true
```

Generate profiling reports:
```bash
xbutil examine -r all -o report.json
```

### 9.3 Resource Reduction

If design doesn't fit:

1. **Reduce AIE tiles:** Simplify graph
2. **Reduce PL area:** Smaller kernels or fewer instances
3. **Use packet switching:** Share interfaces
4. **Enable HLS optimization:** `xlopt=2` in `aiecompiler.cfg`

---

## SECTION 10 — QUICK REFERENCE

### Complete Build Flow (One Script)

```bash
#!/bin/bash
set -e

# Environment setup
source /tools/Xilinx/Vitis/2024.1/settings64.sh
source /opt/xilinx/xrt/setup.sh
export PLATFORM=xilinx_vck190_base_202410_1
export COMMON_IMAGE_DIR=/opt/xilinx/platform/xilinx-versal-common-v2024.1
export SYSROOT=${COMMON_IMAGE_DIR}/sysroots/cortexa72-cortexa53-amd-linux

# 1. Compile AIE
cd aie_component
aiecompiler --target=hw --platform=${PLATFORM} --workdir=Work graph.cpp
cd ..

# 2. Compile PL kernels
v++ -c -t hw --platform ${PLATFORM} -k mm2s_pkt_gen -o mm2s.xo MM2S_pkt_gen/src/mm2s_pkt_gen.cpp
v++ -c -t hw --platform ${PLATFORM} -k s2mm_pkt_parser -o s2mm.xo S2MM_pkt_gen/src/s2mm_pkt_parser.cpp

# 3. Link system
v++ -l -t hw --platform ${PLATFORM} --config system.cfg -o design.xsa mm2s.xo s2mm.xo aie_component/Work/libadf.a

# 4. Build host
source ${COMMON_IMAGE_DIR}/environment-setup-cortexa72-cortexa53-amd-linux
${CXX} host_app/src/host.cpp -o host.exe --sysroot=${SYSROOT} \
  -I${SYSROOT}/usr/include/xrt -L${SYSROOT}/usr/lib -lxrt_coreutil -std=c++17

# 5. Package SD card
v++ -p -t hw --platform ${PLATFORM} --config package.cfg \
  --package.out_dir ./sd_card --package.ps_elf host.exe,a72-0 \
  --package.defer_aie_run design.xsa aie_component/Work/libadf.a

echo "Build complete! SD card files in ./sd_card/"
```

### Key File Extensions

| Extension | Description |
|-----------|-------------|
| `.xo` | Xilinx Object (compiled PL kernel) |
| `.xsa` | Xilinx Support Archive (hardware specification) |
| `.xclbin` | XRT loadable binary (bitstream + metadata) |
| `.a` | Static library (AIE graph: `libadf.a`) |
| `.exe` | ARM executable (host application) |
| `.cfg` | Configuration file (AIE compiler, HLS, system link, package) |

### Common v++ Options

| Option | Description |
|--------|-------------|
| `-c` | Compile mode (generate `.xo`) |
| `-l` | Link mode (integrate PL + AIE) |
| `-p` | Package mode (create SD card image) |
| `-t hw` | Target hardware |
| `-t hw_emu` | Target hardware emulation |
| `-t sw_emu` | Target software emulation |
| `--platform` | Specify platform (`.xpfm`) |
| `--config` | Specify configuration file |
| `--save-temps` | Keep intermediate files |
| `-k` | Kernel name |
| `-o` | Output file |

### Useful Commands

```bash
# Check device status
xbutil examine

# Validate xclbin
xclbinutil --info --input design.xclbin

# Profile application
xrtutil schedule --enable-profile

# Monitor temperature/power
xbutil examine -r platform

# Check AIE status
xbutil top -d 0
```

---

## APPENDIX A — CMake Build System (Alternative)

If using CMake (Vitis IDE generates CMake projects):

### Build AIE Component

```bash
cd aie_component
cmake -S . -B build -DVITIS_TARGET=hw -DVITIS_PLATFORM_PATH=${PLATFORM}
cmake --build build
```

### Build System Project

```bash
cd system_project
cmake -S . -B build -DVITIS_TARGET=hw -DVITIS_PLATFORM_PATH=${PLATFORM}
cmake --build build
cmake --build build --target package
```

### CMake Variables

| Variable | Description |
|----------|-------------|
| `VITIS_TARGET` | `sw_emu`, `hw_emu`, or `hw` |
| `VITIS_PLATFORM_PATH` | Full path to `.xpfm` file |
| `WORKSPACE_DIR` | Workspace root directory |

---

## APPENDIX B — Common Issues and Solutions

### Issue: "Platform not found"

**Solution:**
```bash
export PLATFORM_REPO_PATHS=/tools/Xilinx/Vitis/2024.1/base_platforms
```

### Issue: "XRT library not found during host compilation"

**Solution:**
```bash
# Verify sysroot path
ls ${SYSROOT}/usr/lib/libxrt_coreutil.so

# If missing, reinstall Common Image
```

### Issue: "AIE graph fails at runtime"

**Checklist:**
1. Verify graph was compiled with `--target=hw`
2. Check `--package.defer_aie_run` flag used during packaging
3. Ensure host calls `graph.reset()` and `graph.run()`
4. Enable AIE debug: `enable_aie_debug=true` in `package.cfg`

### Issue: "SD card boots but no device found"

**Solution:**
```bash
# Check XRT installation
lsmod | grep zocl

# If not loaded
sudo modprobe zocl

# Check device tree
ls /sys/bus/platform/devices/ | grep zynqmp
```

---

## APPENDIX C — Resource Links

- **Vitis Unified Software Platform Documentation:**  
  https://docs.amd.com/r/en-US/ug1416-vitis-documentation

- **Versal AI Engine Documentation:**  
  https://docs.amd.com/r/en-US/am009-versal-ai-engine

- **XRT Documentation:**  
  https://xilinx.github.io/XRT/

- **VCK190 Evaluation Board User Guide:**  
  https://docs.amd.com/r/en-US/ug1366-vck190-eval-bd

- **Vitis Application Acceleration Development Flow:**  
  https://docs.amd.com/r/en-US/ug1393-vitis-application-acceleration

---

## SUMMARY CHECKLIST

- [ ] Install Vitis, Vivado, XRT
- [ ] Download and install Common Image
- [ ] Source tool environments
- [ ] Compile AIE graph (`aiecompiler`)
- [ ] Compile PL kernels (`v++ -c`)
- [ ] Link system (`v++ -l`)
- [ ] Cross-compile host application (ARM)
- [ ] Package SD card image (`v++ -p`)
- [ ] Prepare SD card (partition + copy files)
- [ ] Set VCK190 boot switches to SD mode
- [ ] Boot VCK190 and login
- [ ] Verify XRT device (`xbutil examine`)
- [ ] Run application (`./host.exe design.xclbin`)
- [ ] Verify results

---

**Document Version:** 1.0  
**Last Updated:** February 11, 2026  
**Platform:** VCK190 Versal AI Core Series  
**Tools:** Vitis 2024.1, XRT 2.17.0
