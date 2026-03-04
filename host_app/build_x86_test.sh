#!/bin/bash
# Build x86 test version of host application

set -e

echo "=========================================="
echo " Building X86 Test Version"
echo "=========================================="

SRC_DIR="/home/pelayo/work/vitis_workspace/host_app/src"
BUILD_DIR="/home/pelayo/work/vitis_workspace/host_app/build"

mkdir -p ${BUILD_DIR}

echo "Source: ${SRC_DIR}/host_x86_test.cpp"
echo "Output: ${BUILD_DIR}/host_x86_test"

g++ -std=c++17 -O2 \
    ${SRC_DIR}/host_x86_test.cpp \
    -o ${BUILD_DIR}/host_x86_test \
    -I${SRC_DIR}

echo ""
echo "=========================================="
echo " ✓ Build Successful"
echo "=========================================="
echo ""
echo "Binary: ${BUILD_DIR}/host_x86_test"
file ${BUILD_DIR}/host_x86_test
ls -lh ${BUILD_DIR}/host_x86_test

echo ""
echo "To run:"
echo "  ${BUILD_DIR}/host_x86_test ./data"
echo "  ${BUILD_DIR}/host_x86_test ./data 10 100   # 10 PSPs, 100 iterations"
echo ""
echo "First, generate test data:"
echo "  python3 ${SRC_DIR}/generate_test_data.py"
echo ""
