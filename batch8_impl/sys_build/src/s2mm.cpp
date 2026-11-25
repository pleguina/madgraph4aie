// ============================================================================
// s2mm.cpp -- AI Engine -> DDR matrix-element result collector.
//
// Mirror of mm2s.cpp. Six instances cover lanes 0..47; lanes 48 and 49 are
// served by the two-stream variant in s2mm2.cpp. NO PACKET SWITCHING: each of the eight input ports is a
// physically separate 32-bit AXI4-Stream driven by exactly one lane's `out_<l>`
// PLIO port. No packet-merge node, no packet IDs.
//
// Each lane emits one |M|^2 float per phase-space point, in bursts of 8 (one
// per batch slot) per graph invocation. Results are written lane-interleaved,
//   word( p, l ) = mem[ (p * 8) + l ]
// so the round-robin drain below walks DDR linearly.
//
// A single process performs all writes, for the same reason mm2s uses a single
// reader: HLS forbids multiple dataflow processes sharing one m_axi bundle.
// Round-robin blocking reads are safe here because all lanes run the identical
// kernel at the identical initiation interval -- the full-grid cycle-accurate
// run measured a lane-to-lane II spread of 0.014% -- and the AI Engine output
// FIFOs absorb the residual skew.
// ============================================================================
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

typedef ap_axiu<32, 0, 0, 0> pkt_t;

#define NSTRM 8

extern "C" void s2mm(ap_uint<32>* mem,
                     hls::stream<pkt_t>& s0, hls::stream<pkt_t>& s1,
                     hls::stream<pkt_t>& s2, hls::stream<pkt_t>& s3,
                     hls::stream<pkt_t>& s4, hls::stream<pkt_t>& s5,
                     hls::stream<pkt_t>& s6, hls::stream<pkt_t>& s7,
                     int nme_per_lane)
{
#pragma HLS interface m_axi port=mem offset=slave bundle=gmem depth=4096
#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=s4
#pragma HLS interface axis port=s5
#pragma HLS interface axis port=s6
#pragma HLS interface axis port=s7
#pragma HLS interface s_axilite port=mem          bundle=control
#pragma HLS interface s_axilite port=nme_per_lane bundle=control
#pragma HLS interface s_axilite port=return       bundle=control

    int base = 0;
me_loop:
    for (int p = 0; p < nme_per_lane; p++) {
#pragma HLS pipeline II=8
        mem[base + 0] = s0.read().data;
        mem[base + 1] = s1.read().data;
        mem[base + 2] = s2.read().data;
        mem[base + 3] = s3.read().data;
        mem[base + 4] = s4.read().data;
        mem[base + 5] = s5.read().data;
        mem[base + 6] = s6.read().data;
        mem[base + 7] = s7.read().data;
        base += NSTRM;
    }
}
