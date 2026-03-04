#!/bin/bash
# ============================================================================
# SD Card Preparation Script  
# Partitions, formats, and loads SD card with boot files
# ============================================================================

set -e  # Exit on error

echo "=========================================="
echo " VCK190 SD Card Preparation"
echo "=========================================="
echo ""

# ============================================================================
# Configuration
# ============================================================================

WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
SD_CARD_DIR="${SYSTEM_DIR}/build/hw/package/sd_card/sd_card"
COMMON_IMAGE_DIR="/opt/xilinx/platform/xilinx-versal-common-v2024.1"
ROOTFS="${COMMON_IMAGE_DIR}/rootfs.ext4"

# Check if SD card files exist
if [ ! -d "$SD_CARD_DIR" ]; then
    echo "ERROR: SD card directory not found: $SD_CARD_DIR"
    echo "Run package_sd.sh first to generate SD card files!"
    exit 1
fi

# Verify critical files exist in source
echo "Verifying source files..."
MISSING_FILES=0
for file in BOOT.BIN Image boot.scr; do
    if [ ! -f "$SD_CARD_DIR/$file" ]; then
        echo "  ✗ Missing: $file"
        MISSING_FILES=$((MISSING_FILES + 1))
    else
        echo "  ✓ $file"
    fi
done

if [ ! -d "$SD_CARD_DIR/data" ]; then
    echo "  ⚠ Warning: data/ directory not found (application may not run)"
else
    DATA_COUNT=$(ls -1 $SD_CARD_DIR/data 2>/dev/null | wc -l)
    echo "  ✓ data/ directory ($DATA_COUNT files)"
fi

if [ $MISSING_FILES -gt 0 ]; then
    echo ""
    echo "ERROR: $MISSING_FILES critical file(s) missing!"
    echo "Run: sudo ./package_sd.sh"
    exit 1
fi

echo "✓ All source files present"
echo ""

# ============================================================================
# Detect SD Card
# ============================================================================

echo "Available storage devices:"
lsblk -d -o NAME,SIZE,TYPE,MOUNTPOINT | grep -E "disk"
echo ""

read -p "Enter SD card device (e.g., sdb, sdc, mmcblk0): " DEVICE
DEVICE_PATH="/dev/${DEVICE}"

# Validate device
if [ ! -b "$DEVICE_PATH" ]; then
    echo "ERROR: Device $DEVICE_PATH does not exist!"
    exit 1
fi

# Check if device looks like an SD card (reasonable size check)
DEVICE_SIZE=$(lsblk -b -d -o SIZE -n $DEVICE_PATH)
DEVICE_SIZE_GB=$((DEVICE_SIZE / 1024 / 1024 / 1024))

echo ""
echo "Selected device: $DEVICE_PATH"
echo "Device size: ${DEVICE_SIZE_GB} GB"
echo ""

if [ $DEVICE_SIZE_GB -lt 4 ] || [ $DEVICE_SIZE_GB -gt 256 ]; then
    echo "WARNING: Device size seems unusual for an SD card!"
    read -p "Continue anyway? (yes/no): " CONFIRM
    if [ "$CONFIRM" != "yes" ]; then
        echo "Aborted."
        exit 1
    fi
fi

# Final confirmation
echo "=========================================="
echo " WARNING: ALL DATA ON $DEVICE_PATH WILL BE ERASED!"
echo "=========================================="
echo ""
read -p "Type 'YES' to continue: " FINAL_CONFIRM

if [ "$FINAL_CONFIRM" != "YES" ]; then
    echo "Aborted."
    exit 1
fi

# ============================================================================
# Determine partition naming scheme
# ============================================================================

# Different naming for mmcblk vs sd devices
if [[ $DEVICE == mmcblk* ]]; then
    PART1="${DEVICE_PATH}p1"
    PART2="${DEVICE_PATH}p2"
else
    PART1="${DEVICE_PATH}1"
    PART2="${DEVICE_PATH}2"
fi

# ============================================================================
# Unmount any existing partitions
# ============================================================================

echo ""
echo "Unmounting any existing partitions..."
sudo umount ${DEVICE_PATH}* 2>/dev/null || true
sleep 1

