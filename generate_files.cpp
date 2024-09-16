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
        // cout<<word<<"\n";
        string test_line = current_line + " " + word;
        // cout<<test_line<<"\n ------------ \n";
        // cout<<test_line.c_str()<<"\n";
        // Check if the width of the text exceeds the cell width
        // cout<<HPDF_Page_TextWidth(page, test_line.c_str())<<" "<<cell_width<<"\n";
        if (HPDF_Page_TextWidth(page, test_line.c_str()) <= cell_width/7) {
            current_line = test_line;
        } else {
            // cout<<current_line<<"\n \n";
            string break_line = "";
            for(auto &it:current_line){
                break_line+=it;
                if(break_line.size()>cell_width/7){
                    // cout<<break_line<<"\n \n";
                    lines.push_back(break_line);
                    break_line="";
                }
            }
            current_line = break_line;
            // cout<<break_line;
            lines.push_back(current_line);  // Push the current line if it exceeds the width
            current_line = word;  // Start a new line with the current word
        }
    }

    // Add the last line
    if (!current_line.empty()) {
        // cout<<"This is check" << current_line <<"\n";
        lines.push_back(current_line);
    }

    return lines;
}

// Function to generate a PDF with multiple tables from JSON data with borders and text wrapping
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
float key_col_width = (page_width - 2 * margin) * 0.2;  // 30% of the available width for keys
float value_col_width = (page_width - 2 * margin) * 0.8; // 70% of the available width for values

    // Draw a border around the page
    HPDF_Page_SetLineWidth(page, 2.0);
    HPDF_Page_SetRGBStroke(page, 0.0, 0.0, 0.0);
    HPDF_Page_Rectangle(page, margin - 20, margin, page_width + 30 - 2 * margin, page_height + 25 - 2 * margin);
    HPDF_Page_Stroke(page);

    float current_y_position = page_height - margin - 20;
    float line_height = 18;
    float cell_padding = 5;
    float col_width = (page_width - 2 * margin) / 2;  // 2 columns for key-value pairs

    // Loop over the table(s) in the JSON data
    for (const auto& table : jsonData) {
        const Json::Value& rows = table["rows"];

        // Loop over the rows in the table
        // Loop over the rows in the table
for (const auto& row : rows) {
    float max_cell_height = 0;
    vector<vector<string>> wrapped_lines;

    // First pass: calculate the maximum height for the row based on text wrapping
    for (int i = 0; i < row.size(); ++i) {
        string cell_content = row[i].asString();
        float cell_width = (i == 0) ? key_col_width : value_col_width;  // Use different widths for key and value


        vector<string> wrapped = wrapText(cell_content, font, font_size, cell_width - 2 * cell_padding , page);
        for(int i=0;i<wrapped.size();i++){
            if(wrapped[i].empty()){
                wrapped.erase(wrapped.begin()+i);
                // cout<<"yes"<<wrapped[i]<<"yes\n";
            }
        }
        wrapped_lines.push_back(wrapped);

        float cell_height = (wrapped.size() * line_height) + 2 * cell_padding;
        if (cell_height > max_cell_height) {
            max_cell_height = cell_height;
        }
    }

    float x_position = margin;  // Starting x position for each row

    // Second pass: draw the key-value pair
    for (int i = 0; i < row.size(); ++i) {
        const auto& wrapped = wrapped_lines[i];
        string cell_content = row[i].asString();

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

    // Stop writing if no space left on the page
    if (current_y_position - max_cell_height < margin) {
        break;
    }
}


        current_y_position -= line_height;  // Add space between tables (if more than one)
    }

    HPDF_SaveToFile(pdf, pdf_filename.c_str());
    HPDF_Free(pdf);

    cout << "PDF with tables and properly aligned cells created successfully: " << pdf_filename << endl;
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