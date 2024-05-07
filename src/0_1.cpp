#include <fstream>
#include <iostream>

int main() {
    const int size = 64;  // Matrix size 64x64
    std::ofstream outfile("output.txt");

    if (!outfile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return 1;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == 0 || j == 0) {
                outfile << "1";  // First row and first column are zeros
            } else {
                outfile << "0";  // All other positions are ones
            }

            if (j != size - 1) {
                outfile << " ";  // Add a space after each number except the last in the row
            }
        }
        outfile << "\n";  // New line at the end of each row
    }

    outfile.close();  // Close the file after writing
    return 0;
}
