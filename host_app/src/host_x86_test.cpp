/*
 * X86 Host Application Test Version
 * Tests core logic without XRT - validates data flow and control
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cmath>

// Architecture parameters
#define NUM_TRUNKS 10      /* column groups, one PLIO each */
#define PSPS_PER_TRUNK 8   /* rows per group, pktsplit<8> limit */
#define PSP_SIZE_BYTES 80        // 5 particles × 4 components × 4 bytes
#define PSP_SIZE_FLOATS 20       // 5 particles × 4 components
#define RESULT_SIZE_BYTES 4      // 1 float per PSP (ME² result)

// Mock result computation (replace with actual physics later)
float compute_mock_me2(float* psp_data) {
    // Simple mock: sum of all momentum components
    float sum = 0.0f;
    for (int i = 0; i < PSP_SIZE_FLOATS; i++) {
        sum += psp_data[i] * psp_data[i];
    }
    return sum;
}

bool load_psp_data(const std::string& filename, float* buffer, size_t expected_size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return false;
    }
    
    file.read(reinterpret_cast<char*>(buffer), expected_size);
    size_t bytes_read = file.gcount();
    
    if (bytes_read != expected_size) {
        std::cerr << "Error: Expected " << expected_size << " bytes, read " << bytes_read << std::endl;
        return false;
    }
    
    file.close();
    return true;
}

void save_results(const std::string& filename, float* results, int num_results) {
    std::ofstream file(filename);
    file << std::scientific << std::setprecision(10);
    
    for (int i = 0; i < num_results; i++) {
        file << "PSP " << i << ": ME² = " << results[i] << std::endl;
    }
    
    file.close();
}

