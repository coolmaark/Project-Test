#include "PDFDocument.h"
#include "JSONParser.h"
#include <iostream>

int main() {
    std::string input_filename = "data.json";  // The JSON file containing table data
    std::string pdf_filename = "output.pdf";   // The output PDF file

    JSONParser parser;
    Json::Value data = parser.readJson(input_filename);

    if (!data.isNull()) {
        PDFDocument pdfDocument;
        pdfDocument.generatePDFWithTables(data, pdf_filename);
    } else {
        std::cerr << "No data read from file." << std::endl;
    }

    return 0;
}
