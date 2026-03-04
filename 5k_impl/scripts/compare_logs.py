#!/usr/bin/env python3
"""
Compare baseline and 5-kernel simulation logs at all critical checkpoints.
Identifies exact locations of physics discrepancies.
"""

import re
import sys
from pathlib import Path

def parse_complex(s):
    """Parse '(real, imag)' string to complex number."""
    match = re.search(r'\(([-+]?[0-9]*\.?[0-9]+[eE]?[-+]?[0-9]*),\s*([-+]?[0-9]*\.?[0-9]+[eE]?[-+]?[0-9]*)\)', s)
    if match:
        return complex(float(match.group(1)), float(match.group(2)))
    return None

def parse_float(s):
    """Parse scientific notation float."""
    match = re.search(r'([-+]?[0-9]*\.?[0-9]+[eE]?[-+]?[0-9]*)', s)
    if match:
        return float(match.group(1))
    return None

def extract_wavefunction(log_lines, marker, index):
    """Extract wavefunction component at given index."""
    for i, line in enumerate(log_lines):
        if marker in line and f'[{index}]' in line:
            return parse_complex(line)
    return None

def extract_jamp_array(log_lines, marker):
    """Extract full JAMP[6] array after a marker."""
    jamp = []
    found = False
    for i, line in enumerate(log_lines):
        if marker in line:
            found = True
            continue
        if found and 'jamp[' in line:
            val = parse_complex(line)
            if val is not None:
                jamp.append(val)
            if len(jamp) == 6:
                break
    return jamp if len(jamp) == 6 else None

def extract_helicity_me2(log_lines, marker_pattern, hel):
    """Extract me2 for specific helicity."""
    pattern = f"{marker_pattern}.*Hel {hel}:.*hel_me2 = ([-+]?[0-9]*\\.?[0-9]+[eE]?[-+]?[0-9]*)"
    for line in log_lines:
        match = re.search(pattern, line)
        if match:
            return float(match.group(1))
    return None

def extract_final_me2(log_lines, marker):
    """Extract final me2_sum value."""
    for line in log_lines:
        if marker in line:
            return parse_float(line)
    return None

def compare_values(name, baseline, fivekernel, threshold=0.01):
    """Compare two values and report if different."""
    if baseline is None or fivekernel is None:
        print(f"  {name:30s}: MISSING DATA (baseline={baseline}, 5K={fivekernel})")
        return False
    
    if isinstance(baseline, complex):
        diff_real = abs(baseline.real - fivekernel.real)
        diff_imag = abs(baseline.imag - fivekernel.imag)
        mag_base = abs(baseline)
        mag_5k = abs(fivekernel)
        
        if mag_base > 1e-10:
            rel_err = abs(mag_5k - mag_base) / mag_base
            if rel_err > threshold:
                ratio = mag_5k / mag_base if mag_base != 0 else float('inf')
                print(f"  {name:30s}: MISMATCH!")
                print(f"    Baseline: ({baseline.real:.6e}, {baseline.imag:.6e}) |mag|={mag_base:.6e}")
                print(f"    5-Kernel: ({fivekernel.real:.6e}, {fivekernel.imag:.6e}) |mag|={mag_5k:.6e}")
                print(f"    Relative error: {rel_err*100:.2f}%  Ratio: {ratio:.4f}")
                return False
        else:
            if max(abs(fivekernel.real), abs(fivekernel.imag)) > 1e-10:
                print(f"  {name:30s}: MISMATCH (baseline ~0, 5K non-zero)")
                print(f"    Baseline: ({baseline.real:.6e}, {baseline.imag:.6e})")
                print(f"    5-Kernel: ({fivekernel.real:.6e}, {fivekernel.imag:.6e})")
                return False
    else:
        # Float comparison
        if abs(baseline) > 1e-10:
            rel_err = abs(fivekernel - baseline) / abs(baseline)
            if rel_err > threshold:
                ratio = fivekernel / baseline if baseline != 0 else float('inf')
                print(f"  {name:30s}: MISMATCH!")
                print(f"    Baseline: {baseline:.10e}")
                print(f"    5-Kernel: {fivekernel:.10e}")
                print(f"    Relative error: {rel_err*100:.2f}%  Ratio: {ratio:.4f}")
                return False
        else:
            if abs(fivekernel) > 1e-10:
                print(f"  {name:30s}: MISMATCH (baseline ~0, 5K non-zero)")
                print(f"    Baseline: {baseline:.10e}")
                print(f"    5-Kernel: {fivekernel:.10e}")
                return False
    
    return True

