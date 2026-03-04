#include "mm2s_pkt_gen.h"

void mm2s_pkt_gen(
    const float *psp_buffer,
    hls::stream<axis_pkt> &axis_out,
    unsigned int num_packets
) {
#pragma HLS INTERFACE m_axi port=psp_buffer offset=slave bundle=gmem0 depth=160 \
    num_read_outstanding=16 max_read_burst_length=256 latency=64
#pragma HLS INTERFACE axis port=axis_out
#pragma HLS INTERFACE s_axilite port=psp_buffer bundle=control
#pragma HLS INTERFACE s_axilite port=num_packets bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Hard limit for your design (pktsplit<8>, 8 rows per column-group)
    const int MAX_PKTS = 8;
    const int PSP_FLOATS = 20;

    // Clamp to safe max (avoid out-of-bounds if host passes >8 by mistake)
    unsigned int np = (num_packets > MAX_PKTS) ? MAX_PKTS : num_packets;

    // ------------------------------------------------------------------------
    // Phase 1: Preload all PSPs in one contiguous DDR walk
    // ------------------------------------------------------------------------
    float psp_cache[MAX_PKTS][PSP_FLOATS];
#pragma HLS ARRAY_PARTITION variable=psp_cache dim=2 cyclic factor=4
    // dim=2 partition is optional; helps if compiler wants parallel access later.

    // Read np*20 floats contiguously from DDR
    // This is the key: one large linear walk enables efficient bursts.
    READ_ALL:
    for (unsigned int idx = 0; idx < np * PSP_FLOATS; idx++) {
#pragma HLS PIPELINE II=1
        unsigned int p = idx / PSP_FLOATS;
        unsigned int i = idx - (p * PSP_FLOATS);
        psp_cache[p][i] = psp_buffer[idx];
    }

    // ------------------------------------------------------------------------
    // Phase 2: Emit packets (header + 20 data words)
    // ------------------------------------------------------------------------
    SEND_ALL:
    for (unsigned int pkt_id = 0; pkt_id < np; pkt_id++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8

        // Header beat
        axis_pkt header_beat;
        header_beat.data = generate_packet_header(pkt_id);
        header_beat.keep = 0xF;
        header_beat.last = 0;
        axis_out.write(header_beat);

        // Data beats
        SEND_PSP:
        for (int i = 0; i < PSP_FLOATS; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=20 max=20
            axis_pkt data_beat;
            data_beat.data = float_to_bits(psp_cache[pkt_id][i]);
            data_beat.keep = 0xF;
            data_beat.last = (i == PSP_FLOATS - 1) ? 1 : 0;
            axis_out.write(data_beat);
        }
    }
}
