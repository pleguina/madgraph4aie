
## Fundamental Physics Concepts

### What is Helicity?

**Helicity** is the projection of a particle's spin along its direction of motion. For massless or highly relativistic particles, helicity is a good quantum number.

**For our process (gg→tt̄g):**
- **Gluons** (massless, spin-1): helicity = ±1 (left/right circular polarization)
- **Top quarks** (massive, spin-1/2): helicity = ±1 (approximately, in high-energy limit)
  - Actually: "chirality" for massive fermions, but we use helicity formalism

**Why 32 helicity combinations?**
```
Process: g₁ + g₂ → t + t̄ + g₃

Particle:  g₁   g₂   t    t̄   g₃
Helicity:  ±1   ±1   ±1   ±1  ±1
           └─────┴────┴────┴───┘
           2 × 2 × 2 × 2 × 2 = 32 combinations
```

**Physical meaning:**
- Helicity +1: Right-handed (spin aligned with momentum)
- Helicity -1: Left-handed (spin opposite to momentum)

**Why calculate all 32?**
At the LHC, incoming gluons and outgoing particles are **unpolarized** (all helicity states equally likely). To get the unpolarized cross-section, we must:
1. Calculate |M|² for EACH helicity combination
2. Average over initial states, sum over final states
3. This is encoded in the factor 1/256 = 1/(2² for initial spins × 64 for color averaging)

---

### Good Helicity Filtering

Not all helicity combinations contribute equally!

**Observation from MG5_aMC**: For gg→tt̄g at LHC energies:
- ~16 out of 32 helicities contribute > 99.9% of the cross-section
- The other 16 are **suppressed** by angular momentum conservation or dynamics

**"Good helicity" optimization:**
- Pre-compute which helicities matter (using threshold |M|²_hel / max|M|²_hel > 10⁻⁶)
- Only calculate those 16 helicities → **2× speedup**
- Introduce <0.1% error (negligible compared to other uncertainties)

**Our good helicity table:**
```cpp
GOOD_HEL_TABLE[16] = {0, 1, 4, 5, 8, 9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29}
```
These are the indices (0-31) of the significant helicity combinations.

---

### What Are HELAS Functions?

**HELAS** = **HEL**icity **A**mplitude **S**ubroutines

This is a **standard library** (originally in Fortran, now in C++/CUDA) for computing Feynman amplitudes using the **helicity formalism**.

**Key idea:** Instead of computing amplitudes using traditional Dirac spinors and Lorentz indices, HELAS:
1. Represents particle states as **wavefunctions** (complex vectors/arrays)
2. Combines them using **vertex functions** (implementing Feynman rules)
3. Everything is pre-projected onto helicity eigenstates

**Benefits:**
- Numerically efficient (no large matrix multiplications)
- Gauge-invariant by construction
- Easy to code-generate (MadGraph does this automatically)

---

### The Building Blocks: Wavefunction Generators

These functions generate **external wavefunctions** (initial/final state particles):

#### `ixxxxx`: Incoming Fermion (anti-quark in our case)

```cpp
v8c ixxxxx(v4f p, float mass, int helicity, int flow)
```

**Purpose:** Create wavefunction for **incoming fermion** (or outgoing anti-fermion)

**Inputs:**
- `p`: 4-momentum (E, px, py, pz)
- `mass`: particle mass (MT = 173 GeV for top quark)
- `helicity`: ±1 (spin projection)
- `flow`: +1 or -1 (particle vs anti-particle, for Feynman diagram topology)

**Output:**
- `v8c`: Complex vector of 8 floats (4 complex components = 4-component Dirac spinor)
  - Components: [ψ₀.re, ψ₀.im, ψ₁.re, ψ₁.im, ψ₂.re, ψ₂.im, ψ₃.re, ψ₃.im]

**Physics:** This computes the Dirac spinor u(p, h) for incoming fermion or v̄(p, h) for outgoing anti-fermion, pre-projected onto helicity eigenstate.

**In our code:**
```cpp
v8c w3 = ixxxxx(Ptbar, MT, htb, -1);  // Anti-top (incoming to diagram)
```

---

#### `oxxxxx`: Outgoing Fermion (top quark)

```cpp
v8c oxxxxx(v4f p, float mass, int helicity, int flow)
```

**Purpose:** Create wavefunction for **outgoing fermion** (or incoming anti-fermion)

