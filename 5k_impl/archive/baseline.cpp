#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "processing_core.h"
#include "ggttg_params.h"
#include "ggttg_config.h"

// External debug flag from processing_core.cpp
#if defined(__X86SIM__) || defined(__AIESIM__)
extern bool g_ffv1_debug;
#endif

// ============================================================================
//  ggttg_kernel_A2 : Lean batch kernel (Option A2)
//  --------------------------------------------------------------------------
//  Differences vs. kernel_batch_psp<BATCH>:
//   • Reads ONE PSP at a time directly from the input iterator
//   • Computes |M|^2 and writes ONE float, then proceeds to the next PSP
//   • Avoids `PSP local[BATCH]` on the stack → small, stable stack usage
//   • Keeps the same buffer-port signature & math, so the graph change is
//     just swapping the factory to this symbol and pointing to this source.
//
//  Interface: identical to the original kernel (io_buffer in/out).
//  Constraints: conforms to the AI Engine buffer interface rules you posted
//  (128-bit/64-bit/32-bit PLIO are handled by the graph, extents stay the same).
// ============================================================================


namespace ggttg {

template<int BATCH>
void kernel_batch_psp_A2(
	adf::io_buffer<float, adf::direction::in,
								 adf::io_buffer_config<adf::extents<BATCH * 5 * 4>>>& in,
	adf::io_buffer<float, adf::direction::out,
								 adf::io_buffer_config<adf::extents<BATCH>>>& out)
{
	// Iterators over the buffer ports
	auto in_iter  = aie::begin(in);
	auto out_iter = aie::begin(out);

	// Helicity mask computed once from the first PSP (reused for the batch)
	uint32_t good_mask = 0;
	bool     mask_ready = false;

	for (int b = 0; b < BATCH; ++b) {
		// ---- Load a single PSP (5x4 floats) directly from the input ----
		v4f Pg1, Pg2, Pt, Ptbar, Pg3;
		for (int k = 0; k < 4; ++k) Pg1[k]   = *in_iter++;
		for (int k = 0; k < 4; ++k) Pg2[k]   = *in_iter++;
		for (int k = 0; k < 4; ++k) Pt[k]    = *in_iter++;
		for (int k = 0; k < 4; ++k) Ptbar[k] = *in_iter++;
		for (int k = 0; k < 4; ++k) Pg3[k]   = *in_iter++;

		float  me2_sum = 0.0f;
		// Couplings once per PSP
		const cfloat gc10 = get_gc10();
		const cfloat gc11 = get_gc11();
		const cfloat gc12 = get_gc12();

		// ---- Helicity loop (32 combos) ----
		for (int h = 0; h < 32; ++h) {
			if (mask_ready && !(good_mask & (1u << h))) continue;

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (b == 0 && h < 3) {
				printf("[A2_HEL_DEBUG] PSP 0, h=%d: mask_ready=%d, good_mask=0x%08x, skipped=%d\n", 
					h, mask_ready, good_mask, (mask_ready && !(good_mask & (1u << h))) ? 1 : 0);
			}
#endif

			const int hel_g1 = GET_HEL_G1(h);
			const int hel_g2 = GET_HEL_G2(h);
			const int hel_t  = GET_HEL_T(h);
			const int hel_tb = GET_HEL_TB(h);
			const int hel_g3 = GET_HEL_G3(h);

			// External wavefunctions
			v8c w0 = vxxxxx(Pg1,  0.0f, hel_g1, -1);
			v8c w1 = vxxxxx(Pg2,  0.0f, hel_g2, -1);
			v8c w2 = oxxxxx(Pt,    MT,  hel_t,  +1);
			v8c w3 = ixxxxx(Ptbar, MT,  hel_tb, -1);
			v8c w4 = vxxxxx(Pg3,  0.0f, hel_g3, +1);

			// Internal temps
			v8c w5,w6,w7,w8,w9,w10,w11;
			cfloat amp;
			cfloat jamp[6]; for (int i=0;i<6;++i) jamp[i] = {0.0f, 0.0f};

			// Debug: for first PSP, helicities 0, 1, and 2
			bool debug_this = (b == 0 && h == 0);
			bool debug_hel1 = (b == 0 && h == 1);
			bool debug_hel2 = (b == 0 && h == 2);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_DEBUG] PSP 0, Hel 0 - Helicities: [%d, %d, %d, %d, %d]\n", hel_g1, hel_g2, hel_t, hel_tb, hel_g3);
			}
			if (debug_hel1) {
				printf("[A2_HEL1_DEBUG] PSP 0, Hel 1 - Helicities: [%d, %d, %d, %d, %d]\n", hel_g1, hel_g2, hel_t, hel_tb, hel_g3);
			}
			if (debug_hel2) {
				printf("[A2_HEL2_DEBUG] PSP 0, Hel 2 - Helicities: [%d, %d, %d, %d, %d]\n", hel_g1, hel_g2, hel_t, hel_tb, hel_g3);
			}
#endif

			// ---- Diagrams 1..16 (unchanged math) ----
			// Helper macro for debug printing wavefunctions (ALL 8 LANES for cache validation)
#if defined(__X86SIM__) || defined(__AIESIM__)
#define PRINT_WF(name, wf) if (debug_this) { \
	cfloat _c[8]; for(int _k=0;_k<8;_k++) _c[_k]=wf.get(_k); \
	printf("[A2_DEBUG] %s = [(%.6e,%.6e), (%.6e,%.6e), (%.6e,%.6e), (%.6e,%.6e), (%.6e,%.6e), (%.6e,%.6e), **L6:(%.6e,%.6e), L7:(%.6e,%.6e)**]\n", \
		name, (double)_c[0].real,(double)_c[0].imag,(double)_c[1].real,(double)_c[1].imag, \
		(double)_c[2].real,(double)_c[2].imag,(double)_c[3].real,(double)_c[3].imag, \
		(double)_c[4].real,(double)_c[4].imag,(double)_c[5].real,(double)_c[5].imag, \
		(double)_c[6].real,(double)_c[6].imag,(double)_c[7].real,(double)_c[7].imag); }
#define PRINT_WF_HEL1(name, wf) if (debug_hel1) { \
	cfloat _c[8]; for(int _k=0;_k<8;_k++) _c[_k]=wf.get(_k); \
	printf("[A2_HEL1_WF] %s = [(%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e)]\n", \
		name, (double)_c[0].real,(double)_c[0].imag,(double)_c[1].real,(double)_c[1].imag, \
		(double)_c[2].real,(double)_c[2].imag,(double)_c[3].real,(double)_c[3].imag, \
		(double)_c[4].real,(double)_c[4].imag,(double)_c[5].real,(double)_c[5].imag); }
