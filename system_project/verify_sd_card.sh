#!/bin/bash
# ============================================================================
# SD Card Verification Script
# Checks all required files are present and valid for VCK190 boot
# ============================================================================

echo "=========================================="
echo " VCK190 SD Card Verification"
echo "=========================================="
echo ""

# ============================================================================
# Configuration
# ============================================================================

WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
SD_CARD_DIR="${SYSTEM_DIR}/build/hw/package/sd_card/sd_card"
BUILD_DIR="${SYSTEM_DIR}/build/hw"
DATA_DIR="${WORKSPACE_DIR}/5k_impl/data"

# ============================================================================
# Check if SD card directory exists
# ============================================================================

if [ ! -d "$SD_CARD_DIR" ]; then
    echo "❌ ERROR: SD card directory not found: $SD_CARD_DIR"
    echo "Run package_sd.sh first!"
    exit 1
fi

echo "Checking SD card directory: $SD_CARD_DIR"
echo ""

# ============================================================================
# Required Files Check
# ============================================================================

ERRORS=0
WARNINGS=0

echo "=========================================="
echo " Required Files Check"
echo "=========================================="
echo ""

# Function to check file
check_file() {
    local file=$1
    local desc=$2
    local min_size=$3
    local critical=$4
    
    if [ -f "$SD_CARD_DIR/$file" ]; then
        local size=$(stat -c%s "$SD_CARD_DIR/$file" 2>/dev/null || stat -f%z "$SD_CARD_DIR/$file" 2>/dev/null || echo 0)
        if [ $size -gt $min_size ]; then
            local size_mb=$((size / 1024 / 1024))
            local size_kb=$((size / 1024))
            if [ $size_mb -gt 0 ]; then
                echo "✓ $desc: $file (${size_mb}M)"
            elif [ $size_kb -gt 0 ]; then
                echo "✓ $desc: $file (${size_kb}K)"
            else
                echo "✓ $desc: $file ($size bytes)"
            fi
        else
            echo "⚠ $desc: $file exists but seems too small ($size bytes)"
            ((WARNINGS++))
        fi
    else
        if [ "$critical" = "critical" ]; then
            echo "❌ MISSING $desc: $file"
           ((ERRORS++))
        else
            echo "⚠ $desc: $file (optional, but recommended)"
            ((WARNINGS++))
        fi
    fi
}

# Check critical boot files
check_file "BOOT.BIN" "Boot Image" 1000000 "critical"
check_file "Image" "Linux Kernel" 10000000 "critical"
check_file "boot.scr" "Boot Script" 1000 "critical"

# Check xclbin (either name)
if [ -f "$SD_CARD_DIR/binary_container_1.xclbin" ]; then
    check_file "binary_container_1.xclbin" "FPGA Binary" 10000000 "critical"
elif [ -f "$SD_CARD_DIR/a.xclbin" ]; then
    echo "⚠ FPGA Binary: a.xclbin (should be binary_container_1.xclbin)"
    echo "  → Will be renamed by prepare_sd_card.sh"
    ((WARNINGS++))
else
    echo "❌ MISSING FPGA Binary: No xclbin file found!"
    ((ERRORS++))
fi

# Check host application
check_file "host.exe" "Host Application" 10000 "critical"

# Check optional but important files
check_file "platform_desc.txt" "Platform Descriptor" 10 "optional"

# Check data directory
if [ -d "$SD_CARD_DIR/data" ]; then
    DATA_FILES=$(ls -1 "$SD_CARD_DIR/data" 2>/dev/null | wc -l)
    echo "✓ Data Directory: data/ ($DATA_FILES files)"
else
    echo "⚠ Data Directory: data/ (missing - required for application)"
    ((WARNINGS++))
fi

echo ""

# ============================================================================
# File Content Verification
# ============================================================================

echo "=========================================="
echo " File Content Verification"
echo "=========================================="
echo ""

# Check BOOT.BIN signature
if [ -f "$SD_CARD_DIR/BOOT.BIN" ]; then
    # Versal BOOT.BIN should start with specific magic numbers
    MAGIC=$(xxd -p -l 4 "$SD_CARD_DIR/BOOT.BIN" 2>/dev/null)
    if [ ! -z "$MAGIC" ]; then
        echo "✓ BOOT.BIN magic: 0x$MAGIC"
    else
        echo "⚠ Cannot read BOOT.BIN magic number"
        ((WARNINGS++))
    fi
