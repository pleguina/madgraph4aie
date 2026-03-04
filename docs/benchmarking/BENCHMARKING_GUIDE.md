# Complete VCK190 Benchmarking Guide
## Obtaining All Possible Metrics for AIE Inference

## Overview

Your enhanced host application already includes comprehensive metrics collection. Here's how to extract maximum performance data from the VCK190.

---

## Part 1: Board Setup and Deployment

### 1. Prepare SD Card with Test Data

```bash
# Copy test data to SD card package
cd /home/pelayo/work/vitis_workspace/system_project

# Copy test data
sudo mkdir -p build/hw/package/sd_card/sd_card/data
sudo cp /home/pelayo/work/vitis_workspace/host_app/data/psp_in_*.bin \
        build/hw/package/sd_card/sd_card/data/

# Re-package if needed
sudo ./package_sd.sh
```

### 2. Flash SD Card

```bash
# Find SD card device
lsblk

# Flash image (replace /dev/sdX with your SD card)
sudo dd if=build/hw/package/sd_card/sd_card.img \
        of=/dev/sdX bs=4M status=progress conv=fsync

# Or copy files manually to existing partition
sudo mount /dev/sdX1 /mnt
sudo cp build/hw/package/sd_card/sd_card/* /mnt/
sudo cp -r /home/pelayo/work/vitis_workspace/host_app/data /mnt/
sudo umount /mnt
sync
```

### 3. VCK190 Hardware Setup

1. **Insert SD card**
2. **Set boot mode**: SW1 = ON-OFF-OFF-OFF (SD boot)
3. **Connect serial console**: 115200 baud
   ```bash
   sudo picocom -b 115200 /dev/ttyUSB0
   # or
   sudo screen /dev/ttyUSB0 115200
   ```
4. **Connect power measurement** (if available):
   - Use board power rails or external power meter
   - Monitor 12V input current/voltage
5. **Power on**

---

## Part 2: On-Board Benchmarking Scenarios

### Scenario 1: Single Run - Functional Verification

**Purpose**: Verify system works correctly

```bash
# On VCK190 after boot
mount /dev/mmcblk0p1 /mnt
cd /mnt

export XILINX_XRT=/usr
export LD_LIBRARY_PATH=/mnt:/tmp:$LD_LIBRARY_PATH

# Basic run
./host.exe binary_container_1.xclbin ./data
```

**Metrics Obtained**:
- ✓ Setup time (XCLBIN load, buffer allocation)
- ✓ Single iteration execution time
- ✓ Per-PSP latency
- ✓ Throughput (PSP/s)
- ✓ Data transfer times (H2D, D2H)
- ✓ Functional correctness (results generated)

---

### Scenario 2: Performance Characterization

**Purpose**: Measure sustained performance

```bash
# 100 iterations for stable averaging
./host.exe binary_container_1.xclbin ./data 10 100

# 1000 iterations for statistical confidence
./host.exe binary_container_1.xclbin ./data 10 1000
```

**Metrics Obtained**:
- ✓ Average iteration time
- ✓ Min/Max iteration time (jitter analysis)
- ✓ Standard deviation (consistency)
- ✓ Sustained throughput
- ✓ Warmup effects (first iteration vs steady-state)

---

### Scenario 3: Throughput Maximization

**Purpose**: Find peak throughput

```bash
# Continuous run for 30 seconds (estimate ~10,000+ iterations)
./host.exe binary_container_1.xclbin ./data 10 0 &
PID=$!

# Let it run
sleep 30

# Stop gracefully (Ctrl+C or kill)
kill -INT $PID

# Check results
cat /mnt/*.csv  # If CSV output enabled
```

**Metrics Obtained**:
- ✓ Peak sustained throughput
- ✓ Long-term stability
- ✓ Thermal throttling behavior (if any)

---

### Scenario 4: Power Measurement

**Purpose**: Measure power consumption for energy efficiency

#### Method A: Using Enhanced Host App (Software Side)

```bash
# Run in power measurement mode (continuous, updates every 5s)
./host.exe binary_container_1.xclbin ./data 10 0 --power-mode
```

**While running**, measure power externally with:
- Power meter on 12V input
- Board management controller (if available)
- Oscilloscope on power rails

**Metrics Obtained**:
- ✓ Steady-state power consumption
- ✓ Energy per PSP (nJ/PSP)
- ✓ Energy per iteration (J/iter)
- ✓ Power efficiency (PSP/s/W)

