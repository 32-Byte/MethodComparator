#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream> // Add this line

// Struct to store value with its original index
struct ValueIndex {
    Double_t value;
    Int_t originalIndex;
};

// Function to read values from a file
std::vector<Double_t> readFileValues(const TString& filePath) {
    std::ifstream file(filePath.Data());
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filePath << std::endl;
        return {};
    }

    std::vector<Double_t> values;
    Double_t value;
    while (file >> value) {
        values.push_back(value);
    }

    if (values.empty()) {
        std::cerr << "No values found in file: " << filePath << std::endl;
    }

    return values;
}

// Function to print min and max values
void printMinMaxValues(const std::vector<Double_t>& values, const TString& fileName) {
    if (values.empty()) return;

    auto minIt = std::min_element(values.begin(), values.end());
    auto maxIt = std::max_element(values.begin(), values.end());

    std::cout << "File: " << fileName << std::endl;
    std::cout << "  Minimum value: " << *minIt 
              << " (index: " << std::distance(values.begin(), minIt) << ")" << std::endl;
    std::cout << "  Maximum value: " << *maxIt 
              << " (index: " << std::distance(values.begin(), maxIt) << ")" << std::endl;
}

// Function to create sorted indices
std::vector<ValueIndex> createSortedIndices(const std::vector<Double_t>& values) {
    std::vector<ValueIndex> indexedValues;
    for (size_t i = 0; i < values.size(); ++i) {
        indexedValues.push_back({values[i], (Int_t)i});
    }
    
    // Sort based on value
    std::sort(indexedValues.begin(), indexedValues.end(), 
        [](const ValueIndex& a, const ValueIndex& b) { 
            return a.value < b.value; 
        });

    return indexedValues;
}

// Function to reorder values based on sorted indices
std::vector<Double_t> reorderValues(const std::vector<Double_t>& originalValues, 
                                    const std::vector<ValueIndex>& sortedIndices) {
    std::vector<Double_t> reorderedValues(originalValues.size());
    for (size_t i = 0; i < sortedIndices.size(); ++i) {
        reorderedValues[i] = originalValues[sortedIndices[i].originalIndex];
    }
    return reorderedValues;
}

// Main comparison function
void compareFiles(const TString& file1, const TString& file2, Bool_t sortFirstFile = kTRUE) {
    // Read values from files
    std::vector<Double_t> valuesFile1 = readFileValues(file1);
    std::vector<Double_t> valuesFile2 = readFileValues(file2);

    // Validate file sizes
    if (valuesFile1.size() != valuesFile2.size()) {
        std::cerr << "Error: Files must have equal number of values" << std::endl;
        return;
    }

    // Print original min/max
    printMinMaxValues(valuesFile1, file1);
    printMinMaxValues(valuesFile2, file2);

    // Determine which file to sort and which to reorder
    const std::vector<Double_t>& sortValues = sortFirstFile ? valuesFile1 : valuesFile2;
    const std::vector<Double_t>& reorderValuesData = sortFirstFile ? valuesFile2 : valuesFile1;
    TString sortFileName = sortFirstFile ? file1 : file2;
    TString reorderFileName = sortFirstFile ? file2 : file1;

    // Create sorted indices and reordered values
    auto sortedIndices = createSortedIndices(sortValues);
    auto reorderedOtherValues = reorderValues(reorderValuesData, sortedIndices);

    // Create sorted histogram
    TH1F* histSorted = new TH1F("histSorted", 
        Form("Sorted Values (%s)", sortFileName.Data()), 
        sortedIndices.size(), 0, sortedIndices.size());
    for (size_t i = 0; i < sortedIndices.size(); ++i) {
        histSorted->SetBinContent(i + 1, sortedIndices[i].value);
    }
    histSorted->SetLineColor(kBlue);

    // Create reordered histogram
    TH1F* histReordered = new TH1F("histReordered", 
        Form("Reordered Values (%s)", reorderFileName.Data()), 
        reorderedOtherValues.size(), 0, reorderedOtherValues.size());
    for (size_t i = 0; i < reorderedOtherValues.size(); ++i) {
        histReordered->SetBinContent(i + 1, reorderedOtherValues[i]);
    }
    histReordered->SetLineColor(kRed);

    // Create canvas
    TCanvas* canvas = new TCanvas("canvas", "File Comparison", 1200, 800);
    
    // Draw histograms
    histSorted->Draw("HIST");
    histReordered->Draw("HIST SAME");

    // Add legend
    TLegend* legend = new TLegend(0.7, 0.8, 0.9, 0.9);
    legend->AddEntry(histSorted, sortFileName, "l");
    legend->AddEntry(histReordered, reorderFileName, "l");
    legend->Draw();

    // Save outputs
    canvas->SaveAs("comparison_histogram.png");
    TFile* outputFile = new TFile("comparison_histograms.root", "RECREATE");
    histSorted->Write();
    histReordered->Write();
    outputFile->Close();
}
