# aie_cascade_paper — Build Instructions

## Paper title
**"A Deterministic Wavefunction-Token Cascade Architecture for Monte Carlo
Matrix Element Evaluation on AMD Versal AI Engines"**

---

## File structure

```
main.tex                  ← Root document (Springer sn-jnl class)
sec_introduction.tex      ← §1 Introduction + contributions list
sec_background.tex        ← §2 Background: MadGraph, gg→ttg, GPU state-of-art
sec_platform.tex          ← §3 Versal VCK190 / AI Engine tile + cascade interface
sec_architecture.tex      ← §4 Wavefunction-token cascade pipeline (the core contribution)
sec_implementation.tex    ← §5 HELAS-to-AIE adaptations, optimisations
sec_results.tex           ← §6 Results (PM, precision, throughput, energy)
sec_conclusion.tex        ← §7 Conclusion + future work
references.bib            ← Bibliography
```

---

## To compile

**Requires**: `sn-jnl.cls` and the `bst/` folder from `../springer_template/`.

Option A — copy the class files here:
```bash
cp ../springer_template/sn-jnl.cls .
cp -r ../springer_template/bst .
pdflatex main.tex
bibtex main
pdflatex main.tex
pdflatex main.tex
```

Option B — set TEXINPUTS to point to the template folder:
```bash
TEXINPUTS=../springer_template//: pdflatex main.tex
```

---

## TODO before submission

Results section (`sec_results.tex`) contains `\TODO{}` markers for data not
yet measured:

| Item | Location | Notes |
|------|----------|-------|
| Single-pipeline latency (µs/PSP) | §6.3 | Run HW benchmark on VCK190 |
| 80-pipeline throughput (PSP/s) | §6.4, Table 3 | Linear scale from 1-pipe |
| float32 precision scatter plot | §6.2 | 1000-PSP study vs MadGraph64 |
| GPU throughput for gg→ttg specifically | Table 3 | Extract from CUDACPP benchmarks |
| PL LUT/DSP utilisation | §6.6 | From Vivado/Vitis implementation report |
| Energy/PSP at 80 pipes | Table 3 | 45W / throughput |
| Funding acknowledgement | §7 | Fill institution/grant |

### Figures still to create (described in PAPER_RESTRUCTURING_PROPOSAL.md)

| Figure | Content | Section |
|--------|---------|---------|
| Fig. 1 | MadGraph computational graph for gg→ttg (DAG of all 18 WFs, 16 diagrams, JAMP coefficients) | §2 |
| Fig. 2 | PM bar chart: monolithic ~38 KB vs 5-kernel split, red line at 16 KB | §3 |
| Fig. 3 | Wavefunction-token cascade pipeline block diagram (single lane) | §4 |
| Fig. 4 | Token layout: 8-WF extended token (18 beats) + 5-WF standard token (12 beats) | §4 |
| Fig. 5 | Cascade contract timing diagram showing deadlock prevention | §4 |
| Fig. 6 | 80-pipeline array plan view (50×8 tiles, colour-coded by kernel type) | §4 |
| Fig. 7 | Full PS–PL–AIE system block diagram (3-layer stack) | §5 |
| Fig. 8 | ME² scatter plot: AIE vs MadGraph64 for 1000 PSPs + relative-error histogram | §6 |
| Fig. 9 | Program memory utilisation bar chart (all 5 kernels vs 16 KB limit) | §6 |
| Fig. 10 | PLIO bandwidth budget (0.5% utilisation visualisation) | §6 |