#### Method B: XRT Power Monitoring

```bash
# Create xrt.ini on SD card
cat > /mnt/xrt.ini << EOF
[Runtime]
verbosity = 3
power_profile = true

[Debug]
power_profile_interval_ms = 100
EOF

# Run with power profiling
./host.exe binary_container_1.xclbin ./data 10 1000
```

---

### Scenario 5: Detailed CSV Metrics Export

**Purpose**: Export all metrics for offline analysis

```bash
# Run with CSV output
./host.exe binary_container_1.xclbin ./data 10 1000 --csv results.csv

# Check CSV
cat results.csv
```

**CSV Contains**:
- Setup metrics (XCLBIN load, buffer allocation, data load, H2D transfer)
- Execution metrics (iterations, total PSPs, times)
- Per-iteration timing data
- Performance metrics (throughput, latency, bandwidth)
- Statistical analysis (avg, min, max, stddev)

**Copy to host for analysis**:
```bash
# From VCK190
# Insert USB drive
mount /dev/sda1 /media
cp results.csv /media/
umount /media

# Or over network (if configured)
scp results.csv user@host:/path/to/analysis/
```

---

### Scenario 6: Varying Load Sizes

**Purpose**: Characterize scaling behavior

```bash
# Test different PSP counts
for N in 1 2 3 5 8 10; do
    echo "Testing $N PSPs per trunk..."
    ./host.exe binary_container_1.xclbin ./data $N 100 --csv results_${N}psp.csv --quiet
done

# Analyze scaling
ls -lh results_*.csv
```

**Metrics Obtained**:
- ✓ Throughput vs load size
- ✓ Latency vs load size
- ✓ Efficiency at different scales
- ✓ Overhead characterization

---

### Scenario 7: XRT Profiling and Tracing

**Purpose**: Deep dive into kernel and data transfer behavior

#### Create xrt.ini for profiling

```bash
cat > /mnt/xrt.ini << EOF
[Runtime]
runtime_log = console
verbosity = 6

[Debug]
profile = true
timeline_trace = true
device_trace = fine
data_transfer_trace = fine

# Specific for AIE
aie_profile = true
aie_trace = true
aie_profile_interval_us = 1000

# Detailed kernel profiling
stall_trace = true
app_debug = true
EOF

# Run with profiling enabled
./host.exe binary_container_1.xclbin ./data 10 100

# Results in xrt.run_summary
```

**Copy run_summary to host**:
```bash
# On VCK190
mount /dev/sda1 /media
cp /mnt/xrt.run_summary /media/
umount /media

# On host machine
vitis_analyzer xrt.run_summary
```

**Vitis Analyzer Shows**:
- ✓ System diagram
- ✓ Profile summary (kernel execution times)
- ✓ Timeline trace (visual execution timeline)
- ✓ AIE graph activity
- ✓ Data transfer details
- ✓ Memory bandwidth utilization
- ✓ Stall analysis
- ✓ Performance bottlenecks

---

## Part 3: Metrics Collection Matrix

### Host Application Metrics (Built-in)

| Metric | Unit | How to Get |
|--------|------|------------|
| XCLBIN Load Time | ms | Automatic (console output) |
| Buffer Allocation Time | ms | Automatic |
| Data Load Time | ms | Automatic |
| H2D Transfer Time | ms | Automatic |
| Iteration Time (avg) | ms | Run with multiple iterations |
| Iteration Time (min/max) | ms | Run with multiple iterations |
| Std Deviation | ms | Run with multiple iterations |
| D2H Transfer Time | ms | Automatic |
| Throughput | PSP/s | Automatic |
| Latency per PSP | µs | Automatic |
| Input Bandwidth | MB/s | CSV export or console |
| Output Bandwidth | MB/s | CSV export or console |
| Total Bandwidth | MB/s | CSV export or console |

### Power Metrics (External Measurement)

| Metric | Unit | How to Get |
|--------|------|------------|
| Board Power (Idle) | W | Measure before running app |
| Board Power (Active) | W | Measure during --power-mode |
| Power Consumption (Δ) | W | Active - Idle |
| Energy per PSP | nJ | (Power × Time) / Total_PSPs |
| Energy per Iteration | J | Power × Avg_Iteration_Time |
| Power Efficiency | PSP/s/W | Throughput / Power |

### XRT/Hardware Metrics (xrt.run_summary)