#define PRINT_WF_HEL2(name, wf) if (debug_hel2) { \
	cfloat _c[8]; for(int _k=0;_k<8;_k++) _c[_k]=wf.get(_k); \
	printf("[A2_HEL2_WF] %s = [(%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e), (%.10e,%.10e)]\n", \
		name, (double)_c[0].real,(double)_c[0].imag,(double)_c[1].real,(double)_c[1].imag, \
		(double)_c[2].real,(double)_c[2].imag,(double)_c[3].real,(double)_c[3].imag, \
		(double)_c[4].real,(double)_c[4].imag,(double)_c[5].real,(double)_c[5].imag); }
#else
#define PRINT_WF(name, wf)
#define PRINT_WF_HEL1(name, wf)
#define PRINT_WF_HEL2(name, wf)
#endif

			// Print external wavefunctions w0-w4
			PRINT_WF("w0=vxxxxx(Pg1)", w0);
			PRINT_WF("w1=vxxxxx(Pg2)", w1);
			PRINT_WF("w2=oxxxxx(Pt)", w2);
			PRINT_WF("w3=ixxxxx(Ptbar)", w3);
			PRINT_WF("w4=vxxxxx(Pg3)", w4);

			// Print external wavefunctions for Hel 1
			PRINT_WF_HEL1("w0=vxxxxx(Pg1)", w0);
			PRINT_WF_HEL1("w1=vxxxxx(Pg2)", w1);
			PRINT_WF_HEL1("w2=oxxxxx(Pt)", w2);
			PRINT_WF_HEL1("w3=ixxxxx(Ptbar)", w3);
			PRINT_WF_HEL1("w4=vxxxxx(Pg3)", w4);

			// Print external wavefunctions for Hel 2
			PRINT_WF_HEL2("w0=vxxxxx(Pg1)", w0);
			PRINT_WF_HEL2("w1=vxxxxx(Pg2)", w1);
			PRINT_WF_HEL2("w2=oxxxxx(Pt)", w2);
			PRINT_WF_HEL2("w3=ixxxxx(Ptbar)", w3);
			PRINT_WF_HEL2("w4=vxxxxx(Pg3)", w4);

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_hel1) {
				printf("[A2_HEL1_DEBUG] PSP 0, Hel 1 - Helicities: [%d, %d, %d, %d, %d]\n", hel_g1, hel_g2, hel_t, hel_tb, hel_g3);
			}
			// Print external wavefunctions [0] element
			if (debug_this) {
				cfloat _w0 = w0.get(0), _w1 = w1.get(0), _w2 = w2.get(0), _w3 = w3.get(0), _w4 = w4.get(0);
				printf("[A2_EXTERNAL] w0[0] = (%.10e, %.10e)\n", (double)_w0.real, (double)_w0.imag);
				printf("[A2_EXTERNAL] w1[0] = (%.10e, %.10e)\n", (double)_w1.real, (double)_w1.imag);
				printf("[A2_EXTERNAL] w2[0] = (%.10e, %.10e)\n", (double)_w2.real, (double)_w2.imag);
				printf("[A2_EXTERNAL] w3[0] = (%.10e, %.10e)\n", (double)_w3.real, (double)_w3.imag);
				printf("[A2_EXTERNAL] w4[0] = (%.10e, %.10e)\n", (double)_w4.real, (double)_w4.imag);
			}
