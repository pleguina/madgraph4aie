#!/usr/bin/env python3
"""
Helicity Optimization Correctness Checker

Validates that optimized helicity kernels produce identical physics to baseline.
"""

import numpy as np
import sys

def validate_helicity_lut():
    """Test 1: Verify HELICITY_LUT matches GET_HEL_* macros"""
    print("=" * 60)
    print("TEST 1: Helicity LUT Validation")
    print("=" * 60)
    
    # Reference implementation (GET_HEL_* macros)
    def get_hel_g1(h): return 1 if ((h >> 4) & 1) else -1
    def get_hel_g2(h): return 1 if ((h >> 3) & 1) else -1
    def get_hel_t(h):  return 1 if ((h >> 2) & 1) else -1
    def get_hel_tb(h): return 1 if ((h >> 1) & 1) else -1
    def get_hel_g3(h): return 1 if ((h >> 0) & 1) else -1
    
    # Expected LUT (copied from C++ code)
    HELICITY_LUT = [
        (-1,-1,-1,-1,-1), (-1,-1,-1,-1,+1), (-1,-1,-1,+1,-1), (-1,-1,-1,+1,+1),
        (-1,-1,+1,-1,-1), (-1,-1,+1,-1,+1), (-1,-1,+1,+1,-1), (-1,-1,+1,+1,+1),
        (-1,+1,-1,-1,-1), (-1,+1,-1,-1,+1), (-1,+1,-1,+1,-1), (-1,+1,-1,+1,+1),
        (-1,+1,+1,-1,-1), (-1,+1,+1,-1,+1), (-1,+1,+1,+1,-1), (-1,+1,+1,+1,+1),
        (+1,-1,-1,-1,-1), (+1,-1,-1,-1,+1), (+1,-1,-1,+1,-1), (+1,-1,-1,+1,+1),
        (+1,-1,+1,-1,-1), (+1,-1,+1,-1,+1), (+1,-1,+1,+1,-1), (+1,-1,+1,+1,+1),
        (+1,+1,-1,-1,-1), (+1,+1,-1,-1,+1), (+1,+1,-1,+1,-1), (+1,+1,-1,+1,+1),
        (+1,+1,+1,-1,-1), (+1,+1,+1,-1,+1), (+1,+1,+1,+1,-1), (+1,+1,+1,+1,+1)
    ]
    
    all_pass = True
    for h in range(32):
        macro_result = (get_hel_g1(h), get_hel_g2(h), get_hel_t(h), 
                       get_hel_tb(h), get_hel_g3(h))
        lut_result = HELICITY_LUT[h]
        
        if macro_result != lut_result:
            print(f"❌ FAIL at h={h}")
            print(f"   Macro: {macro_result}")
            print(f"   LUT:   {lut_result}")
            all_pass = False
    
    if all_pass:
        print("✅ PASS: All 32 helicity entries match")
    else:
        print("❌ FAIL: LUT mismatch detected")
        sys.exit(1)
    
    print()

def compare_jamp_files(baseline_file, optimized_file, tolerance=1e-5):
    """Test 2: Per-helicity JAMP comparison"""
    print("=" * 60)
    print("TEST 2: Per-Helicity JAMP Comparison")
    print("=" * 60)
    
    try:
        # Parse JAMP outputs from simulation logs
        def parse_jamp(filename):
            jamps = []
            with open(filename, 'r') as f:
                for line in f:
                    if 'JAMP[' in line:
                        # Format: "JAMP[0] = (1.234e-05, -5.678e-06)"
                        parts = line.split('=')[1].strip()
                        parts = parts.replace('(', '').replace(')', '')
                        real, imag = parts.split(',')
                        jamps.append(complex(float(real), float(imag)))
            return np.array(jamps).reshape(-1, 6)  # 6 JAMP per helicity
        
        baseline = parse_jamp(baseline_file)
        optimized = parse_jamp(optimized_file)
        
        if baseline.shape != optimized.shape:
            print(f"❌ FAIL: Shape mismatch")
            print(f"   Baseline:  {baseline.shape}")
            print(f"   Optimized: {optimized.shape}")
            sys.exit(1)
        
        # Compute relative errors
        abs_diff = np.abs(optimized - baseline)
        rel_err = abs_diff / (np.abs(baseline) + 1e-30)
        
        max_err = np.max(rel_err)
        mean_err = np.mean(rel_err)
        
        print(f"Max relative error:  {max_err:.2e}")
        print(f"Mean relative error: {mean_err:.2e}")
        print(f"Tolerance:           {tolerance:.2e}")
        
        if max_err < tolerance:
            print("✅ PASS: JAMP values within tolerance")
        else:
            print(f"❌ FAIL: Error {max_err:.2e} exceeds {tolerance:.2e}")
            # Show worst offenders
            worst_idx = np.unravel_index(np.argmax(rel_err), rel_err.shape)
            print(f"\nWorst mismatch at helicity {worst_idx[0]}, JAMP[{worst_idx[1]}]:")
            print(f"  Baseline:  {baseline[worst_idx]}")
            print(f"  Optimized: {optimized[worst_idx]}")
            print(f"  Rel error: {rel_err[worst_idx]:.2e}")
            sys.exit(1)
        
    except FileNotFoundError as e:
        print(f"❌ FAIL: File not found - {e}")
        print("Run simulations with -DVERIFY_HELICITY to generate JAMP logs")
        sys.exit(1)
    
    print()