| Metric | Source | Analysis Tool |
|--------|--------|---------------|
| Kernel Execution Time | Timeline Trace | Vitis Analyzer |
| Kernel Utilization | Profile Summary | Vitis Analyzer |
| AIE Compute Time | AIE Trace | Vitis Analyzer |
| AIE Stalls | AIE Profile | Vitis Analyzer |
| Memory Read/Write | Device Trace | Vitis Analyzer |
| DDR Bandwidth | Memory Trace | Vitis Analyzer |
| Stream Transactions | Timeline | Vitis Analyzer |
| DMA Transfer Rates | Device Trace | Vitis Analyzer |

---

## Part 4: Complete Testing Script

### Create On-Board Test Script

```bash
# On VCK190, create comprehensive_benchmark.sh
cat > /mnt/comprehensive_benchmark.sh << 'EOFSCRIPT'
#!/bin/sh
# Comprehensive VCK190 Benchmark Script

echo "==========================================="
echo " VCK190 Comprehensive Benchmark"
echo "==========================================="

export XILINX_XRT=/usr
export LD_LIBRARY_PATH=/mnt:/tmp:$LD_LIBRARY_PATH
cd /mnt

# Test 1: Functional verification
echo ""
echo "[Test 1/6] Functional Verification (10 PSPs, 1 iteration)"
./host.exe binary_container_1.xclbin ./data 10 1 --csv test1_functional.csv

# Test 2: Performance characterization
echo ""
echo "[Test 2/6] Performance Characterization (10 PSPs, 1000 iterations)"
./host.exe binary_container_1.xclbin ./data 10 1000 --csv test2_performance.csv

# Test 3: Scaling analysis
echo ""
echo "[Test 3/6] Scaling Analysis"
for N in 1 2 5 8 10; do
    echo "  - Testing $N PSPs per trunk..."
    ./host.exe binary_container_1.xclbin ./data $N 100 --csv test3_scale_${N}psp.csv --quiet
done

# Test 4: Minimal overhead
echo ""
echo "[Test 4/6] Minimal Overhead (1 PSP, 100 iterations)"
./host.exe binary_container_1.xclbin ./data 1 100 --csv test4_overhead.csv

# Test 5: Maximum throughput
echo ""
echo "[Test 5/6] Maximum Throughput (10 PSPs, 10000 iterations)"
./host.exe binary_container_1.xclbin ./data 10 10000 --csv test5_throughput.csv

# Test 6: Power measurement (60 seconds continuous)
echo ""
echo "[Test 6/6] Power Measurement Mode (60 seconds)"
echo "*** MEASURE POWER NOW ***"
timeout 60 ./host.exe binary_container_1.xclbin ./data 10 0 --power-mode > test6_power.log 2>&1 || true

echo ""
echo "==========================================="
echo " Benchmark Complete!"
echo "==========================================="
echo "Results:"
ls -lh test*.csv test*.log

echo ""
echo "Copy results to USB:"
echo "  mount /dev/sda1 /media"
echo "  cp test*.csv test*.log /media/"
echo "  umount /media"
EOFSCRIPT

chmod +x /mnt/comprehensive_benchmark.sh
```

### Run Complete Benchmark

```bash
# On VCK190
./comprehensive_benchmark.sh
```

---

## Part 5: Advanced Metrics

### AIE-Specific Metrics (If Access Available)

```bash
# Check AIE status (requires root)
cat /sys/class/amdaie/amdaie.0.auto/status

# AIE statistics
cat /sys/class/amdaie/amdaie.0.auto/statistics

# Monitor continuously
watch -n 1 'cat /sys/class/amdaie/amdaie.0.auto/statistics'
```

### System Resource Monitoring

```bash
# CPU usage during run
top -b -n 1 > system_usage.log &
./host.exe binary_container_1.xclbin ./data 10 1000
cat system_usage.log

# Memory usage
free -h

# Temperature
sensors  # If available

# Clock frequencies
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq
```

---

## Part 6: Data Analysis on Host Machine

### Process CSV Results

