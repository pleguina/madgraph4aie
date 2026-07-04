# Documentation Index

## Packet-Switched AIE Architecture for gg→tt̄g Matrix Element Computation

---

### Architecture

- [PAPER_ARCHITECTURE_FIGURES.md](architecture/PAPER_ARCHITECTURE_FIGURES.md) — **Primary reference**: Complete architecture with figures, performance data, and physics validation
- [AIE_ARCHITECTURE.md](architecture/AIE_ARCHITECTURE.md) — Hybrid cascade architecture technical specification

### Analysis

- [PHYSICS_BEHIND.md](analysis/PHYSICS_BEHIND.md) — AIE pipeline optimization plan and physics background

### Build

- [system_building_instructions.md](build/system_building_instructions.md) — Full system build workflow
- [system_verification_instructions.md](build/system_verification_instructions.md) — Emulation & verification flow
- [v++_commands.md](build/v++_commands.md) — Vitis v++ command reference
- [packet_switching.md](build/packet_switching.md) — Packet switching reference (Vitis docs)

### Benchmarking

- [BENCHMARKING_GUIDE.md](benchmarking/BENCHMARKING_GUIDE.md) — Complete benchmarking methodology

### Deploy

- [VCK190_DEPLOYMENT_GUIDE.md](deploy/VCK190_DEPLOYMENT_GUIDE.md) — **Complete step-by-step deployment guide**
- [SD_CARD_QUICK_REFERENCE.md](deploy/SD_CARD_QUICK_REFERENCE.md) — Quick reference for SD card issues

### Validation

- [PRECISION_ANALYSIS.md](validation/PRECISION_ANALYSIS.md) — AIE vs MadGraph5 precision comparison
- [TESTING_BEFORE_DEPLOYMENT.md](validation/TESTING_BEFORE_DEPLOYMENT.md) — Pre-deployment test procedures

### Troubleshooting

- [AIESIM_PLIO_DEADLOCK_FIX.md](troubleshooting/AIESIM_PLIO_DEADLOCK_FIX.md) — Why the cycle-accurate AIE sim appears to hang (build recipe overwriting `scsim_config.json` and stripping PLIO drivers → input starvation) and how to run it to completion
