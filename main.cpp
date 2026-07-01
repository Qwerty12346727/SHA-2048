#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <thread>
#include <mutex>

constexpr size_t STATE_BYTES = 256;
constexpr size_t BLOCK_SIZE = 4096;

std::mutex cout_mutex;

inline uint8_t CHOOSE(uint8_t e, uint8_t f, uint8_t g) {
    return (e & f) ^ (~e & g);
}

inline uint8_t MAJORITY(uint8_t a, uint8_t b, uint8_t c) {
    return (a & b) ^ (a & c) ^ (b & c);
}

inline uint8_t ROTATE_LEFT(uint8_t val, int shift) {
    return (val << shift) | (val >> (8 - shift));
}

void print_progress(uint64_t processed, uint64_t total) {
    int bar_width = 30;
    float progress = (total > 0) ? (float)processed / total : 1.0f;
    
    std::cout << "\r[";
    int pos = bar_width * progress;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% (" 
              << processed << "/" << total << " bytes) " << std::flush;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: sha2048 <file_path>\n";
        return 1;
    }

    std::string file_path = argv[1];
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open target file.\n";
        return 1;
    }

    uint64_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> internal_state(STATE_BYTES);
    for (size_t i = 0; i < STATE_BYTES; ++i) {
        internal_state[i] = static_cast<uint8_t>((i * 101) % 256);
    }

    unsigned int total_threads = std::thread::hardware_concurrency();
    if (total_threads == 0) total_threads = 4; // Fallback
    
    std::vector<uint8_t> buffer(BLOCK_SIZE);
    uint64_t total_processed_bytes = 0;
    int loop_counter = 0;

    while (file.read(reinterpret_cast<char*>(buffer.data()), BLOCK_SIZE) || file.gcount() > 0) {
        size_t bytes_read = file.gcount();
        total_processed_bytes += bytes_read;

        if (loop_counter % 5 == 0 && bytes_read > 3) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "\n[MATH EVENT] Round " << loop_counter << " Operations Log:\n"
                      << "  -> CHOOSE(0x" << std::hex << (int)buffer[0] << ", 0x" << (int)buffer[1] << ", 0x" << (int)buffer[2] << ") = 0x" << (int)CHOOSE(buffer[0], buffer[1], buffer[2]) << "\n"
                      << "  -> MAJORITY(0x" << (int)internal_state[0] << ", 0x" << (int)internal_state[1] << ", 0x" << (int)internal_state[2] << ") = 0x" << (int)MAJORITY(internal_state[0], internal_state[1], internal_state[2]) << "\n"
                      << "  -> ROTATE_LEFT(0x" << (int)buffer[3] << ", 3) = 0x" << (int)ROTATE_LEFT(buffer[3], 3) << std::dec << "\n\n";
        }

        // Isolated state storage per thread
        std::vector<std::vector<uint8_t>> thread_states(total_threads, std::vector<uint8_t>(STATE_BYTES, 0));
        std::vector<std::thread> workers;

        for (unsigned int t = 0; t < total_threads; ++t) {
            workers.emplace_back([&, t, bytes_read]() {
                // STRIDE PATTERN: Thread 't' strictly processes index t, t+total_threads, t+2*total_threads...
                // This completely eliminates atomic racing and guarantees unchangeable workloads.
                for (size_t idx = t; idx < bytes_read; idx += total_threads) {
                    uint8_t data_byte = buffer[idx];
                    size_t state_target = idx % STATE_BYTES;

                    for (int round = 0; round < 500; ++round) { 
                        thread_states[t][state_target] ^= CHOOSE(data_byte, static_cast<uint8_t>(idx * 31), thread_states[t][(state_target + 1) % STATE_BYTES]);
                        thread_states[t][(state_target + 7) % STATE_BYTES] += MAJORITY(thread_states[t][state_target], data_byte, 13);
                        thread_states[t][state_target] = ROTATE_LEFT(thread_states[t][state_target], 3);
                    }
                }
            });
        }

        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }

        // Merging state data sequentially
        for (unsigned int t = 0; t < total_threads; ++t) {
            for (size_t i = 0; i < STATE_BYTES; ++i) {
                internal_state[i] ^= thread_states[t][i];
            }
        }

        print_progress(total_processed_bytes, file_size);
        loop_counter++;
    }

    for (size_t i = 0; i < STATE_BYTES; ++i) {
        internal_state[i] = ROTATE_LEFT(internal_state[i] ^ internal_state[(i + 13) % STATE_BYTES], 5);
        internal_state[i] += internal_state[(i + 97) % STATE_BYTES];
    }

    std::cout << "\n\nSHA-2048 Hash Output Completed Successfully:\n";
    std::cout << "\033[1;32m"; 
    for (size_t i = 0; i < STATE_BYTES; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)internal_state[i];
    }
    std::cout << "\033[0m\n\n"; 

    return 0;
}
