# SD Card Quick Reference

## Issue: xclbin file naming mismatch

**Problem**: Documentation said `binary_container_1.xclbin` but build generated `a.xclbin`  
**Solution**: Fixed in `package_sd.sh` - now generates correct name  
**Script**: Use `./prepare_sd_card.sh` for automated SD card setup

---

## Quick Commands

### Build & Package
```bash
cd /home/pelayo/work/vitis_workspace/system_project

# Full build
./build_all.sh

# Or package only (if already built)
./package_sd.sh
```

### Prepare SD Card (Automated)
```bash
# Run the automated script
./prepare_sd_card.sh

# Follow prompts to:
# 1. Select SD card device (e.g., sdb, sdc)
# 2. Confirm formatting (data will be erased!)
# 3. Script handles everything else
```

### Manual SD Card Setup (if needed)
```bash
# 1. Partition
sudo fdisk /dev/sdX
# Create: 1GB FAT32 (type c) + remaining ext4

# 2. Format
sudo mkfs.vfat -F 32 -n BOOT /dev/sdX1
sudo mkfs.ext4 -L ROOT /dev/sdX2

# 3. Copy boot files
sudo mount /dev/sdX1 /mnt
sudo cp ./build/hw/package/sd_card/sd_card/* /mnt/
sudo umount /mnt

# 4. Install rootfs
sudo dd if=/opt/xilinx/platform/xilinx-versal-common-v2024.1/rootfs.ext4 \
        of=/dev/sdX2 bs=4M status=progress
```

---

## VCK190 Setup

### Hardware
1. Insert SD card
2. Set SW1: **ON-OFF-OFF-OFF** (SD boot mode)
3. Connect USB UART (115200 baud)
4. Power on

### Serial Console
```bash
# Option 1: picocom
sudo picocom -b 115200 /dev/ttyUSB0

# Option 2: screen
sudo screen /dev/ttyUSB0 115200

# Option 3: minicom
sudo minicom -D /dev/ttyUSB0 -b 115200
```

### After Boot (on VCK190)
```bash
# Login
Username: root
Password: root

# Navigate to SD card
cd /run/media/mmcblk0p1

# Verify files
ls -lh
# Should see: BOOT.BIN, binary_container_1.xclbin, Image, boot.scr, host.exe, data/

# Run application
./host.exe binary_container_1.xclbin ./data
```

---

## File Locations

### Build Artifacts
```
system_project/
├── build/hw/
│   ├── binary_container_1.xsa        # Hardware design
│   ├── host.exe                       # Host application
│   └── package/sd_card/sd_card/
│       ├── BOOT.BIN                   # Bootloader
│       ├── binary_container_1.xclbin  # FPGA binary ✓ FIXED
│       ├── Image                      # Linux kernel
│       └── boot.scr                   # Boot script
```

### SD Card Structure
```
SD Card
├── Partition 1 (FAT32, 1GB, bootable)
│   ├── BOOT.BIN
│   ├── binary_container_1.xclbin
│   ├── Image
│   ├── boot.scr
│   ├── host.exe
│   ├── platform_desc.txt
│   └── data/
└── Partition 2 (ext4, remaining space)
    └── (rootfs contents)
```

---

## Troubleshooting

### Build Issues

**"ERROR: Hardware file not found"**
```bash
# Run link first
./build_link.sh
```

**"ERROR: Host executable not found"**
```bash
# Build host application
./build_host.sh
```

**"ERROR: AIE library not found"**
```bash
# Check AIE build
cd ../5k_impl
make clean && make
```

### SD Card Issues

**"Device does not exist"**
```bash
# Check available devices
lsblk
# Use device name without partition number (sdb, not sdb1)
```

**"Permission denied"**
```bash
# Script needs sudo for disk operations
# Run as regular user - script will use sudo internally
```

**SD card won't boot**
```bash
# Verify boot mode switches: SW1 = ON-OFF-OFF-OFF
# Check serial console connection
# Reformat and copy files again
```

### Runtime Issues

**"xclbin file not found"**
```bash
# Check actual filename
ls /run/media/mmcblk0p1/*.xclbin

# Use correct name (should be binary_container_1.xclbin after fix)
./host.exe <actual_filename>.xclbin ./data
```

**"Failed to open device"**
```bash
# Check XRT installation
xbutil examine

# Verify xclbin loaded
xbutil examine -d 0
```

**"AIE graph initialization failed"**
```bash
# Check dmesg for errors
dmesg | tail -50

# Verify AIE is accessible
xbutil examine | grep -i aie
```

---

## Expected Output

### Successful Packaging
```
✓ SD Card Packaging SUCCESSFUL
SD card files created in: /home/pelayo/work/.../sd_card

Contents:
-rw-r--r-- 1 pelayo pelayo  12M BOOT.BIN
-rw-r--r-- 1 pelayo pelayo  17M binary_container_1.xclbin  ✓
-rw-r--r-- 1 pelayo pelayo  23M Image
-rw-r--r-- 1 pelayo pelayo 3.4K boot.scr
```

### Successful Boot
```
Xilinx Versal Platform Loader and Manager
...
Starting kernel...
[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd441]
...
PetaLinux 2024.1 xilinx-vck190-20241 /dev/ttyPS0

xilinx-vck190-20241 login: root
Password: root
root@xilinx-vck190-20241:~#
```

### Successful Application Run
```
cd /run/media/mmcblk0p1
root@xilinx-vck190-20241:/run/media/mmcblk0p1# ./host.exe binary_container_1.xclbin ./data

Loading XCLBIN: binary_container_1.xclbin
Programming FPGA...
Initializing AIE graph...
Processing phase space points...
✓ AIE graph started successfully
...
```

---

## Related Documentation

- **Build Instructions**: `system_building_instructions.md`
- **Deployment Guide**: `DEPLOYMENT_READY.md`
- **VCK190 Setup**: `VCK190_DEPLOYMENT_GUIDE.md`
- **Benchmarking**: `BENCHMARKING_GUIDE.md`
- **Fix Details**: `XCLBIN_NAMING_FIX.md` ⬅️ Read this for technical details

---

## Summary

✅ xclbin naming fixed: now generates `binary_container_1.xclbin`  
✅ Automated SD card script: `./prepare_sd_card.sh`  
✅ Documentation updated for consistency  
✅ Ready for deployment to VCK190  

**Last Updated**: February 13, 2026
