// ============================================================================
// mm2s_pkt_gen.h
//
// Header for MM2S Packet Generator HLS kernel
// Generates AIE packet-switched stream from DDR PSP data
// ============================================================================

#ifndef MM2S_PKT_GEN_H
#define MM2S_PKT_GEN_H

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

// AXI-Stream 32-bit packet (matches AIE stream switch native width)
typedef ap_axiu<32, 0, 0, 0> axis_pkt;

// PSP data structure (5 particles × 4-vector = 20 floats)
struct psp_data_t {
    float p[5][4];  // p[particle][px, py, pz, E]
};

// ============================================================================
// Packet Header Generation (matches generate_packet_data.py)
// ============================================================================

inline ap_uint<32> generate_packet_header(ap_uint<5> packet_id) {
    #pragma HLS INLINE
    
    ap_uint<32> header = 0;
    
    // Bit layout (per Versal AIE packet format):
    //   [4:0]   - Packet ID (0-31)
    //   [11:5]  - Reserved (7'b0)
    //   [14:12] - Packet Type (0 for data)
    //   [15]    - Reserved (1'b1)
    //   [20:16] - Source Row (0x1F = -1 for PL origin)
    //   [27:21] - Source Column (0x7F = -1 for PL origin)
    //   [30:28] - Reserved (3'b0)
    //   [31]    - Parity bit (odd parity of bits 30:0)
    
    header.range(4, 0)   = packet_id;        // Packet ID
    header.range(11, 5)  = 0;                // Reserved (must be 0)
    header.range(14, 12) = 0;                // Packet type
    header[15]           = 1;                // Reserved (must be 1)
    header.range(20, 16) = 0x1F;             // Source row (-1)
    header.range(27, 21) = 0x7F;             // Source column (-1)
    header.range(30, 28) = 0;                // Reserved
    
    // Calculate odd parity for bits [30:0]
    ap_uint<1> parity = 0;
    for (int i = 0; i < 31; i++) {
        #pragma HLS UNROLL
        parity ^= header[i];
    }
    header[31] = ~parity;  // Odd parity (invert to make total 1s odd)
    
    return header;
}

// ============================================================================
// Float to integer reinterpretation (not cast!)
// ============================================================================

inline ap_uint<32> float_to_bits(float f) {
    #pragma HLS INLINE
    union {
        float f;
        unsigned int i;
    } converter;
    converter.f = f;
    return ap_uint<32>(converter.i);
}

// ============================================================================
// Top-level kernel function
// ============================================================================

void mm2s_pkt_gen(
    const float *psp_buffer,         // DDR input (m_axi)
    hls::stream<axis_pkt> &axis_out, // AXI-Stream output
    unsigned int num_packets         // Control parameter
);

#endif // MM2S_PKT_GEN_H