# ============================================================================
# Partition the SD Card
# ============================================================================

echo ""
echo "=========================================="
echo " Partitioning SD Card"
echo "=========================================="
echo ""

# Use fdisk to partition
sudo fdisk $DEVICE_PATH <<EOF
o
n
p
1

+1G
t
c
n
p
2


w
EOF

echo ""
echo "Partitioning complete. Waiting for device to settle..."
sleep 2
sudo partprobe $DEVICE_PATH 2>/dev/null || true
sleep 1

# ============================================================================
# Format Partitions
# ============================================================================

echo ""
echo "=========================================="
echo " Formatting Partitions"
echo "=========================================="
echo ""

echo "Formatting boot partition (FAT32)..."
sudo mkfs.vfat -F 32 -n BOOT $PART1

echo "Formatting root partition (ext4)..."
sudo mkfs.ext4 -L ROOT $PART2

echo "Formatting complete."

# ============================================================================
# Mount and Copy Boot Files
# ============================================================================

echo ""
echo "=========================================="
echo " Copying Boot Files"
echo "=========================================="
echo ""

# Create mount points
BOOT_MOUNT="/mnt/sd_boot_$$"
ROOT_MOUNT="/mnt/sd_root_$$"
sudo mkdir -p $BOOT_MOUNT
sudo mkdir -p $ROOT_MOUNT

# Mount boot partition
echo "Mounting boot partition..."
sudo mount $PART1 $BOOT_MOUNT

