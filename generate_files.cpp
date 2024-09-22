#include <hpdf.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp> // nlohmann/json header

using json = nlohmann::json; // Using alias for convenience
using namespace std;

class TextWrapper {
public:
    static vector<string> wrapText(const string &text, HPDF_Font font, float font_size, float cell_width, HPDF_Page page) {
        vector<string> lines;
        string current_line;
        istringstream words(text);
        string word;

        HPDF_Page_SetFontAndSize(page, font, font_size);

        while (words >> word) {
            string test_line = current_line + " " + word;
            if (HPDF_Page_TextWidth(page, test_line.c_str()) <= cell_width) {
                current_line = test_line;
            } else {
                if (!current_line.empty()) {
                    lines.push_back(current_line);
                }
                current_line = word;
            }
        }

        if (!current_line.empty()) {
            lines.push_back(current_line);
        }

        return lines;
    }
};

class PDFDocument {
public:
    PDFDocument(const string &filename) 
        : pdf_filename(filename), pdf(HPDF_New(error_handler, nullptr)), current_y_position(0.0f), page_number(1) {
        if (pdf == nullptr) {
            throw runtime_error("Failed to create PDF object.");
        }
        addNewPage();
    }

    ~PDFDocument() {
        HPDF_Free(pdf);
    }

    void generatePDF(const json &jsonData) {
        loadImagesAndText();
        for (const auto &entry : jsonData) {
            drawTableForEntry(entry);
        }

        HPDF_SaveToFile(pdf, pdf_filename.c_str());
    }

private:
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Font font;
    string pdf_filename;
    float font_size = 12.0f;
    float margin = 50.0f;
    float line_height = 18.0f;
    float cell_padding = 5.0f;
    float key_col_width;
    float value_col_width;
    float current_y_position;
    int page_number;