fi

# Check boot.scr is U-Boot script
if [ -f "$SD_CARD_DIR/boot.scr" ]; then
    HEAD=$(xxd -p -l 4 "$SD_CARD_DIR/boot.scr" 2>/dev/null)
    if [ "$HEAD" = "27051956" ]; then
        echo "✓ boot.scr is valid U-Boot script"
    else
        echo "❌ boot.scr is not a valid U-Boot script (magic: 0x$HEAD)"
        ((ERRORS++))
    fi
fi

# Check Image is ARM64 kernel
if [ -f "$SD_CARD_DIR/Image" ]; then
    KERNEL_MAGIC=$(xxd -p -s 0x38 -l 4 "$SD_CARD_DIR/Image" 2>/dev/null)
    if [ "$KERNEL_MAGIC" = "644d5241" ]; then
        echo "✓ Image is valid ARM64 Linux kernel"
    else
        echo "⚠ Image magic number unexpected: 0x$KERNEL_MAGIC"
        ((WARNINGS++))
    fi
fi

# Check host.exe is ARM executable
if [ -f "$SD_CARD_DIR/host.exe" ]; then
    FILE_TYPE=$(file "$SD_CARD_DIR/host.exe" 2>/dev/null)
    if echo "$FILE_TYPE" | grep -q "ARM aarch64"; then
        echo "✓ host.exe is ARM64 executable"
    elif echo "$FILE_TYPE" | grep -q "ELF 64-bit"; then
        echo "✓ host.exe is 64-bit ELF executable"
    else
        echo "❌ host.exe is not an ARM executable: $FILE_TYPE"
        ((ERRORS++))
    fi
fi

echo ""

# ============================================================================
# File Size Summary
# ============================================================================

echo "=========================================="
echo " File Size Summary"
echo "=========================================="
echo ""

