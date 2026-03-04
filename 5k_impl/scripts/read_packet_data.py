#!/usr/bin/env python3
"""
Read and decode packet-switched output data from AIE simulator

Packet format for 32-bit PLIO output:
  Beat 0: Packet ID (0-9)
  Beat 1: TLAST marker
  Beat 2: Result value (float encoded as 32-bit integer)
  Beat 3: TLAST marker (end of packet)

Decodes raw integers back to floats and validates against baseline.
"""

import struct
import sys
import os
import argparse

def int_to_float(raw_int):
    """Convert 32-bit unsigned int to float (reinterpret bytes)"""
    # Handle both signed and unsigned representations
    if raw_int > 0x7FFFFFFF:  # Negative in two's complement
        raw_int = raw_int - 0x100000000
    return struct.unpack('f', struct.pack('i', raw_int))[0]

def read_packet_output(filepath):
    """
    Parse packet output file
    
    Returns:
        List of (packet_id, float_value) tuples
    """
    with open(filepath, 'r') as f:
        lines = [l.strip() for l in f.readlines() if l.strip()]
    
    packets = []
    i = 0
    while i < len(lines):
        if lines[i].isdigit() or (lines[i].startswith('-') and lines[i][1:].isdigit()):
            # Packet ID
            pkt_id = int(lines[i])
            i += 1
            
            # Skip TLAST after packet ID
            if i < len(lines) and lines[i] == 'TLAST':
                i += 1
            
            # Raw integer value
            if i < len(lines) and (lines[i].isdigit() or lines[i].startswith('-')):
                raw_int = int(lines[i])
                float_val = int_to_float(raw_int)
                packets.append((pkt_id, float_val))
                i += 1
                
                # Skip TLAST after value
                if i < len(lines) and lines[i] == 'TLAST':
                    i += 1
            else:
                i += 1
        else:
            i += 1
    
    return packets

def load_baseline():
    """Load baseline result (expected value for packet 0)"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace_dir = os.path.dirname(script_dir)
    baseline_file = os.path.join(workspace_dir, '..', 'me2_out_0_baseline.txt')
    
    with open(baseline_file, 'r') as f:
        return float(f.read().strip())

def validate_trunk_results(trunk_id, sim_dir, baseline, verbose=False):
    """
    Validate results for one trunk
    
    Args:
        trunk_id: Trunk number (0-7)
        sim_dir: Path to x86simulator_output directory
        baseline: Expected value for packet 0
        verbose: Print all packets if True
    
    Returns:
        (success, packet_count, match_str)
    """
    filepath = os.path.join(sim_dir, 'data', f'me2_out_{trunk_id}.txt')
    
    if not os.path.exists(filepath):
        return False, 0, f"File not found: {filepath}"
    
    packets = read_packet_output(filepath)
    
    if not packets:
        return False, 0, "No packets found"
    
    # Sort by packet ID for consistent output
    packets.sort(key=lambda x: x[0])
    
    # Find packet 0 (baseline comparison)
    pkt0_results = [val for pid, val in packets if pid == 0]
    
    if not pkt0_results:
        return False, len(packets), "Packet 0 not found"
    
    pkt0_val = pkt0_results[0]
    diff = abs(pkt0_val - baseline)
    
    if diff < 1e-10:
        match_str = f"✓ MATCH: {pkt0_val:.9e} (diff: {diff:.3e})"
        success = True
    else:
        match_str = f"✗ MISMATCH: {pkt0_val:.9e} vs {baseline:.9e} (diff: {diff:.3e})"
        success = False
    
    if verbose:
        print(f"\n=== Trunk {trunk_id} - All Packets ===")
        for pid, val in packets:
            status = "✓" if (pid == 0 and success) else " "
            print(f"  {status} Packet {pid}: {val:.9e}")
    
    return success, len(packets), match_str

def main():
    parser = argparse.ArgumentParser(
        description='Decode and validate packet-switched AIE simulation results'
    )
    parser.add_argument(
        '--sim-dir',
        default='build/x86sim/x86simulator_output',
        help='Path to x86simulator_output directory (default: build/x86sim/x86simulator_output)'
    )
    parser.add_argument(
        '--trunk',
        type=int,
        help='Validate specific group only (0-9)'
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Print all packet results'
    )
    parser.add_argument(
        '--summary',
        action='store_true',
        help='Print summary table only (default)'
    )
    
    args = parser.parse_args()
    
    # Get workspace directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace_dir = os.path.dirname(script_dir)
    os.chdir(workspace_dir)
    
    # Load baseline
    try:
        baseline = load_baseline()
        print(f"Baseline (expected): {baseline:.9e}\n")
    except Exception as e:
        print(f"Error loading baseline: {e}")
        return 1
    
    # Determine which trunks to validate
    if args.trunk is not None:
        trunks = [args.trunk]
    else:
        trunks = range(10)
    
    # Validate each trunk
    print("=" * 80)
    print(f"{'Trunk':<8} {'Packets':<10} {'Status':<10} {'Result'}")
    print("=" * 80)
    
    total_success = 0
    total_trunks = 0
    
    for trunk_id in trunks:
        success, pkt_count, match_str = validate_trunk_results(
            trunk_id, args.sim_dir, baseline, args.verbose
        )
        
        status = "PASS" if success else "FAIL"
        print(f"{trunk_id:<8} {pkt_count:<10} {status:<10} {match_str}")
        
        if success:
            total_success += 1
        total_trunks += 1
    
    print("=" * 80)
    print(f"\nSummary: {total_success}/{total_trunks} trunks passed validation")
    
    if total_success == total_trunks:
        print("\n✓ ALL TRUNKS PASS - Physics results match baseline!")
        return 0
    else:
        print(f"\n✗ {total_trunks - total_success} trunk(s) failed validation")
        return 1

if __name__ == '__main__':
    sys.exit(main())
