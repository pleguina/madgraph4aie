#!/bin/bash
# ============================================================================
# Master Build Script
# Complete end-to-end build for VCK190 deployment
# ============================================================================

set -e  # Exit on error

echo ""
echo "=========================================================================="
echo " 80-Pipeline Packet-Switched Architecture - Complete Build"
echo " Target: VCK190 Versal AI Core"
echo "=========================================================================="
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${SCRIPT_DIR}

# ============================================================================
# Build Options
# ============================================================================
DO_LINK=true
DO_HOST=true
DO_PACKAGE=true

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --link-only)
            DO_HOST=false
            DO_PACKAGE=false
            shift
            ;;
        --host-only)
            DO_LINK=false
            DO_PACKAGE=false
            shift
            ;;
        --package-only)
            DO_LINK=false
            DO_HOST=false
            shift
            ;;
        --no-link)
            DO_LINK=false
            shift
            ;;
        --no-host)
            DO_HOST=false
            shift
            ;;
        --no-package)
            DO_PACKAGE=false
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--link-only|--host-only|--package-only|--no-link|--no-host|--no-package]"
            exit 1
            ;;
    esac
done

# ============================================================================
# Display Build Plan
# ============================================================================
echo "Build Plan:"
echo "  System Link:    $([ "$DO_LINK" = true ] && echo "YES" || echo "SKIP")"
echo "  Host App:       $([ "$DO_HOST" = true ] && echo "YES" || echo "SKIP")"
echo "  SD Package:     $([ "$DO_PACKAGE" = true ] && echo "YES" || echo "SKIP")"
echo ""

START_TIME=$(date +%s)

# ============================================================================
# Step 1: System Link (PL + AIE)
# ============================================================================
if [ "$DO_LINK" = true ]; then
    echo ""
    echo "=========================================================================="
    echo " STEP 1/3: System Linking (PL + AIE)"
    echo "=========================================================================="
    echo ""
    
    ./build_link.sh
    
    if [ $? -ne 0 ]; then
        echo "ERROR: System linking failed!"
        exit 1
    fi
fi

# ============================================================================
# Step 2: Build Host Application
# ============================================================================
if [ "$DO_HOST" = true ]; then
    echo ""
    echo "=========================================================================="
    echo " STEP 2/3: Building Host Application"
    echo "=========================================================================="
    echo ""
    
    ./build_host.sh
    
    if [ $? -ne 0 ]; then
        echo "ERROR: Host build failed!"
        exit 1
    fi
fi

# ============================================================================
# Step 3: Package SD Card
# ============================================================================
if [ "$DO_PACKAGE" = true ]; then
    echo ""
    echo "=========================================================================="
    echo " STEP 3/3: Packaging SD Card Image"
    echo "=========================================================================="
    echo ""
    
    ./package_sd.sh
    
    if [ $? -ne 0 ]; then
        echo "ERROR: SD card packaging failed!"
        exit 1
    fi
fi

# ============================================================================
# Build Complete
# ============================================================================
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))
ELAPSED_MIN=$((ELAPSED / 60))
ELAPSED_SEC=$((ELAPSED % 60))

echo ""
echo "=========================================================================="
echo " ✓ BUILD COMPLETE!"
echo "=========================================================================="
echo ""
echo "Total build time: ${ELAPSED_MIN}m ${ELAPSED_SEC}s"
echo ""

if [ "$DO_PACKAGE" = true ]; then
    echo "Deployment files ready in: deploy_*/"
    echo ""
    echo "Next steps:"
    echo "  1. Prepare SD card (see package_sd.sh output for instructions)"
    echo "  2. Boot VCK190 from SD card"
    echo "  3. Run: ./host.exe binary_container_1.xclbin ./data"
    echo ""
fi

echo "=========================================================================="
