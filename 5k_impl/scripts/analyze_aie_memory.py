#!/usr/bin/env python3
"""
AIE Memory Analyzer - Architecture Agnostic

Analyzes AIE compilation output to extract:
- Program memory usage per core
- Function-level memory breakdown
- Stack usage
- Code size analysis

Works with any AIE graph implementation (4k_split, token_cascade, fir_style, etc.)
"""

import os
import sys
import re
import json
from pathlib import Path
from collections import defaultdict
import subprocess

class AIEMemoryAnalyzer:
    def __init__(self, work_dir):
        self.work_dir = Path(work_dir)
        self.aie_dir = self.work_dir / 'aie'
        self.cores = {}
        self.pm_limit = 16384  # 16KB program memory limit
        
    def find_cores(self):
        """Find all AIE cores in the work directory"""
        if not self.aie_dir.exists():
            print(f"Error: AIE directory not found: {self.aie_dir}")
            return []
        
        cores = []
        for item in self.aie_dir.iterdir():
            if item.is_dir() and re.match(r'\d+_\d+', item.name):
                cores.append(item.name)
        
        return sorted(cores)
    
    def parse_map_file(self, core_name):
        """Parse the .map file to extract function sizes"""
        map_file = self.aie_dir / core_name / 'Release' / f'{core_name}.map'
        
        if not map_file.exists():
            return None
        
        functions = {}
        current_section = None
        
        try:
            with open(map_file, 'r') as f:
                for line in f:
                    # Look for .text section (code)
                    if '.text' in line and 'LOAD' in line:
                        current_section = 'text'
                    
                    # Parse function entries in memory map
                    # Format typically: address  size  alignment  function_name
                    match = re.match(r'\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(.+)', line)
                    if match and current_section == 'text':
                        addr = int(match.group(1), 16)
                        size = int(match.group(2), 16)
                        name = match.group(3).strip()
                        
                        if size > 0:
                            functions[name] = {
                                'address': f'0x{addr:x}',
                                'size': size
                            }
        except Exception as e:
            print(f"Warning: Error parsing map file {map_file}: {e}")
            return None
        
        return functions
    
    def get_elf_info(self, core_name):
        """Extract information from ELF file using readelf"""
        elf_file = self.aie_dir / core_name / 'Release' / core_name
        
        if not elf_file.exists():
            return None
        
        info = {
            'program_size': 0,
            'sections': {}
        }
        
        try:
            # Get section sizes
            result = subprocess.run(
                ['readelf', '-S', str(elf_file)],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            for line in result.stdout.split('\n'):
                # Parse section header table
                match = re.search(r'\[\s*\d+\]\s+(\.\w+)\s+\w+\s+[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)', line)
                if match:
                    section_name = match.group(1)
                    size = int(match.group(2), 16)
                    info['sections'][section_name] = size
                    
                    # Program memory sections
                    if section_name in ['.text', '.rodata', '.init', '.fini']:
                        info['program_size'] += size
            
            # Try to get symbol table for function sizes
            result = subprocess.run(
                ['readelf', '-s', str(elf_file)],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            functions = {}
            for line in result.stdout.split('\n'):
                # Parse symbol table entries for FUNC type
                match = re.search(r'\s+\d+:\s+([0-9a-f]+)\s+(\d+)\s+FUNC\s+\w+\s+\w+\s+\w+\s+(.+)', line)
                if match:
                    addr = match.group(1)
                    size = int(match.group(2))
                    name = match.group(3).strip()
                    if size > 0:
                        functions[name] = {
                            'address': f'0x{addr}',
                            'size': size
                        }
            
            info['functions'] = functions
            
        except subprocess.TimeoutExpired:
            print(f"Warning: readelf timeout for {elf_file}")
        except FileNotFoundError:
            print("Warning: readelf not found in PATH")
        except Exception as e:
            print(f"Warning: Error running readelf on {elf_file}: {e}")
        
        return info
    
    def get_stack_info(self, core_name):
        """Extract stack usage from log files"""
        log_file = self.aie_dir / core_name / f'{core_name}.log'
        
        stack_info = {
            'required': None,
            'reserved': None,
            'exceeds': False
        }
        
        if not log_file.exists():
            return stack_info
        
        try:
            with open(log_file, 'r') as f:
                content = f.read()
                
                # Look for stack warnings
                match = re.search(r'required stack size:\s*(\d+)', content, re.IGNORECASE)
                if match:
                    stack_info['required'] = int(match.group(1))
                
                match = re.search(r'reserved stack area:.*?(\d+)\.\.(\d+)', content, re.IGNORECASE)
                if match:
                    start = int(match.group(1))
                    end = int(match.group(2))
                    stack_info['reserved'] = end - start
                
                if 'stack size exceeds' in content.lower():
                    stack_info['exceeds'] = True
        
        except Exception as e:
            print(f"Warning: Error reading log file {log_file}: {e}")
        
        return stack_info
    
    def analyze_core(self, core_name, verbose=False):
        """Perform comprehensive analysis of a single core"""
        if verbose:
            print(f"\nAnalyzing core: {core_name}")
        
        core_info = {
            'name': core_name,
            'program_size': 0,
            'functions': {},
            'stack': {},
            'status': 'OK'
        }
        
        # Get ELF information
        elf_info = self.get_elf_info(core_name)
        if elf_info:
            core_info['program_size'] = elf_info['program_size']
            core_info['sections'] = elf_info['sections']
            core_info['functions'] = elf_info.get('functions', {})
        
        # Try map file as fallback/supplement
        map_functions = self.parse_map_file(core_name)
        if map_functions and not core_info['functions']:
            core_info['functions'] = map_functions
        
        # Get stack information
        core_info['stack'] = self.get_stack_info(core_name)
        
        # Determine status
        if core_info['program_size'] > self.pm_limit:
            core_info['status'] = 'EXCEEDS LIMIT'
            core_info['overflow'] = core_info['program_size'] - self.pm_limit
        elif core_info['stack']['exceeds']:
            core_info['status'] = 'STACK OVERFLOW'
        
        return core_info
    
    def print_summary(self, cores_info):
        """Print a formatted summary of all cores"""
        print("\n" + "="*80)
        print("AIE MEMORY USAGE SUMMARY")
        print(f"Work Directory: {self.work_dir}")
        print("="*80)
        
        # Overall summary
        print(f"\n{'Core':<10} {'Program Size':<15} {'Limit':<10} {'Status':<15} {'Stack':<20}")
        print("-"*80)
        
        for core_info in sorted(cores_info, key=lambda x: x['program_size'], reverse=True):
            name = core_info['name']
            size = core_info['program_size']
            status = core_info['status']
            
            stack_str = ""
            if core_info['stack']['required']:
                stack_str = f"{core_info['stack']['required']} bytes"
                if core_info['stack']['exceeds']:
                    stack_str += " (EXCEEDS)"
            
            overflow_str = ""
            if status == 'EXCEEDS LIMIT':
                overflow_str = f"(+{core_info['overflow']} bytes)"
            
            print(f"{name:<10} {size:<15} {self.pm_limit:<10} {status:<15} {stack_str:<20}")
            if overflow_str:
                print(f"           {overflow_str}")
        
        # Problem cores
        problem_cores = [c for c in cores_info if c['status'] != 'OK']
        if problem_cores:
            print(f"\n⚠️  {len(problem_cores)} core(s) with issues")
        else:
            print(f"\n✓ All {len(cores_info)} cores within limits")
    
    def print_detailed_core_analysis(self, core_info):
        """Print detailed function-level analysis for a single core"""
        print("\n" + "="*80)
        print(f"DETAILED ANALYSIS: Core {core_info['name']}")
        print("="*80)
        
        print(f"\nTotal Program Size: {core_info['program_size']} bytes")
        print(f"Limit: {self.pm_limit} bytes")
        print(f"Status: {core_info['status']}")
        if 'overflow' in core_info:
            print(f"Overflow: +{core_info['overflow']} bytes ({core_info['overflow']/self.pm_limit*100:.1f}% over limit)")
        
        # Stack info
        if core_info['stack'].get('required'):
            print(f"\nStack Usage:")
            print(f"  Required: {core_info['stack']['required']} bytes")
            if core_info['stack'].get('reserved'):
                print(f"  Reserved: {core_info['stack']['reserved']} bytes")
            if core_info['stack'].get('exceeds'):
                print(f"  ⚠️  STACK OVERFLOW DETECTED")
        
        # Section breakdown
        if 'sections' in core_info and core_info['sections']:
            print(f"\nSection Breakdown:")
            print(f"{'Section':<20} {'Size':>10}")
            print("-"*32)
            for section, size in sorted(core_info['sections'].items(), key=lambda x: x[1], reverse=True):
                if size > 0:
                    print(f"{section:<20} {size:>10} bytes")
        
        # Function breakdown
        if not core_info['functions']:
            print("\n(No function-level data available)")
            return
        
        print(f"\nAll Functions ({len(core_info['functions'])} total):")
        print(f"{'Function':<60} {'Size':>10} {'% of PM':>8}")
        print("-"*80)
        
        # Sort functions by size
        sorted_funcs = sorted(
            core_info['functions'].items(),
            key=lambda x: x[1]['size'],
            reverse=True
        )
        
        total_accounted = 0
        for fname, finfo in sorted_funcs:
            size = finfo['size']
            pct = (size / self.pm_limit) * 100 if self.pm_limit > 0 else 0
            total_accounted += size
            
            # Demangle C++ names for readability
            display_name = self._demangle_function_name(fname)
            if len(display_name) > 57:
                display_name = display_name[:54] + '...'
            
            print(f"{display_name:<60} {size:>10} {pct:>7.1f}%")
        
        print("-"*80)
        print(f"{'Total Accounted':<60} {total_accounted:>10}")
        if core_info['program_size'] > total_accounted:
            unaccounted = core_info['program_size'] - total_accounted
            print(f"{'Unaccounted (data, padding, etc.)':<60} {unaccounted:>10}")
    
    def _demangle_function_name(self, mangled_name):
        """Attempt to demangle C++ function names for readability"""
        # Remove leading underscore patterns
        name = mangled_name
        
        # Try to extract meaningful parts from mangled names
        if name.startswith('_ZN'):
            # C++ mangled name - try basic demangling
            try:
                import subprocess
                result = subprocess.run(
                    ['c++filt', name],
                    capture_output=True,
                    text=True,
                    timeout=1
                )
                if result.returncode == 0:
                    return result.stdout.strip()
            except:
                pass
        
        # Fallback: just clean up common patterns
        if '::' in name:
            parts = name.split('::')
            # Return last 2-3 parts for namespace::class::function
            return '::'.join(parts[-3:]) if len(parts) > 3 else name
        
        return name
    
    def print_function_breakdown(self, cores_info, top_n=10):
        """Print top functions by size for problem cores"""
        problem_cores = [c for c in cores_info if c['status'] == 'EXCEEDS LIMIT']
        
        if not problem_cores:
            return
        
        print("\n" + "="*80)
        print("TOP FUNCTIONS BY SIZE (for cores exceeding limit)")
        print("="*80)
        
        for core_info in problem_cores:
            print(f"\nCore: {core_info['name']} (Total: {core_info['program_size']} bytes)")
            print("-"*60)
            
            if not core_info['functions']:
                print("  (No function-level data available)")
                continue
            
            # Sort functions by size
            sorted_funcs = sorted(
                core_info['functions'].items(),
                key=lambda x: x[1]['size'],
                reverse=True
            )[:top_n]
            
            print(f"{'Function':<40} {'Size':<10} {'Address':<12}")
            print("-"*60)
            for fname, finfo in sorted_funcs:
                # Clean up function name (remove namespaces for readability)
                clean_name = fname.split('::')[-1] if '::' in fname else fname
                if len(clean_name) > 37:
                    clean_name = clean_name[:34] + '...'
                
                print(f"{clean_name:<40} {finfo['size']:<10} {finfo['address']:<12}")
    
    def export_json(self, cores_info, output_file):
        """Export detailed analysis to JSON"""
        output = {
            'work_directory': str(self.work_dir),
            'pm_limit': self.pm_limit,
            'cores': cores_info
        }
        
        with open(output_file, 'w') as f:
            json.dump(output, f, indent=2)
        
        print(f"\nDetailed analysis exported to: {output_file}")
    
    def run(self, json_output=None, verbose=False, detail_core=None):
        """Run the complete analysis"""
        cores = self.find_cores()
        
        if not cores:
            print(f"No AIE cores found in {self.work_dir}")
            return 1
        
        # If detail_core specified, analyze only that core
        if detail_core:
            if detail_core not in cores:
                print(f"Error: Core '{detail_core}' not found in {self.work_dir}")
                print(f"Available cores: {', '.join(sorted(cores)[:10])}...")
                return 1
            
            print(f"Analyzing single core: {detail_core}")
            core_info = self.analyze_core(detail_core, verbose=True)
            self.print_detailed_core_analysis(core_info)
            return 0
        
        print(f"Found {len(cores)} AIE cores")
        
        cores_info = []
        for core_name in cores:
            core_info = self.analyze_core(core_name, verbose=verbose)
            cores_info.append(core_info)
        
        self.print_summary(cores_info)
        self.print_function_breakdown(cores_info)
        
        if json_output:
            self.export_json(cores_info, json_output)
        
        # Return non-zero if any cores have issues
        problem_cores = [c for c in cores_info if c['status'] != 'OK']
        return 1 if problem_cores else 0


def main():
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Analyze AIE core memory usage from compilation output',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Analyze default Work directory
  %(prog)s
  
  # Analyze specific work directory
  %(prog)s Work_token_arch_v0
  
  # Export detailed analysis to JSON
  %(prog)s Work --json memory_analysis.json
  
  # Analyze multiple architectures
  %(prog)s Work Work_sim Work_token_arch_v0
        """
    )
    
    parser.add_argument(
        'work_dirs',
        nargs='*',
        default=['Work'],
        help='Work directory/directories to analyze (default: Work)'
    )
    
    parser.add_argument(
        '--core',
        metavar='CORE_NAME',
        help='Analyze a single core in detail (e.g., 30_2, 0_0). Shows all inner functions.'
    )
    
    parser.add_argument(
        '--json',
        metavar='FILE',
        help='Export detailed analysis to JSON file'
    )
    
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Verbose output'
    )
    
    args = parser.parse_args()
    
    exit_code = 0
    
    for work_dir in args.work_dirs:
        if not os.path.exists(work_dir):
            print(f"Error: Directory not found: {work_dir}")
            exit_code = 1
            continue
        
        analyzer = AIEMemoryAnalyzer(work_dir)
        result = analyzer.run(json_output=args.json, verbose=args.verbose, detail_core=args.core)
        exit_code = max(exit_code, result)
    
    return exit_code


if __name__ == '__main__':
    sys.exit(main())