#endif

			// D1: VVV1_0(w5, w6, w4) where w5=VVV1P0_1(w0,w1), w6=FFV1P0_3(w3,w2)
			w5 = VVV1P0_1(w0, w1, gc10, 0.0f, 0.0f);
			w6 = FFV1P0_3(w3, w2, gc11, 0.0f, 0.0f);
			PRINT_WF("w5=VVV1P0_1(w0,w1)", w5);
			PRINT_WF("w6=FFV1P0_3(w3,w2)", w6);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w5_01 = w5.get(0);
				printf("[A2_VVV_PROP] w5_01[0] = (%.10e, %.10e)\n", (double)_w5_01.real, (double)_w5_01.imag);
			}
#endif
			amp = VVV1_0(w5, w6, w4, gc10);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_D1_AMP] D1 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D1 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] -= amp; jamp[2] += amp; jamp[4] += amp; jamp[5] -= amp;

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_AFTER_D1] JAMP after D1 only:\n");
				for (int i=0; i<6; i++) printf("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
			}
#endif

			// D2: FFV1_0(w3, w7, w5) where w7=FFV1_1(w2,w4)
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w2 = w2.get(0), _w4 = w4.get(0);
				printf("[A2_BEFORE_FFV] Before w7=FFV1_1(w2,w4): w2[0]=(%.10e,%.10e), w4[0]=(%.10e,%.10e)\n",
					(double)_w2.real, (double)_w2.imag, (double)_w4.real, (double)_w4.imag);
				// Enable FFV debug for this computation
				g_ffv1_debug = true;
			}
#endif
			w7 = FFV1_1(w2, w4, gc11, MT, WT);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				g_ffv1_debug = false; // Disable after computation
			}
#endif
			PRINT_WF("w7=FFV1_1(w2,w4)", w7);
			amp = FFV1_0(w3, w7, w5, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D2 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D2 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[4] += CI * amp; jamp[5] -= CI * amp;

			// D3: FFV1_0(w8, w2, w5) where w8=FFV1_2(w3,w4)
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w3 = w3.get(0), _w4 = w4.get(0);
				printf("[A2_BEFORE_FFV] Before w8=FFV1_2(w3,w4): w3[0]=(%.10e,%.10e), w4[0]=(%.10e,%.10e)\n",
					(double)_w3.real, (double)_w3.imag, (double)_w4.real, (double)_w4.imag);
			}
#endif
			w8 = FFV1_2(w3, w4, gc11, MT, WT);
			PRINT_WF("w8=FFV1_2(w3,w4)", w8);
			amp = FFV1_0(w8, w2, w5, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D3 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D3 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] += CI * amp; jamp[2] -= CI * amp;

			// D4: FFV1_0(w9, w5, w4) where w5=FFV1_1(w2,w0), w9=FFV1_2(w3,w1)
			// NOTE: w5 is REASSIGNED here (MadGraph's w[9])
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w2 = w2.get(0), _w0 = w0.get(0);
				printf("[A2_BEFORE_FFV] Before w5=FFV1_1(w2,w0): w2[0]=(%.10e,%.10e), w0[0]=(%.10e,%.10e)\n",
					(double)_w2.real, (double)_w2.imag, (double)_w0.real, (double)_w0.imag);
			}
