#!/bin/bash
# ============================================================================
# Serial Console Diagnostic Script
# Tests serial console connections for VCK190
# ============================================================================

echo "=========================================="
echo " VCK190 Serial Console Diagnostic"
echo "=========================================="
echo ""

# Check USB devices
echo "1. Checking USB devices:"
lsusb | grep -i 'FTDI\|Silicon\|Prolific\|UART\|Serial' || echo "   No serial devices found via lsusb"
echo ""

# Check tty devices
echo "2. Available serial devices:"
ls -l /dev/ttyUSB* 2>/dev/null || echo "   No /dev/ttyUSB* devices found"
ls -l /dev/ttyACM* 2>/dev/null || echo "   No /dev/ttyACM* devices found"
echo ""

# Check permissions
echo "3. Checking permissions:"
for dev in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2; do
    if [ -e "$dev" ]; then
        ls -l $dev
        if groups | grep -q dialout; then
            echo "   ✓ User in dialout group"
        else
            echo "   ⚠ User NOT in dialout group"
            echo "     Fix: sudo usermod -a -G dialout $USER (then logout/login)"
        fi
    fi
done
echo ""

# Check if devices are in use
echo "4. Checking if devices are in use:"
for dev in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2; do
    if [ -e "$dev" ]; then
        fuser $dev 2>&1 | grep -q $dev && echo "   ⚠ $dev is in use" || echo "   ✓ $dev is free"
    fi
done
echo ""

# Test each device
echo "5. Testing serial devices (5 seconds each):"
echo "   Press Ctrl+C if you see output or to skip..."
echo ""

for dev in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2; do
    if [ -e "$dev" ]; then
        echo "   Testing $dev..."
        timeout 5s cat $dev 2>/dev/null && echo "   Data received from $dev!" || echo "   No data from $dev"
    fi
done
echo ""

# Check dmesg for USB serial info
echo "6. Recent USB serial kernel messages:"
dmesg | grep -i 'ttyUSB\|ttyACM\|FTDI\|ch341\|cp210x' | tail -10 || echo "   No relevant messages"
echo ""

# VCK190 specific info
echo "=========================================="
echo " VCK190 Specific Information"
echo "=========================================="
echo ""
echo "The VCK190 typically presents as:"
echo "  - FTDI Quad RS232-HS device"
echo "  - Usually creates 4 serial ports (ttyUSB0-3)"
echo ""
echo "Port mapping:"
echo "  /dev/ttyUSB0 - UART0 (PS UART, main console) ⬅ USE THIS"
echo "  /dev/ttyUSB1 - JTAG UART"
echo "  /dev/ttyUSB2 - UART1"  
echo "  /dev/ttyUSB3 - UART2"
echo ""
echo "Settings: 115200 baud, 8N1, no flow control"
echo ""

# Connection test
echo "=========================================="
echo " Connection Test"
echo "=========================================="
echo ""
echo "To test connection (will monitor for 10 seconds):"
echo "  1. Make sure VCK190 is powered ON"
echo "  2. Press any key to continue..."
read -n 1 -s
echo ""

if [ -e "/dev/ttyUSB0" ]; then
    echo "Monitoring /dev/ttyUSB0 for 10 seconds..."
    echo "(If board is booting, you should see output)"
    echo "(Press Ctrl+C to stop early)"
    echo ""
    timeout 10s cat /dev/ttyUSB0 2>/dev/null || echo "No output received"
else
    echo "❌ /dev/ttyUSB0 not found!"
    echo ""
    echo "Troubleshooting steps:"
    echo "  1. Check USB cable is connected (both ends)"
    echo "  2. Try a different USB port on your computer"
    echo "  3. Check if VCK190 power is ON (LED indicators)"
    echo "  4. Run: lsusb | grep FTDI"
    echo "  5. Try: sudo modprobe ftdi_sio"
fi

echo ""
echo "=========================================="
echo " Manual Connection Commands"
echo "=========================================="
echo ""
echo "Try these commands to connect:"
echo ""
echo "Option 1: picocom (recommended)"
echo "  sudo picocom -b 115200 /dev/ttyUSB0"
echo "  (Exit: Ctrl+A, Ctrl+X)"
echo ""
echo "Option 2: screen"
echo "  screen /dev/ttyUSB0 115200"
echo "  (Exit: Ctrl+A, K, Y)"
echo ""
echo "Option 3: minicom"
echo "  sudo minicom -D /dev/ttyUSB0 -b 115200"
echo "  (Exit: Ctrl+A, Q)"
echo ""
echo "Option 4: cu"
echo "  sudo cu -l /dev/ttyUSB0 -s 115200"
echo "  (Exit: ~.)"
echo ""

echo "=========================================="
