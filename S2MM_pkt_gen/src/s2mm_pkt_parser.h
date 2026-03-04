// ============================================================================
// s2mm_pkt_parser.h
//
// Header for S2MM Packet Parser HLS kernel
// Parses AIE packet-switched output and writes results to DDR
// ============================================================================

#ifndef S2MM_PKT_PARSER_H
#define S2MM_PKT_PARSER_H

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

// AXI-Stream 32-bit packet (matches AIE stream switch native width)
typedef ap_axiu<32, 0, 0, 0> axis_pkt;

// ============================================================================
// Packet Header Parsing
// ============================================================================

inline ap_uint<5> extract_packet_id(ap_uint<32> header) {
    #pragma HLS INLINE
    // Packet ID is in bits [4:0]
    return header.range(4, 0);
}

// ============================================================================
// Integer to float reinterpretation (not cast!)
// ============================================================================

inline float bits_to_float(ap_uint<32> bits) {
    #pragma HLS INLINE
    union {
        unsigned int i;
        float f;
    } converter;
    converter.i = (unsigned int)bits;
    return converter.f;
}

// Top-level kernel function
void s2mm_pkt_parser(
    hls::stream<axis_pkt> &axis_in,  // AXI-Stream input from AIE
    float *result_buffer,             // DDR output (m_axi)
    unsigned int num_packets          // Control parameter
);

#endif // S2MM_PKT_PARSER_H
