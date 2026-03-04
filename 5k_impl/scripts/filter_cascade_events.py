#!/usr/bin/env python3
"""
Filter CASCADE events from AIE simulator events.txt file
Makes the large events file more readable by showing only CASCADE-related information
"""

import argparse
import re
from collections import defaultdict


class CascadeEvent:
    """Represents a CASCADE event with parsed fields"""
    
    def __init__(self, line):
        self.raw = line.strip()
        self.time = None
        self.event = None
        self.col = None
        self.row = None
        self.data_val = None
        self.data_type = None
        self.data_port = None
        self.pc = None
        self.info = None
        
        self._parse()
    
    def _parse(self):
        """Parse event line into structured fields"""
        # Extract key fields using regex
        time_match = re.search(r'time=(\d+)', self.raw)
        if time_match:
            self.time = int(time_match.group(1))
        
        event_match = re.search(r'event=([^,]+)', self.raw)
        if event_match:
            self.event = event_match.group(1)
        
        col_match = re.search(r'col=(\d+)', self.raw)
        if col_match:
            self.col = int(col_match.group(1))
        
        row_match = re.search(r'row=(\d+)', self.raw)
        if row_match:
            self.row = int(row_match.group(1))
        
        pc_match = re.search(r'pc=(\d+)', self.raw)
        if pc_match:
            self.pc = int(pc_match.group(1))
        
        # Extract info (kernel function name)
        info_match = re.search(r'info=([^,]+)', self.raw)
        if info_match:
            self.info = info_match.group(1)
        
        # Extract cascade stream data
        data_val_match = re.search(r'data_val=([^,]+)', self.raw)
        if data_val_match:
            self.data_val = data_val_match.group(1)
        
        data_type_match = re.search(r'data_type=([^,]+)', self.raw)
        if data_type_match:
            self.data_type = data_type_match.group(1)
        
        data_port_match = re.search(r'data_port=([^,]+)', self.raw)
        if data_port_match:
            self.data_port = data_port_match.group(1)
    
    def is_cascade_stream(self):
        """Check if this is a CASCADE INSTREAM/OUTSTREAM event"""
        return self.event in ['CORE_CASCADE_INSTREAM', 'CORE_CASCADE_OUTSTREAM']
    
    def get_kernel_name(self):
        """Extract simplified kernel name from info"""
        if not self.info:
            return "unknown"
        
        # Extract kernel name (k1_wfgen, k2a_ff_diag, etc.)
        if 'k1_wfgen' in self.info:
            return 'K1_WFGEN'
        elif 'k2a_ff_diag' in self.info:
            return 'K2a_FF_DIAG_A'
        elif 'k2b_ff_diag' in self.info:
            return 'K2b_FF_DIAG_B'
        elif 'k3_vvv_amp' in self.info:
            return 'K3_VVV_AMP'
        elif 'k4_vvvv_color' in self.info:
            return 'K4_VVVV_COLOR'
        else:
            return self.info[:30]  # First 30 chars
    
    def __str__(self):
        """Readable string representation"""
        parts = []
        parts.append(f"Time: {self.time:>10}")
        parts.append(f"Event: {self.event:<25}")
        parts.append(f"Tile: ({self.col},{self.row})")
        parts.append(f"Kernel: {self.get_kernel_name():<20}")
        
        if self.data_port:
            parts.append(f"Port: {self.data_port}")
        
        if self.data_val:
            parts.append(f"Data: {self.data_val}")
        
        return " | ".join(parts)


