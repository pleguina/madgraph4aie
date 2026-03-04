#!/bin/bash
# ============================================================================
# SD Card Package Script
# Creates bootable SD card image with BOOT.BIN, kernel, xclbin, host app
# ============================================================================

set -e  # Exit on error
set -o pipefail  # Catch errors in pipes

# Trap signals to handle Ctrl+C and other interrupts
trap 'echo ""; echo "❌ ERROR: Packaging interrupted by user (Ctrl+C)"; exit 130' INT
trap 'echo ""; echo "❌ ERROR: Packaging terminated"; exit 143' TERM

echo "=========================================="
echo " Packaging SD Card Image"
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
BUILD_DIR="${SYSTEM_DIR}/build/hw"
PACKAGE_DIR="${BUILD_DIR}/package"

# Platform
PLATFORM="/tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/xilinx_vck190_base_202410_1.xpfm"
PLATFORM_SW="/tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/sw/xilinx_vck190_base_202410_1"

# Linux boot files (REQUIRED for Linux with fixed XSA)
SYSTEM_DTB="${PLATFORM_SW}/boot/system.dtb"
BL31_ELF="${PLATFORM_SW}/boot/bl31.elf"
UBOOT_ELF="${PLATFORM_SW}/boot/u-boot.elf"
BOOT_SCR="${PLATFORM_SW}/xrt/image/boot.scr"

# Input files
XSA_FILE="${BUILD_DIR}/binary_container_1.xsa"
HOST_EXE="${BUILD_DIR}/host.exe"
AIE_LIBADF="${WORKSPACE_DIR}/5k_impl/build/hw/libadf.a"
DATA_DIR="${WORKSPACE_DIR}/5k_impl/data"

# Output
# Note: v++ creates a 'sd_card' subdirectory inside the --package.out directory
SD_CARD_DIR="${PACKAGE_DIR}/sd_card/sd_card"

# ============================================================================
# Verify Input Files
# ============================================================================
echo ""
echo "Verifying input files..."

if [ ! -f "$XSA_FILE" ]; then
    echo "ERROR: Hardware file not found: $XSA_FILE"
    echo "Run build_link.sh first!"
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

# Verify Linux boot files (CRITICAL for Versal Linux boot)
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
    echo "This is MANDATORY for Linux boot automation!"
    exit 1
fi

if [ ! -d "$DATA_DIR" ]; then
    echo "WARNING: Data directory not found: $DATA_DIR"
    echo "Application may need input data files"
fi

echo "✓ Hardware:   $XSA_FILE"
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
# Create Package Directory
# ============================================================================
mkdir -p ${PACKAGE_DIR}
cd ${PACKAGE_DIR}

# Create package configuration file
cat > package.cfg <<EOF
[package]
out_dir=./sd_card
kernel_image=${IMAGE}
rootfs=${ROOTFS}
enable_aie_debug=true
EOF

echo "Package configuration created: ${PACKAGE_DIR}/package.cfg"
echo ""

# ============================================================================
# Run v++ Package
# ============================================================================
echo "=========================================="
echo " Running v++ Package..."
echo "=========================================="
echo ""
echo "IMPORTANT: Including all required Linux boot components:"
echo "  - Device Tree (system.dtb)"
echo "  - BL31 Trusted Firmware (bl31.elf)"
echo "  - U-Boot Bootloader (u-boot.elf)"
echo "  - Boot Script (boot.scr) - MANDATORY for Linux"
echo ""

v++ -p \
    --target hw \
    --platform ${PLATFORM} \
    -o binary_container_1.xclbin \
    --package.out_dir ${SD_CARD_DIR} \
    --package.boot_mode sd \
    --package.image_format ext4 \
    --package.kernel_image ${IMAGE} \
    --package.rootfs ${ROOTFS} \
    --package.dtb ${SYSTEM_DTB} \
    --package.bl31_elf ${BL31_ELF} \
    --package.uboot ${UBOOT_ELF} \
    --package.sd_file ${BOOT_SCR} \
    --package.sd_file ${HOST_EXE} \
    --package.sd_dir ${DATA_DIR} \
    --package.defer_aie_run \
    ${XSA_FILE} \
    ${AIE_LIBADF}

PACKAGE_STATUS=$?

echo ""
echo "=========================================="

