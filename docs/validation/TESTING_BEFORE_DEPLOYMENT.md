# Testing System Before VCK190 Deployment

## Why Test in Hardware Emulation First?

Hardware emulation (hw_emu) provides:
- **Cycle-accurate simulation** of AIE + PL + PS integration
- **Full visibility** with waveforms, profiling, and debug capabilities
- **Faster iteration** than full hardware compile cycles
- **Early bug detection** before committing to hardware run
- **Validation** of timing, bandwidth, and system behavior

## Prerequisites

Your system is ready for emulation testing:
- ✅ AIE graph compiled: `/home/pelayo/work/vitis_workspace/5k_impl/build/hw/libadf.a`
- ✅ PL kernels: MM2S and S2MM .xo files compiled
- ✅ Host app: Cross-compiled for ARM
- ✅ System linked: `binary_container_1.xsa` created

⚠️ **But you built for `hw` target, not `hw_emu`!**

## Problem: Need to Rebuild for Emulation

Your current build used `--target hw` which generates hardware bitstream. For emulation testing, you need `--target hw_emu`.

### Option 1: Build Emulation Target (Recommended for Testing)

```bash
cd /home/pelayo/work/vitis_workspace/system_project

# Create emulation build directory
mkdir -p build/hw_emu

# Build for hardware emulation
v++ -l \
    --target hw_emu \
    --platform /tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/xilinx_vck190_base_202410_1.xpfm \
    --config hw_link/system_link.cfg \
    --output build/hw_emu/binary_container_1.xsa \
    /home/pelayo/work/vitis_workspace/MM2S_pkt_gen/build/hw/mm2s_pkt_gen.xo \
    /home/pelayo/work/vitis_workspace/S2MM_pkt_gen/build/hw/s2mm_pkt_parser.xo \
    /home/pelayo/work/vitis_workspace/5k_impl/build/hw/libadf.a
```

**Build time**: ~30-60 minutes (much faster than hw target)

### Option 2: Skip Emulation, Test Directly on Hardware

If you're confident in your design, you can skip emulation and go straight to hardware testing. However, debugging on hardware is **much harder** than in emulation.

## If You Choose Emulation Testing

### Step 1: Package for Hardware Emulation

```bash
cd /home/pelayo/work/vitis_workspace/system_project

# Package for hw_emu
v++ -p \
    --target hw_emu \
    --platform /tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/xilinx_vck190_base_202410_1.xpfm \
    --package.out_dir build/hw_emu/package \
    --package.boot_mode sd \
    --package.kernel_image /opt/xilinx/platform/xilinx-versal-common-v2024.1/Image \
    --package.rootfs /opt/xilinx/platform/xilinx-versal-common-v2024.1/rootfs.ext4 \
    --package.ps_elf build/hw/host.exe,a72-0 \
    --package.defer_aie_run \
    build/hw_emu/binary_container_1.xsa \
    /home/pelayo/work/vitis_workspace/5k_impl/build/hw/libadf.a
```

This creates `launch_hw_emu.sh` script in the package directory.

### Step 2: Create xrt.ini for Profiling

Create `/home/pelayo/work/vitis_workspace/system_project/build/hw_emu/package/xrt.ini`:

```ini
[Runtime]
verbosity = 6
runtime_log = console

[Emulation]
debug_mode = batch
# or debug_mode = gui  # for waveform viewer

[Debug]
profile = true
timeline_trace = true
device_trace = fine
aie_profile = true
aie_trace = true

# Enable detailed logging
xtlm_aximm_log = true
xtlm_axis_log = true
```

### Step 3: Prepare Test Data

Make sure you have input data files:

```bash
mkdir -p /home/pelayo/work/vitis_workspace/system_project/build/hw_emu/package/data

# Generate or copy PSP input files (80 bytes each, 10 PSPs per trunk)
# psp_in_0.bin through psp_in_7.bin
```

### Step 4: Launch Hardware Emulation

```bash
cd /home/pelayo/work/vitis_workspace/system_project/build/hw_emu/package

# Launch QEMU + hardware simulator
./launch_hw_emu.sh
```

**Wait for Linux boot** (can take 5-10 minutes)

### Step 5: Run in QEMU Environment

Once you see the PetaLinux prompt:

