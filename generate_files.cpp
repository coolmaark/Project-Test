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

// Function to wrap text within a given width
vector<string> wrapText(const string& text, HPDF_Font font, float font_size, float cell_width, HPDF_Page page) {
    vector<string> lines;
    string current_line = "";
    istringstream words(text);
    string word;

    // Set font and size for accurate width calculation
    HPDF_Page_SetFontAndSize(page, font, font_size);

    while (words >> word) {
        string test_line = current_line + " " + word;
        if (HPDF_Page_TextWidth(page, test_line.c_str()) <= cell_width / 6) {
            current_line = test_line;
        } else {
            string break_line = "";
            for (auto &it : current_line) {
                break_line += it;
                if (break_line.size() > cell_width / 6) {
                    lines.push_back(break_line);
                    break_line = "";
                }
            }
            current_line = break_line;
            lines.push_back(current_line);  // Push the current line if it exceeds the width
            current_line = word;  // Start a new line with the current word
        }
    }

    // Add the last line
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }

    return lines;
}

// Inside addNewPage function
// Function to add a new page and handle page numbering
void addNewPage(HPDF_Doc pdf, HPDF_Page &page, HPDF_Font font, float font_size, float margin, float &current_y_position, int page_number) {
    page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    // Set font for the new page
    HPDF_Page_SetFontAndSize(page, font, 12);

    // Reset the current y position for the new page
    current_y_position = HPDF_Page_GetHeight(page) - margin - 20;

    // Optionally, draw a border on the new page
    float page_width = HPDF_Page_GetWidth(page);
    float page_height = HPDF_Page_GetHeight(page);

    HPDF_Page_SetLineWidth(page, 2.0);
    HPDF_Page_SetRGBStroke(page, 0.0, 0.0, 0.0);
    HPDF_Page_Rectangle(page, margin - 20, margin, page_width + 30 - 2 * margin, page_height + 25 - 2 * margin);
    HPDF_Page_Stroke(page);

    // Define padding for the page number
    float padding = 20.0;  // Adjust this value as needed

    // Add the page number at the bottom center of the page with padding
    std::ostringstream page_num_str;
    page_num_str << "Page " << page_number;

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, 12);
    float text_width = HPDF_Page_TextWidth(page, page_num_str.str().c_str());
    float text_x_position = (page_width / 2) - (text_width / 2);  // Center the page number horizontally
    float text_y_position = margin - 10 - padding;  // Adjust vertically with padding

    HPDF_Page_MoveTextPos(page, text_x_position, text_y_position);  // Move to position with padding
    HPDF_Page_ShowText(page, page_num_str.str().c_str());
    HPDF_Page_EndText(page);
}

// Function to generate a PDF with multiple tables from JSON data with borders, text wrapping, and page numbers
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
    float font_size = 12;
    HPDF_Page_SetFontAndSize(page, font, font_size);

    // Get page dimensions and define margin for the border
    float page_width = HPDF_Page_GetWidth(page);
    float page_height = HPDF_Page_GetHeight(page);
    float margin = 50;
    float key_col_width = (page_width - 2 * margin) * 0.2;  // 20% of the available width for keys
    float value_col_width = (page_width - 2 * margin) * 0.8; // 80% of the available width for values

    // Page numbering
    int page_number = 1;

    // Draw a border around the page
    HPDF_Page_SetLineWidth(page, 2.0);
    HPDF_Page_SetRGBStroke(page, 0.0, 0.0, 0.0);
    HPDF_Page_Rectangle(page, margin - 20, margin, page_width + 30 - 2 * margin, page_height + 25 - 2 * margin);
    HPDF_Page_Stroke(page);

    // Add the first page number
    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, 10);
    HPDF_Page_MoveTextPos(page, (page_width / 2) - 20, margin - 10 - 20);  // Center the page number
    HPDF_Page_ShowText(page, ("Page " + std::to_string(page_number)).c_str());
    HPDF_Page_EndText(page);

    // Load and place images on the top left and top right
    HPDF_Image img_left = HPDF_LoadJpegImageFromFile(pdf, "left_image.jpeg");
    HPDF_Image img_right = HPDF_LoadJpegImageFromFile(pdf, "left_image.jpeg");
    HPDF_Page_DrawImage(page, img_left, margin, page_height - 110, 60, 60);  // Adjust position and size
    HPDF_Page_DrawImage(page, img_right, page_width - margin - 80, page_height - 110, 60, 60);

    // Add "Hello" text in the top middle between the images
    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, 20);
    float hello_text_width = HPDF_Page_TextWidth(page, "Hello");
    HPDF_Page_MoveTextPos(page, (page_width / 2) - (hello_text_width / 2), page_height - 70);  // Adjust position
    HPDF_Page_ShowText(page, "Hello");
    HPDF_Page_EndText(page);

    float current_y_position = page_height - margin - 120;  // Adjust after placing images and text
    float line_height = 18;
    float cell_padding = 5;
    float col_width = (page_width - 2 * margin) / 2;  // 2 columns for key-value pairs

    // Loop over the table(s) in the JSON data
    for (const auto& table : jsonData) {
        const Json::Value& rows = table["rows"];

        // Loop over the rows in the table
        for (const auto& row : rows) {
            float max_cell_height = 0;
            vector<vector<string>> wrapped_lines;

            // First pass: calculate the maximum height for the row based on text wrapping
            for (int i = 0; i < row.size(); ++i) {
                string cell_content = row[i].asString();
                float cell_width = (i == 0) ? key_col_width : value_col_width;  // Use different widths for key and value

                vector<string> wrapped = wrapText(cell_content, font, font_size, cell_width - 2 * cell_padding, page);
                wrapped_lines.push_back(wrapped);

                float cell_height = (wrapped.size() * line_height) + 2 * cell_padding;
                if (cell_height > max_cell_height) {
                    max_cell_height = cell_height;
                }
            }

            // Check if adding the entire row would exceed the bottom margin
            if (current_y_position - max_cell_height < margin) {
                // If the row doesn't fit, move to the next page
                addNewPage(pdf, page, font, font_size, margin, current_y_position, ++page_number);
            }

            float x_position = margin;  // Starting x position for each row

            // Second pass: draw the key-value pair
            for (int i = 0; i < row.size(); ++i) {
                const auto& wrapped = wrapped_lines[i];
                float cell_width = (i == 0) ? key_col_width : value_col_width;  // Apply different widths

                // Draw border for each cell
                HPDF_Page_SetLineWidth(page, 1.0);
                HPDF_Page_Rectangle(page, x_position, current_y_position - max_cell_height, cell_width, max_cell_height);
                HPDF_Page_Stroke(page);

                // Draw text inside the cell
                float text_y_position = current_y_position - cell_padding - line_height;
                for (const auto& line : wrapped) {
                    HPDF_Page_BeginText(page);
                    HPDF_Page_MoveTextPos(page, x_position + cell_padding, text_y_position);
                    HPDF_Page_ShowText(page, line.c_str());
                    HPDF_Page_EndText(page);
                    text_y_position -= line_height;  // Move down for the next wrapped line
                }

                x_position += cell_width;  // Move to the next column (key or value)
            }

            // Move down by the max height of the current row
            current_y_position -= max_cell_height;
        }

        current_y_position -= line_height;  // Add space between tables (if more than one)
    }

    HPDF_SaveToFile(pdf, pdf_filename.c_str());
    HPDF_Free(pdf);

    cout << "PDF with tables, page numbers, images, and hello text created successfully: " << pdf_filename << endl;
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