def filter_cascade_events(input_file, output_file=None, stream_only=False, 
                          filter_col=None, filter_row=None, summary=False):
    """
    Filter CASCADE events from events.txt file
    
    Args:
        input_file: Path to events.txt
        output_file: Optional output file (prints to stdout if None)
        stream_only: If True, only show CORE_CASCADE_INSTREAM/OUTSTREAM events
        filter_col: Only show events from specific column
        filter_row: Only show events from specific row
        summary: If True, show summary statistics
    """
    
    cascade_events = []
    total_lines = 0
    
    print(f"Reading {input_file}...")
    
    with open(input_file, 'r') as f:
        for line in f:
            total_lines += 1
            
            # Quick filter - check if line contains CASCADE
            if 'cascade' not in line.lower():
                continue
            
            event = CascadeEvent(line)
            
            # Apply filters
            if stream_only and not event.is_cascade_stream():
                continue
            
            if filter_col is not None and event.col != filter_col:
                continue
            
            if filter_row is not None and event.row != filter_row:
                continue
            
            cascade_events.append(event)
    
    print(f"Found {len(cascade_events)} CASCADE events out of {total_lines} total lines")
    print()
    
    # Output results
    if output_file:
        out = open(output_file, 'w')
    else:
        out = None
    
    # Print header
    header = "="*120
    title = "CASCADE EVENTS FILTERED VIEW"
    output_line(header, out)
    output_line(title.center(120), out)
    output_line(header, out)
    output_line("", out)
    
    # Print events
    for event in cascade_events:
        output_line(str(event), out)
    
    # Print summary if requested
    if summary:
        output_line("", out)
        output_line(header, out)
        output_line("SUMMARY STATISTICS", out)
        output_line(header, out)
        
        # Count by event type
        event_counts = defaultdict(int)
        tile_counts = defaultdict(int)
        kernel_counts = defaultdict(int)
        
        for event in cascade_events:
            event_counts[event.event] += 1
            if event.col is not None and event.row is not None:
                tile_counts[(event.col, event.row)] += 1
            kernel_counts[event.get_kernel_name()] += 1
        
        output_line(f"\nTotal CASCADE events: {len(cascade_events)}", out)
        
        output_line("\nEvents by Type:", out)
        for event_type, count in sorted(event_counts.items(), key=lambda x: -x[1]):
            output_line(f"  {event_type:<30}: {count:>6}", out)
        
        output_line("\nEvents by Tile (col,row):", out)
        for tile, count in sorted(tile_counts.items(), key=lambda x: (x[0][0], x[0][1])):
            output_line(f"  Tile ({tile[0]},{tile[1]}): {count:>6}", out)
        
        output_line("\nEvents by Kernel:", out)
        for kernel, count in sorted(kernel_counts.items(), key=lambda x: -x[1]):
            output_line(f"  {kernel:<20}: {count:>6}", out)
    
    if out:
        out.close()
        print(f"\nResults written to {output_file}")


def output_line(text, file=None):
    """Print to stdout and optionally to file"""
    print(text)
    if file:
        file.write(text + '\n')


def main():
    parser = argparse.ArgumentParser(
        description='Filter CASCADE events from AIE simulator events.txt file',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Show all CASCADE-related events
  python filter_cascade_events.py events.txt
  
  # Show only CASCADE stream IN/OUT events
  python filter_cascade_events.py events.txt --stream-only
  
  # Filter by tile location (column 0, row 4)
  python filter_cascade_events.py events.txt --col 0 --row 4
  
  # Save to file with summary
  python filter_cascade_events.py events.txt -o cascade_filtered.txt --summary
  
  # K4 tile (col 4, row 4) CASCADE streams only
  python filter_cascade_events.py events.txt --col 4 --row 4 --stream-only
        """
    )
    
    parser.add_argument('input', help='Input events.txt file path')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    parser.add_argument('-s', '--stream-only', action='store_true',
                       help='Show only CORE_CASCADE_INSTREAM/OUTSTREAM events')
    parser.add_argument('-c', '--col', type=int,
                       help='Filter by column number')
    parser.add_argument('-r', '--row', type=int,
                       help='Filter by row number')
    parser.add_argument('--summary', action='store_true',
                       help='Show summary statistics')
    
    args = parser.parse_args()
    
    filter_cascade_events(
        args.input,
        output_file=args.output,
        stream_only=args.stream_only,
        filter_col=args.col,
        filter_row=args.row,
        summary=args.summary
    )


if __name__ == '__main__':
    main()
