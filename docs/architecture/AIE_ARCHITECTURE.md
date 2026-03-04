# Hybrid Cascade Architecture: Complete Technical Specification
## gg→tt̄g Matrix Element Calculation on AMD Versal AI Engine

**Document Version**: 2.0
**Date**: January 8, 2026
**Target Platform**: AMD Versal VCK190 (VC1902 AI Core)
**Vitis Version**: 2024.1
**AIE Architecture**: AIE-1 (400 tiles)

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [AIE Hardware Architecture](#2-aie-hardware-architecture)
3. [Hybrid Cascade Design Principles](#3-hybrid-cascade-design-principles)
4. [Complete Data Flow Specification](#4-complete-data-flow-specification)
5. [Pipeline Architecture](#5-pipeline-architecture)
6. [Performance Analysis](#6-performance-analysis)
7. [Scalability and Resource Utilization](#7-scalability-and-resource-utilization)
8. [Implementation Details](#8-implementation-details)
9. [Verification and Validation](#9-verification-and-validation)
10. [Appendices](#10-appendices)

---

## 1. Executive Summary

### 1.1 Architecture Overview

This document specifies the **Hybrid Cascade Architecture** for accelerating the gg→tt̄g (gluon-gluon to top-antitop-gluon) matrix element calculation on AMD Versal AI Engine cores. The architecture achieves optimal performance by:

- **Leveraging streams** for PSP (phase-space point) broadcast with flexible routing
- **Leveraging cascades** for JAMP (color amplitude) accumulation with zero-latency transfers
- **Implementing good helicity filtering** to process 16 relevant helicities instead of 32
- **Daisy-chain cascade topology** for deterministic, deadlock-free JAMP reduction

### 1.2 Key Performance Metrics

| Metric | Value | Notes |
|--------|-------|-------|
| **Tiles per Pipeline** | 8 | FEEDER, FF, DUP1, A1, DUP2, A2, B, COLOR |
| **Batch Size** | 10 PSPs | Configurable (tested 10-32) |
| **Helicities Processed** | 16 good hels | Out of 32 total combinations |
| **Input Bandwidth** | 80 bytes/PSP | 20 floats (5×4-momentum) |
| **Output Bandwidth** | 4 bytes/PSP | 1 float (\|M\|²) |
| **Cascade Bandwidth** | 48 GB/s @ 1 GHz | 384 bits/cycle |
| **Stream Bandwidth** | 4 GB/s @ 1 GHz | 32 bits/cycle per stream |
| **Pipeline Throughput** | ~800K PSPs/sec | Per pipeline estimate |
| **VCK190 Total Throughput** | ~40M PSPs/sec | 50 pipelines |

### 1.3 Why This Architecture is Optimal

**Problem Solved**: Previous stream-only architectures experienced FIFO deadlocks when aggregating JAMP data (6,144 floats per batch >> default FIFO depth of 32-64).

**Solution**: Hybrid approach uses:
1. **Streams** where parallelism and flexibility matter (PSP broadcast: 1→4 fan-out)
2. **Cascades** where accumulation and determinism matter (JAMP reduction: 4→1 fan-in)

**Result**:
- Eliminates 240 KB of FIFO traffic per batch
- Zero cascade latency (direct register-to-register transfer)
- 12× bandwidth advantage (384 vs 32 bits/cycle)
- Deterministic synchronization (no FIFO starvation)

---

## 2. AIE Hardware Architecture

### 2.1 VCK190 (VC1902) Device Specifications

**Source**: Xilinx UG1079 - AI Engine Kernel Coding Best Practices Guide v2021.2

#### 2.1.1 AI Engine Array Layout

```
AMD Versal VC1902 AI Engine Array
═══════════════════════════════════════════════════════════════
Array Dimensions:     50 columns × 8 rows = 400 AI Engine tiles
Array Topology:       2D grid with horizontal cascades
Operating Frequency:  1.0 - 1.25 GHz (typical: 1.0 GHz)
Power Domain:         Independent per tile (~0.5-1W per tile)
═══════════════════════════════════════════════════════════════

Physical Layout:
        Column 0    Column 1  ...  Column 48   Column 49
      ┌──────────┬──────────┬───────────────┬──────────┐
Row 7 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade R→L)
      ├──────────┼──────────┼───────────────┼──────────┤
Row 6 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade L→R)
      ├──────────┼──────────┼───────────────┼──────────┤
Row 5 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade R→L)
      ├──────────┼──────────┼───────────────┼──────────┤
Row 4 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade L→R) ← Used
      ├──────────┼──────────┼───────────────┼──────────┤
Row 3 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade R→L)
      ├──────────┼──────────┼───────────────┼──────────┤
Row 2 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade L→R)
      ├──────────┼──────────┼───────────────┼──────────┤
Row 1 │ Tile     │ Tile     │      ...      │ Tile     │ (Cascade R→L)
      ├──────────┼──────────┼───────────────┼──────────┤
Row 0 │ SHIM     │ SHIM     │      ...      │ SHIM     │ (PLIO Interface)
      └──────────┴──────────┴───────────────┴──────────┘
              ▲ PLIO/GMIO connections to PL
```

**Key Constraint**: Cascades flow horizontally only, direction alternates per row:
- **Even rows (0,2,4,6)**: Cascade flows LEFT → RIGHT
- **Odd rows (1,3,5,7)**: Cascade flows RIGHT → LEFT

#### 2.1.2 Single AI Engine Tile Specifications

```
┌─────────────────────────────────────────────────────────────────┐
│                    AI Engine Tile (AIE-1)                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Scalar Processing Unit (SPU)                            │  │
│  │  • 32-bit scalar ALU                                     │  │
│  │  • 16 × 32-bit general purpose registers                 │  │
│  │  • 1-cycle latency for most operations                   │  │
│  │  • 3-cycle latency for multiplication                    │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Vector Processing Unit (VPU)                            │  │
│  │  • 512-bit wide vector ALU (SIMD)                        │  │
│  │  • 32 MACs/cycle @ int16×int16                           │  │
│  │  • 16 MACs/cycle @ int32×int16                           │  │
│  │  •  8 MACs/cycle @ float32×float32                       │  │
│  │  •  8 × 1024-bit vector registers                        │  │
│  │  • 12 × 768-bit accumulator registers                    │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Data Memory                                             │  │
│  │  • 32 KB data memory (4 banks × 8 KB)                    │  │
│  │  • 2 × 256-bit load units (128/256-bit access)           │  │
│  │  • 1 × 256-bit store unit (128/256-bit access)           │  │
│  │  • 128-bit alignment requirement                         │  │
│  │  • Dual-ported per bank                                  │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Program Memory                                          │  │
│  │  • 16 KB program memory                                  │  │
│  │  • VLIW instruction format (7-way)                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Stream Interface                                        │  │
│  │  • 2 × 32-bit AXI4-Stream INPUTS                         │  │
│  │  • 2 × 32-bit AXI4-Stream OUTPUTS                        │  │
│  │  • 32-bit access per cycle per stream                    │  │
│  │  • Connected to AXI4-Stream Switch                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Cascade Interface                                       │  │
│  │  • 1 × 384-bit CASCADE INPUT (from West/East neighbor)   │  │
│  │  • 1 × 384-bit CASCADE OUTPUT (to West/East neighbor)    │  │
│  │  • Direct register-to-register transfer                  │  │
│  │  • 1-cycle latency                                       │  │
│  │  • Zero memory overhead                                  │  │
│  │  • 4-deep, 384-bit FIFO on input (hardware buffer)       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  DMA Controllers                                         │  │
│  │  • S2MM (Stream-to-Memory): 2 channels                   │  │
│  │  • MM2S (Memory-to-Stream): 2 channels                   │  │
│  │  • Independent operation                                 │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### 2.1.3 Bandwidth Analysis per Tile

| Interface | Direction | Width | Frequency | Bandwidth @ 1 GHz | Utilization |
|-----------|-----------|-------|-----------|-------------------|-------------|
| **Data Memory Load 0** | Read | 256-bit | 1 GHz | 32 GB/s | Shared (2 units) |
| **Data Memory Load 1** | Read | 256-bit | 1 GHz | 32 GB/s | Shared (2 units) |
| **Data Memory Store** | Write | 256-bit | 1 GHz | 32 GB/s | Single unit |
| **Stream Input 0** | Read | 32-bit | 1 GHz | 4 GB/s | Per PSP broadcast |
| **Stream Input 1** | Read | 32-bit | 1 GHz | 4 GB/s | Unused in design |
| **Stream Output 0** | Write | 32-bit | 1 GHz | 4 GB/s | Per output |
| **Stream Output 1** | Write | 32-bit | 1 GHz | 4 GB/s | Unused in design |
| **Cascade Input** | Read | 384-bit | 1 GHz | **48 GB/s** | JAMP accumulation |
| **Cascade Output** | Write | 384-bit | 1 GHz | **48 GB/s** | JAMP forwarding |

**Key Observation**: Cascade bandwidth (48 GB/s) is **12× higher** than stream bandwidth (4 GB/s), making it ideal for high-volume JAMP data transfer.

### 2.2 PLIO Interface Specifications

PLIO (Programmable Logic I/O) connects AI Engine array to PL (Programmable Logic) fabric:

```
┌───────────────────────────────────────────────────────────────┐
│                    PLIO Interface Topology                     │
├───────────────────────────────────────────────────────────────┤
│                                                               │
│  Programmable Logic (PL)                                      │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  HLS Kernels / Custom Logic                             │ │
│  │  (e.g., data movers, pre-processing)                    │ │
│  └──────────────────┬──────────────────────────────────────┘ │
│                     │                                         │
│                     ▼                                         │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  AXI4-Stream Network (PL-side)                          │ │
│  └──────────────────┬──────────────────────────────────────┘ │
│                     │                                         │
│  ═══════════════════╪═════════════════════════════════════  │
│   PLIO Interface    │                                         │
│  ═══════════════════╪═════════════════════════════════════  │
│                     │                                         │
│                     ▼                                         │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  AI Engine SHIM Row (Row 0)                             │ │
│  │  ┌──────┬──────┬──────┬──────┬──────┬───────┐          │ │
│  │  │SHIM0 │SHIM1 │SHIM2 │SHIM3 │ ...  │SHIM49 │          │ │
│  │  │PLIO  │PLIO  │PLIO  │PLIO  │      │PLIO   │          │ │
│  │  └──┬───┴──┬───┴──┬───┴──┬───┴──────┴───┬───┘          │ │
│  │     │      │      │      │               │              │ │
│  │     ▼      ▼      ▼      ▼               ▼              │ │
│  │  [AIE Array Rows 1-7: Stream Switch connections]        │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                               │
└───────────────────────────────────────────────────────────────┘
```

#### 2.2.1 PLIO Port Specifications

**Maximum PLIO Ports**: 32 total for VCK190 (device-specific limit)

**PLIO Width Options** (Vitis 2024.1):

| Width | Enum | Bandwidth @ 312.5 MHz (PL) | Bandwidth @ 500 MHz (PL) | Floats/Cycle |
|-------|------|----------------------------|--------------------------|--------------|
| 32-bit | `plio_32_bits` | 1.25 GB/s | 2.0 GB/s | 1 float/cycle |
| 64-bit | `plio_64_bits` | 2.5 GB/s | 4.0 GB/s | 2 floats/cycle |
| 128-bit | `plio_128_bits` | 5.0 GB/s | 8.0 GB/s | 4 floats/cycle |
| 256-bit | `plio_256_bits` | 10.0 GB/s | 16.0 GB/s | 8 floats/cycle |

**Current Design Configuration**:
```cpp
#define GGTTG_IN_PLIO_BITS  128  // 128-bit input PLIO
#define GGTTG_OUT_PLIO_BITS 128  // 128-bit output PLIO
```

**Rationale for 128-bit**:
- Input: 20 floats/PSP → 80 bytes/PSP → at 128-bit (16 bytes/cycle) = 5 cycles/PSP
- Output: 1 float/PSP → 4 bytes/PSP → at 128-bit (16 bytes/cycle) < 1 cycle/PSP
- Balanced bandwidth without wasting PLIO resources

#### 2.2.2 PLIO Frequency Constraints

**PL Clock Domain**: Independent from AIE clock (1.0 GHz AIE ≠ PL frequency)

**Typical PL Frequencies**:
- **312.5 MHz**: Default, conservative
- **500 MHz**: High-performance designs
- **250 MHz**: Low-power designs

**Current Design**: Uses 500 MHz PL clock for PLIO (configurable)

---