**Inputs/Output:** Same structure as `ixxxxx`

**Physics:** Computes ū(p, h) for outgoing fermion or v(p, h) for incoming anti-fermion.

**In our code:**
```cpp
v8c w2 = oxxxxx(Pt, MT, ht, +1);  // Outgoing top quark
```

**Note:** The difference between `ixxxxx` and `oxxxxx` is the **direction of momentum flow** in the Feynman diagram, which affects the spinor structure (u vs ū, v vs v̄).

---

#### `vxxxxx`: Vector Boson (gluons)

```cpp
v8c vxxxxx(v4f p, float mass, int helicity, int flow)
```

**Purpose:** Create wavefunction for **vector boson** (gluon, photon, W, Z)

**Inputs:**
- `p`: 4-momentum
- `mass`: 0 for gluons (massless)
- `helicity`: ±1 (polarization)
- `flow`: ±1 (incoming vs outgoing)

**Output:**
- `v8c`: Complex vector (4 complex components = polarization vector ε^μ)

**Physics:** Computes polarization vector ε^μ(p, h) for the gluon. For helicity ±1, this gives circular polarization states.

**In our code:**
```cpp
v8c w0 = vxxxxx(Pg1, 0.f, hg1, -1);  // Incoming gluon 1
v8c w1 = vxxxxx(Pg2, 0.f, hg2, -1);  // Incoming gluon 2
v8c w4 = vxxxxx(Pg3, 0.f, hg3, +1);  // Outgoing gluon 3
```

---

### Off-Shell Wavefunctions: Internal Propagators

For **internal** (virtual) particles in diagrams, we need propagators:

#### `FFV1_1`: Off-shell fermion from outgoing fermion + vector

```cpp
v8c FFV1_1(v8c fermion_out, v8c vector, cfloat coupling, float mass, float width)
```

**Purpose:** Compute **outgoing off-shell fermion** from vertex F̄-F-V

**Physics:**
```
         ┌─── F_out (off-shell)
    F ───┤
         └─── V (on-shell gluon)
```

Returns: (iγ^μ coupling / (p² - m² + imΓ)) × ε_μ(V) × u(F)

**Example:**
```cpp
v8c f1_20 = FFV1_1(w2, w0, gc11, MT, WT);
// Top quark (w2) emits gluon (w0) → off-shell top
```

---

#### `FFV1_2`: Off-shell fermion from incoming fermion + vector

```cpp
v8c FFV1_2(v8c fermion_in, v8c vector, cfloat coupling, float mass, float width)
```

**Purpose:** Compute **incoming off-shell fermion** from vertex F-F̄-V

**Physics:**
```
    F_in (off-shell) ───┬─── F
                        └─── V
```

**Example:**
```cpp
v8c f2_30 = FFV1_2(w3, w0, gc11, MT, WT);
// Anti-top (w3) absorbs gluon (w0) → off-shell anti-top
```

---

#### `VVV1P0_1`: Off-shell vector from 3-gluon vertex

```cpp
v8c VVV1P0_1(v8c v1, v8c v2, cfloat coupling, float mass, float width)
```

**Purpose:** Compute **off-shell gluon** from 3-gluon vertex V-V-V

**Physics:** Non-abelian gauge theory vertex (gluon self-coupling)
```
    V1 ───┬─── V_out (off-shell)
    V2 ───┘
```

Implements the f^abc structure constant vertex of QCD.

**Example:**
```cpp
v8c v5_04 = VVV1P0_1(w0, w4, gc10, 0.f, 0.f);
// Gluon 1 + Gluon 3 → virtual gluon
```

**P0 suffix:** "Propagator with 0 mass" (gluons are massless)

---

#### `VVVV1P0_1`, `VVVV3P0_1`, `VVVV4P0_1`: 4-gluon vertex

```cpp
v8c VVVV1P0_1(v8c v1, v8c v2, v8c v3, cfloat coupling, float mass, float width)
```

**Purpose:** Compute off-shell gluon from **4-gluon contact vertex** (quartic coupling)

**Physics:** This comes from the (F^μν)² term in the QCD Lagrangian, where F is the field strength tensor. Expanding this yields 3-gluon AND 4-gluon vertices.

There are **3 different topologies** for how 4 gluons can connect:
- **VVVV1**: First topology (specific Lorentz structure)
- **VVVV3**: Second topology
- **VVVV4**: Third topology

