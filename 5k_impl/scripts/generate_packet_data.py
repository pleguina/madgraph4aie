#!/usr/bin/env python3
"""
Generate packet-switched input data for AIE simulator (Phase 1: 10 pipelines)

Packet format for 32-bit PLIO:
  Beat 0: Packet header (ID, parity, source row/col, etc.)
  Beats 1-20: PSP data (20 floats, one per beat)
  Beat 20: Last beat with TLAST=1

Each packet is routed to one of 10 pipelines based on packet ID (0-9).
"""

import struct
import sys
import os

def generate_packet_header(packet_id, packet_type=0):
    """
    Generate 32-bit packet header per Versal AIE packet format
    
    Bit layout:
      [4:0]   - Packet ID (0-31)
      [11:5]  - Reserved (7'b0 per AMD spec)
      [14:12] - Packet Type (user-defined)
      [15]    - Reserved (1'b1 per AMD spec)
      [20:16] - Source Row (0x1F = -1 for PL origin)
      [27:21] - Source Column (0x7F = -1 for PL origin)
      [30:28] - Reserved (3'b0 per AMD spec)
      [31]    - Parity bit (odd parity of bits 30:0)
    """
    header = 0
    header |= (packet_id & 0x1F)              # Bits 4:0 - Packet ID
    header |= (0 << 5)                        # Bits 11:5 - Reserved (must be 0, not 0x7F)
    header |= ((packet_type & 0x7) << 12)     # Bits 14:12 - Packet type
    header |= (1 << 15)                       # Bit 15 - Reserved (1)
    header |= (0x1F << 16)                    # Bits 20:16 - Source row (-1 = PL)
    header |= (0x7F << 21)                    # Bits 27:21 - Source col (-1 = PL)
    header |= (0 << 28)                       # Bits 30:28 - Reserved
    
    # Parity bit (bit 31): odd parity of bits[30:0]
    parity = bin(header & 0x7FFFFFFF).count('1') % 2
    header |= ((1 - parity) << 31)
    
    return header

def float_to_int(f):
    """Convert float to 32-bit int (reinterpret bytes, not cast)"""
    return struct.unpack('i', struct.pack('f', f))[0]

def int_to_float(i):
    """Convert 32-bit int to float (for verification)"""
    return struct.unpack('f', struct.pack('i', i))[0]

def generate_psp_packet(packet_id, psp_data):
    """
    Generate one PSP packet (header + 20 floats)
    
    Args:
        packet_id: Pipeline ID (0-9) for routing
        psp_data: List of 20 floats (4-vectors for 5 particles)
    
    Returns:
        List of strings for AIE simulator input file
    """
    if len(psp_data) != 20:
        raise ValueError(f"PSP must have exactly 20 floats, got {len(psp_data)}")
    
    lines = []
    
    # Packet header
    header = generate_packet_header(packet_id)
    lines.append(str(header))
    
    # PSP data (20 floats as 20 separate 32-bit words)
    for i, value in enumerate(psp_data):
        int_value = float_to_int(value)
        if i == 19:  # Last word gets TLAST
            lines.append("TLAST")
        lines.append(str(int_value))
    
    return lines

def load_test_psp(pipe_id):
    """
    Load real PSP data from baseline file
    
    Reads from psp_in_0_base.txt and extracts PSPs sequentially
    PSP format: [p0.px, p0.py, p0.pz, p0.E, p1.px, ..., p4.E] (20 floats)
    
    If pipe_id exceeds available PSPs, wraps around (reuses PSPs)
    """
    # Get path to baseline data file
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace_dir = os.path.dirname(script_dir)
    baseline_file = os.path.join(workspace_dir, 'data', 'psp_in_0_base.txt')
    
    # Read all floats from baseline file
    with open(baseline_file, 'r') as f:
        content = f.read()
        all_values = [float(x) for x in content.split()]
    
    # Calculate how many PSPs are available
    num_available_psps = len(all_values) // 20
    
    # Wrap around if pipe_id exceeds available PSPs
    effective_pipe_id = pipe_id % num_available_psps
    
    # Each PSP is 20 floats, extract the PSP for this effective pipe_id
    start_idx = effective_pipe_id * 20
    end_idx = start_idx + 20
    
    return all_values[start_idx:end_idx]

