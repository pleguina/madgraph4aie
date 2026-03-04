// ============================================================================
// mm2s_pkt_gen_tb.cpp
//
// Testbench for MM2S Packet Generator
// Validates packet format matches AIE simulator expectations
// ============================================================================

#include "mm2s_pkt_gen.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

// Union for float/int reinterpretation
union float_int {
    float f;
    unsigned int i;
};

// Generate test PSP data (5 particles × 4-vector)
void generate_test_psp(float *psp_buffer, unsigned int num_packets) {
    // Simple test pattern: each packet has different values
    for (unsigned int pkt = 0; pkt < num_packets; pkt++) {
        for (int i = 0; i < 20; i++) {
            // Exactly representable values (N + 0.5)
            psp_buffer[pkt * 20 + i] = (float)(pkt * 100 + i) + 0.5f;
        }
    }
}

// Verify packet header format + parity matches the kernel generator
bool verify_packet_header(unsigned int header, unsigned int expected_id) {
    unsigned int packet_id  = header & 0x1F;
    unsigned int reserved1  = (header >> 5) & 0x7F;
    unsigned int pkt_type   = (header >> 12) & 0x7;
    unsigned int reserved2  = (header >> 15) & 0x1;
    unsigned int src_row    = (header >> 16) & 0x1F;
    unsigned int src_col    = (header >> 21) & 0x7F;
    unsigned int reserved3  = (header >> 28) & 0x7;
    unsigned int parity_bit = (header >> 31) & 0x1;

    cout << "  Packet ID: " << packet_id << " (expected: " << expected_id << ")" << endl;
    cout << "  Reserved1: 0x" << hex << reserved1 << " (expect 0x0)" << endl;
    cout << "  Pkt Type: " << dec << pkt_type << " (expect 0)" << endl;
    cout << "  Reserved2: " << reserved2 << " (expect 1)" << endl;
    cout << "  Src Row: 0x" << hex << src_row << " (expect 0x1F)" << endl;
    cout << "  Src Col: 0x" << src_col << " (expect 0x7F)" << endl;
    cout << "  Reserved3: 0x" << reserved3 << " (expect 0x0)" << endl;
    cout << "  Parity: " << dec << parity_bit << endl;

    // Parity check EXACTLY as in generate_packet_header():
    // p = XOR(bits[30:0]); parity_bit must be (~p)&1
    unsigned int p = 0;
    for (int i = 0; i < 31; i++) {
        p ^= (header >> i) & 1u;
    }
    bool parity_ok = (parity_bit == ((~p) & 1u));

    bool ok = (packet_id == expected_id) &&
              (reserved1 == 0) &&
              (pkt_type == 0) &&
              (reserved2 == 1) &&
              (src_row == 0x1F) &&
              (src_col == 0x7F) &&
              (reserved3 == 0) &&
              parity_ok;

    if (!parity_ok) {
        cout << "  ERROR: Parity check failed!" << endl;
    }
    if (!ok) {
        cout << "  ERROR: Header validation failed!" << endl;
    }

    return ok;
}

int main() {
    const unsigned int NUM_PACKETS = 8;
    const unsigned int PSP_FLOATS  = 20;
    const unsigned int EXPECTED_BEATS = NUM_PACKETS * (1 + PSP_FLOATS);

    cout << "========================================" << endl;
    cout << "MM2S Packet Generator Testbench" << endl;
    cout << "========================================" << endl;

    // Allocate and initialize test data
    float *psp_buffer = new float[NUM_PACKETS * PSP_FLOATS];
    generate_test_psp(psp_buffer, NUM_PACKETS);

    cout << "\nGenerated " << NUM_PACKETS << " test PSPs" << endl;

    // AXI-Stream output
    hls::stream<axis_pkt> axis_out("axis_out");

    // Run DUT
    cout << "\nRunning DUT..." << endl;
    mm2s_pkt_gen(psp_buffer, axis_out, NUM_PACKETS);
    cout << "DUT completed." << endl;

    // Verify output
    bool all_ok = true;

    // Read exactly expected beats (and fail if stream underflows)
    for (unsigned int pkt = 0; pkt < NUM_PACKETS; pkt++) {
        cout << "\n--- Packet " << pkt << " ---" << endl;

        // Header
        if (axis_out.empty()) {
            cout << "ERROR: Stream ended early (missing header)" << endl;
            all_ok = false;
            break;
        }
        axis_pkt header_beat = axis_out.read();
        cout << "Header: 0x" << hex << header_beat.data << dec << endl;

        bool header_ok = verify_packet_header((unsigned int)header_beat.data, pkt);
        all_ok &= header_ok;

        if (header_beat.last != 0) {
            cout << "ERROR: Header has TLAST=1 (should be 0)" << endl;
            all_ok = false;
        }

        // Data beats
        cout << "PSP data (first 5 values):" << endl;
        for (int i = 0; i < (int)PSP_FLOATS; i++) {
            if (axis_out.empty()) {
                cout << "ERROR: Stream ended early (missing data beat " << i << ")" << endl;
                all_ok = false;
                break;
            }

            axis_pkt data_beat = axis_out.read();

            float expected = psp_buffer[pkt * PSP_FLOATS + i];
            float_int exp_c;
            exp_c.f = expected;

            bool data_ok = ((unsigned int)data_beat.data == exp_c.i);

            if (i < 5) {
                float_int rx;
                rx.i = (unsigned int)data_beat.data;
                cout << "  [" << i << "] " << rx.f
                     << " (expected: " << expected << ")"
                     << (data_ok ? " OK" : " ERROR") << endl;
            }

            if (!data_ok) {
                cout << "ERROR: Data mismatch at packet " << pkt << ", index " << i << endl;
                all_ok = false;
            }

            bool last_expected = (i == (int)PSP_FLOATS - 1);
            if ((bool)data_beat.last != last_expected) {
                cout << "ERROR: TLAST mismatch at packet " << pkt << ", index " << i
                     << " (got " << (int)data_beat.last
                     << ", expected " << (int)last_expected << ")" << endl;
                all_ok = false;
            }
        }
    }

    // Check for extra unexpected beats
    if (!axis_out.empty()) {
        cout << "ERROR: Stream has extra beats after expected output!" << endl;
        all_ok = false;
    }

    delete[] psp_buffer;

    cout << "\n========================================" << endl;
    cout << (all_ok ? "TESTBENCH PASSED" : "TESTBENCH FAILED") << endl;
    cout << "========================================" << endl;

    return all_ok ? 0 : 1;
}
