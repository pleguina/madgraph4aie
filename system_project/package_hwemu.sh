#!/bin/bash
# ============================================================================
# HW Emulation Package Script
# Creates emulation environment with QEMU + simulators
# ============================================================================

set -e  # Exit on error
set -o pipefail  # Catch errors in pipes

# Trap signals to handle Ctrl+C and other interrupts
trap 'echo ""; echo "❌ ERROR: Packaging interrupted by user (Ctrl+C)"; exit 130' INT
trap 'echo ""; echo "❌ ERROR: Packaging terminated"; exit 143' TERM

echo "=========================================="
echo " Packaging HW Emulation Environment"
echo "=========================================="

# ============================================================================
# Environment Setup
# ============================================================================

# Source Vitis
if [ -z "$XILINX_VITIS" ]; then
    echo "Sourcing Vitis environment..."
    source /tools/Xilinx/Vitis/2024.1/settings64.sh
fi

# Common Image paths
COMMON_IMAGE_DIR="/opt/xilinx/platform/xilinx-versal-common-v2024.1"
IMAGE="${COMMON_IMAGE_DIR}/Image"
ROOTFS="${COMMON_IMAGE_DIR}/rootfs.ext4"

# Verify Common Image
if [ ! -f "$IMAGE" ]; then
    echo "ERROR: Kernel image not found: $IMAGE"
    exit 1
fi

if [ ! -f "$ROOTFS" ]; then
    echo "ERROR: Root filesystem not found: $ROOTFS"
    exit 1
fi

echo "Kernel Image: $IMAGE"
echo "Root FS:      $ROOTFS"

# ============================================================================
# Paths
# ============================================================================
WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
BUILD_DIR="${SYSTEM_DIR}/build/hw_emu"
PACKAGE_DIR="${BUILD_DIR}/package"

# Platform
PLATFORM="/tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/xilinx_vck190_base_202410_1.xpfm"
PLATFORM_SW="/tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/sw/xilinx_vck190_base_202410_1"

# Linux boot files
SYSTEM_DTB="${PLATFORM_SW}/boot/system.dtb"
BL31_ELF="${PLATFORM_SW}/boot/bl31.elf"
UBOOT_ELF="${PLATFORM_SW}/boot/u-boot.elf"
BOOT_SCR="${PLATFORM_SW}/xrt/image/boot.scr"

# Input files
XSA_FILE="${BUILD_DIR}/binary_container_1.xsa"
HOST_EXE="${WORKSPACE_DIR}/host_app/build/host.exe"
AIE_LIBADF="${WORKSPACE_DIR}/5k_impl/build/hw/libadf.a"
DATA_DIR="${WORKSPACE_DIR}/5k_impl/data"

# Output directory
EMU_DIR="${PACKAGE_DIR}"

# ============================================================================
# Verify Input Files
# ============================================================================
echo ""
echo "Verifying input files..."

if [ ! -f "$XSA_FILE" ]; then
    echo "ERROR: XSA file not found: $XSA_FILE"
    echo "Run build_link_hwemu.sh first!"
    exit 1
fi

if [ ! -f "$HOST_EXE" ]; then
    echo "ERROR: Host executable not found: $HOST_EXE"
    echo "Run build_host.sh first!"
    exit 1
fi

if [ ! -f "$AIE_LIBADF" ]; then
    echo "ERROR: AIE library not found: $AIE_LIBADF"
    exit 1
fi

# Verify Linux boot files
if [ ! -f "$SYSTEM_DTB" ]; then
    echo "ERROR: Device tree not found: $SYSTEM_DTB"
    exit 1
fi

if [ ! -f "$BL31_ELF" ]; then
    echo "ERROR: BL31 bootloader not found: $BL31_ELF"
    exit 1
fi

if [ ! -f "$UBOOT_ELF" ]; then
    echo "ERROR: U-Boot not found: $UBOOT_ELF"
    exit 1
fi

if [ ! -f "$BOOT_SCR" ]; then
    echo "ERROR: Boot script not found: $BOOT_SCR"
    exit 1
fi

echo "✓ XSA file:   $XSA_FILE"
echo "✓ Host app:   $HOST_EXE"
echo "✓ AIE graph:  $AIE_LIBADF"
echo "✓ Device tree: $SYSTEM_DTB"
echo "✓ BL31:       $BL31_ELF"
echo "✓ U-Boot:     $UBOOT_ELF"
echo "✓ Boot script: $BOOT_SCR"
if [ -d "$DATA_DIR" ]; then
    echo "✓ Data dir:   $DATA_DIR"
fi
echo ""

# ============================================================================
# Create Package Directory and xrt.ini
# ============================================================================
mkdir -p ${PACKAGE_DIR}
cd ${PACKAGE_DIR}

# Create xrt.ini for emulation (must be packaged to /mnt in QEMU)
XRT_INI="${PACKAGE_DIR}/xrt.ini"
cat > ${XRT_INI} <<EOF
[Emulation]
debug_mode=batch

[Runtime]
runtime_log=console
verbosity=7

[Debug]
profile=true
timeline_trace=true
device_trace=coarse
EOF

echo "Created xrt.ini: ${XRT_INI}"
echo ""

# ============================================================================
# Run v++ Package for HW Emulation
# ============================================================================
echo "=========================================="
echo " Running v++ Package for hw_emu"
echo "=========================================="
echo ""

v++ -p \
    --target hw_emu \
    --platform ${PLATFORM} \
    --package.defer_aie_run \
    --package.enable_aie_debug \
    --package.out_dir ${EMU_DIR} \
    --package.rootfs ${ROOTFS} \
    --package.kernel_image ${IMAGE} \
    --package.boot_mode=sd \
    --package.image_format=ext4 \
    --package.sd_file ${HOST_EXE} \
    --package.sd_file ${XRT_INI} \
    --package.sd_dir ${DATA_DIR} \
    ${XSA_FILE} \
    ${AIE_LIBADF}

PKG_STATUS=$?

# ============================================================================
# Check Results
# ============================================================================
echo ""
echo "=========================================="

if [ $PKG_STATUS -eq 0 ]; then
    echo " ✓ HW Emulation Packaging SUCCESSFUL"
    echo "=========================================="
    echo ""
    echo "Emulation directory: ${EMU_DIR}"
    echo ""
    echo "Key files created:"
    echo "  - launch_hw_emu.sh"
    echo "  - BOOT.BIN"
    echo "  - binary_container_1.xclbin"
    echo "  - host.exe"
    echo ""
    ls -lh ${EMU_DIR}/launch_hw_emu.sh 2>/dev/null || echo "  Note: launch_hw_emu.sh location may vary"
    echo ""
    echo "Next step:"
    echo "  ./run_hwemu.sh"
    echo ""
elif [ $PKG_STATUS -ne 0 ]; then
    echo " ✗ HW Emulation Packaging FAILED (Exit Code: $PKG_STATUS)"
    echo "=========================================="
    echo ""
    exit $PKG_STATUS
else
    echo " ✗ HW Emulation Packaging FAILED"
    echo "=========================================="
    echo ""
    exit 1
fi
