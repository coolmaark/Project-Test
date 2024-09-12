#include <hpdf.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <json/json.h>  // jsoncpp header

using namespace std;

// Error handler for Haru PDF library
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << endl;
}

// Function to read JSON data from a file using jsoncpp
Json::Value readJson(const string& filename) {
    ifstream file(filename, ifstream::binary);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return Json::nullValue;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    string errors;

    if (!Json::parseFromStream(builder, file, &root, &errors)) {
        cerr << "Failed to parse JSON: " << errors << endl;
        return Json::nullValue;
    }

    return root;
}

// Function to generate a PDF with multiple tables from JSON data
void generatePDFWithTables(const Json::Value& jsonData, const string& pdf_filename) {
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        cerr << "Failed to create PDF object." << endl;
        return;
    }

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    // Set font for the text
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize(page, font, 12);

    // Get page dimensions and define margin for the border
    float page_width = HPDF_Page_GetWidth(page);
    float page_height = HPDF_Page_GetHeight(page);
    float margin = 50;

    // Draw a border around the page
    HPDF_Page_SetLineWidth(page, 2.0);
    HPDF_Page_SetRGBStroke(page, 0.0, 0.0, 0.0);
    HPDF_Page_Rectangle(page, margin - 20, margin, page_width + 30 - 2 * margin, page_height + 25 - 2 * margin);
    HPDF_Page_Stroke(page);

    // Start writing the tables inside the page
    HPDF_Page_BeginText(page);
    HPDF_Page_MoveTextPos(page, margin, page_height - margin - 20);

    float current_y_position = page_height - margin - 20;
    float line_height = 18;

    // Loop over tables in JSON data
    for (const auto& table : jsonData) {
        // Check if the current element is an object with "name" and "rows" fields
        if (!table.isMember("name") || !table.isMember("rows")) {
            cerr << "Invalid table format in JSON." << endl;
            continue;
        }

        // Draw table name or heading
        string table_name = table["name"].asString();
        HPDF_Page_ShowText(page, ("Table: " + table_name).c_str());
        HPDF_Page_MoveTextPos(page, 0, -line_height);
        current_y_position -= line_height;

        // Draw table rows
        const Json::Value& rows = table["rows"];
        for (const auto& row : rows) {
            string row_content;
            for (const auto& cell : row) {
                row_content += cell.asString() + "   ";  // Add some spacing between cells
            }

            if (current_y_position - line_height < margin) {
                break;  // Stop writing if no space left
            }

            HPDF_Page_ShowText(page, row_content.c_str());
            HPDF_Page_MoveTextPos(page, 0, -line_height);
            current_y_position -= line_height;
        }

        HPDF_Page_MoveTextPos(page, 0, -line_height);  // Add space between tables
        current_y_position -= line_height;
    }

    HPDF_Page_EndText(page);
    HPDF_SaveToFile(pdf, pdf_filename.c_str());
    HPDF_Free(pdf);

    cout << "PDF with tables created successfully: " << pdf_filename << endl;
}

int main() {
    string input_filename = "data.json";  // The JSON file containing table data
    string pdf_filename = "output.pdf";   // The output PDF file

    Json::Value data = readJson(input_filename);

    if (!data.isNull()) {
        generatePDFWithTables(data, pdf_filename);
    } else {
        cerr << "No data read from file." << endl;
    }

    return 0;
}
