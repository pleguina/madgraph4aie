// ============================================================================
// mm2s2.cpp -- DDR -> AI Engine phase-space-point data mover, two streams.
//
// Two-stream variant of mm2s.cpp, serving lanes 48 and 49 of the 50-lane grid
// (the six eight-stream mm2s instances serve lanes 0..47). Same code with two
// output ports instead of eight. NO PACKET SWITCHING: each port is a
// physically separate 32-bit AXI4-Stream feeding exactly one lane's
// `psp_in_<l>` PLIO port.
//
// Memory layout is PSP-major and lane-interleaved:
//   word( p, l, w ) = mem[ ((p * 2) + l) * 20 + w ]
// so the round-robin delivery below walks DDR strictly linearly. One lane
// consumes 8 PSP x 80 B per 163.6 us initiation interval (3.9 MB/s).
// ============================================================================
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

typedef ap_axiu<32, 0, 0, 0> pkt_t;

// One phase-space point = 20 single-precision floats
//   (E, px, py, pz) x (g1, g2, t, tbar, g3)
#define PSP_WORDS 20
#define NSTRM 2

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

extern "C" void mm2s2(const ap_uint<32>* mem,
                     hls::stream<pkt_t>& s0, hls::stream<pkt_t>& s1,
                     int npsp_per_lane)
{
#pragma HLS interface m_axi port=mem offset=slave bundle=gmem depth=16384
#pragma HLS interface axis port=s0
#pragma HLS interface axis port=s1
#pragma HLS interface s_axilite port=mem           bundle=control
#pragma HLS interface s_axilite port=npsp_per_lane bundle=control
#pragma HLS interface s_axilite port=return        bundle=control

    int base = 0;
psp_loop:
    for (int p = 0; p < npsp_per_lane; p++) {
        const bool last = (p == npsp_per_lane - 1);
        emit_psp(mem + base, s0, last); base += PSP_WORDS;
        emit_psp(mem + base, s1, last); base += PSP_WORDS;
    }
}
