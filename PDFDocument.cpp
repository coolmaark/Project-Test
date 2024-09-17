#include "PDFDocument.h"
#include "Utilities.h"
#include <iostream>
#include <sstream>
using namespace std;
PDFDocument::PDFDocument() {}

PDFDocument::~PDFDocument() {}

void PDFDocument::error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    std::cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << std::endl;
}

void PDFDocument::addNewPage(HPDF_Doc pdf, HPDF_Page& page, HPDF_Font font, float font_size, float margin, float& current_y_position, int page_number) {
    page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Page_SetFontAndSize(page, font, font_size);
    current_y_position = HPDF_Page_GetHeight(page) - margin;

    // Optionally, draw a border
    float page_width = HPDF_Page_GetWidth(page);
    float page_height = HPDF_Page_GetHeight(page);

    HPDF_Page_SetLineWidth(page, 2.0);
    HPDF_Page_Rectangle(page, margin - 20, margin, page_width - 2 * margin, page_height - 2 * margin);
    HPDF_Page_Stroke(page);

    // Page number
    std::ostringstream page_num_str;
    page_num_str << "Page " << page_number;

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, 12);
    float text_width = HPDF_Page_TextWidth(page, page_num_str.str().c_str());
    HPDF_Page_MoveTextPos(page, (page_width / 2) - (text_width / 2), margin - 10);
    HPDF_Page_ShowText(page, page_num_str.str().c_str());
    HPDF_Page_EndText(page);
}

std::vector<std::string> PDFDocument::wrapText(const std::string& text, HPDF_Font font, float font_size, float cell_width, HPDF_Page page) {
    std::vector<std::string> lines;
    std::string current_line;
    std::istringstream words(text);
    std::string word;

    HPDF_Page_SetFontAndSize(page, font, font_size);

    while (words >> word) {
        std::string test_line = current_line + " " + word;
        if (HPDF_Page_TextWidth(page, test_line.c_str()) <= cell_width) {
            current_line = test_line;
        } else {
            lines.push_back(current_line);
            current_line = word;
        }
    }

    if (!current_line.empty()) {
        lines.push_back(current_line);
    }

    return lines;
}

void PDFDocument::generatePDFWithTables(const Json::Value& jsonData, const std::string& pdf_filename) {
    // Function implementation
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        cerr << "Failed to create PDF object." << endl;
        return;
    }

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    // Set font for the page
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize(page, font, 12);

    // Example: Add a simple text to verify functionality
    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, 20);
    HPDF_Page_MoveTextPos(page, 50, HPDF_Page_GetHeight(page) - 50);
    HPDF_Page_ShowText(page, "This is a test.");
    HPDF_Page_EndText(page);

    // Save the PDF
    HPDF_SaveToFile(pdf, pdf_filename.c_str());
    HPDF_Free(pdf);

    cout << "PDF created successfully: " << pdf_filename << endl;
}
