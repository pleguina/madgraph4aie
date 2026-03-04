#!/bin/bash
# ============================================================================
# Quick Host.exe Update Script for SD Card
# Updates only the host application without reformatting/repackaging
# ============================================================================

set -e

echo "=========================================="
echo " Quick SD Card Host Update"
echo "=========================================="
echo ""

# ============================================================================
# Configuration
# ============================================================================

WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
HOST_EXE="${SYSTEM_DIR}/build/hw/host.exe"

# Check if host.exe exists
if [ ! -f "$HOST_EXE" ]; then
    echo "ERROR: Host executable not found: $HOST_EXE"
    echo ""
    echo "Build it first with:"
    echo "  cd ${SYSTEM_DIR}"
    echo "  ./build_host.sh"
    exit 1
fi

echo "✓ Host executable: $HOST_EXE"
echo "  Size: $(ls -lh $HOST_EXE | awk '{print $5}')"
echo "  Modified: $(stat -c %y $HOST_EXE | cut -d'.' -f1)"
echo ""

# ============================================================================
# Detect SD Card
# ============================================================================

echo "Available storage devices:"
lsblk -d -o NAME,SIZE,TYPE,MOUNTPOINT | grep -E "disk"
echo ""

read -p "Enter SD card device (e.g., sda, sdb, mmcblk0): " DEVICE
DEVICE_PATH="/dev/${DEVICE}"

# Validate device
if [ ! -b "$DEVICE_PATH" ]; then
    echo "ERROR: Device $DEVICE_PATH does not exist!"
    exit 1
fi

# Determine partition naming scheme
if [[ $DEVICE == mmcblk* ]]; then
    BOOT_PART="${DEVICE_PATH}p1"
else
    BOOT_PART="${DEVICE_PATH}1"
fi

echo ""
echo "Target device: $DEVICE_PATH"
echo "Boot partition: $BOOT_PART"
echo ""

read -p "Update host.exe on this SD card? (y/n): " CONFIRM
if [ "$CONFIRM" != "y" ]; then
    echo "Aborted."
    exit 0
fi

# ============================================================================
# Mount and Update
# ============================================================================

MOUNT_POINT="/mnt/sd_update_$$"
sudo mkdir -p $MOUNT_POINT

echo ""
echo "Mounting boot partition..."
if ! sudo mount $BOOT_PART $MOUNT_POINT 2>/dev/null; then
    echo "ERROR: Failed to mount $BOOT_PART"
    echo "Make sure the SD card is inserted and has a valid boot partition."
    sudo rmdir $MOUNT_POINT
    exit 1
fi

echo "✓ Mounted at $MOUNT_POINT"
echo ""

# Backup old host.exe if exists
if [ -f "$MOUNT_POINT/host.exe" ]; then
    BACKUP="$MOUNT_POINT/host.exe.bak.$(date +%Y%m%d_%H%M%S)"
    echo "Backing up old host.exe..."
    sudo cp "$MOUNT_POINT/host.exe" "$BACKUP"
    echo "  Backup: $(basename $BACKUP)"
    echo ""
fi

# Copy new host.exe
echo "Copying new host.exe..."
sudo cp -v "$HOST_EXE" "$MOUNT_POINT/host.exe"
sudo chmod +x "$MOUNT_POINT/host.exe"
echo ""

# Verify files on SD card
echo "Current SD card contents:"
sudo ls -lh $MOUNT_POINT/ | grep -E "BOOT|xclbin|host|Image|boot.scr|data"
echo ""

# Unmount
echo "Syncing and unmounting..."
sudo sync
sudo umount $MOUNT_POINT
sudo rmdir $MOUNT_POINT

echo ""
echo "=========================================="
echo " ✓ Host Updated Successfully"
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Safely eject: sudo eject $DEVICE_PATH"
echo "  2. Insert SD card into VCK190"
echo "  3. Boot Linux and run:"
echo "     cd /run/media/BOOT-mmcblk0p1/"
echo "     ./host.exe binary_container_1.xclbin"
echo ""
echo "Note: Host auto-generates PSPs with RAMBO - no data files needed!"
echo "      Use --seed <N> flag for reproducible PSPs"
echo ""

# Offer to eject
read -p "Eject SD card now? (y/n): " EJECT
if [ "$EJECT" = "y" ] || [ "$EJECT" = "Y" ]; then
    # Try udisksctl first for better unmounting
    if command -v udisksctl &> /dev/null; then
        udisksctl unmount -b $BOOT_PART 2>/dev/null || true
        udisksctl power-off -b $DEVICE_PATH 2>/dev/null || true
    fi
    sudo eject $DEVICE_PATH 2>/dev/null || true
    echo "✓ SD card ejected. Safe to remove."
fi

echo ""
echo "Done!"