def main():
    baseline_log = Path('/home/pelayo/work/vitis_workspace/x86simulator_baseline.log')
    fivekernel_log = Path('/home/pelayo/work/vitis_workspace/5k_impl/build/x86sim/x86simulator_output/x86simulator.log')
    
    if not baseline_log.exists():
        print(f"ERROR: Baseline log not found: {baseline_log}")
        sys.exit(1)
    
    if not fivekernel_log.exists():
        print(f"ERROR: 5-kernel log not found: {fivekernel_log}")
        sys.exit(1)
    
    print("=" * 80)
    print("BASELINE vs 5-KERNEL LOG COMPARISON")
    print("=" * 80)
    
    baseline_lines = baseline_log.read_text().splitlines()
    fivekernel_lines = fivekernel_log.read_text().splitlines()
    
    all_match = True
    
    # Check external wavefunctions (w0-w4)
    print("\n[1] EXTERNAL WAVEFUNCTIONS (w0-w4)")
    for wf in ['w0', 'w1', 'w2', 'w3', 'w4']:
        for idx in [0, 5]:
            base_val = extract_wavefunction(baseline_lines, f"A2_{wf.upper()}", idx)
            fk_val = extract_wavefunction(fivekernel_lines, f"K1_DEBUG] {wf}", idx)
            if not compare_values(f"{wf}[{idx}]", base_val, fk_val):
                all_match = False
    
    # Check VVV propagators
    print("\n[2] VVV PROPAGATORS (w5_01, w10, w5_04)")
    for prop, base_marker, fk_marker in [
        ('w5_01', 'A2_VVV_PROP_5_01', 'K1_DEBUG] w5_01'),
        ('w10', 'A2_VVV_PROP', 'K1_W10_COMPUTED] w10'),
        ('w5_04', 'A2_VVV_PROP_5_04', 'K1_DEBUG] w5_04')
    ]:
        for idx in [0, 5]:
            base_val = extract_wavefunction(baseline_lines, base_marker, idx)
            fk_val = extract_wavefunction(fivekernel_lines, fk_marker, idx)
            if not compare_values(f"{prop}[{idx}]", base_val, fk_val):
                all_match = False
    
    # Check JAMP after D7
    print("\n[3] JAMP AFTER D7 (D2-D7 FFV diagrams)")
    base_jamp_d7 = extract_jamp_array(baseline_lines, 'A2_AFTER_D7')
    fk_jamp_d7 = extract_jamp_array(fivekernel_lines, 'K2A_AFTER_D7')
    if base_jamp_d7 and fk_jamp_d7:
        for i in range(6):
            if not compare_values(f"jamp[{i}] after D7", base_jamp_d7[i], fk_jamp_d7[i]):
                all_match = False
    else:
        print("  ERROR: Could not extract JAMP after D7")
        all_match = False
    
    # Check JAMP after D14
    print("\n[4] JAMP AFTER D14 (D8-D14 FFV diagrams)")
    base_jamp_d14 = extract_jamp_array(baseline_lines, 'A2_AFTER_D14')
    fk_jamp_d14 = extract_jamp_array(fivekernel_lines, 'K2B_AFTER_D14')
    if base_jamp_d14 and fk_jamp_d14:
        for i in range(6):
            if not compare_values(f"jamp[{i}] after D14", base_jamp_d14[i], fk_jamp_d14[i]):
                all_match = False
    else:
        print("  ERROR: Could not extract JAMP after D14")
        all_match = False
    
    # Check JAMP after D15 (final JAMP from K3)
    print("\n[5] JAMP AFTER D15 (Final from K3: D1, D12, D15)")
    base_jamp_d15 = extract_jamp_array(baseline_lines, 'A2_AFTER_D15')
    fk_jamp_d15 = extract_jamp_array(fivekernel_lines, 'K3_JAMP')
    if base_jamp_d15 and fk_jamp_d15:
        for i in range(6):
            if not compare_values(f"jamp[{i}] after D15", base_jamp_d15[i], fk_jamp_d15[i]):
                all_match = False
    else:
        print("  ERROR: Could not extract JAMP after D15")
        all_match = False
    
    # Check per-helicity me2 for first few helicities
    print("\n[6] PER-HELICITY ME2 VALUES")
    for hel in [0, 1, 16, 31]:
        base_hel_me2 = extract_helicity_me2(baseline_lines, 'A2_HEL', hel)
        fk_hel_me2 = extract_helicity_me2(fivekernel_lines, 'K4_HEL', hel)
        if not compare_values(f"hel_me2[{hel}]", base_hel_me2, fk_hel_me2):
            all_match = False
    
    # Check final me2_sum
    print("\n[7] FINAL ME2_SUM")
    base_me2_before = extract_final_me2(baseline_lines, 'me2_sum (before /256)')
    fk_me2_before = extract_final_me2(fivekernel_lines, 'me2_sum (before /256)')
    compare_values("me2_sum (before /256)", base_me2_before, fk_me2_before)
    
    base_me2_final = extract_final_me2(baseline_lines, 'me2_sum (AFTER /256)')
    fk_me2_final = extract_final_me2(fivekernel_lines, 'me2_sum (after /256)')
    if not compare_values("me2_sum (final)", base_me2_final, fk_me2_final):
        all_match = False
    
    print("\n" + "=" * 80)
    if all_match:
        print("✅ ALL CHECKS PASSED - Baseline and 5-kernel match!")
    else:
        print("❌ MISMATCHES FOUND - See details above")
    print("=" * 80)
    
    sys.exit(0 if all_match else 1)

if __name__ == '__main__':
    main()