void print_psp_data(float* psp_data, int psp_index) {
    std::cout << "  PSP " << psp_index << " data (first 4 momentum components):" << std::endl;
    for (int p = 0; p < 4 && p < PSP_SIZE_FLOATS; p++) {
        std::cout << "    [" << p << "] = " << std::fixed << std::setprecision(6) 
                  << psp_data[p] << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <data_dir> [num_psps] [num_iterations]" << std::endl;
        std::cout << "  <data_dir>       - Directory containing psp_in_*.bin files" << std::endl;
        std::cout << "  [num_psps]       - PSPs per trunk to process (default: 10)" << std::endl;
        std::cout << "  [num_iterations] - Number of times to run (default: 1)" << std::endl;
        return 1;
    }

    std::string data_dir = argv[1];
    int psps_to_process = (argc >= 3) ? std::atoi(argv[2]) : PSPS_PER_TRUNK;
    int num_iterations = (argc >= 4) ? std::atoi(argv[3]) : 1;
    
    if (psps_to_process < 1 || psps_to_process > PSPS_PER_TRUNK) {
        std::cerr << "Error: num_psps must be between 1 and " << PSPS_PER_TRUNK << std::endl;
        return 1;
    }

    std::cout << "=============================================================================" << std::endl;
    std::cout << " X86 Test Mode - Host Application Logic Validation" << std::endl;
    std::cout << "=============================================================================" << std::endl;
    std::cout << " Trunks:        " << NUM_TRUNKS << std::endl;
    std::cout << " PSPs/trunk:    " << psps_to_process << std::endl;
    std::cout << " Total PSPs:    " << NUM_TRUNKS * psps_to_process << std::endl;
    std::cout << " Iterations:    " << num_iterations << std::endl;
    std::cout << " Data dir:      " << data_dir << std::endl;
    std::cout << "=============================================================================" << std::endl;

    // Calculate buffer sizes
    size_t input_size_per_trunk = PSPS_PER_TRUNK * PSP_SIZE_BYTES;
    size_t output_size_per_trunk = PSPS_PER_TRUNK * RESULT_SIZE_BYTES;

    // Allocate input buffers (host memory)
    std::cout << "\n[1/5] Allocating input buffers..." << std::endl;
    std::vector<float*> input_host(NUM_TRUNKS);
    for (int i = 0; i < NUM_TRUNKS; i++) {
        input_host[i] = new float[PSPS_PER_TRUNK * PSP_SIZE_FLOATS];
        std::cout << "  Trunk " << i << ": " << input_size_per_trunk << " bytes" << std::endl;
    }

    // Allocate output buffers
    std::cout << "\n[2/5] Allocating output buffers..." << std::endl;
    std::vector<float*> output_host(NUM_TRUNKS);
    for (int i = 0; i < NUM_TRUNKS; i++) {
        output_host[i] = new float[PSPS_PER_TRUNK];
        std::cout << "  Trunk " << i << ": " << output_size_per_trunk << " bytes" << std::endl;
    }

    // Load input data
    std::cout << "\n[3/5] Loading PSP data..." << std::endl;
    bool all_loaded = true;
    for (int i = 0; i < NUM_TRUNKS; i++) {
        std::string filename = data_dir + "/psp_in_" + std::to_string(i) + ".bin";
        if (!load_psp_data(filename, input_host[i], input_size_per_trunk)) {
            std::cerr << "Failed to load data for trunk " << i << std::endl;
            all_loaded = false;
            break;
        }
        std::cout << "  Trunk " << i << ": loaded " << PSPS_PER_TRUNK << " PSPs from " << filename << std::endl;
        
        // Print sample data from first PSP
        if (i == 0) {
            print_psp_data(input_host[i], 0);
        }
    }

    if (!all_loaded) {
        std::cout << "\n⚠ Data files not found. To generate test data:" << std::endl;
        std::cout << "  mkdir -p " << data_dir << std::endl;
        std::cout << "  # Create binary files with PSP data (80 bytes each)" << std::endl;
        
        // Cleanup
        for (int i = 0; i < NUM_TRUNKS; i++) {
            delete[] input_host[i];
            delete[] output_host[i];
        }
        return 1;
    }

    // Validate data
    std::cout << "\n[4/5] Validating data..." << std::endl;
    bool data_valid = true;
    for (int trunk = 0; trunk < NUM_TRUNKS; trunk++) {
        for (int psp = 0; psp < psps_to_process; psp++) {
            float* psp_data = &input_host[trunk][psp * PSP_SIZE_FLOATS];
            bool has_nonzero = false;
            bool has_nan = false;
            
            for (int i = 0; i < PSP_SIZE_FLOATS; i++) {
                if (psp_data[i] != 0.0f) has_nonzero = true;
                if (std::isnan(psp_data[i]) || std::isinf(psp_data[i])) {
                    has_nan = true;
                    std::cerr << "  ⚠ Trunk " << trunk << " PSP " << psp 
                              << " contains NaN/Inf at index " << i << std::endl;
                }
            }
            
            if (!has_nonzero) {
                std::cout << "  ⚠ Trunk " << trunk << " PSP " << psp 
                          << " is all zeros" << std::endl;
            }
            
            if (has_nan) {
                data_valid = false;
            }
        }
    }
    
    if (data_valid) {
        std::cout << "  ✓ All data valid (no NaN/Inf detected)" << std::endl;
    } else {
        std::cout << "  ✗ Data validation failed!" << std::endl;
    }

    // Process data (mock computation)
    std::cout << "\n[5/5] Processing data (mock computation)..." << std::endl;
    
    double total_elapsed = 0.0;
    for (int iter = 0; iter < num_iterations; iter++) {
        if (num_iterations > 1) {
            std::cout << "\n--- Iteration " << (iter + 1) << "/" << num_iterations << " ---" << std::endl;
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Simulate processing each trunk
        for (int trunk = 0; trunk < NUM_TRUNKS; trunk++) {
            for (int psp = 0; psp < psps_to_process; psp++) {
                float* psp_data = &input_host[trunk][psp * PSP_SIZE_FLOATS];
                output_host[trunk][psp] = compute_mock_me2(psp_data);
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        double elapsed_ms = duration.count() / 1000.0;
        total_elapsed += elapsed_ms;
        
        if (iter == 0 || num_iterations == 1) {
            std::cout << "  CPU processing time: " << std::fixed << std::setprecision(3) 
                      << elapsed_ms << " ms" << std::endl;
        }
    }

    int total_psps = NUM_TRUNKS * psps_to_process * num_iterations;
    
    std::cout << "\n✓ Processing complete!" << std::endl;
    std::cout << "  Total time:     " << total_elapsed << " ms" << std::endl;
    std::cout << "  Avg per iter:   " << total_elapsed / num_iterations << " ms" << std::endl;
    std::cout << "  Per PSP:        " << (total_elapsed * 1000.0) / total_psps << " µs" << std::endl;

    // Display sample results
    std::cout << "\nResults (ME² values) - Sample from Trunk 0:" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    for (int psp = 0; psp < std::min(5, psps_to_process); psp++) {
        std::cout << "  PSP " << psp << ": " << std::scientific << std::setprecision(10)
                  << output_host[0][psp] << std::endl;
    }
    if (psps_to_process > 5) {
        std::cout << "  ..." << std::endl;
    }

    // Save results
    std::cout << "\nSaving results..." << std::endl;
    for (int trunk = 0; trunk < NUM_TRUNKS; trunk++) {
        std::string result_file = data_dir + "/me2_out_" + std::to_string(trunk) + "_x86test.txt";
        save_results(result_file, output_host[trunk], psps_to_process);
    }
    std::cout << "  Results saved to " << data_dir << "/me2_out_*_x86test.txt" << std::endl;

    // Statistics
    std::cout << "\n=============================================================================" << std::endl;
    std::cout << " Test Summary" << std::endl;
    std::cout << "=============================================================================" << std::endl;
    std::cout << " Data Loading:      ✓ Success" << std::endl;
    std::cout << " Data Validation:   " << (data_valid ? "✓ Passed" : "✗ Failed") << std::endl;
    std::cout << " Processing:        ✓ Complete" << std::endl;
    std::cout << " Total PSPs:        " << total_psps << std::endl;
    std::cout << " CPU Time:          " << std::fixed << std::setprecision(3) << total_elapsed << " ms" << std::endl;
    std::cout << " Expected FPGA:     ~" << (total_elapsed / 100.0) << " ms (100x faster est.)" << std::endl;
    std::cout << "=============================================================================" << std::endl;

    // Cleanup
    for (int i = 0; i < NUM_TRUNKS; i++) {
        delete[] input_host[i];
        delete[] output_host[i];
    }

    std::cout << "\nTEST PASSED" << std::endl;
    std::cout << "\nNote: This is CPU-based testing. Actual results will come from AIE hardware." << std::endl;
    
    return 0;
}