All three contribute to Diagram 16, which is why it's computationally expensive.

**Example (D16):**
```cpp
v8c v10_4g = VVVV1P0_1(w0, w1, w4, gc12, 0.f, 0.f);
v8c v6_4g  = VVVV3P0_1(w0, w1, w4, gc12, 0.f, 0.f);
v8c v9_4g  = VVVV4P0_1(w0, w1, w4, gc12, 0.f, 0.f);
// Each gives a different way g1 + g2 + g3 interact
```

---

### Vertex Contraction: Computing Amplitudes

Once we have all wavefunctions (on-shell external + off-shell internal), we **contract** them at vertices to get complex amplitudes:

#### `FFV1_0`: Contract fermion-fermion-vector → amplitude

```cpp
cfloat FFV1_0(v8c fermion_in, v8c fermion_out, v8c vector, cfloat coupling)
```

**Purpose:** Compute amplitude from **closed F̄-F-V vertex**

**Physics:**
```
    F_in ───┬─── F_out
            │
            V
```

Returns: coupling × ū(F_out) × γ^μ × u(F_in) × ε_μ(V)

This is the **Feynman rule** for QCD quark-gluon vertex, contracted with wavefunctions.

**Example (Diagram 3):**
```cpp
cfloat amp = FFV1_0(f2_34, w2, w1, gc11);
// Anti-top line, top, gluon → complex amplitude
```

**"_0" suffix:** This is a **contraction** (returns scalar amplitude), not a propagator (which would return vector wavefunction).

---

#### `VVV1_0`: Contract 3-gluon vertex → amplitude

```cpp
cfloat VVV1_0(v8c v1, v8c v2, v8c v3, cfloat coupling)
```

**Purpose:** Amplitude from **closed 3-gluon vertex**

**Physics:**
```
    V1 ───┬─── V3
    V2 ───┘
```

Returns: coupling × f^abc × (ε₁ · ε₂)(p₁ - p₂) · ε₃ + cyclic permutations

This implements the triple gluon vertex from QCD.

**Example (Diagram 1):**
```cpp
cfloat amp = VVV1_0(v5, v6, w4, gc10);
// Three gluon wavefunctions → amplitude for this diagram topology
```

---

### Why This HELAS Approach?

**Traditional method** (Feynman calculus):
1. Write down Feynman rules with Dirac matrices (γ^μ), spinor indices
2. Contract all Lorentz indices manually
3. Sum over spins using trace theorems
4. Highly error-prone for complex processes

**HELAS method:**
1. Break diagram into **building blocks** (external WFs + vertices)
2. Each building block is a **pre-computed function**
3. Helicity is **encoded** in the wavefunctions (no explicit spin sum needed)
4. Just call functions in the right order!

**For Diagram 3 (example):**

Traditional way:
```
M₃ = ū(t) γ^μ v(t̄) × (propagator) × ε_μ(g3)
   × [bunch of index contractions and matrix multiplications]
```

HELAS way:
```cpp
v8c f2_34 = FFV1_2(w3, w4, gc11, MT, WT);  // Anti-top absorbs g3
cfloat M3 = FFV1_0(f2_34, w2, w1, gc11);    // Contract at remaining vertex
```

**Much cleaner!** And **code-generators like MadGraph** can automatically write these sequences from Feynman diagrams.

---

### Coupling Constants

**gc10, gc11, gc12** are the QCD coupling constants:

| Constant | Physics | Value | Used In |
|----------|---------|-------|---------|
| `gc10` | Triple gluon coupling (g₃) | -1.217716 | VVV1 vertices |
| `gc11` | Quark-gluon coupling (g_s γ^μ T^a) | 0 + 1.217716i | FFV1 vertices |
| `gc12` | 4-gluon coupling (g₄) | 0 + 1.48283i | VVVV vertices |

**Why complex?**
- QCD couplings involve the gauge group generators T^a and structure constants f^abc
- In helicity formalism, these become complex phases
- The imaginary parts arise from γ₅ (chirality operator) in the vertex rules

**Relation to α_s:**
These are related to the strong coupling constant α_s ≈ 0.118 at M_Z scale, but are effective couplings at the energy scale of top production (√s ~ 500 GeV).

---

### Putting It All Together: Why This Calculation Structure?

**The matrix element calculation follows this logic:**

