#!/bin/bash
# ============================================================================
# update_ssh.sh - Remote VCK190 Update Script
#
# Usage:
#   ./update_ssh.sh --host          # host.exe only         (fast, no reboot)
#   ./update_ssh.sh --xclbin        # xclbin only           (fast, no reboot)
#   ./update_ssh.sh --full          # BOOT.BIN+xclbin+host  (reboots board)
#   ./update_ssh.sh --host --xclbin # host + xclbin         (no reboot)
# ============================================================================

set -e

UPDATE_HOST=false
UPDATE_XCLBIN=false
UPDATE_BOOT=false

if [[ $# -eq 0 ]]; then
    echo "Usage: $0 [--host] [--xclbin] [--full]"
    echo ""
    echo "  --host    Update host.exe only (no reboot needed)"
    echo "  --xclbin  Update binary_container_1.xclbin only (no reboot needed)"
    echo "  --full    Update BOOT.BIN + xclbin + host.exe (board will reboot)"
    exit 1
fi

for arg in "$@"; do
    case $arg in
        --host)   UPDATE_HOST=true ;;
        --xclbin) UPDATE_XCLBIN=true ;;
        --full)   UPDATE_HOST=true; UPDATE_XCLBIN=true; UPDATE_BOOT=true ;;
        *) echo "Unknown argument: $arg"; exit 1 ;;
    esac
done

echo "=========================================="
echo " VCK190 SSH Update"
echo -n " Updating:"
$UPDATE_HOST   && echo -n " host.exe"
$UPDATE_XCLBIN && echo -n " xclbin"
$UPDATE_BOOT   && echo -n " BOOT.BIN"
echo ""
$UPDATE_BOOT && echo " WARNING: Board will reboot"
echo "=========================================="
echo ""

# ============================================================================
# Configuration
# ============================================================================

WORKSPACE_DIR="/home/pelayo/work/vitis_workspace"
SYSTEM_DIR="${WORKSPACE_DIR}/system_project"
HOST_EXE="${SYSTEM_DIR}/build/hw/host.exe"

SD_CARD_DIR="${SYSTEM_DIR}/build/hw/package/sd_card/sd_card/sd_card"
XCLBIN="${SD_CARD_DIR}/binary_container_1.xclbin"
BOOT_BIN="${SD_CARD_DIR}/BOOT.BIN"
BOOT_SCR="${SD_CARD_DIR}/boot.scr"

REMOTE_USER="petalinux"
REMOTE_HOST="156.35.89.171"
REMOTE_DIR="/run/media/BOOT-mmcblk0p1"
REMOTE_TARGET="${REMOTE_USER}@${REMOTE_HOST}"

# ============================================================================
# Check Local Files
# ============================================================================

if $UPDATE_HOST && [ ! -f "$HOST_EXE" ]; then
    echo "ERROR: host.exe not found: $HOST_EXE"
    echo "Build it first: cd ${SYSTEM_DIR} && ./build_host.sh"
    exit 1
fi
$UPDATE_HOST && echo "OK host.exe   $(ls -lh $HOST_EXE | awk '{print $5}')  $(stat -c %y $HOST_EXE | cut -d'.' -f1)"

if $UPDATE_XCLBIN && [ ! -f "$XCLBIN" ]; then
    echo "ERROR: xclbin not found: $XCLBIN"
    echo "Re-run build_link + package_sd first."
    exit 1
fi
$UPDATE_XCLBIN && echo "OK xclbin     $(ls -lh $XCLBIN | awk '{print $5}')  $(stat -c %y $XCLBIN | cut -d'.' -f1)"

if $UPDATE_BOOT && [ ! -f "$BOOT_BIN" ]; then
    echo "ERROR: BOOT.BIN not found: $BOOT_BIN"
    echo "Re-run build_link + package_sd first."
    exit 1
fi
$UPDATE_BOOT && echo "OK BOOT.BIN   $(ls -lh $BOOT_BIN | awk '{print $5}')  $(stat -c %y $BOOT_BIN | cut -d'.' -f1)"
echo ""

# ============================================================================
# Check SSH Connection
# ============================================================================

echo "Target: ${REMOTE_TARGET}:${REMOTE_DIR}"
echo ""
echo "Testing SSH connection..."
if ! ssh -o ConnectTimeout=5 -o BatchMode=yes "${REMOTE_TARGET}" "echo 'SSH OK'" 2>/dev/null; then
    echo "WARNING: SSH requires password (passwordless SSH not configured)"
    echo "TIP: run once: ssh-copy-id ${REMOTE_TARGET}"
    echo ""
fi

read -p "Continue? (y/n): " CONFIRM
if [ "$CONFIRM" != "y" ] && [ "$CONFIRM" != "Y" ]; then
    echo "Aborted."
    exit 0
fi
echo ""

# ============================================================================
# Transfer Files
# ============================================================================

FILES_TO_SEND=()
$UPDATE_HOST   && FILES_TO_SEND+=("${HOST_EXE}")
$UPDATE_XCLBIN && FILES_TO_SEND+=("${XCLBIN}")
$UPDATE_BOOT   && FILES_TO_SEND+=("${BOOT_BIN}" "${BOOT_SCR}")

