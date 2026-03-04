#!/usr/bin/env python3
"""
Compare AIE ME² results with MadGraph5 baseline and calculate precision metrics.
Validates against required tolerances:
- Mean Absolute Error: < 3.2 × 10⁻¹¹
- Max Absolute Error: < 2.9 × 10⁻¹⁰
- Mean Relative Error: < 8.7 × 10⁻¹⁰
- Max Relative Error: < 4.2 × 10⁻⁹
- Target: All errors < 1e-9 (relative error)
"""

import numpy as np
import sys
from pathlib import Path

def load_madgraph_results(filepath):
    """
    Load MadGraph5 expected results.
    Format: index me2_value log_weight
    """
    data = []
    with open(filepath, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                idx = int(parts[0])
                me2 = float(parts[1])
                data.append((idx, me2))
    return np.array(data)

def load_aie_results(filepath):
    """
    Load AIE results.
    Different formats possible - auto-detect.
    """
    results = []
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            
            # Try different formats
            if 'PSP' in line and 'ME²' in line:
                # Format: PSP 0: ME² = 2.7854670000e+06
                parts = line.split('=')
                if len(parts) >= 2:
                    me2 = float(parts[-1].strip())
                    results.append(me2)
            elif 'ME2' in line or 'me2' in line:
                # Format: ME2: 1.234e-04
                parts = line.split(':')
                if len(parts) >= 2:
                    me2 = float(parts[-1].strip())
                    results.append(me2)
            else:
                # Try to parse as float directly
                try:
                    me2 = float(line)
                    results.append(me2)
                except ValueError:
                    continue
    
    return np.array(results)

def calculate_metrics(expected, actual):
    """
    Calculate precision metrics between expected and actual results.
    Returns dict with all metrics.
    """
    expected = np.array(expected)
    actual = np.array(actual)
    
    if len(expected) != len(actual):
        print(f"WARNING: Length mismatch: expected={len(expected)}, actual={len(actual)}")
        min_len = min(len(expected), len(actual))
        expected = expected[:min_len]
        actual = actual[:min_len]
    
    # Absolute errors
    abs_errors = np.abs(actual - expected)
    mean_abs_error = np.mean(abs_errors)
    max_abs_error = np.max(abs_errors)
    
    # Relative errors (avoid division by zero)
    rel_errors = np.abs((actual - expected) / (expected + 1e-30))
    mean_rel_error = np.mean(rel_errors)
    max_rel_error = np.max(rel_errors)
    
    # Find worst cases
    worst_abs_idx = np.argmax(abs_errors)
    worst_rel_idx = np.argmax(rel_errors)
    
    return {
        'num_samples': len(expected),
        'mean_abs_error': mean_abs_error,
        'max_abs_error': max_abs_error,
        'mean_rel_error': mean_rel_error,
        'max_rel_error': max_rel_error,
        'worst_abs_case': {
            'index': worst_abs_idx,
            'expected': expected[worst_abs_idx],
            'actual': actual[worst_abs_idx],
            'error': abs_errors[worst_abs_idx]
        },
        'worst_rel_case': {
            'index': worst_rel_idx,
            'expected': expected[worst_rel_idx],
            'actual': actual[worst_rel_idx],
            'error': rel_errors[worst_rel_idx]
        },
        'abs_errors': abs_errors,
        'rel_errors': rel_errors
    }

def print_metrics(metrics, thresholds):
    """
    Print metrics and compare against required thresholds.
    """
    print("=" * 80)
    print("AIE vs MadGraph5 Precision Comparison")
    print("=" * 80)
    print(f"\nNumber of samples: {metrics['num_samples']}")
    
    print("\n" + "-" * 80)
    print("ABSOLUTE ERRORS")
    print("-" * 80)
    print(f"Mean Absolute Error:     {metrics['mean_abs_error']:.6e}")
    print(f"  Required threshold:    < {thresholds['mean_abs_error']:.6e}")
    status = "✅ PASS" if metrics['mean_abs_error'] < thresholds['mean_abs_error'] else "❌ FAIL"
    print(f"  Status:                {status}")
    
    print(f"\nMax Absolute Error:      {metrics['max_abs_error']:.6e}")
    print(f"  Required threshold:    < {thresholds['max_abs_error']:.6e}")
    status = "✅ PASS" if metrics['max_abs_error'] < thresholds['max_abs_error'] else "❌ FAIL"
    print(f"  Status:                {status}")
    
    print("\n" + "-" * 80)
    print("RELATIVE ERRORS")
    print("-" * 80)
    print(f"Mean Relative Error:     {metrics['mean_rel_error']:.6e}")
    print(f"  Required threshold:    < {thresholds['mean_rel_error']:.6e}")
    status = "✅ PASS" if metrics['mean_rel_error'] < thresholds['mean_rel_error'] else "❌ FAIL"
    print(f"  Status:                {status}")
    
    print(f"\nMax Relative Error:      {metrics['max_rel_error']:.6e}")
    print(f"  Required threshold:    < {thresholds['max_rel_error']:.6e}")
    status = "✅ PASS" if metrics['max_rel_error'] < thresholds['max_rel_error'] else "❌ FAIL"
    print(f"  Status:                {status}")
    
    print(f"\nTarget (1e-9):           {metrics['max_rel_error']:.6e}")
    status = "✅ PASS" if metrics['max_rel_error'] < 1e-9 else "❌ FAIL"
    print(f"  Status:                {status}")
    
    print("\n" + "-" * 80)
    print("WORST CASE ANALYSIS")
    print("-" * 80)
    wac = metrics['worst_abs_case']
    print(f"\nWorst Absolute Error:")
    print(f"  Index:                 {wac['index']}")
    print(f"  Expected (MadGraph):   {wac['expected']:.15e}")
    print(f"  Actual (AIE):          {wac['actual']:.15e}")
    print(f"  Absolute Error:        {wac['error']:.15e}")
    
    wrc = metrics['worst_rel_case']
    print(f"\nWorst Relative Error:")
    print(f"  Index:                 {wrc['index']}")
    print(f"  Expected (MadGraph):   {wrc['expected']:.15e}")
    print(f"  Actual (AIE):          {wrc['actual']:.15e}")
    print(f"  Relative Error:        {wrc['error']:.15e}")
    
    print("\n" + "-" * 80)
    print("ERROR DISTRIBUTION")
    print("-" * 80)
    rel_errors = metrics['rel_errors']
    bins = [(0, 1e-12, "< 1e-12"), 
            (1e-12, 1e-11, "1e-12 to 1e-11"),
            (1e-11, 1e-10, "1e-11 to 1e-10"),
            (1e-10, 1e-9, "1e-10 to 1e-9"),
            (1e-9, 1e-8, "1e-9 to 1e-8"),
            (1e-8, 1e-7, "1e-8 to 1e-7"),
            (1e-7, float('inf'), "> 1e-7")]
    
    for low, high, label in bins:
        count = np.sum((rel_errors >= low) & (rel_errors < high))
        pct = 100.0 * count / len(rel_errors)
        bar = "█" * int(pct / 2)
        print(f"{label:20s}: {count:5d} ({pct:5.1f}%) {bar}")
    
    # Overall pass/fail
    print("\n" + "=" * 80)
    all_pass = (
        metrics['mean_abs_error'] < thresholds['mean_abs_error'] and
        metrics['max_abs_error'] < thresholds['max_abs_error'] and
        metrics['mean_rel_error'] < thresholds['mean_rel_error'] and
        metrics['max_rel_error'] < thresholds['max_rel_error'] and
        metrics['max_rel_error'] < 1e-9
    )
    
    if all_pass:
        print("✅ OVERALL: PASS - All metrics within required tolerances")
    else:
        print("❌ OVERALL: FAIL - Some metrics exceed tolerances")
    print("=" * 80)

def main():
    # Required thresholds from paper
    thresholds = {
        'mean_abs_error': 3.2e-11,
        'max_abs_error': 2.9e-10,
        'mean_rel_error': 8.7e-10,
        'max_rel_error': 4.2e-9,
        'target_rel_error': 1e-9
    }
    
    # File paths
    base_dir = Path(__file__).parent.parent
    expected_file = base_dir / "data" / "expected_me2_1000.txt"
    
    # Check if expected file exists
    if not expected_file.exists():
        print(f"ERROR: Expected results file not found: {expected_file}")
        sys.exit(1)
    
    # Load MadGraph expected results
    print(f"Loading MadGraph5 baseline: {expected_file}")
    mg_data = load_madgraph_results(expected_file)
    expected_me2 = mg_data[:, 1]  # Extract ME² values
    
    print(f"Loaded {len(expected_me2)} expected values")
    
    # Check command line argument first
    aie_file = None
    if len(sys.argv) > 1:
        aie_file = Path(sys.argv[1])
        if not aie_file.exists():
            print(f"\nERROR: Specified file not found: {aie_file}")
            sys.exit(1)
    else:
        # Try to find AIE results in default locations
        possible_aie_files = [
            base_dir / "build" / "x86sim" / "x86simulator_output" / "data" / "me2_out_0.txt",
            base_dir.parent / "me2_out_0_baseline.txt",
            base_dir / "build" / "x86sim" / "me2_out.txt",
            base_dir / "build" / "hw" / "me2_out.txt",
            base_dir / "data" / "me2_out.txt",
        ]
        
        for f in possible_aie_files:
            if f.exists():
                aie_file = f
                break
        
        if aie_file is None:
            print("\nERROR: No AIE results file found.")
            print("Searched locations:")
            for f in possible_aie_files:
                print(f"  - {f}")
            print("\nPlease specify AIE results file as command line argument:")
            print(f"  {sys.argv[0]} <aie_results.txt>")
            sys.exit(1)
    
    print(f"Loading AIE results: {aie_file}")
    aie_results = load_aie_results(aie_file)
    
    if len(aie_results) == 0:
        print("ERROR: No results loaded from AIE file")
        sys.exit(1)
    
    print(f"Loaded {len(aie_results)} AIE values")
    
    # Calculate metrics
    metrics = calculate_metrics(expected_me2, aie_results)
    
    # Print results
    print_metrics(metrics, thresholds)

if __name__ == "__main__":
    main()