```bash
# Mount SD card
mount /dev/mmcblk0p1 /mnt
cd /mnt

# Set environment
export LD_LIBRARY_PATH=/mnt:/tmp:$LD_LIBRARY_PATH
export XCL_EMULATION_MODE=hw_emu
export XILINX_XRT=/usr
export XILINX_VITIS=/mnt

# Run your application
./host.exe binary_container_1.xclbin ./data
```

### Step 6: Collect Results

After execution completes:

```bash
# Results are in /mnt/xrt.run_summary

# Exit QEMU (Ctrl+A, then X)
# Press: Ctrl+A
# Then press: X
```

Copy results from QEMU to host:

```bash
# From another terminal on your host machine
scp -P 1440 root@localhost:/mnt/xrt.run_summary ./
scp -P 1440 root@localhost:/mnt/*.csv ./
```

### Step 7: Analyze Results

```bash
# View comprehensive reports
vitis_analyzer xrt.run_summary
```

This opens:
- **Summary**: Overall execution statistics
- **System Diagram**: Visual representation of your design  
- **Profile Summary**: Kernel execution times, data transfers
- **Timeline Trace**: Detailed execution timeline
- **Waveform View**: Signal-level debugging (if enabled)

## What to Look For in Emulation

### ✅ Success Indicators

1. **Application completes without errors**
2. **All 80 PSPs processed** (8 trunks × 10 PSPs)
3. **Results match expected values** (if you have golden reference)
4. **No deadlocks or hangs**
5. **Memory transfers complete successfully**
6. **AIE graph executes all iterations**

### ⚠️ Issues to Debug

1. **Deadlocks**: Check FIFO depths, stream connections
2. **Incorrect results**: Verify data flow, check kernel logic
3. **Performance issues**: Analyze timeline, look for stalls
4. **Memory errors**: Check DDR access patterns
5. **AIE stalls**: Examine AIE trace, check data dependencies

## Common Emulation Debug Techniques

### Enable Detailed XRT Logging

In `xrt.ini`:
```ini
[Runtime]
hal_log = xrt_hal.log
verbosity = 7
```

### View DDR Memory Content

```bash
# From QEMU terminal
hexdump /tmp/qemu-memory-_ddr@0x00000000 -s 0x0 -n 4096 -v -e '1/4 "%08X" "\n"'
```

### View AIE AXI Transactions

Set environment before running emulation:
```bash
export ENABLE_AIE_DBG_TRACE=1
```

Check logs in `package/sim/behav_waveform/xsim/aie_log/`

### Waveform Debugging

If you used `-g` option during v++ link:

```bash
# In xrt.ini
[Emulation]
debug_mode = gui

# Launch emulation - Vivado simulator GUI will open
./launch_hw_emu.sh -g
```

## Quick Smoke Test (No Emulation)

If you want to quickly verify the host app logic **without running full emulation**:

```bash
# On x86 host (won't actually run kernels, but tests host code structure)
cd /home/pelayo/work/vitis_workspace/system_project/build/hw

# This will fail at device open, but validates compilation
file host.exe  # Should show ARM aarch64 executable

# Can't run ARM binary on x86, but you've already verified it compiles
```

## Recommendation

Given your setup:

**For First-Time Design:**
- ✅ **Do hardware emulation** - catches 90% of issues before hardware
- Typical emulation runtime: 10-60 minutes depending on data size
- Debug time saved: Hours to days vs debugging on hardware

**If Time-Critical:**
- ⚠️ **Skip to hardware** - but be prepared for longer debug cycles
- Have JTAG/serial console ready
- Plan for multiple SD card re-flashing cycles

## Next Steps

Choose your path:

### Path A: Rigorous Testing (Recommended)
1. Rebuild system for `hw_emu` target (~30 min)
2. Package and run emulation (~15 min setup + runtime)
3. Analyze results and fix issues
4. Once clean, rebuild for `hw` and deploy to board

### Path B: Direct to Hardware
1. Use existing `hw` build
2. Flash SD card with current image
3. Boot VCK190 and test
4. Debug any issues on hardware (slower iteration)

## Summary

**Your current build is hw-targeted** which is correct for deployment but cannot be tested in emulation. The documentation recommends:

1. **Build for hw_emu first** to validate
2. **Run in QEMU + simulator** environment  
3. **Profile and debug** with full visibility
4. **Then build for hw** once validated
5. **Deploy to hardware** with confidence

The emulation flow would have caught issues early. You can either:
- Go back and build hw_emu for testing (safer)
- Proceed to hardware and debug there (faster but riskier)

Your call based on confidence level and time constraints!