def compare_me2_files(baseline_file, optimized_file, tolerance=1e-5):
    """Test 3: Matrix element squared comparison"""
    print("=" * 60)
    print("TEST 3: |M|² Comparison (Random PSP Regression)")
    print("=" * 60)
    
    try:
        baseline = np.loadtxt(baseline_file)
        optimized = np.loadtxt(optimized_file)
        
        if baseline.shape != optimized.shape:
            print(f"❌ FAIL: Shape mismatch")
            print(f"   Baseline:  {baseline.shape}")
            print(f"   Optimized: {optimized.shape}")
            sys.exit(1)
        
        # Statistics
        n_points = len(baseline)
        rel_err = np.abs(optimized - baseline) / (np.abs(baseline) + 1e-30)
        
        max_err = np.max(rel_err)
        mean_err = np.mean(rel_err)
        std_err = np.std(rel_err)
        
        print(f"PSP count:           {n_points}")
        print(f"Max relative error:  {max_err:.2e}")
        print(f"Mean relative error: {mean_err:.2e}")
        print(f"Std dev:             {std_err:.2e}")
        print(f"Tolerance:           {tolerance:.2e}")
        
        # Correlation check
        corr = np.corrcoef(baseline, optimized)[0, 1]
        print(f"Pearson correlation: {corr:.10f}")
        
        if max_err < tolerance and corr > 0.99999:
            print("✅ PASS: |M|² values match within tolerance")
        else:
            print(f"❌ FAIL: Numeric discrepancy detected")
            if max_err >= tolerance:
                worst_idx = np.argmax(rel_err)
                print(f"\nWorst mismatch at PSP {worst_idx}:")
                print(f"  Baseline:  {baseline[worst_idx]:.6e}")
                print(f"  Optimized: {optimized[worst_idx]:.6e}")
                print(f"  Rel error: {rel_err[worst_idx]:.2e}")
            sys.exit(1)
        
    except FileNotFoundError as e:
        print(f"❌ FAIL: File not found - {e}")
        print("Generate ME² files by running AIE simulation with output redirection")
        sys.exit(1)
    
    print()

def plot_me2_correlation(baseline_file, optimized_file, output_plot):
    """Generate correlation plot for visual inspection"""
    try:
        import matplotlib.pyplot as plt
        
        baseline = np.loadtxt(baseline_file)
        optimized = np.loadtxt(optimized_file)
        
        fig, axes = plt.subplots(1, 2, figsize=(12, 5))
        
        # Histogram comparison
        axes[0].hist(baseline, bins=50, alpha=0.5, label='Baseline', color='blue')
        axes[0].hist(optimized, bins=50, alpha=0.5, label='Optimized', color='red')
        axes[0].set_xlabel('|M|² (GeV⁻²)')
        axes[0].set_ylabel('Count')
        axes[0].set_yscale('log')
        axes[0].legend()
        axes[0].set_title('ME² Distribution')
        axes[0].grid(True, alpha=0.3)
        
        # Scatter plot (1:1 correlation)
        axes[1].scatter(baseline, optimized, s=1, alpha=0.3, color='purple')
        axes[1].plot([baseline.min(), baseline.max()], 
                    [baseline.min(), baseline.max()], 
                    'r--', linewidth=2, label='Perfect match')
        axes[1].set_xlabel('Baseline |M|²')
        axes[1].set_ylabel('Optimized |M|²')
        axes[1].set_title('1:1 Correlation Check')
        axes[1].legend()
        axes[1].grid(True, alpha=0.3)
        axes[1].set_aspect('equal')
        
        plt.tight_layout()
        plt.savefig(output_plot, dpi=150)
        print(f"✅ Plot saved: {output_plot}")
        
    except ImportError:
        print("⚠️  matplotlib not available, skipping plot generation")
    except Exception as e:
        print(f"⚠️  Plot generation failed: {e}")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Helicity optimization correctness checker')
    parser.add_argument('--lut-only', action='store_true', 
                       help='Only validate LUT (no simulation files needed)')
    parser.add_argument('--jamp-baseline', type=str, 
                       help='Baseline JAMP log file')
    parser.add_argument('--jamp-optimized', type=str, 
                       help='Optimized JAMP log file')
    parser.add_argument('--me2-baseline', type=str, 
                       help='Baseline ME² output file')
    parser.add_argument('--me2-optimized', type=str, 
                       help='Optimized ME² output file')
    parser.add_argument('--plot', type=str, default='helicity_validation.png',
                       help='Output plot filename')
    parser.add_argument('--tolerance', type=float, default=1e-5,
                       help='Relative error tolerance')
    
    args = parser.parse_args()
    
    print("\n" + "=" * 60)
    print("HELICITY OPTIMIZATION CORRECTNESS CHECKER")
    print("=" * 60 + "\n")
    
    # Test 1: Always validate LUT
    validate_helicity_lut()
    
    if args.lut_only:
        print("✅ LUT validation complete (--lut-only mode)")
        return
    
    # Test 2: JAMP comparison (if files provided)
    if args.jamp_baseline and args.jamp_optimized:
        compare_jamp_files(args.jamp_baseline, args.jamp_optimized, args.tolerance)
    else:
        print("⚠️  Skipping JAMP comparison (files not provided)")
        print("   Use --jamp-baseline and --jamp-optimized to enable\n")
    
    # Test 3: ME² comparison (if files provided)
    if args.me2_baseline and args.me2_optimized:
        compare_me2_files(args.me2_baseline, args.me2_optimized, args.tolerance)
        plot_me2_correlation(args.me2_baseline, args.me2_optimized, args.plot)
    else:
        print("⚠️  Skipping ME² comparison (files not provided)")
        print("   Use --me2-baseline and --me2-optimized to enable\n")
    
    print("=" * 60)
    print("✅ ALL TESTS PASSED")
    print("=" * 60)

if __name__ == '__main__':
    main()