echo "Transferring ${#FILES_TO_SEND[@]} file(s) to /tmp on ${REMOTE_HOST}..."
scp "${FILES_TO_SEND[@]}" "${REMOTE_TARGET}:/tmp/" || {
    echo "ERROR: Transfer failed"
    exit 1
}
echo "Transfer complete. Installing to ${REMOTE_DIR}..."
echo ""

# ============================================================================
# Remote Install
# ============================================================================

REMOTE_CMDS=""
TS="\$(date +%Y%m%d_%H%M%S)"

if $UPDATE_BOOT; then
    REMOTE_CMDS+="
sudo cp ${REMOTE_DIR}/BOOT.BIN ${REMOTE_DIR}/BOOT.BIN.bak.${TS} 2>/dev/null || true
sudo mv /tmp/BOOT.BIN ${REMOTE_DIR}/BOOT.BIN
sudo mv /tmp/boot.scr ${REMOTE_DIR}/boot.scr
sudo ls -lh ${REMOTE_DIR}/BOOT.BIN && echo 'INSTALLED: BOOT.BIN'
"
fi

if $UPDATE_XCLBIN; then
    REMOTE_CMDS+="
sudo cp ${REMOTE_DIR}/binary_container_1.xclbin ${REMOTE_DIR}/binary_container_1.xclbin.bak.${TS} 2>/dev/null || true
sudo mv /tmp/binary_container_1.xclbin ${REMOTE_DIR}/binary_container_1.xclbin
sudo ls -lh ${REMOTE_DIR}/binary_container_1.xclbin && echo 'INSTALLED: xclbin'
"
fi

if $UPDATE_HOST; then
    REMOTE_CMDS+="
sudo cp ${REMOTE_DIR}/host.exe ${REMOTE_DIR}/host.exe.bak.${TS} 2>/dev/null || true
sudo mv /tmp/host.exe ${REMOTE_DIR}/host.exe
sudo chmod +x ${REMOTE_DIR}/host.exe
sudo ls -lh ${REMOTE_DIR}/host.exe && echo 'INSTALLED: host.exe'
"
fi

if $UPDATE_BOOT; then
    REMOTE_CMDS+="
echo ''
echo 'Rebooting board...'
sudo reboot
"
fi

ssh -t "${REMOTE_TARGET}" "$REMOTE_CMDS" || $UPDATE_BOOT  # disconnect on reboot is expected

# ============================================================================
# Wait for reboot if needed
# ============================================================================

if $UPDATE_BOOT; then
    echo ""
    echo "Board rebooting. Waiting 60 seconds..."
    sleep 60
    for i in {1..12}; do
        if nc -z -w 3 "${REMOTE_HOST}" 22 2>/dev/null; then
            echo "Board is back online (SSH port open)."
            break
        fi
        echo "  Not yet (attempt $i/12)... waiting 15s"
        sleep 15
    done
fi

# ============================================================================
# Verify deployment
# ============================================================================

echo ""
echo "Verifying on remote board..."
VERIFY_CMDS=""
$UPDATE_BOOT   && VERIFY_CMDS+="sudo ls -lh ${REMOTE_DIR}/BOOT.BIN 2>/dev/null | awk '{print \"  BOOT.BIN: \" \$5 \"  \" \$6 \" \" \$7}' || echo '  ERROR: BOOT.BIN missing'"$'\n'
$UPDATE_XCLBIN && VERIFY_CMDS+="sudo ls -lh ${REMOTE_DIR}/binary_container_1.xclbin 2>/dev/null | awk '{print \"  xclbin:   \" \$5 \"  \" \$6 \" \" \$7}' || echo '  ERROR: xclbin missing'"$'\n'
$UPDATE_HOST   && VERIFY_CMDS+="sudo ls -lh ${REMOTE_DIR}/host.exe 2>/dev/null | awk '{print \"  host.exe: \" \$5 \"  \" \$6 \" \" \$7}' || echo '  ERROR: host.exe missing'"$'\n'

ssh -t "${REMOTE_TARGET}" "$VERIFY_CMDS"

echo ""
echo "=========================================="
echo " Update Complete"
echo "=========================================="
echo ""
echo "Usage:"
echo "  ./update_ssh.sh --host          # host.exe only"
echo "  ./update_ssh.sh --xclbin        # xclbin only"
echo "  ./update_ssh.sh --host --xclbin # host + xclbin"
echo "  ./update_ssh.sh --full          # BOOT.BIN + xclbin + host (reboots)"
echo ""
echo "Run on board:"
echo "  ssh -t ${REMOTE_TARGET} 'cd ${REMOTE_DIR} && sudo ./host.exe binary_container_1.xclbin'"
echo ""

read -p "Run host.exe on remote board now? (y/n): " RUN_NOW
if [ "$RUN_NOW" = "y" ] || [ "$RUN_NOW" = "Y" ]; then
    echo ""
    ssh -t "${REMOTE_TARGET}" "cd ${REMOTE_DIR} && sudo ./host.exe binary_container_1.xclbin"
fi

echo "Done!"