```python
#!/usr/bin/env python3
"""Analyze benchmark results"""
import pandas as pd
import matplotlib.pyplot as plt

# Load all CSV files
csvs = {
    'functional': 'test1_functional.csv',
    'performance': 'test2_performance.csv',
    'throughput': 'test5_throughput.csv'
}

for name, file in csvs.items():
    df = pd.read_csv(file)
    print(f"\n{name.upper()}:")
    print(df[df['unit'] == 'PSP/s'])  # Throughput
    print(df[df['unit'] == 'us'])     # Latency
    print(df[df['unit'] == 'MB/s'])   # Bandwidth

# Plot scaling
scale_files = [f'test3_scale_{n}psp.csv' for n in [1,2,5,8,10]]
throughputs = []
for f in scale_files:
    df = pd.read_csv(f)
    tp = df[(df['metric'] == 'throughput') & (df['unit'] == 'PSP/s')]['value'].values[0]
    throughputs.append(tp)

plt.plot([1,2,5,8,10], throughputs, 'o-')
plt.xlabel('PSPs per Trunk')
plt.ylabel('Throughput (PSP/s)')
plt.title('Scaling Performance')
plt.grid(True)
plt.savefig('scaling_analysis.png')
```

---

## Part 7: Metrics Summary Template

Create a comprehensive report:

```markdown
# VCK190 Benchmark Report

## Configuration
- Platform: VCK190
- Design: 80-pipeline packet-switched (8 trunks × 10 pipelines)
- Process: g g → t t̄ g
- Test Date: [DATE]

## Performance Metrics

### Throughput
- Peak: _____ PSP/s
- Sustained (1000 iter): _____ PSP/s
- Theoretical Max: _____ PSP/s

### Latency
- Average: _____ µs/PSP
- Min: _____ µs/PSP
- Max: _____ µs/PSP
- Jitter (stddev): _____ µs

### Bandwidth
- Input: _____ MB/s
- Output: _____ MB/s
- Total: _____ MB/s
- DDR Utilization: _____ %

### Power & Energy
- Active Power: _____ W
- Idle Power: _____ W
- Power Consumption: _____ W
- Energy/PSP: _____ nJ
- Power Efficiency: _____ PSP/s/W

### Timing Breakdown
- XCLBIN Load: _____ ms
- Buffer Allocation: _____ ms
- Data Load: _____ ms
- H2D Transfer: _____ ms
- Compute: _____ ms/iteration
- D2H Transfer: _____ ms

### Scaling
- 1 PSP: _____ PSP/s
- 2 PSPs: _____ PSP/s
- 5 PSPs: _____ PSP/s
- 8 PSPs: _____ PSP/s
- 10 PSPs: _____ PSP/s

## Comparison
- CPU Performance: _____ PSP/s (x86 test)
- FPGA Speedup: _____x
- GPU Equivalent: _____ PSP/s (if known)
```

---

## Quick Reference Commands

### On VCK190

```bash
# Basic benchmark
./host.exe binary_container_1.xclbin ./data 10 1000

# With CSV export
./host.exe binary_container_1.xclbin ./data 10 1000 --csv results.csv

# Power measurement mode
./host.exe binary_container_1.xclbin ./data 10 0 --power-mode

# Quiet mode (minimal output)
./host.exe binary_container_1.xclbin ./data 10 1000 --csv results.csv --quiet

# Scaling test
for N in 1 2 5 8 10; do
    ./host.exe binary_container_1.xclbin ./data $N 100 --csv scale_${N}.csv --quiet
done

# With XRT profiling
export XRT_INI_PATH=/mnt/xrt.ini
./host.exe binary_container_1.xclbin ./data 10 100
```

### Copy Results to Host

```bash
# Method 1: USB drive
mount /dev/sda1 /media
cp *.csv *.log xrt.run_summary /media/
umount /media

# Method 2: SCP (if network configured)
scp *.csv *.log xrt.run_summary user@host:/path/

# Method 3: Serial transfer (slow but always works)
cat results.csv  # Copy from terminal
```

---

## Success Criteria

✅ **Functional**: Application runs without errors  
✅ **Performance**: Throughput meets design target (estimate ~10,000+ PSP/s)  
✅ **Stability**: <5% variation across iterations  
✅ **Power**: Energy efficiency within expectations  
✅ **Scaling**: Linear scaling up to 10 PSPs  
✅ **Reproducibility**: Consistent results across runs  

---

## Troubleshooting

### If Throughput is Low
- Check AIE utilization in xrt.run_summary
- Look for stalls in Timeline Trace
- Verify clock frequencies
- Check DDR bandwidth saturation

### If High Variance
- Check thermal throttling
- Look for background processes (top)
- Verify consistent power supply
- Check for system interrupts

### If Power Seems High
- Measure idle power separately
- Check if cooling fans running
- Verify all unused peripherals disabled
- Compare with board specifications

---

**Ready to benchmark? Follow Parts 1-2 on the VCK190!**
