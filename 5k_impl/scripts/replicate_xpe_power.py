#!/usr/bin/env python3
"""
Replicate row 1 power data across all 8 rows for approximate power estimation.
This creates an XPE with all 400 cores active based on row 1's pattern.
"""

import re
import sys

def replicate_xpe_power(input_xpe, output_xpe):
    """Replicate row 1 (trunk 1) power pattern to rows 2-8"""
    
    with open(input_xpe, 'r') as f:
        content = f.read()
    
    # Extract header (everything before first TILE)
    tile_start = content.find('<TILE')
    header = content[:tile_start]
    footer = '    </AIE_MODULE>\n  </AIE>\n</POWERDATA>\n'
    
    # Extract all TILE blocks from row 1 (coordinates="col,1")
    tile_pattern = r'(<TILE[^>]*coordinates="(\d+),1"[^>]*>.*?</TILE>)'
    row1_tiles = []
    
    for match in re.finditer(tile_pattern, content, re.DOTALL):
        tile_xml = match.group(1)
        col = int(match.group(2))
        row1_tiles.append((col, tile_xml))
    
    print(f"Found {len(row1_tiles)} active tiles in row 1 (columns 0-{len(row1_tiles)-1})")
    
    # Sort by column
    row1_tiles.sort(key=lambda x: x[0])
    
    # Build new XPE with all 8 rows
    new_tiles = []
    
    for target_row in range(1, 9):  # Rows 1-8
        print(f"Replicating to row {target_row}...")
        for col, tile_xml in row1_tiles:
            # Adjust tile for new row
            new_tile = tile_xml
            
            # Update coordinates
            new_tile = re.sub(
                r'coordinates="(\d+),1"',
                f'coordinates="{col},{target_row}"',
                new_tile
            )
            
            # Update tile name CR(col,row_offset) where row_offset = row - 1
            row_offset = target_row - 1
            # Find current name
            old_name_match = re.search(r'name="CR\((\d+),0\)"', new_tile)
            if old_name_match:
                old_col = old_name_match.group(1)
                new_tile = re.sub(
                    r'name="CR\(\d+,0\)"',
                    f'name="CR({col},{row_offset})"',
                    new_tile
                )
            
            new_tiles.append(new_tile)
    
    # Also include all the zero-load tiles from other rows (to maintain structure)
    # Extract tiles from rows 2-8 that were already in the file
    other_rows_pattern = r'(<TILE[^>]*coordinates="(\d+),([2-8])"[^>]*>.*?</TILE>)'
    for match in re.finditer(other_rows_pattern, content, re.DOTALL):
        tile_xml = match.group(1)
        col = int(match.group(2))
        row = int(match.group(3))
        
        # Replace with our replicated version if we have it
        # (skip, we'll use our replicated ones)
        pass
    
    # Write new XPE
    with open(output_xpe, 'w') as f:
        f.write(header)
        for tile in new_tiles:
            f.write('      ' + tile + '\n')
        f.write(footer)
    
    print(f"\nCreated {output_xpe}")
    print(f"Total active tiles: {len(new_tiles)} (should be 400)")
    print(f"Active rows: 8 (rows 1-8)")
    print(f"This is a COARSE approximation - recompile for accurate power!")

if __name__ == '__main__':
    input_xpe = '/home/pelayo/work/vitis_workspace/5k_impl/build/hw/aiesim_xpe/ggttg_graph_main_80pipe.xpe'
    output_xpe = '/home/pelayo/work/vitis_workspace/5k_impl/build/hw/aiesim_xpe/ggttg_graph_main_80pipe_400cores_approx.xpe'
    
    if len(sys.argv) > 1:
        input_xpe = sys.argv[1]
    if len(sys.argv) > 2:
        output_xpe = sys.argv[2]
    
    replicate_xpe_power(input_xpe, output_xpe)