if [ -d "$SD_CARD_DIR" ]; then
    du -h "$SD_CARD_DIR"/* 2>/dev/null | sort -h || ls -lh "$SD_CARD_DIR"
    echo ""
    TOTAL_SIZE=$(du -sh "$SD_CARD_DIR" | cut -f1)
    echo "Total SD card contents: $TOTAL_SIZE"
    echo ""
    
    # Check if it fits on typical SD cards
    TOTAL_KB=$(du -sk "$SD_CARD_DIR" | cut -f1)
    echo "Space required: ~$(( (TOTAL_KB + 1024*1024 + 2048*1024) / 1024 ))MB (boot ~1GB + rootfs ~2GB)"
fi

echo ""

# ============================================================================
# Missing Files Remediation
# ============================================================================

if [ $ERRORS -gt 0 ] || [ $WARNINGS -gt 0 ]; then
    echo "=========================================="
    echo " Issues Found - Remediation Steps"
    echo "=========================================="
    echo ""
    
    # Missing host.exe
    if [ ! -f "$SD_CARD_DIR/host.exe" ]; then
        echo "❌ Missing host.exe:"
        if [ -f "$BUILD_DIR/host.exe" ]; then
            echo "   Found at: $BUILD_DIR/host.exe"
            echo "   Fix: sudo cp $BUILD_DIR/host.exe $SD_CARD_DIR/"
        else
            echo "   Not found in build directory!"
            echo "   Fix: Run ./build_host.sh first"
        fi
        echo ""
    fi
    
    # Wrong xclbin name
    if [ -f "$SD_CARD_DIR/a.xclbin" ] && [ ! -f "$SD_CARD_DIR/binary_container_1.xclbin" ]; then
        echo "⚠ xclbin naming:"
        echo "   Fix: sudo mv $SD_CARD_DIR/a.xclbin $SD_CARD_DIR/binary_container_1.xclbin"
        echo ""
    fi
    
    # Missing xclbin entirely
    if [ ! -f "$SD_CARD_DIR/binary_container_1.xclbin" ] && [ ! -f "$SD_CARD_DIR/a.xclbin" ]; then
        echo "❌ Missing xclbin file:"
        echo "   This is a critical file generated by v++ package"
        echo "   Fix: Rerun ./package_sd.sh"
        echo ""
    fi
    
    # Missing data directory
    if [ ! -d "$SD_CARD_DIR/data" ]; then
        echo "⚠ Missing data directory:"
        if [ -d "$DATA_DIR" ]; then
            echo "   Found at: $DATA_DIR"
            echo "   Fix: sudo cp -r $DATA_DIR $SD_CARD_DIR/"
        else
            echo "   Generate input data first"
        fi
        echo ""
    fi
    
    # Missing platform_desc.txt
    if [ ! -f "$SD_CARD_DIR/platform_desc.txt" ]; then
        echo "⚠ Missing platform_desc.txt:"
        echo "   Fix: echo 'xilinx_vck190_base_202410_1' | sudo tee $SD_CARD_DIR/platform_desc.txt"
        echo ""
    fi
    
    echo "Quick fix all (run as root or with sudo):"
    echo ""
    cat << 'EOF'
sudo bash << 'FIXSCRIPT'
cd /home/pelayo/work/vitis_workspace/system_project/build/hw/package/sd_card/sd_card

# Fix host.exe
[ -f ../../../host.exe ] && cp -v ../../../host.exe ./ && chmod +x host.exe

# Fix xclbin naming
[ -f a.xclbin ] && [ ! -f binary_container_1.xclbin ] && mv -v a.xclbin binary_container_1.xclbin

# Add platform descriptor
echo 'xilinx_vck190_base_202410_1' > platform_desc.txt

# Copy data if available
[ -d /home/pelayo/work/vitis_workspace/5k_impl/data ] && cp -rv /home/pelayo/work/vitis_workspace/5k_impl/data ./

echo "Files fixed!"
ls -lh
FIXSCRIPT
EOF
    echo ""
fi

# ============================================================================
# Serial Console Troubleshooting
# ============================================================================

echo "=========================================="
echo " Serial Console Troubleshooting"
echo "=========================================="
echo ""

echo "If /dev/ttyUSB0 shows no output:"
echo ""
echo "1. Check USB cable connection (use a data cable, not charge-only)"
echo ""
echo "2. Verify UART device:"
echo "   lsusb | grep -i 'FTDI\|Silicon\|Prolific'"
echo "   ls -l /dev/ttyUSB*"
echo ""
echo "3. Check permissions:"
echo "   sudo chmod 666 /dev/ttyUSB0"
echo "   OR add yourself to dialout group:"
echo "   sudo usermod -a -G dialout \$USER"
echo "   (then logout and login)"
echo ""
echo "4. Try different serial devices:"
echo "   /dev/ttyUSB0  (usually UART 0)"
echo "   /dev/ttyUSB1  (usually JTAG)"
echo "   /dev/ttyUSB2"
echo ""
echo "5. Test serial connection:"
echo "   sudo picocom -b 115200 /dev/ttyUSB0"
echo "   screen /dev/ttyUSB0 115200"
echo "   minicom -D /dev/ttyUSB0 -b 115200"
echo ""
echo "6. Check boot mode switches (SW1 on VCK190):"
echo "   Position: ON-OFF-OFF-OFF (for SD boot)"
echo "   [1=ON, 2=OFF, 3=OFF, 4=OFF]"
echo ""
echo "7. Power cycle the board after checking switches"
echo ""
echo "8. If still no output, check:"
echo "   - SD card is properly inserted"
echo "   - SD card has correct files (this script checks that)"
echo "   - Power LED is on"
echo "   - Try removing and reinserting SD card while powered off"
echo ""

# ============================================================================
# Summary
# ============================================================================

echo "=========================================="
echo " Verification Summary"
echo "=========================================="
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo "✅ ALL CHECKS PASSED!"
    echo ""
    echo "SD card directory is ready. Next steps:"
    echo "  1. Run: ./prepare_sd_card.sh"
    echo "  2. Or manually copy files to SD card"
    echo ""
elif [ $ERRORS -eq 0 ]; then
    echo "⚠️  $WARNINGS WARNING(S) - Should fix but not critical"
    echo ""
    echo "SD card will likely work, but recommended to fix warnings."
    echo ""
else
    echo "❌ $ERRORS ERROR(S), $WARNINGS WARNING(S)"
    echo ""
    echo "SD card is NOT ready. Fix errors above before proceeding."
    echo ""
    exit 1
fi

echo "=========================================="
