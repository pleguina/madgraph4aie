#!/usr/bin/env python3
"""
Generate test PSP (Phase Space Point) data for g g → t t̄ g process
Creates binary files with momentum 4-vectors for 5 particles
"""

import struct
import numpy as np
import os

# Constants
NUM_TRUNKS = 10      # column groups, one PLIO each
PSPS_PER_TRUNK = 8   # rows per group (pktsplit<8> limit)
NUM_PARTICLES = 5  # 2 incoming gluons, top, anti-top, outgoing gluon
NUM_COMPONENTS = 4  # E, px, py, pz
PSP_SIZE_FLOATS = NUM_PARTICLES * NUM_COMPONENTS  # 20 floats
PSP_SIZE_BYTES = PSP_SIZE_FLOATS * 4  # 80 bytes

def generate_random_psp():
    """
    Generate a random PSP with physically reasonable values
    Returns array of 20 floats: [E1,px1,py1,pz1, E2,px2,py2,pz2, ...]
    """
    psp = np.zeros(PSP_SIZE_FLOATS, dtype=np.float32)
    
    # Incoming gluons (massless, along beam axis)
    # Gluon 1: moving in +z direction
    E1 = np.random.uniform(100.0, 500.0)  # GeV
    psp[0] = E1    # E
    psp[1] = 0.0   # px
    psp[2] = 0.0   # py
    psp[3] = E1    # pz (E = |p| for massless)
    
    # Gluon 2: moving in -z direction
    E2 = np.random.uniform(100.0, 500.0)  # GeV
    psp[4] = E2    # E
    psp[5] = 0.0   # px
    psp[6] = 0.0   # py
    psp[7] = -E2   # pz (opposite direction)
    
    # Top quark (mass ~173 GeV)
    mt = 173.0
    pt = np.random.uniform(50.0, 200.0)
    phi = np.random.uniform(0, 2*np.pi)
    eta = np.random.uniform(-2.5, 2.5)
    
    px_t = pt * np.cos(phi)
    py_t = pt * np.sin(phi)
    pz_t = pt * np.sinh(eta)
    E_t = np.sqrt(px_t**2 + py_t**2 + pz_t**2 + mt**2)
    
    psp[8] = E_t
    psp[9] = px_t
    psp[10] = py_t
    psp[11] = pz_t
    
    # Anti-top quark
    pt_tbar = np.random.uniform(50.0, 200.0)
    phi_tbar = np.random.uniform(0, 2*np.pi)
    eta_tbar = np.random.uniform(-2.5, 2.5)
    
    px_tbar = pt_tbar * np.cos(phi_tbar)
    py_tbar = pt_tbar * np.sin(phi_tbar)
    pz_tbar = pt_tbar * np.sinh(eta_tbar)
    E_tbar = np.sqrt(px_tbar**2 + py_tbar**2 + pz_tbar**2 + mt**2)
    
    psp[12] = E_tbar
    psp[13] = px_tbar
    psp[14] = py_tbar
    psp[15] = pz_tbar
    
    # Outgoing gluon (massless)
    # Use momentum conservation (approximately)
    px_g = -(px_t + px_tbar) + np.random.uniform(-10, 10)
    py_g = -(py_t + py_tbar) + np.random.uniform(-10, 10)
    pz_g = (pz_t + pz_tbar) + np.random.uniform(-50, 50)
    E_g = np.sqrt(px_g**2 + py_g**2 + pz_g**2)  # massless
    
    psp[16] = E_g
    psp[17] = px_g
    psp[18] = py_g
    psp[19] = pz_g
    
    return psp

def save_psp_trunk(filename, num_psps):
    """Save PSPs for one trunk to binary file"""
    with open(filename, 'wb') as f:
        for i in range(num_psps):
            psp = generate_random_psp()
            # Pack as 20 floats (little-endian)
            data = struct.pack(f'<{PSP_SIZE_FLOATS}f', *psp)
            assert len(data) == PSP_SIZE_BYTES, f"Expected {PSP_SIZE_BYTES} bytes, got {len(data)}"
            f.write(data)
    
    print(f"  Created {filename}: {num_psps} PSPs ({num_psps * PSP_SIZE_BYTES} bytes)")

def print_psp_info(psp):
    """Print PSP information in readable format"""
    print("    Particle momenta (E, px, py, pz):")
    particles = ['g1 (in)', 'g2 (in)', 'top', 'tbar', 'g (out)']
    for i, name in enumerate(particles):
        idx = i * 4
        E, px, py, pz = psp[idx:idx+4]
        print(f"      {name:8s}: E={E:8.2f}, px={px:8.2f}, py={py:8.2f}, pz={pz:8.2f}")

def main():
    # Create data directory
    data_dir = "/home/pelayo/work/vitis_workspace/host_app/data"
    os.makedirs(data_dir, exist_ok=True)
    
    print("=" * 60)
    print(" Generating Test PSP Data")
    print("=" * 60)
    print(f"Process: g g → t t̄ g (gluon-gluon → top-antitop + gluon)")
    print(f"Trunks: {NUM_TRUNKS}")
    print(f"PSPs per trunk: {PSPS_PER_TRUNK}")
    print(f"Total PSPs: {NUM_TRUNKS * PSPS_PER_TRUNK}")
    print(f"Output directory: {data_dir}")
    print()
    
    # Generate data for each trunk
    for trunk in range(NUM_TRUNKS):
        filename = os.path.join(data_dir, f"psp_in_{trunk}.bin")
        save_psp_trunk(filename, PSPS_PER_TRUNK)
    
    print()
    print("✓ Test data generation complete!")
    print()
    
    # Show sample PSP
    print("Sample PSP (Trunk 0, PSP 0):")
    with open(os.path.join(data_dir, "psp_in_0.bin"), 'rb') as f:
        data = f.read(PSP_SIZE_BYTES)
        psp = struct.unpack(f'<{PSP_SIZE_FLOATS}f', data)
        print_psp_info(psp)
    
    print()
    print("To test host application:")
    print(f"  cd /home/pelayo/work/vitis_workspace/host_app")
    print(f"  ./build_x86_test.sh")
    print(f"  ./build/host_x86_test ./data")
    print()

if __name__ == "__main__":
    main()