# Copy boot files (use -r to include data/ subdirectory)
echo "Copying boot files from: $SD_CARD_DIR"
echo "  - Copying all files and directories..."
sudo cp -rv $SD_CARD_DIR/* $BOOT_MOUNT/

echo ""
echo "Verifying critical files copied:"
for file in BOOT.BIN Image boot.scr binary_container_1.xclbin host.exe system.dtb; do
    if [ -f "$BOOT_MOUNT/$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ MISSING: $file"
        echo "ERROR: Critical file $file not found!"
        exit 1
    fi
done

if [ -d "$BOOT_MOUNT/data" ]; then
    FILE_COUNT=$(ls -1 $BOOT_MOUNT/data | wc -l)
    echo "  ✓ data/ directory ($FILE_COUNT files)"
else
    echo "  ⚠ WARNING: data/ directory not found"
    echo "    This may prevent the application from running!"
fi

# Check if xclbin was created with wrong name
if [ -f "$BOOT_MOUNT/a.xclbin" ] && [ ! -f "$BOOT_MOUNT/binary_container_1.xclbin" ]; then
    echo ""
    echo "Note: Renaming a.xclbin to binary_container_1.xclbin for consistency..."
    sudo mv -v $BOOT_MOUNT/a.xclbin $BOOT_MOUNT/binary_container_1.xclbin
fi

# Create platform descriptor
echo "Creating platform descriptor..."
sudo bash -c "echo 'xilinx_vck190_base_202410_1' > $BOOT_MOUNT/platform_desc.txt"

echo ""
echo "Boot partition contents:"
sudo ls -lh $BOOT_MOUNT/

# Unmount boot partition
sudo umount $BOOT_MOUNT

# ============================================================================
# Copy Root Filesystem
# ============================================================================

echo ""
echo "=========================================="
echo " Installing Root Filesystem"
echo "=========================================="
echo ""

if [ -f "$ROOTFS" ]; then
    echo "Root filesystem source: $ROOTFS"
    ROOTFS_SIZE=$(du -h $ROOTFS | cut -f1)
    echo "Size: $ROOTFS_SIZE"
    echo ""
    echo "Writing to: $PART2"
    echo "This may take several minutes..."
    echo ""
    
    # Direct dd copy of ext4 image with error checking
    if sudo dd if=$ROOTFS of=$PART2 bs=4M status=progress conv=fsync; then
        echo ""
        echo "✓ Root filesystem installed successfully."
        
        # Verify the partition has content
        echo "Verifying rootfs content..."
        sudo mount $PART2 $ROOT_MOUNT
        DIR_COUNT=$(sudo ls $ROOT_MOUNT | wc -l)
        sudo umount $ROOT_MOUNT
        
        if [ $DIR_COUNT -gt 3 ]; then
            echo "✓ Rootfs verified: $DIR_COUNT directories found"
        else
            echo "✗ ERROR: Rootfs appears empty (only $DIR_COUNT items)"
            exit 1
        fi
    else
        echo ""
        echo "✗ ERROR: Root filesystem dd copy failed!"
        exit 1
    fi
else
    echo "✗ ERROR: Root filesystem not found at: $ROOTFS"
    echo ""
    echo "Expected location: $ROOTFS"
    echo "This file is required for Linux to boot!"
    exit 1
fi

# ============================================================================
# Cleanup and Final Steps
# ============================================================================

echo ""
echo "=========================================="
echo " Finalizing SD Card"
echo "=========================================="
echo ""
echo "Syncing filesystems..."
sudo sync
sleep 1

echo "Unmounting all partitions..."
# Force unmount all partitions multiple times to handle auto-remount
for i in {1..3}; do
    sudo umount ${DEVICE_PATH}* 2>/dev/null || true
    sleep 0.5
done

# Remove mount points
sudo rmdir $BOOT_MOUNT 2>/dev/null || true
sudo rmdir $ROOT_MOUNT 2>/dev/null || true

echo "✓ All data written and unmounted"

# ============================================================================
# Summary
# ============================================================================

echo ""
echo "=========================================="
echo " SD Card Ready! ✓"
echo "=========================================="
echo ""
echo "SD Card: $DEVICE_PATH"
echo "Boot partition: $PART1 (FAT32)"
echo "Root partition: $PART2 (ext4)"
echo ""
echo "Files on boot partition:"
echo "  - BOOT.BIN          (bootloader + PDI)"
echo "  - binary_container_1.xclbin  (hardware binary)"
echo "  - Image             (Linux kernel)"
echo "  - boot.scr          (boot script)"
echo "  - host.exe          (application)"
echo "  - data/             (input data)"
echo ""
echo "=========================================="
echo " VCK190 Hardware Setup"
echo "=========================================="
echo ""
echo "1. Safely eject SD card:"
echo "   sudo eject $DEVICE_PATH"
echo ""
echo "2. Insert SD card into VCK190"
echo ""
echo "3. Set boot mode switches (SW1):"
echo "   Position: ON-OFF-OFF-OFF  (SD boot mode)"
echo ""
echo "4. Connect serial console:"
echo "   sudo picocom -b 115200 /dev/ttyUSB0"
echo "   (or use: sudo screen /dev/ttyUSB0 115200)"
echo ""
echo "5. Power on the board"
echo ""
echo "6. After Linux boots, login as 'root' (password: 'root')"
echo ""
echo "7. Run the application:"
echo "   cd /run/media/mmcblk0p1"
echo "   ./host.exe binary_container_1.xclbin ./data"
echo ""
echo "=========================================="
echo ""

# Automatically eject SD card and prevent auto-remount
echo "Ejecting SD card..."

# Try udisksctl first (prevents auto-mount better)
if command -v udisksctl &> /dev/null; then
    echo "Using udisksctl to power off device..."
    # Unmount each partition with udisksctl
    for part in ${DEVICE_PATH}*; do
        if [ -b "$part" ]; then
            udisksctl unmount -b $part 2>/dev/null || true
        fi
    done
    # Power off the entire device
    udisksctl power-off -b $DEVICE_PATH 2>/dev/null || true
fi

# Also use traditional eject as fallback
sudo eject $DEVICE_PATH 2>/dev/null || true

sleep 1

# Verify nothing is mounted
MOUNTED=$(mount | grep "$DEVICE_PATH" || true)
if [ -z "$MOUNTED" ]; then
    echo ""
    echo "✓ SD card ejected and safe to remove!"
    echo "✓ No partitions mounted - device is offline"
else
    echo ""
    echo "⚠ Warning: Some partitions may still be auto-mounted by system:"
    echo "$MOUNTED"
    echo ""
    echo "Manually unmount with: udisksctl unmount -b ${DEVICE_PATH}1"
    echo "                       udisksctl unmount -b ${DEVICE_PATH}2"
fi

echo ""
echo "Done!"
