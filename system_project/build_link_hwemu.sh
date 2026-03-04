#!/bin/bash
# ============================================================================
# System Link Script for Hardware Emulation (hw_emu)
# Links AIE graph + PL kernels → creates emulation binary
# ============================================================================

set -e  # Exit on error
set -o pipefail  # Catch errors in pipes

# Trap signals to handle Ctrl+C and other interrupts
trap 'echo ""; echo "❌ ERROR: Build interrupted by user (Ctrl+C)"; exit 130' INT
trap 'echo ""; echo "❌ ERROR: Build terminated"; exit 143' TERM

# ============================================================================
# Environment Setup
# ============================================================================
echo "=========================================="
echo " HW Emulation Link: 80-Pipeline Architecture"
echo "=========================================="

# Source Vitis
if [ -z "$XILINX_VITIS" ]; then
    echo "Sourcing Vitis environment..."
    source /tools/Xilinx/Vitis/2024.1/settings64.sh
fi

# Workspace paths
WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
AIE_DIR="${WORKSPACE_DIR}/5k_impl"
MM2S_DIR="${WORKSPACE_DIR}/MM2S_pkt_gen/MM2S_pkt_gen"
S2MM_DIR="${WORKSPACE_DIR}/S2MM_pkt_gen/S2MM_pkt_gen"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
BUILD_DIR="${SYSTEM_DIR}/build/hw_emu"

# Platform
PLATFORM="/tools/Xilinx/Vitis/2024.1/base_platforms/xilinx_vck190_base_202410_1/xilinx_vck190_base_202410_1.xpfm"

# Target
TARGET="hw_emu"

# Output
OUTPUT_NAME="binary_container_1"

# ============================================================================
# Verify Input Files
# ============================================================================
echo ""
echo "Verifying input files..."

AIE_LIBADF="${AIE_DIR}/build/hw/libadf.a"
MM2S_XO="${MM2S_DIR}/mm2s_pkt_gen.xo"
S2MM_XO="${S2MM_DIR}/s2mm_pkt_parser.xo"
LINK_CFG="${SYSTEM_DIR}/hw_link/system_link.cfg"

if [ ! -f "$AIE_LIBADF" ]; then
    echo "ERROR: AIE library not found: $AIE_LIBADF"
    exit 1
fi

if [ ! -f "$MM2S_XO" ]; then
    echo "ERROR: MM2S kernel not found: $MM2S_XO"
    exit 1
fi

if [ ! -f "$S2MM_XO" ]; then
    echo "ERROR: S2MM kernel not found: $S2MM_XO"
    exit 1
fi

if [ ! -f "$LINK_CFG" ]; then
    echo "ERROR: Link configuration not found: $LINK_CFG"
    exit 1
fi

echo "✓ AIE library:  $AIE_LIBADF"
echo "✓ MM2S kernel:  $MM2S_XO"
echo "✓ S2MM kernel:  $S2MM_XO"
echo "✓ Link config:  $LINK_CFG"

# ============================================================================
# Create Build Directory
# ============================================================================
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo ""
echo "Build directory: ${BUILD_DIR}"
echo "Output file:     ${OUTPUT_NAME}.xsa"
echo ""

# ============================================================================
# Run v++ Link for HW Emulation
# ============================================================================
echo "=========================================="
echo " Starting v++ Link for hw_emu (30-40 min)"
echo "=========================================="
echo ""

v++ -l \
    --target ${TARGET} \
    --platform ${PLATFORM} \
    --config ${LINK_CFG} \
    -g \
    --save-temps \
    --temp_dir ./temp \
    --log_dir ./logs \
    --report_dir ./reports \
    --verbose \
    --connectivity.sp s2mm_0.result_buffer:DDR \
    --connectivity.sp s2mm_1.result_buffer:DDR \
    --connectivity.sp s2mm_2.result_buffer:DDR \
    --connectivity.sp s2mm_3.result_buffer:DDR \
    --connectivity.sp s2mm_4.result_buffer:DDR \
    --connectivity.sp s2mm_5.result_buffer:DDR \
    --connectivity.sp s2mm_6.result_buffer:DDR \
    --connectivity.sp s2mm_7.result_buffer:DDR \
    -o ${OUTPUT_NAME}.xsa \
    ${MM2S_XO} \
    ${S2MM_XO} \
    ${AIE_LIBADF}

LINK_STATUS=$?

# ============================================================================
# Check Results
# ============================================================================
echo ""
echo "=========================================="

# Check both exit status AND output file existence
if [ $LINK_STATUS -eq 0 ] && [ -f "${BUILD_DIR}/${OUTPUT_NAME}.xsa" ]; then
    echo " ✓ HW Emulation Link SUCCESSFUL"
    echo "=========================================="
    echo ""
    echo "Output file: ${BUILD_DIR}/${OUTPUT_NAME}.xsa"
    echo ""
    echo "Reports available in:"
    echo "  - ${BUILD_DIR}/reports/"
    echo "  - ${BUILD_DIR}/logs/"
    echo ""
    echo "Link summary:"
    ls -lh ${BUILD_DIR}/${OUTPUT_NAME}.xsa
    echo ""
    echo "Next steps:"
    echo "  1. Check reports in reports/"
    echo "  2. Package for emulation (run package_hwemu.sh)"
    echo "  3. Launch emulation (run run_hwemu.sh)"
elif [ $LINK_STATUS -ne 0 ]; then
    echo " ✗ HW Emulation Link FAILED (Exit Code: $LINK_STATUS)"
    echo "=========================================="
    echo ""
    echo "Check logs for errors:"
    echo "  tail -100 ${BUILD_DIR}/logs/link.log"
    echo ""
    exit $LINK_STATUS
else
    echo " ✗ HW Emulation Link FAILED (Output file not generated)"
    echo "=========================================="
    echo ""
    echo "Check logs for errors:"
    echo "  tail -100 ${BUILD_DIR}/logs/link.log"
    echo ""
    exit 1
fi
