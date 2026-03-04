#!/usr/bin/env python3
"""
Convert MadGraph momenta format to PSP (Phase Space Point) format for AIE testing.

Input format (test_data_momenta.txt):
  # Event N
  # Weight: X.XXX
  0 E Px Py Pz
  1 E Px Py Pz
  ...
  
Output format (psp_in_N.txt):
  E Px Py Pz
  E Px Py Pz
  ...
  (5 lines per event, no comments, no particle indices)
"""

import argparse
import os
import sys


def parse_momenta_file(input_file):
    """
    Parse MadGraph momenta file and extract events.
    
    Returns:
        list of events, where each event is a list of 5 four-momenta,
        and each four-momentum is [E, Px, Py, Pz]
    """
    events = []
    current_event = []
    
    with open(input_file, 'r') as f:
        for line in f:
            line = line.strip()
            
            # Skip empty lines and comments
            if not line or line.startswith('#'):
                continue
            
            # Parse momentum line: "N E Px Py Pz"
            parts = line.split()
            if len(parts) == 5:
                particle_idx = int(parts[0])
                four_momentum = [float(parts[i]) for i in range(1, 5)]
                current_event.append(four_momentum)
                
                # When we have all 5 particles, save the event
                if len(current_event) == 5:
                    events.append(current_event)
                    current_event = []
    
    return events


def write_psp_file(events, output_file, precision=17):
    """
    Write events in PSP format (no comments, no indices).
    
    Args:
        events: list of events from parse_momenta_file
        output_file: path to output file
        precision: number of decimal places (default: 17)
    """
    with open(output_file, 'w') as f:
        for event in events:
            for four_momentum in event:
                # Write E Px Py Pz in scientific notation
                line = ' '.join([f"{val:.{precision}e}" for val in four_momentum])
                f.write(line + '\n')


def main():
    parser = argparse.ArgumentParser(
        description='Convert MadGraph momenta to PSP format for AIE testing',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Convert all events to a single file
  %(prog)s -i test_data_momenta.txt -o psp_in_0.txt
  
  # Convert first 100 events
  %(prog)s -i test_data_momenta.txt -o psp_in_0.txt -n 100
  
  # Split into multiple files with 250 events each
  %(prog)s -i test_data_momenta.txt -o psp_in.txt -n 250 --split
        """
    )
    
    parser.add_argument('-i', '--input', required=True,
                        help='Input momenta file (MadGraph format)')
    parser.add_argument('-o', '--output', required=True,
                        help='Output PSP file (or prefix if --split)')
    parser.add_argument('-n', '--nevents', type=int, default=None,
                        help='Number of events to process (default: all)')
    parser.add_argument('--split', action='store_true',
                        help='Split into multiple files with -n events each')
    parser.add_argument('-p', '--precision', type=int, default=17,
                        help='Number of decimal places (default: 17)')
    
    args = parser.parse_args()
    
    # Check input file exists
    if not os.path.exists(args.input):
        print(f"Error: Input file not found: {args.input}", file=sys.stderr)
        return 1
    
    # Parse all events
    print(f"Reading {args.input}...")
    all_events = parse_momenta_file(args.input)
    print(f"Parsed {len(all_events)} events")
    
    # Determine how many events to process
    if args.nevents is None:
        events_to_process = all_events
    else:
        events_to_process = all_events[:args.nevents]
        print(f"Processing first {len(events_to_process)} events")
    
    # Write output
    if not args.split:
        # Single output file
        print(f"Writing {len(events_to_process)} events to {args.output}")
        write_psp_file(events_to_process, args.output, args.precision)
        print(f"Done! Output: {args.output}")
    else:
        # Split into multiple files
        if args.nevents is None:
            print("Error: --split requires -n/--nevents to specify events per file",
                  file=sys.stderr)
            return 1
        
        events_per_file = args.nevents
        num_files = (len(events_to_process) + events_per_file - 1) // events_per_file
        
        # Determine output pattern
        output_base = args.output.replace('.txt', '')
        
        for file_idx in range(num_files):
            start_idx = file_idx * events_per_file
            end_idx = min(start_idx + events_per_file, len(events_to_process))
            file_events = events_to_process[start_idx:end_idx]
            
            output_file = f"{output_base}_{file_idx}.txt"
            print(f"Writing events {start_idx}-{end_idx-1} to {output_file}")
            write_psp_file(file_events, output_file, args.precision)
        
        print(f"Done! Created {num_files} files")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