#endif
			w5 = FFV1_1(w2, w0, gc11, MT, WT);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w3 = w3.get(0), _w1 = w1.get(0);
				printf("[A2_BEFORE_FFV] Before w9=FFV1_2(w3,w1): w3[0]=(%.10e,%.10e), w1[0]=(%.10e,%.10e)\n",
					(double)_w3.real, (double)_w3.imag, (double)_w1.real, (double)_w1.imag);
			}
#endif
			w9 = FFV1_2(w3, w1, gc11, MT, WT);
			PRINT_WF("w5=FFV1_1(w2,w0) [MG w9]", w5);
			PRINT_WF("w9=FFV1_2(w3,w1) [MG w10]", w9);
			amp = FFV1_0(w9, w5, w4, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D4 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D4 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[1] -= amp;

			// D5: FFV1_0(w3, w5, w10) where w10=VVV1P0_1(w1,w4)
			w10 = VVV1P0_1(w1, w4, gc10, 0.0f, 0.0f);
			PRINT_WF("w10=VVV1P0_1(w1,w4) [MG w11]", w10);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w10_0 = w10.get(0), _w10_5 = w10.get(5);
				printf("[A2_VVV_PROP] w10[0] = (%.10e, %.10e)\n", (double)_w10_0.real, (double)_w10_0.imag);
				printf("[A2_VVV_PROP] w10[5] = (%.10e, %.10e)\n", (double)_w10_5.real, (double)_w10_5.imag);
			}
#endif
			amp = FFV1_0(w3, w5, w10, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D5 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D5 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] += CI * amp; jamp[1] -= CI * amp;

			// D6: FFV1_0(w8, w5, w1) - uses existing w8 and w5
			amp = FFV1_0(w8, w5, w1, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D6 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D6 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] -= amp;

			// D7: FFV1_0(w5, w11, w4) where w5=FFV1_2(w3,w0), w11=FFV1_1(w2,w1)
			// NOTE: w5 is REASSIGNED here (MadGraph's w[12])
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w3 = w3.get(0), _w0 = w0.get(0);
				printf("[A2_BEFORE_FFV] Before w5=FFV1_2(w3,w0): w3[0]=(%.10e,%.10e), w0[0]=(%.10e,%.10e)\n",
					(double)_w3.real, (double)_w3.imag, (double)_w0.real, (double)_w0.imag);
			}
#endif
			w5  = FFV1_2(w3, w0, gc11, MT, WT);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w2 = w2.get(0), _w1 = w1.get(0);
				printf("[A2_BEFORE_FFV] Before w11=FFV1_1(w2,w1): w2[0]=(%.10e,%.10e), w1[0]=(%.10e,%.10e)\n",
					(double)_w2.real, (double)_w2.imag, (double)_w1.real, (double)_w1.imag);
			}
#endif
			w11 = FFV1_1(w2, w1, gc11, MT, WT);
			PRINT_WF("w5=FFV1_2(w3,w0) [MG w12]", w5);
			PRINT_WF("w11=FFV1_1(w2,w1) [MG w13]", w11);
			amp = FFV1_0(w5, w11, w4, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D7 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D7 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[3] -= amp;

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_AFTER_D7] PSP 0, Hel 0 - JAMP after D2-D7:\n");
				for (int i=0; i<6; i++) printf("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
			}
#endif

			// D8: FFV1_0(w5, w2, w10) - uses existing w5 and w10
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				cfloat _w10_0 = w10.get(0), _w10_5 = w10.get(5), _w2_0 = w2.get(0), _w5_0 = w5.get(0);
				printf("[A2_BEFORE_D8] w10[0] = (%.10e, %.10e)\n", (double)_w10_0.real, (double)_w10_0.imag);
				printf("[A2_BEFORE_D8] w10[5] = (%.10e, %.10e)\n", (double)_w10_5.real, (double)_w10_5.imag);
				printf("[A2_BEFORE_D8] w2[0] = (%.10e, %.10e)\n", (double)_w2_0.real, (double)_w2_0.imag);
				printf("[A2_BEFORE_D8] w5[0] (w5_30) = (%.10e, %.10e)\n", (double)_w5_0.real, (double)_w5_0.imag);
			}