# Check both exit status AND critical output files
if [ $PACKAGE_STATUS -eq 0 ] && [ -f "${SD_CARD_DIR}/BOOT.BIN" ] && [ -f "${SD_CARD_DIR}/binary_container_1.xclbin" ]; then
    echo " ✓ SD Card Packaging SUCCESSFUL"
    echo "=========================================="
    echo ""
    echo "SD card files created in: ${SD_CARD_DIR}"
    echo ""
    echo "Contents:"
    ls -lh ${SD_CARD_DIR}/
    echo ""
    
    # Create a deployment package
    DEPLOY_DIR="${WORKSPACE_DIR}/deploy_$(date +%Y%m%d_%H%M%S)"
    mkdir -p ${DEPLOY_DIR}
    
    echo "Creating deployment package..."
    echo ""
    
    # Copy essential files
    cp -v ${SD_CARD_DIR}/BOOT.BIN ${DEPLOY_DIR}/ 2>/dev/null || echo "Note: BOOT.BIN will be in sd_card/"
    cp -v ${SD_CARD_DIR}/*.xclbin ${DEPLOY_DIR}/ 2>/dev/null || echo "Note: xclbin will be in sd_card/"
    cp -v ${SD_CARD_DIR}/Image ${DEPLOY_DIR}/ 2>/dev/null || echo "Note: Image will be in sd_card/"
    cp -v ${SD_CARD_DIR}/boot.scr ${DEPLOY_DIR}/ 2>/dev/null || true
    cp -v ${HOST_EXE} ${DEPLOY_DIR}/
    
    # Copy data files if they exist
    if [ -d "${WORKSPACE_DIR}/5k_impl/data" ]; then
        mkdir -p ${DEPLOY_DIR}/data
        cp -v ${WORKSPACE_DIR}/5k_impl/data/*.bin ${DEPLOY_DIR}/data/ 2>/dev/null || true
    fi
    
    echo ""
    echo "=========================================="
    echo " Deployment Package Ready!"
    echo "=========================================="
    echo ""
    echo "Location: ${DEPLOY_DIR}"
    echo ""
    echo "To prepare SD card (Linux):"
    echo "  1. Insert SD card and identify device (e.g., /dev/sdb)"
    echo "     lsblk"
    echo ""
    echo "  2. Format SD card (FAT32):"
    echo "     sudo mkfs.vfat -F 32 -n BOOT /dev/sdb1"
    echo ""
    echo "  3. Mount SD card:"
    echo "     sudo mount /dev/sdb1 /mnt"
    echo ""
    echo "  4. Copy files:"
    echo "     sudo cp ${SD_CARD_DIR}/* /mnt/"
    echo "     sudo umount /mnt"
    echo ""
    echo "  5. On VCK190:"
    echo "     - Insert SD card"
    echo "     - Set boot mode switches to SD (SW1: ON-OFF-OFF-OFF)"
    echo "     - Connect serial console (115200 baud)"
    echo "     - Power on"
    echo "     - Login as root"
    echo "     - Run: ./host.exe binary_container_1.xclbin ./data"
    echo ""
    
elif [ $PACKAGE_STATUS -ne 0 ]; then
    echo " ✗ SD Card Packaging FAILED (Exit Code: $PACKAGE_STATUS)"
    echo "=========================================="
    echo ""
    echo "v++ package command failed with error code $PACKAGE_STATUS"
    echo ""
    echo "Common causes:"
    echo "  - Mismatched XSA and libadf.a (different build times)"
    echo "  - Missing boot files (dtb, bl31, u-boot, boot.scr)"
    echo "  - Corrupted XSA or libadf.a files"
    echo ""
    echo "Check logs for errors:"
    echo "  tail -100 ${PACKAGE_DIR}/_x/logs/package/v++.log"
    echo ""
    exit $PACKAGE_STATUS
else
    echo " ✗ SD Card Packaging FAILED (Output files not generated)"
    echo "=========================================="
    echo ""
    echo "v++ completed but critical files missing:"
    [ ! -f "${SD_CARD_DIR}/BOOT.BIN" ] && echo "  ❌ Missing: BOOT.BIN"
    [ ! -f "${SD_CARD_DIR}/binary_container_1.xclbin" ] && echo "  ❌ Missing: binary_container_1.xclbin"
    echo ""
    echo "Check logs for errors:"
    echo "  tail -100 ${PACKAGE_DIR}/_x/logs/package/v++.log"
    echo ""
    exit 1
fi

echo "=========================================="
echo " Packaging Complete!"
echo "=========================================="
