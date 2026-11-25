// ============================================================================
// s2mm2.cpp -- AI Engine -> DDR matrix-element result collector, two streams.
//
// Two-stream variant of s2mm.cpp, serving lanes 48 and 49 of the 50-lane grid
// (the six eight-stream s2mm instances serve lanes 0..47). Same code with two
// input ports instead of eight. NO PACKET SWITCHING: each port is a
// physically separate 32-bit AXI4-Stream driven by exactly one lane's
// `out_<l>` PLIO port.
//
// Results are written lane-interleaved,
//   word( p, l ) = mem[ (p * 2) + l ]
// so the round-robin drain below walks DDR linearly. Round-robin blocking
// reads are safe because all lanes run at the same initiation interval
// (lane-to-lane II spread 0.014 % in the 50-lane cycle-accurate run).
// ============================================================================
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

typedef ap_axiu<32, 0, 0, 0> pkt_t;

#define NSTRM 2

extern "C" void s2mm2(ap_uint<32>* mem,
                     hls::stream<pkt_t>& s0, hls::stream<pkt_t>& s1,
                     int nme_per_lane)
{
#pragma HLS interface m_axi port=mem offset=slave bundle=gmem depth=4096
#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface s_axilite port=mem          bundle=control
#pragma HLS interface s_axilite port=nme_per_lane bundle=control
#pragma HLS interface s_axilite port=return       bundle=control

    int base = 0;
me_loop:
    for (int p = 0; p < nme_per_lane; p++) {
#pragma HLS pipeline II=2
        mem[base + 0] = s0.read().data;
        mem[base + 1] = s1.read().data;
        base += NSTRM;
    }
}