1. **External particles** → Wavefunctions (ixxxxx, oxxxxx, vxxxxx)
   ```cpp
   v8c w0 = vxxxxx(Pg1, ...);  // Incoming gluon 1
   v8c w1 = vxxxxx(Pg2, ...);  // Incoming gluon 2
   v8c w2 = oxxxxx(Pt, ...);   // Outgoing top
   v8c w3 = ixxxxx(Ptbar, ...);// Outgoing anti-top
   v8c w4 = vxxxxx(Pg3, ...);  // Outgoing gluon 3
   ```

2. **Internal propagators** → Off-shell wavefunctions (FFV1_1, FFV1_2, VVV1P0_1, etc.)
   ```cpp
   v8c f1_20 = FFV1_1(w2, w0, ...);  // Top emits gluon 1
   v8c f2_30 = FFV1_2(w3, w0, ...);  // Anti-top absorbs gluon 1
   ```

3. **Final contraction** → Complex amplitude (FFV1_0, VVV1_0)
   ```cpp
   cfloat M_i = FFV1_0(f2_30, f1_20, w4, ...);  // One diagram
   ```

4. **Color structure** → Accumulate into JAMP
   ```cpp
   jamp[0] += CI * M_i;  // This diagram contributes to JAMP[0]
   jamp[2] -= CI * M_i;  // Also contributes (with opposite sign) to JAMP[2]
   ```

5. **Repeat for all 16 diagrams** → Get full jamp[6] array

6. **Color matrix** → |M|²_hel = Σᵢⱼ jamp[i] × C[i][j] × jamp*[j]

7. **Sum over helicities** → Unpolarized |M|²_total

**This structure is dictated by:**
- **Feynman rules** (QCD vertex structure)
- **HELAS formalism** (efficient helicity amplitude calculation)
- **Color decomposition** (SU(3) gauge group structure)
- **Numerical efficiency** (reuse wavefunctions across diagrams)

---

## Physics and Mathematics Background

### What Are We Actually Computing?

The goal is to calculate the **squared matrix element** |M|² for the process:

```
g + g → t + t̄ + g
(gluon + gluon → top quark + top antiquark + gluon)
```

This is a fundamental calculation in particle physics for predicting collision rates at the LHC.

---

### The 16 Feynman Diagrams

The process gg→tt̄g has **16 Feynman diagrams** that contribute. Each diagram represents a different way the particles can interact. These diagrams are NOT independent - they interfere with each other quantum mechanically.

**Example diagrams**:
```
Diagram 3 (FF-only):        Diagram 1 (V4G):
    g₁ ──┐                      g₁ ──┬──── g₃
         │─── t                      │
    g₂ ──┘                      g₂ ──┴────
         └─── t̄                      │
    g₃ ────────                      ├─── t
                                     └─── t̄
```

---

### Why Group Diagrams as FF vs V4G?

The 16 diagrams naturally split into two classes based on their **vertex structure**:

#### **FF Group (6 diagrams: D3, D4, D7, D9, D13, D14)**

**"FF" = Fermion-Fermion only**

- These diagrams contain ONLY fermion-fermion-vector vertices (like t-t̄-g)
- They use the FFV1 vertex function (Feynman rule for QCD quark-gluon interaction)
- **No 4-gluon vertex** (the quartic gluon self-interaction)
- **No 3-gluon vertex** (the triple gluon self-interaction at intermediate stages)
- Computationally simpler: ~6 FFV operations per diagram

**Physics**: These represent direct quark production from gluon splitting, with the third gluon radiated from either the top or anti-top quark line.

**Example (D3)**:
```
Step 1: g₁ + g₂ → virtual gluon (g*)
Step 2: g* → t + t̄
Step 3: t̄ radiates g₃
```

#### **V4G Group (10 diagrams: D1, D5, D10, D12, D15, D16)**

**"V4G" = Contains Vector (V) interactions, including 4-Gluon (4G) vertex**

- These diagrams involve gluon self-interactions
- They use VVV1 (3-gluon vertex) and VVVV (4-gluon vertex) functions
- **Computationally complex**: D16 alone has 3 sub-diagrams from the 4-gluon vertex
- More vector propagators, more complex wavefunction calculations

**Physics**: These exploit the non-abelian nature of QCD - gluons carry color charge and self-interact.

**Example (D1)**:
```
Step 1: g₁ + g₂ → virtual gluon (g*)
Step 2: g* + g₃ → another virtual gluon (g**)
Step 3: g** → t + t̄
```

