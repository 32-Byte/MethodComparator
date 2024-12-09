#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>  // For std::abs and std::sqrt
#include <algorithm> // For std::max
#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TColor.h"

#define RED TColor::GetColor(255, 0, 0)  // Red
#define BLUE TColor::GetColor(0, 0, 255) // Blue

namespace disp {

    // Function to read values from a file
    void readValues2(const std::string& filePath, std::vector<double>& values) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return;
        }

        double value;
        while (file >> value) {
            values.push_back(value);
        }
        file.close();
    }

    // Function to calculate the Mean Absolute Deviation (MAD)
    double calculateMAD(const std::vector<double>& values) {
        double sum = 0.0;
        for (const double& val : values) {
            sum += std::abs(val);
        }
        return sum / values.size();
    }

    // Function to calculate the Root Mean Square (RMS)
    double calculateRMS(const std::vector<double>& values) {
        double sum = 0.0;
        for (const double& val : values) {
            sum += val * val;
        }
        return std::sqrt(sum / values.size());
    }

    int disp(const std::string& file1, const std::string& file2) {
        // Vectors to store the values from both files
        std::vector<double> valuesFile1;
        std::vector<double> valuesFile2;

        // Read the values from both files
        readValues2(file1, valuesFile1);
        readValues2(file2, valuesFile2);

        // Check if files have data
        if (valuesFile1.empty() || valuesFile2.empty()) {
            std::cerr << "Error: One or both files are empty!" << std::endl;
            return 1;
        }

        // Calculate MAD and RMS for both files
        double madFile1 = calculateMAD(valuesFile1);
        double rmsFile1 = calculateRMS(valuesFile1);
        double madFile2 = calculateMAD(valuesFile2);
        double rmsFile2 = calculateRMS(valuesFile2);

        // Print the results
        std::cout << "File: " << file1 << "\n";
        std::cout << "MAD: " << madFile1 << ", RMS: " << rmsFile1 << "\n\n";
        std::cout << "File: " << file2 << "\n";
        std::cout << "MAD: " << madFile2 << ", RMS: " << rmsFile2 << "\n\n";

        // Determine which file has values closer to 0
        if (madFile1 < madFile2) {
            std::cout << "File '" << file1 << "' has values closer to zero (MAD).\n";
        } else {
            std::cout << "File '" << file2 << "' has values closer to zero (MAD).\n";
        }

        if (rmsFile1 < rmsFile2) {
            std::cout << "File '" << file1 << "' has values closer to zero (RMS).\n";
        } else {
            std::cout << "File '" << file2 << "' has values closer to zero (RMS).\n";
        }

        // Create histograms
        int maxLines = std::max(valuesFile1.size(), valuesFile2.size());
        TH1F hist1("hist1", "Values from Before;Line Number;Value", maxLines, 0, maxLines);
        TH1F hist2("hist2", "Values from After;Line Number;Value", maxLines, 0, maxLines);

        for (size_t i = 0; i < valuesFile1.size(); ++i) {
            hist1.SetBinContent(i + 1, valuesFile1[i]);
        }
        for (size_t i = 0; i < valuesFile2.size(); ++i) {
            hist2.SetBinContent(i + 1, valuesFile2[i]);
        }

        // Create a canvas to draw the histograms
        TCanvas canvas("c", "Histograms", 8000, 6000);
        hist1.SetLineColor(RED);
        hist2.SetLineColor(BLUE);
        hist2.SetLineStyle(2); 

        hist1.Draw("HIST");
        hist2.Draw("HIST SAME");

        // Save histograms to a ROOT file
        TFile outputFile("histograms.root", "RECREATE");
        if (!outputFile.IsOpen()) {
            std::cerr << "Error creating ROOT file!" << std::endl;
            return 1;
        }

        hist1.Write();
        hist2.Write();
        outputFile.Close();

        // Save the canvas as an image
        canvas.SaveAs("histograms.png");

        std::cout << "Histograms saved to histograms.root" << std::endl;
        std::cout << "Canvas saved as histograms.png" << std::endl;

        return 0;
    }

} // namespace disp

int main(int argc, char* argv[]) {
    // Check if we have enough arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file1> <file2>\n";
        return 1;
    }

    // Get the file names from the command-line arguments
    std::string file1 = argv[1];
    std::string file2 = argv[2];

    // Call the disp function with the file names
    return disp::disp(file1, file2);
}

