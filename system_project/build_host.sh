#!/bin/bash
# ============================================================================
# Host Application Build Script
# Cross-compiles host.cpp for VCK190 ARM cores with XRT
# ============================================================================

set -e  # Exit on error
set -o pipefail  # Catch errors in pipes

# Trap signals to handle Ctrl+C and other interrupts
trap 'echo ""; echo "❌ ERROR: Build interrupted by user (Ctrl+C)"; exit 130' INT
trap 'echo ""; echo "❌ ERROR: Build terminated"; exit 143' TERM

echo "=========================================="
echo " Building Host Application for VCK190 ARM"
echo "=========================================="

# ============================================================================
# Environment Setup
# ============================================================================

# Common Image paths
COMMON_IMAGE_DIR="/opt/xilinx/platform/xilinx-versal-common-v2024.1"
SDK_INSTALLER="${COMMON_IMAGE_DIR}/sdk.sh"
SYSROOT_INSTALL_DIR="/opt/xilinx/sysroot"
SYSROOT="${SYSROOT_INSTALL_DIR}/sysroots/cortexa72-cortexa53-xilinx-linux"

# Check if SDK installer exists but sysroot not installed
if [ ! -d "$SYSROOT" ]; then
    if [ -f "$SDK_INSTALLER" ]; then
        echo "Sysroot not installed. Installing now..."
        echo "This will install the cross-compilation toolchain to $SYSROOT_INSTALL_DIR"
        echo ""
        
        # Create directory if needed
        sudo mkdir -p "$SYSROOT_INSTALL_DIR"
        
        # Run SDK installer
        sudo "$SDK_INSTALLER" -d "$SYSROOT_INSTALL_DIR" -y
        
        if [ $? -ne 0 ]; then
            echo "ERROR: SDK installation failed"
            exit 1
        fi
        
        echo "SDK installed successfully"
        echo ""
    else
        echo "ERROR: Common Image SDK not found at $SDK_INSTALLER"
        echo "Please download the Common Image from:"
        echo "https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-platforms.html"
        exit 1
    fi
fi

# Verify sysroot now exists
if [ ! -d "$SYSROOT" ]; then
    echo "ERROR: Sysroot still not found at $SYSROOT"
    echo "Searched locations:"
    find /opt/xilinx -name "sysroots" 2>/dev/null
    exit 1
fi

echo "Common Image: $COMMON_IMAGE_DIR"
echo "Sysroot:      $SYSROOT"

# Source cross-compilation environment
echo "Setting up cross-compilation environment..."
if [ -f "${SYSROOT_INSTALL_DIR}/environment-setup-cortexa72-cortexa53-xilinx-linux" ]; then
    source ${SYSROOT_INSTALL_DIR}/environment-setup-cortexa72-cortexa53-xilinx-linux
else
    echo "ERROR: Environment setup script not found"
    echo "Expected: ${SYSROOT_INSTALL_DIR}/environment-setup-cortexa72-cortexa53-xilinx-linux"
    exit 1
fi

# Verify cross-compiler
if [ -z "$CXX" ]; then
    echo "ERROR: Cross-compiler not set. Environment setup failed."
    exit 1
fi

echo "Cross-compiler: $CXX"
$CXX --version | head -1

# ============================================================================
# Paths
# ============================================================================
WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
HOST_SRC_DIR="${WORKSPACE_DIR}/host_app/src"
BUILD_DIR="${WORKSPACE_DIR}/host_app/build"
OUTPUT_DIR="${WORKSPACE_DIR}/system_project/build/hw"

# Create build directory
mkdir -p ${BUILD_DIR}
mkdir -p ${OUTPUT_DIR}

# Source files
HOST_SRC="${HOST_SRC_DIR}/host.cpp"
RAMBO_SRC="${HOST_SRC_DIR}/rambo.cpp"
PSP_GEN_SRC="${HOST_SRC_DIR}/psp_generator.cpp"
HOST_EXE="${BUILD_DIR}/host.exe"

# Verify source exists
if [ ! -f "$HOST_SRC" ]; then
    echo "ERROR: Host source not found: $HOST_SRC"
    exit 1
fi

if [ ! -f "$RAMBO_SRC" ]; then
    echo "ERROR: RAMBO source not found: $RAMBO_SRC"
    exit 1
fi

if [ ! -f "$PSP_GEN_SRC" ]; then
    echo "ERROR: PSP generator source not found: $PSP_GEN_SRC"
    exit 1
fi

echo "Source files:"
echo "  - $HOST_SRC"
echo "  - $RAMBO_SRC"
echo "  - $PSP_GEN_SRC"
echo "Output:       $HOST_EXE"
echo ""

# ============================================================================
# Compile Host Application
# ============================================================================
echo "=========================================="
echo " Compiling Host Application..."
echo "=========================================="
echo ""

${CXX} ${HOST_SRC} ${RAMBO_SRC} ${PSP_GEN_SRC} \
    -o ${HOST_EXE} \
    --sysroot=${SYSROOT} \
    -I${HOST_SRC_DIR} \
    -I${SYSROOT}/usr/include/xrt \
    -I${SYSROOT}/usr/include \
    -L${SYSROOT}/usr/lib \
    -lxrt_coreutil \
    -luuid \
    -std=c++17 \
    -O3 \
    -Wall \
    -Wno-unused-label \
    -pthread

COMPILE_STATUS=$?

echo ""
echo "=========================================="

# Check both exit status AND output file existence
if [ $COMPILE_STATUS -eq 0 ] && [ -f "${HOST_EXE}" ]; then
    echo " ✓ Host Application Build SUCCESSFUL"
    echo "=========================================="
    echo ""
    
    # Verify it's an ARM binary
    file ${HOST_EXE}
    
    echo ""
    echo "Binary size:"
    ls -lh ${HOST_EXE}
    
    # Copy to system project output directory
    echo ""
    echo "Copying to system project..."
    cp ${HOST_EXE} ${OUTPUT_DIR}/
    
    echo "✓ Host executable ready: ${OUTPUT_DIR}/host.exe"
    echo ""
    echo "Next steps:"
    echo "  1. Run package_sd.sh to create SD card image"
elif [ $COMPILE_STATUS -ne 0 ]; then
    echo " ✗ Host Application Build FAILED (Exit Code: $COMPILE_STATUS)"
    echo "=========================================="
    echo ""
    echo "Compilation failed. Check compiler errors above."
    echo ""
    exit $COMPILE_STATUS
else
    echo " ✗ Host Application Build FAILED (Output file not generated)"
    echo "=========================================="
    echo ""
    echo "Compiler completed but host.exe not found at: ${HOST_EXE}"
    echo ""
    exit 1
fi

echo ""
echo "=========================================="
echo " Build Complete!"
echo "=========================================="
