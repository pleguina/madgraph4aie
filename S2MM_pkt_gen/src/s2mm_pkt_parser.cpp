// ============================================================================
// s2mm_pkt_parser.cpp
//
// S2MM Packet Parser for AIE Packet-Switched Output
//
// FUNCTIONALITY:
//   - Reads 32-bit AXI-Stream packets from AIE PLIO
//   - Discards header beat, writes result beats to DDR in arrival order
//   - Packet arrival order is NOT guaranteed (stream switch arbitration)
//
// PACKET FORMAT (from pktsplit<8> / k4 output, one PLIO per column-group):
//   Beat 0:    Packet header (discarded)
//   Beat 1:    ME² result (float as 32-bit word), TLAST = 1
//
// NOTE: K4 kernel sends exactly 2 beats per packet.
//   Results are written positionally (first-come-first-served).
//   The host is responsible for re-ordering if needed.
// ============================================================================

#include "s2mm_pkt_parser.h"

void s2mm_pkt_parser(
    hls::stream<axis_pkt> &axis_in,  // AXI-Stream input from AIE
    float *result_buffer,            // DDR output (m_axi)
    unsigned int num_packets          // number of packets (<=8)
) {
#pragma HLS INTERFACE axis port=axis_in
#pragma HLS INTERFACE m_axi port=result_buffer offset=slave bundle=gmem0 depth=8 \
    num_write_outstanding=16 max_write_burst_length=256 latency=64
#pragma HLS INTERFACE s_axilite port=result_buffer bundle=control
#pragma HLS INTERFACE s_axilite port=num_packets bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // pktsplit<8>: 8 rows per column-group, packet IDs 0..7
    const unsigned int MAX_PKTS = 8;
    unsigned int np = (num_packets > MAX_PKTS) ? MAX_PKTS : num_packets;
    unsigned int pkt_idx = 0;  // positional write index; order not guaranteed

    // Total beats = 2 per packet (header + result)
    unsigned int total_beats = np * 2;

    // State machine: track whether next beat is a header or result
    bool expect_result = false;

BEAT_LOOP:
    for (unsigned int b = 0; b < total_beats; b++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=2 max=16

        axis_pkt beat = axis_in.read();

        if (!expect_result) {
            // Header beat: discard (order not tracked).
            expect_result = true;
        } else {
            // Result beat: write positionally (first-come-first-served).
            float me2_result = bits_to_float(beat.data);

            if (beat.last != 1) {
                me2_result = 0.0f;
            }

            if (pkt_idx < np) {
                result_buffer[pkt_idx++] = me2_result;
            }

            expect_result = false;
        }
    }
}