    static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
        cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << endl;
    }

    void addNewPage() {
        page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
        font = HPDF_GetFont(pdf, "Helvetica", nullptr);
        HPDF_Page_SetFontAndSize(page, font, font_size);

        float page_height = HPDF_Page_GetHeight(page);
        current_y_position = page_height - margin; // Reset Y position

        float page_width = HPDF_Page_GetWidth(page);
        key_col_width = (page_width - 2 * margin) * 0.2f;
        value_col_width = (page_width - 2 * margin) * 0.8f;

        drawPageBorder();
        drawPageNumber();
    }

    void drawPageBorder() {
        float page_width = HPDF_Page_GetWidth(page);
        float page_height = HPDF_Page_GetHeight(page);

        HPDF_Page_SetLineWidth(page, 2.0f);
        HPDF_Page_Rectangle(page, margin - 5.0f, margin - 5.0f,
                            page_width - 2 * margin + 10.0f,
                            page_height - 2 * margin + 10.0f);
        HPDF_Page_Stroke(page);
    }

    void loadImagesAndText() {
        HPDF_Image img_left = HPDF_LoadJpegImageFromFile(pdf, "left_image.jpeg");
        HPDF_Image img_right = HPDF_LoadJpegImageFromFile(pdf, "right_image.jpeg");

        float page_height = HPDF_Page_GetHeight(page);
        float page_width = HPDF_Page_GetWidth(page);

        if (img_left != nullptr) {
            float img_left_x = margin;
            float img_left_y = page_height - 110.0f;
            HPDF_Page_DrawImage(page, img_left, img_left_x, img_left_y, 60.0f, 60.0f); // Top left
        }

        if (img_right != nullptr) {
            float img_right_x = page_width - margin - 60.0f;
            float img_right_y = page_height - 110.0f;
            HPDF_Page_DrawImage(page, img_right, img_right_x, img_right_y, 60.0f, 60.0f); // Top right
        }

        const string centered_text = "Name_Head";
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 20.0f);
        float text_width = HPDF_Page_TextWidth(page, centered_text.c_str());
        float text_x_position = (page_width / 2) - (text_width / 2);
        float text_y_position = page_height - 70.0f;
        HPDF_Page_MoveTextPos(page, text_x_position, text_y_position);
        HPDF_Page_ShowText(page, centered_text.c_str());
        HPDF_Page_EndText(page);

        current_y_position -= 80.0f; // Space for images and title (adjust as needed)
    }

    void drawPageNumber() {
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 12.0f);
        stringstream page_num_str;
        page_num_str << "Page " << page_number;
        float page_width = HPDF_Page_GetWidth(page);
        float text_width = HPDF_Page_TextWidth(page, page_num_str.str().c_str());
        HPDF_Page_MoveTextPos(page, (page_width / 2) - (text_width / 2), margin - 20.0f);
        HPDF_Page_ShowText(page, page_num_str.str().c_str());
        HPDF_Page_EndText(page);
        page_number++;
    }

    void drawTableForEntry(const json &entry) {
        const vector<string> keys = {"cmd_name", "data", "range", "status"};

        for (const auto &key : keys) {
            if (entry.contains(key)) {
                vector<string> row_data;
                row_data.push_back(key);
                row_data.push_back(entry[key].get<string>());

                vector<vector<string>> wrapped_lines;
                float max_cell_height = wrapTextInRow(row_data, wrapped_lines);

                if (current_y_position - max_cell_height < margin) {
                    addNewPage();
                }

                drawRow(row_data, wrapped_lines, current_y_position, max_cell_height);
                current_y_position -= max_cell_height;
            }
        }

        current_y_position -= 20.0f; // Space between tables
    }

    float wrapTextInRow(const vector<string> &row, vector<vector<string>> &wrapped_lines) {
        float max_cell_height = 0.0f;
        for (size_t i = 0; i < row.size(); ++i) {
            const string cell_content = row[i];
            const float cell_width = (i == 0) ? key_col_width : value_col_width;
            vector<string> wrapped = TextWrapper::wrapText(cell_content, font, font_size, cell_width - 2 * cell_padding, page);
            wrapped_lines.push_back(wrapped);
            const float cell_height = (wrapped.size() * line_height) + 2 * cell_padding;
            if (cell_height > max_cell_height) {
                max_cell_height = cell_height;
            }
        }
        return max_cell_height;
    }

    void drawRow(const vector<string> &row, const vector<vector<string>> &wrapped_lines, float y_position, float max_cell_height) {
        float x_position = margin;

        for (size_t i = 0; i < row.size(); ++i) {
            const auto &wrapped = wrapped_lines[i];
            const float cell_width = (i == 0) ? key_col_width : value_col_width;

            // Set fill color based on the status
            if (row[i] == "pass" || row[i] == "Pass") {
                HPDF_Page_SetRGBFill(page, 0.0f, 0.5f, 0.0f); // Green
            } else if (row[i] == "fail" || row[i] == "Fail") {
                HPDF_Page_SetRGBFill(page, 0.5f, 0.0f, 0.0f); // Red
            }

            if (i == 3) { // Assuming the status is in the fourth column (index 3)
                HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
                HPDF_Page_Fill(page); // Fill the cell with the color
                HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f); // Reset to black for text
            } else {
                HPDF_Page_SetLineWidth(page, 1.0f);
                HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
                HPDF_Page_Stroke(page);
            }

            float text_y_position = y_position - cell_padding - line_height;

            for (const auto &line : wrapped) {
                HPDF_Page_BeginText(page);
                HPDF_Page_SetFontAndSize(page, font, font_size);
                HPDF_Page_MoveTextPos(page, x_position + cell_padding, text_y_position);
                HPDF_Page_ShowText(page, line.c_str());
                HPDF_Page_EndText(page);
                text_y_position -= line_height;
            }

            x_position += cell_width;
            HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
        }
    }
};

class JSONReader {
public:
    static json readJson(const string &filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }

        json root;
        file >> root; // Directly read JSON into the object
        return root;
    }
};

int main(int argc, char *argv[]) {
    try {
        if (argc < 3) {
            cerr << "Usage: " << argv[0] << " <input_json_file> <output_pdf_file>" << endl;
            return 1;
        }

        string input_filename = argv[1];
        string output_filename = argv[2];

        json jsonData = JSONReader::readJson(input_filename);
        PDFDocument pdfDoc(output_filename);
        pdfDoc.generatePDF(jsonData);

        cout << "PDF generated successfully: " << output_filename << endl;
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
