#!/bin/bash
# ============================================================================
# Hardware Emulation Run Script
# Launches QEMU + RTL simulator for system-level emulation
# ============================================================================

set -e  # Exit on error

echo "=========================================="
echo " Launching HW Emulation"
echo "=========================================="

# ============================================================================
# Environment Setup
# ============================================================================

# Source Vitis
if [ -z "$XILINX_VITIS" ]; then
    echo "Sourcing Vitis environment..."
    source /tools/Xilinx/Vitis/2024.1/settings64.sh
fi

# Paths
WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
EMU_DIR="${SYSTEM_DIR}/build/hw_emu/package"
AIE_DIR="${WORKSPACE_DIR}/5k_impl"

# Find launch script (v++ creates it in different locations)
LAUNCH_SCRIPT=""
if [ -f "${EMU_DIR}/launch_hw_emu.sh" ]; then
    LAUNCH_SCRIPT="${EMU_DIR}/launch_hw_emu.sh"
elif [ -f "${EMU_DIR}/sim/behav_waveform/xsim/launch_hw_emu.sh" ]; then
    LAUNCH_SCRIPT="${EMU_DIR}/sim/behav_waveform/xsim/launch_hw_emu.sh"
elif [ -f "${SYSTEM_DIR}/build/hw_emu/launch_hw_emu.sh" ]; then
    LAUNCH_SCRIPT="${SYSTEM_DIR}/build/hw_emu/launch_hw_emu.sh"
else
    echo "ERROR: launch_hw_emu.sh not found!"
    echo "Search locations:"
    echo "  - ${EMU_DIR}/launch_hw_emu.sh"
    echo "  - ${EMU_DIR}/sim/behav_waveform/xsim/launch_hw_emu.sh"
    echo ""
    echo "Run package_hwemu.sh first!"
    exit 1
fi

echo "Launch script: $LAUNCH_SCRIPT"
echo ""
echo "NOTE: xrt.ini is packaged and will be available at /mnt/xrt.ini in QEMU"
echo ""

# AIE simulation options (if needed)
AIE_SIM_OPTIONS=""
if [ -d "${AIE_DIR}/aiesimulator_output" ] && [ -f "${AIE_DIR}/aiesimulator_output/aiesim_options.txt" ]; then
    AIE_SIM_OPTIONS="-aie-sim-options ${AIE_DIR}/aiesimulator_output/aiesim_options.txt"
    echo "Using AIE simulation options from: ${AIE_DIR}/aiesimulator_output/aiesim_options.txt"
fi

# ============================================================================
# Display Instructions
# ============================================================================
echo ""
echo "=========================================="
echo " HARDWARE EMULATION INSTRUCTIONS"
echo "=========================================="
echo ""
echo "Emulation will boot QEMU (ARM Linux) + Vivado Simulator (PL/AIE)"
echo ""
echo "IMPORTANT STEPS:"
echo ""
echo "1. Wait for QEMU to boot (shows 'PetaLinux' login prompt)"
echo ""
echo "2. Login credentials:"
echo "   Username: petalinux"
echo "   Password: (none - just press Enter)"
echo ""
echo "3. In QEMU shell, run these commands:"
echo ""
echo "   mount /dev/mmcblk0p1 /mnt"
echo "   cd /mnt"
echo "   export LD_LIBRARY_PATH=/mnt:/tmp:\$LD_LIBRARY_PATH"
echo "   export XCL_EMULATION_MODE=hw_emu"
echo "   export XILINX_XRT=/usr"
echo ""
echo "4. Run the application:"
echo ""
echo "   ./host.exe binary_container_1.xclbin"
echo ""
echo "5. Check output:"
echo ""
echo "   Look for: S2MM group_id(0): 0  (should be 0, not 65535!)"
echo ""
echo "6. Exit emulation:"
echo ""
echo "   Press: Ctrl+A then X"
echo ""
echo "=========================================="
echo ""
echo "Press Enter to launch emulation..."
read

# ============================================================================
# Launch Emulation
# ============================================================================
cd ${EMU_DIR}

echo ""
echo "Launching emulation..."
echo ""

# Launch with debug and AIE options if available
# Note: Ignore Unicode errors in log parsing (known Vitis issue)
${LAUNCH_SCRIPT} ${AIE_SIM_OPTIONS} || true

EMU_STATUS=$?

# ============================================================================
# Post-run
# ============================================================================
echo ""
echo "=========================================="

if [ $EMU_STATUS -eq 0 ]; then
    echo " Emulation completed"
    echo "=========================================="
    echo ""
    echo "Check results:"
    echo "  - xrt.run_summary (open with: vitis_analyzer xrt.run_summary)"
    echo "  - Console output above"
    echo ""
    
    # Look for run summary
    if [ -f "${EMU_DIR}/xrt.run_summary" ]; then
        echo "Run summary found: ${EMU_DIR}/xrt.run_summary"
        echo ""
        echo "To analyze:"
        echo "  vitis_analyzer ${EMU_DIR}/xrt.run_summary"
        echo ""
    fi
else
    echo " Emulation exited with code: $EMU_STATUS"
    echo "=========================================="
    echo ""
fi
