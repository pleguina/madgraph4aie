// ============================================================================
// s2mm_pkt_parser_tb.cpp
//
// Testbench for S2MM Packet Parser
// 8 packets per invocation (one per row in a column-group, pktsplit<8>)
// Packet arrival order is NOT guaranteed; DUT writes positionally.
//   Beat 0: header (discarded by DUT)
//   Beat 1: result (float bits), TLAST=1
// ============================================================================

#include "s2mm_pkt_parser.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

// Union for float/int reinterpretation
union float_int {
    float f;
    unsigned int i;
};

int main() {
    cout << "===============================================" << endl;
    cout << " S2MM Packet Parser Testbench (8 pkts/group)" << endl;
    cout << "===============================================" << endl;

    const unsigned int NUM_PACKETS = 8;
    hls::stream<axis_pkt> axis_test_stream("axis_test_stream");
    float result_buffer[NUM_PACKETS];

    // Initialize result buffer to a known value (helps debugging)
    for (unsigned int i = 0; i < NUM_PACKETS; i++) {
        result_buffer[i] = -12345.0f;
    }

    // Test ME² values (simulated AIE outputs)
    float test_me2_values[NUM_PACKETS] = {
        2.093192e-04f,  // Row 0
        3.989184e-04f,  // Row 1
        1.369670e-04f,  // Row 2
        2.199694e-04f,  // Row 3
        6.209597e-05f,  // Row 4
        1.688165e-03f,  // Row 5
        4.127358e-04f,  // Row 6
        3.012456e-04f   // Row 7
    };

    // ===================================
    // Generate test packets (2 beats/packet)
    // ===================================
    cout << "\nGenerating " << NUM_PACKETS << " test packets (2 beats each)..." << endl;

    for (unsigned int pkt_id = 0; pkt_id < NUM_PACKETS; pkt_id++) {
        // Beat 0: Packet header (discarded by DUT; carry realistic ID anyway)
        axis_pkt header_beat;
        header_beat.data = (ap_uint<32>)pkt_id; // bits[4:0] = row ID
        header_beat.keep = 0xF;
        header_beat.last = 0;
        axis_test_stream.write(header_beat);

        // Beat 1: ME² result + TLAST
        axis_pkt result_beat;
        float_int conv;
        conv.f = test_me2_values[pkt_id];
        result_beat.data = (ap_uint<32>)conv.i;
        result_beat.keep = 0xF;
        result_beat.last = 1;
        axis_test_stream.write(result_beat);

        cout << "  Packet " << pkt_id << ": ME² = "
             << scientific << setprecision(6) << test_me2_values[pkt_id] << endl;
    }

    // ===================================
    // Run DUT
    // ===================================
    cout << "\nRunning S2MM Parser..." << endl;
    s2mm_pkt_parser(axis_test_stream, result_buffer, NUM_PACKETS);

    // ===================================
    // Verify results (bit-exact, since we're moving raw float bits)
    // ===================================
    cout << "\nVerifying results (bit-exact):" << endl;
    cout << "--------------------------------------------" << endl;

    int errors = 0;
    for (unsigned int i = 0; i < NUM_PACKETS; i++) {
        float expected = test_me2_values[i];
        float actual   = result_buffer[i];

        float_int exp_c, act_c;
        exp_c.f = expected;
        act_c.f = actual;

        bool ok = (exp_c.i == act_c.i);

        cout << "Result[" << i << "]: "
             << scientific << setprecision(9) << actual;

        if (ok) {
            cout << " ✓ PASS" << endl;
        } else {
            cout << " ✗ FAIL (expected " << expected
                 << ", got_bits=0x" << hex << act_c.i
                 << ", exp_bits=0x" << exp_c.i << dec << ")" << endl;
            errors++;
        }
    }

    // ===================================
    // Check stream fully consumed (no extra beats)
    // ===================================
    if (!axis_test_stream.empty()) {
        cout << "\nERROR: Stream not fully consumed! Extra beats remain." << endl;
        errors++;
    }

    // ===================================
    // Final report
    // ===================================
    cout << "\n===============================================" << endl;
    if (errors == 0) {
        cout << " ✓ ALL TESTS PASSED (" << NUM_PACKETS << "/" << NUM_PACKETS << ")" << endl;
        cout << "===============================================" << endl;
        return 0;
    } else {
        cout << " ✗ TESTS FAILED (" << errors << " errors)" << endl;
        cout << "===============================================" << endl;
        return 1;
    }
}
