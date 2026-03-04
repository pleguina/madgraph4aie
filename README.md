# Packet-Switched AIE Architecture for gg→tt̄g Matrix Element Computation

**80-pipeline implementation on AMD Versal VCK190 (VC1902)**

Computes gg→tt̄g scattering matrix elements using a packet-switched AI Engine array with 5-kernel cascade pipelines. Achieves **912k PSP/s** aggregate throughput at **49 nJ/PSP** energy efficiency — 100× better than GPU and 920× better than CPU baselines.

## Repository Map

```
├── 5k_impl/                  # AIE graph & kernels (core compute)
│   ├── src/                  #   K1–K4 kernels, graph definitions
│   ├── helas/                #   HELAS physics functions (headers)
│   ├── scripts/              #   Python analysis & validation scripts
│   ├── data/                 #   Sample PSP input vectors & expected outputs
│   ├── alternatives/         #   Alternative graph architectures
│   ├── hslice_experimental/  #   Helicity-parallel experimental design
│   └── archive/              #   Old kernel versions (reference)
├── host_app/                 # PS host application (ARM A72)
│   ├── src/                  #   host.cpp, RAMBO PSP generator, MadGraph params
│   └── data/                 #   Test PSP binaries & expected ME² outputs
├── MM2S_pkt_gen/             # HLS kernel: DDR→AIE packet generator
│   └── src/                  #   mm2s_pkt_gen.cpp + testbench
├── S2MM_pkt_gen/             # HLS kernel: AIE→DDR packet parser
│   └── src/                  #   s2mm_pkt_parser.cpp + testbench
├── platform/                 # Vitis platform export (VCK190)
├── system_project/           # System link, package, deploy scripts
│   ├── hw_link/              #   v++ link configuration
│   ├── package/              #   SD card packaging config
│   └── *.sh                  #   Build & deploy automation scripts
├── daisy_chain_impl/         # Alternative: daisy-chain architecture
├── paper/                    # LaTeX paper sources
├── docs/                     # Organized documentation (see below)
└── _quarantine/              # Removed build artifacts (not for Git)
```

## Quick Start

### Prerequisites

- **Vitis 2024.1** with VCK190 platform support
- **Common Image**: `/opt/xilinx/platform/xilinx-versal-common-v2024.1/`
- **Platform XSA**: Place `hw.xsa` and `hw_emu.xsa` in `platform/hw/` and `platform/hw_emu/` (see [Platform XSA](#platform-xsa) below)

### 1. Compile AIE Graph (x86 simulation)

```bash
cd 5k_impl
mkdir -p build/x86sim && cd build/x86sim
cmake ../.. -DTARGET=x86sim
make -j$(nproc)
```

### 2. Compile AIE Graph (hardware)

```bash
cd 5k_impl
mkdir -p build/hw && cd build/hw
cmake ../.. -DTARGET=hw
make -j$(nproc)
# Produces: libadf.a
```

### 3. Build HLS Kernels

```bash
# MM2S packet generator
v++ -c --mode hls --config MM2S_pkt_gen/hls_config.cfg
# S2MM packet parser
v++ -c --mode hls --config S2MM_pkt_gen/hls_config.cfg
```

### 4. System Link & Package

```bash
cd system_project
./build_all.sh          # Links AIE + PL + PS, packages SD card image
# Or step-by-step:
./build_link.sh         # v++ link
./build_host.sh         # Compile host.exe
./package_sd.sh         # Create SD card image
```

### 5. Deploy to VCK190

```bash
cd system_project
./prepare_sd_card.sh    # Copy to SD card
# Or remote update:
./update_ssh.sh --full  # SSH deploy (requires board network access)
```

See [docs/deploy/VCK190_DEPLOYMENT_GUIDE.md](docs/deploy/VCK190_DEPLOYMENT_GUIDE.md) for detailed instructions.

## Architecture

Each pipeline computes one phase-space point through 5 cascaded AIE kernels:

```
PSP → [K1: WFGen] → [K2a: FF 2-7] → [K2b: FF 8-14] → [K3: VVV] → [K4: VVVV+Color] → ME²
       15 µs          18 µs           18 µs             12 µs         17 µs    = 80 µs
```

80 pipelines are organized in 8 trunks (rows) × 10 pipelines, using native `pktsplit<10>` / `pktmerge<10>` for zero-overhead packet routing across the full 400-tile AIE array.

See [docs/architecture/PAPER_ARCHITECTURE_FIGURES.md](docs/architecture/PAPER_ARCHITECTURE_FIGURES.md) for the complete architecture reference with figures.

## Key Results

| Metric | Value |
|--------|-------|
| Throughput | 912k PSP/s (80 pipelines) |
| Latency | 80 µs per PSP (deterministic) |
| Energy | 49 nJ/PSP |
| AIE Utilization | 100% (400/400 tiles) |
| Accuracy | < 1e-9 relative error vs. MadGraph5 |

## Platform XSA

The Vitis platform XSA files (`hw.xsa`, `hw_emu.xsa`, ~65 MB each) are **not tracked in Git** due to size. To reproduce builds:

1. **Export from Vivado**: Open the VCK190 base platform project and export XSA
2. **Or download**: Obtain from AMD/Xilinx VCK190 platform packages for Vitis 2024.1
3. Place in: `platform/hw/hw.xsa` and `platform/hw_emu/hw_emu.xsa`

## Documentation

Full documentation index: [docs/README.md](docs/README.md)

| Category | Description |
|----------|-------------|
| [Architecture](docs/architecture/) | System design, AIE array layout, packet switching |
| [Build](docs/build/) | Build instructions for each component |
| [Deploy](docs/deploy/) | VCK190 SD card and deployment guides |
| [Validation](docs/validation/) | Physics correctness and precision analysis |
| [Benchmarking](docs/benchmarking/) | Throughput and power measurement |
| [Troubleshooting](docs/troubleshooting/) | Deadlocks, cascade issues, fixes |
| [Analysis](docs/analysis/) | Design exploration and feasibility studies |
| [Reference](docs/reference/) | Configuration guides and checklists |

## Tools & Versions

- **Vitis**: 2024.1
- **Target**: Versal VC1902 (VCK190 evaluation board)
- **AIE**: 400-tile array, 1 GHz
- **Compiler**: aiecompiler with `-Oz` optimization