#endif
			amp = FFV1_0(w5, w2, w10, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D8 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D8 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[3] += CI * amp; jamp[5] -= CI * amp;

			// D9: FFV1_0(w5, w7, w1) - uses existing w5 and w7
			amp = FFV1_0(w5, w7, w1, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D9 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D9 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[5] -= amp;

			// D10: FFV1_0(w3, w11, w5) where w5=VVV1P0_1(w0,w4)
			// NOTE: w5 is REASSIGNED here (MadGraph's w[14])
			w5 = VVV1P0_1(w0, w4, gc10, 0.0f, 0.0f);
			PRINT_WF("w5=VVV1P0_1(w0,w4) [MG w14]", w5);
			amp = FFV1_0(w3, w11, w5, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D10 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D10 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[2] += CI * amp; jamp[3] -= CI * amp;

			// D11: FFV1_0(w9, w2, w5) - uses existing w9 and w5
			amp = FFV1_0(w9, w2, w5, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D11 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D11 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[1] += CI * amp; jamp[4] -= CI * amp;

			// D12: VVV1_0(w5, w1, w6) - uses existing w5, w1, w6
			amp = VVV1_0(w5, w1, w6, gc10);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D12 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D12 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[1] += amp; jamp[2] -= amp; jamp[3] += amp; jamp[4] -= amp;

			// D13: FFV1_0(w8, w11, w0) - uses existing w8, w11, w0
			amp = FFV1_0(w8, w11, w0, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D13 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D13 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[2] -= amp;

			// D14: FFV1_0(w9, w7, w0) - uses existing w9, w7, w0
			amp = FFV1_0(w9, w7, w0, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D14 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D14 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[4] -= amp;

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_AFTER_D14] PSP 0, Hel 0 - JAMP after D2-D14:\n");
				for (int i=0; i<6; i++) printf("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
			}
#endif

			// D15: VVV1_0(w0, w10, w6) - uses existing w0, w10, w6
			amp = VVV1_0(w0, w10, w6, gc10);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D15 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D15 amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] += amp; jamp[1] -= amp; jamp[3] -= amp; jamp[5] += amp;

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_AFTER_D15] PSP 0, Hel 0 - JAMP after D1-D15 (before D16):\n");
				for (int i=0; i<6; i++) printf("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
			}
#endif

			// D16 (3 sub-diagrams): 4-gluon vertices
			w10 = VVVV1P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
			w6  = VVVV3P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
			w9  = VVVV4P0_1(w0, w1, w4, gc12, 0.0f, 0.0f);
			PRINT_WF("w10=VVVV1P0_1 [MG w15]", w10);
			PRINT_WF("w6=VVVV3P0_1 [MG w16]", w6);
			PRINT_WF("w9=VVVV4P0_1 [MG w17]", w9);
			amp = FFV1_0(w3, w2, w10, gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D16a amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D16a amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] += amp; jamp[1] -= amp; jamp[3] -= amp; jamp[5] += amp;
			amp = FFV1_0(w3, w2, w6,  gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D16b amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D16b amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[1] -= amp; jamp[2] += amp; jamp[3] -= amp; jamp[4] += amp;
			amp = FFV1_0(w3, w2, w9,  gc11);
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) printf("[A2_DEBUG] D16c amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
			if (debug_hel1) printf("[A2_HEL1_AMP] D16c amp = (%.10e, %.10e)\n", (double)amp.real, (double)amp.imag);
#endif
			jamp[0] -= amp; jamp[2] += amp; jamp[4] += amp; jamp[5] -= amp;

#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_DEBUG] Final JAMP for PSP 0, Hel 0:\n");
				for (int i=0; i<6; i++) printf("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
			}
			if (debug_hel1) {
				printf("[A2_HEL1_DEBUG] Final JAMP for PSP 0, Hel 1:\n");
				for (int i=0; i<6; i++) printf("  jamp[%d] = (%.10e, %.10e)\n", i, (double)jamp[i].real, (double)jamp[i].imag);
			}
#endif

			// Mark helicity as "good" if any color flow is non-zero
			bool nonzero = false;
			for (int i=0;i<6;++i){ if (jamp[i].real!=0.0f || jamp[i].imag!=0.0f){ nonzero = true; break; } }
			if ((!mask_ready || GGTTG_MASK_PER_PSP) && nonzero) good_mask |= (1u << h);

			// Color reduction: full symmetric matrix sum
			// MadGraph formula: sum_i Re( (cf @ jamp)[i] * conj(jamp[i]) ) / denom[i]
			// where denom[i] = 9 for all i (COLOR_DENOM)
			// cf[i][j] is symmetric, compute sum_i sum_j cf[i][j] * Re[jamp[i]* * jamp[j]] / 9
			// = sum_i cf[i][i] * |jamp[i]|^2 / 9 + 2 * sum_{i<j} cf[i][j] * Re[jamp[i]* * jamp[j]] / 9
			float hel_me2 = 0.0f;
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_VERSION_CHECK] *** EXECUTING LATEST A2 KERNEL WITH /COLOR_DENOM FIX ***\n");
				printf("[A2_VERSION_CHECK] COLOR_DENOM = %.1f\n", (double)COLOR_DENOM);
			}
#endif
			for (int i = 0; i < 6; ++i) {
				// Diagonal term: cf[i][i] * |jamp[i]|^2 / COLOR_DENOM
				float diag_contrib = COLOR_MATRIX[i][i] * (jamp[i].real * jamp[i].real + jamp[i].imag * jamp[i].imag) / COLOR_DENOM;
				hel_me2 += diag_contrib;
#if defined(__X86SIM__) || defined(__AIESIM__)
				if (debug_this && i < 2) {
					float before_denom = COLOR_MATRIX[i][i] * (jamp[i].real * jamp[i].real + jamp[i].imag * jamp[i].imag);
					printf("[A2_COLOR_DEBUG] i=%d: |jamp|^2=%.10e, cf[%d][%d]=%.1f, before/9=%.10e, after/9=%.10e\n", 
						i, (double)(jamp[i].real * jamp[i].real + jamp[i].imag * jamp[i].imag),
						i, i, (double)COLOR_MATRIX[i][i], (double)before_denom, (double)diag_contrib);
				}
#endif
				// Off-diagonal terms (upper triangle, factor of 2 for symmetry)
				for (int j = i + 1; j < 6; ++j) {
					const float c = COLOR_MATRIX[i][j];
					// Re[jamp[i]* * jamp[j]] = jamp[i].re * jamp[j].re + jamp[i].im * jamp[j].im
					hel_me2 += 2.0f * c * (jamp[i].real * jamp[j].real + jamp[i].imag * jamp[j].imag) / COLOR_DENOM;
				}
			}
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (debug_this) {
				printf("[A2_DEBUG_FIXED_v3] hel_me2 for PSP 0, Hel 0 = %.10e (AFTER /COLOR_DENOM in loop)\n", (double)hel_me2);
			}
			// Print all helicities for PSP 0
			if (b == 0 && h < 32) {
				printf("[A2_HEL_ME2] PSP 0, Hel %d: hel_me2 = %.10e\n", h, (double)hel_me2);
			}
#endif
			me2_sum += hel_me2;
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (b == 0 && (h == 0 || h == 16 || h == 31)) {
				printf("[A2_HEL_ME2] PSP 0, Hel %d: hel_me2 = %.10e, running sum = %.10e\n", h, (double)hel_me2, (double)me2_sum);
			}
#endif
		} // helicities

#if defined(__X86SIM__) || defined(__AIESIM__)
		if (b == 0) {
			printf("[A2_SUM_DEBUG] PSP 0: me2_sum (before /256) = %.10e\n", (double)me2_sum);
		}
#endif

		if (!mask_ready) {
			mask_ready = true;
			if (good_mask == 0) good_mask = 0xFFFFFFFFu; // fallback: keep all
#if defined(__X86SIM__) || defined(__AIESIM__)
			if (b == 0) {
				printf("[A2_MASK_DEBUG] PSP 0: After first PSP, good_mask = 0x%08x\n", good_mask);
				int ngood = __builtin_popcount(good_mask);
				printf("[A2_MASK_DEBUG] PSP 0: Number of good helicities = %d / 32\n", ngood);
			}
#endif
		}

		// Spin averaging: divide by 256 (spin-color averaging factor)
		// Note: Color averaging by /9 already applied in loop above
		// MadGraph uses denominators[nprocesses] = {256} for g g -> t t~ g
		me2_sum *= (1.0f / 256.0f);
		
#if defined(__X86SIM__) || defined(__AIESIM__)
		if (b == 0) {
			printf("[A2_FINAL_DEBUG] PSP 0: me2_sum (AFTER /256) = %.10e\n", (double)me2_sum);
			printf("[A2_FINAL_DEBUG] *** NORMALIZATION: divided by COLOR_DENOM=9 in loop, then by 256 = total /2304 ***\n");
		}
#endif
		*out_iter++ = me2_sum;
	} // batch PSPs
}

} // namespace ggttg