**Example (D16)**: Has a 4-gluon vertex (g₁-g₂-g₃-g*) that spawns 3 different sub-contributions.

---

### Why A1, A2, B Subgroups Within V4G?

The V4G group (10 diagrams) is **too large** to fit in one AIE kernel (would exceed 16KB program memory). So we split it further:

| Group | Diagrams | Complexity | Program Memory |
|-------|----------|------------|----------------|
| **A1** | D1, D12 | 2 diagrams, moderate | ~5 KB |
| **A2** | D5, D10, D15 | 3 diagrams, complex | ~22 KB (too large!) |
| **A2a** | D5, D10 | 2 diagrams (A2 split) | ~8 KB |
| **A2b** | D15 | 1 diagram (A2 split) | ~6 KB |
| **B** | D16 | 1 diagram, 3 sub-diagrams | ~7 KB |

**Why this split?**
- **A1** and **A2** are topologically similar (both involve certain gluon combinations)
- **B (D16)** is unique because it's the ONLY diagram with a 4-gluon vertex (VVVV)
- The grouping is pragmatic: keep related diagrams together, but respect the 16KB limit

---

### The Mathematical Flow: Diagrams → JAMP → |M|²

Here's the key mathematical structure that drives the architecture:

#### Step 1: Calculate Individual Diagram Amplitudes

For each helicity combination (32 total), each diagram computes a **complex amplitude** M_i:

```
M₃ = FFV1_0(f₂₃₄, f₁₂₁, w₄, gc11)   // Diagram 3
M₁ = VVV1_0(v₅, v₆, w₄, gc10)       // Diagram 1
...
```

Each M_i is a complex number representing the quantum amplitude for that specific diagram and helicity.

#### Step 2: Sum Diagrams into Color Amplitudes (JAMP)

The 16 diagrams don't just add up directly. They contribute to **6 color amplitudes** called JAMP[0..5], where each JAMP represents a different **color flow** (how the color charge is exchanged).

**Key insight**: Each diagram contributes to MULTIPLE JAMPs with different signs:

```cpp
// Example from Diagram 3 (FF group):
cfloat amp = FFV1_0(f₂₃₄, w₂, w₁, gc11);
jamp_ff[0] += CI * amp;   // Contributes +i*amp to JAMP[0]
jamp_ff[2] -= CI * amp;   // Contributes -i*amp to JAMP[2]

// Example from Diagram 1 (V4G group):
cfloat amp = VVV1_0(v₅, v₆, w₄, gc10);
jamp_v4g[0] -= amp;       // Contributes -amp to JAMP[0]
jamp_v4g[2] += amp;       // Contributes +amp to JAMP[2]
jamp_v4g[4] += amp;       // Contributes +amp to JAMP[4]
jamp_v4g[5] -= amp;       // Contributes -amp to JAMP[5]
```

**This is why we need accumulation**:
- FF diagrams produce jamp_ff[6]
- V4G diagrams produce jamp_v4g[6]
- We must sum them: jamp_total[i] = jamp_ff[i] + jamp_v4g[i]

**Why separate FF and V4G first?** Because:
1. They're computed by different kernels (FF kernel vs V4G kernel)
2. They can run in PARALLEL
3. Only at the end do we need to combine them

#### Step 3: Color Matrix Multiplication → |M|²

Once we have the total JAMP[6] for a given helicity, we compute the squared amplitude for that helicity:

```cpp
|M|²_hel = Σᵢⱼ JAMP[i] × COLOR_MATRIX[i][j] × JAMP*[j]
```

Where:
- `COLOR_MATRIX[6][6]` encodes the color algebra (how the 6 color flows interfere)
- `JAMP*[j]` is the complex conjugate of JAMP[j]

This is a **Hermitian quadratic form** - essentially a weighted sum of all pair-wise interferences between color amplitudes.

#### Step 4: Sum Over Helicities

Finally, we sum |M|²_hel over all helicities to get the unpolarized cross-section contribution:

```cpp
|M|²_total = (1/256) × Σ_hel |M|²_hel
```

The factor 1/256 = 1/(4×4×16) accounts for:
- 4 gluon spin states (2 for each initial gluon)
- 4 quark spin states (2 for t, 2 for t̄)
- 16 = 2⁴ gluon color states... wait, actually this is the color averaging factor built into the color matrix normalization

---

