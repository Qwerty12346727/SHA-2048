#include <iostream>
#include <string>
#include "sha2048.h"

std::mutex cout_mutex; // Allocates structural thread lock memory

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage Error:\n"
                  << "  Normal Mode   : sha2048 <file_path>\n"
                  << "  Checksum Mode : sha2048 --checksum <file_1> <file_2>\n";
        return 1;
    }

    std::string primary_mode = argv[1];

    // ==========================================
    // ROUTE A: Dual-File Checksum Verification
    // ==========================================
    if (primary_mode == "--checksum") {
        if (argc < 4) {
            std::cerr << "Error: Checksum execution requires two distinct file path arguments.\n"
                      << "Format: sha2048 --checksum \"file1.py\" \"file2.py\"\n";
            return 1;
        }

        std::string path1 = argv[2];
        std::string path2 = argv[3];

        std::cout << "Validating storage configurations...\n";
        std::cout << "Processing File 1: " << path1 << "\n";
        std::string hash1 = compute_sha2048(path1, true); // Runs quiet to keep layout clean

        std::cout << "Processing File 2: " << path2 << "\n";
        std::string hash2 = compute_sha2048(path2, true);

        if (hash1 == "ERROR_OPEN_FAILED" || hash2 == "ERROR_OPEN_FAILED") {
            std::cerr << "\n\033[1;31mError: One or both target file paths are invalid.\033[0m\n";
            return 1;
        }

        std::cout << "\n-----------------------------------------------------------\n";
        if (hash1 == hash2) {
            std::cout << "\033[1;32mSuccess\033[0m\n";
            std::cout << "Both files are identical.\n";
            std::cout << "Hash: \033[32m" << hash1 << "\033[0m\n";
        } else {
            std::cout << "\033[1;31mWarning\033[0m\n";
            std::cout << "The target files do not match.\n\n";
            std::cout << "File 1 Hash: \033[31m" << hash1 << "\033[0m\n";
            std::cout << "File 2 Hash: \033[31m" << hash2 << "\033[0m\n";
        }
        std::cout << "-----------------------------------------------------------\n";
        return 0;
    }

    // ==========================================
    // ROUTE B: Normal Mode (Your Exact OG Output)
    // ==========================================
    std::string target_file = primary_mode;
    std::string hash_out = compute_sha2048(target_file, false);

    if (hash_out == "ERROR_OPEN_FAILED") {
        std::cerr << "Error: Could not open target file.\n";
        return 1;
    }

    std::cout << "\n\nSHA-2048 Hash Output Completed Successfully:\n";
    std::cout << "\033[1;32m" << hash_out << "\033[0m\n\n";

    return 0;
}