def generate_test_data(output_file, num_pipelines=10):
    """Generate packet data for all pipelines"""
    print(f"Generating packet-switched data for {num_pipelines} pipelines...")
    
    with open(output_file, 'w') as f:
        for pipe_id in range(num_pipelines):
            # Load PSP data (replace with real source)
            psp_data = load_test_psp(pipe_id)
            
            # Generate packet
            packet_lines = generate_psp_packet(pipe_id, psp_data)
            
            # Write to file
            for line in packet_lines:
                f.write(line + '\n')
            
            print(f"  Pipeline {pipe_id}: packet ID={pipe_id}, header=0x{generate_packet_header(pipe_id):08X}")
    
    print(f"\nSuccess! Generated {output_file}")
    print(f"Total lines: {num_pipelines * (1 + 20 + 1)} (header + 20 data + TLAST per packet)")

def verify_header(header_int):
    """Verify packet header format (debug helper)"""
    packet_id = header_int & 0x1F
    reserved1 = (header_int >> 5) & 0x7F
    pkt_type = (header_int >> 12) & 0x7
    reserved2 = (header_int >> 15) & 0x1
    src_row = (header_int >> 16) & 0x1F
    src_col = (header_int >> 21) & 0x7F
    reserved3 = (header_int >> 28) & 0x7
    parity = (header_int >> 31) & 0x1
    
    print(f"  Packet ID: {packet_id}")
    print(f"  Pkt Type:  {pkt_type}")
    print(f"  Src Row:   {src_row} (0x{src_row:02X})")
    print(f"  Src Col:   {src_col} (0x{src_col:02X})")
    print(f"  Parity:    {parity}")

if __name__ == '__main__':
    # Output directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace_dir = os.path.dirname(script_dir)
    data_dir = os.path.join(workspace_dir, 'data')
    
    # Create data directory if it doesn't exist
    os.makedirs(data_dir, exist_ok=True)
    
    # Architecture: 10 column-groups x 8 rows = 80 pipelines
    # Each group file (psp_in_g_streamed.txt) contains 8 packets,
    # one per row (packet ID 0-7 = row 0-7 within the group).
    num_groups    = 10 if '--full' in sys.argv else 1
    rows_per_group = 8

    print(f"Generating packet data for {num_groups} group(s), {rows_per_group} rows each")
    print(f"Total pipelines: {num_groups * rows_per_group}")
    print()

    # Generate one file per column-group
    for group_id in range(num_groups):
        output_file = os.path.join(data_dir, f'psp_in_{group_id}_streamed.txt')

        print(f"--- Group {group_id} (cols {group_id*5}-{group_id*5+4}) ---")

        with open(output_file, 'w') as f:
            for row in range(rows_per_group):
                # Global pipeline ID
                global_pipe_id = group_id * rows_per_group + row

                # Packet ID = row index within the group (0-7)
                packet_id = row

                # Load PSP data
                psp_data = load_test_psp(global_pipe_id)

                # Generate packet with row-based ID
                packet_lines = generate_psp_packet(packet_id, psp_data)

                # Write to file
                for line in packet_lines:
                    f.write(line + '\n')

                print(f"  Pipeline {global_pipe_id:2d} (group {group_id}, row {row}, pkt_id {packet_id}): header=0x{generate_packet_header(packet_id):08X}")

        print(f"  Saved: {output_file}")
        print()

    print("="*70)
    print(f"Success! Generated {num_groups} group file(s)")
    print(f"Total packets: {num_groups * rows_per_group}")
    print("="*70)
    
    # Verification example
    if '--verify' in sys.argv:
        print("\n" + "="*60)
        print("Verification: Packet header format for Pipeline 0")
        print("="*60)
        verify_header(generate_packet_header(0))

