// ============================================================================
// mm2s.cpp -- DDR -> AI Engine phase-space-point data mover.
//
// NO PACKET SWITCHING. Each of the eight output ports is a physically separate
// 32-bit AXI4-Stream feeding exactly one lane's `psp_in_<l>` PLIO port,
// matching src/graph_batch8_grid.h one-for-one. The only thing shared between
// the ports is the AXI-MM read port into DDR; the streams themselves are never
// multiplexed, so no packet header, packet ID or packet-split node exists
// anywhere in this design.
//
// Six instances cover lanes 0..47 (instance i serves lanes 8i .. 8i+7);
// lanes 48 and 49 are served by the two-stream variant in mm2s2.cpp.
//
// Memory layout is PSP-major and lane-interleaved:
//   word( p, l, w ) = mem[ ((p * 8) + l) * 20 + w ]
// so the round-robin delivery below walks DDR strictly linearly and every read
// burst is sequential. A single process performs all reads: HLS forbids
// multiple dataflow processes sharing one m_axi bundle, and the bandwidth here
// does not need them -- one lane consumes 8 PSP x 80 B per 163.6 us initiation
// interval, i.e. 3.9 MB/s, so all eight together need 31 MB/s against the
// 1.25 GB/s this port sustains at 312.5 MHz.
// ============================================================================
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

typedef ap_axiu<32, 0, 0, 0> pkt_t;

// One phase-space point = 20 single-precision floats
//   (E, px, py, pz) x (g1, g2, t, tbar, g3)
#define PSP_WORDS 20
#define NSTRM 8

static inline void emit_psp(const ap_uint<32>* src, hls::stream<pkt_t>& s,
                            bool last_psp) {
emit_psp_loop:
    for (int w = 0; w < PSP_WORDS; w++) {
#pragma HLS pipeline II=1
        pkt_t p;
        p.data = src[w];
        p.keep = -1;
        p.strb = -1;
        p.last = (last_psp && w == PSP_WORDS - 1);
        s.write(p);
    }
}

extern "C" void mm2s(const ap_uint<32>* mem,
                     hls::stream<pkt_t>& s0, hls::stream<pkt_t>& s1,
                     hls::stream<pkt_t>& s2, hls::stream<pkt_t>& s3,
                     hls::stream<pkt_t>& s4, hls::stream<pkt_t>& s5,
                     hls::stream<pkt_t>& s6, hls::stream<pkt_t>& s7,
                     int npsp_per_lane)
{
#pragma HLS interface m_axi port=mem offset=slave bundle=gmem depth=16384
#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface axis port=s2
#pragma HLS interface axis port=s3
#pragma HLS interface axis port=s4
#pragma HLS interface axis port=s5
#pragma HLS interface axis port=s6
#pragma HLS interface axis port=s7
#pragma HLS interface s_axilite port=mem           bundle=control
#pragma HLS interface s_axilite port=npsp_per_lane bundle=control
#pragma HLS interface s_axilite port=return        bundle=control

    int base = 0;
psp_loop:
    for (int p = 0; p < npsp_per_lane; p++) {
        const bool last = (p == npsp_per_lane - 1);
        emit_psp(mem + base, s0, last); base += PSP_WORDS;
        emit_psp(mem + base, s1, last); base += PSP_WORDS;
        emit_psp(mem + base, s2, last); base += PSP_WORDS;
        emit_psp(mem + base, s3, last); base += PSP_WORDS;
        emit_psp(mem + base, s4, last); base += PSP_WORDS;
        emit_psp(mem + base, s5, last); base += PSP_WORDS;
        emit_psp(mem + base, s6, last); base += PSP_WORDS;
        emit_psp(mem + base, s7, last); base += PSP_WORDS;
    }
}
