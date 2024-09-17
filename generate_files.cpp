#include <hpdf.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <json/json.h> // jsoncpp header

using namespace std;

// Forward declaration of the TextWrapper class
// Class to handle text wrapping
class TextWrapper
{
public:
    static vector<string> wrapText(const string &text, HPDF_Font font, float font_size, float cell_width, HPDF_Page page)
    {
        vector<string> lines;
        string current_line = "";
        istringstream words(text);
        string word;

        HPDF_Page_SetFontAndSize(page, font, font_size);

        while (words >> word)
        {
            string test_line = current_line + " " + word;
            if (HPDF_Page_TextWidth(page, test_line.c_str()) <= cell_width)
            {
                current_line = test_line;
            }
            else
            {
                string break_line = "";
                for (auto &it : current_line)
                {
                    break_line += it;
                    if (break_line.size() > cell_width / 5)
                    {
                        lines.push_back(break_line);
                        break_line = "";
                    }
                }
                current_line = break_line;
                lines.push_back(current_line); // Push the current line if it exceeds the width
                current_line = word;
            }
        }

        if (!current_line.empty())
        {
            lines.push_back(current_line);
        }

        return lines;
    }
};

// Class to handle the PDF document creation and management
class PDFDocument
{
public:
    PDFDocument(const string &filename) : pdf_filename(filename), pdf(HPDF_New(error_handler, NULL))
    {
        if (!pdf)
        {
            throw runtime_error("Failed to create PDF object.");
        }
        // Add the first page
        addNewPage();
    }

    ~PDFDocument()
    {
        HPDF_Free(pdf);
    }

    void generatePDF(const Json::Value &jsonData)
    {
        float current_y_position = HPDF_Page_GetHeight(page) - margin - 120;
        int page_number = 1;
        loadImagesAndText(); // Load images and initial text ("Hello")

        // Loop over tables in JSON data
        for (const auto &table : jsonData)
        {
            const Json::Value &rows = table["rows"];
            for (const auto &row : rows)
            {
                vector<vector<string>> wrapped_lines;
                float max_cell_height = wrapTextInRow(row, wrapped_lines);

                // Check if row fits in the current page
                if (current_y_position - max_cell_height < margin)
                {
                    addNewPage(++page_number);
                    current_y_position = HPDF_Page_GetHeight(page) - margin;
                }
                drawRow(row, wrapped_lines, current_y_position, max_cell_height);
                current_y_position -= max_cell_height; // Move down by row height
            }
            current_y_position -= line_height; // Add space between tables
        }
        HPDF_SaveToFile(pdf, pdf_filename.c_str());
    }

private:
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Font font;
    string pdf_filename;
    float font_size = 12;
    float margin = 50;
    float line_height = 18;
    float cell_padding = 5;
    float key_col_width;
    float value_col_width;

    static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
    {
        cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << endl;
    }

    void addNewPage(int page_number = 1)
    {
        page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
        font = HPDF_GetFont(pdf, "Helvetica", NULL);
        HPDF_Page_SetFontAndSize(page, font, font_size);

        // Calculate column widths based on page width
        float page_width = HPDF_Page_GetWidth(page);
        key_col_width = (page_width - 2 * margin) * 0.2;
        value_col_width = (page_width - 2 * margin) * 0.8;

        drawPageBorder();
        drawPageNumber(page_number);
    }

    void drawPageBorder()
    {
        float page_width = HPDF_Page_GetWidth(page);
        float page_height = HPDF_Page_GetHeight(page);

        HPDF_Page_SetLineWidth(page, 2.0);
        HPDF_Page_Rectangle(page, margin - 20, margin, page_width + 30 - 2 * margin, page_height + 25 - 2 * margin);
        HPDF_Page_Stroke(page);
    }

    void drawPageNumber(int page_number)
    {
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 12);
        stringstream page_num_str;
        page_num_str << "Page " << page_number;
        float page_width = HPDF_Page_GetWidth(page);
        float text_width = HPDF_Page_TextWidth(page, page_num_str.str().c_str());
        HPDF_Page_MoveTextPos(page, (page_width / 2) - (text_width / 2), margin - 10 - 20);
        HPDF_Page_ShowText(page, page_num_str.str().c_str());
        HPDF_Page_EndText(page);
    }

    void loadImagesAndText()
    {
        // Load and place images
        HPDF_Image img_left = HPDF_LoadJpegImageFromFile(pdf, "left_image.jpeg");
        HPDF_Image img_right = HPDF_LoadJpegImageFromFile(pdf, "right_image.jpeg");
        float page_height = HPDF_Page_GetHeight(page);
        float page_width = HPDF_Page_GetWidth(page);
        HPDF_Page_DrawImage(page, img_left, margin, page_height - 110, 60, 60);
        HPDF_Page_DrawImage(page, img_right, page_width - margin - 80, page_height - 110, 60, 60);

        // Add "Hello" text
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 20);
        float name_Text_Width = HPDF_Page_TextWidth(page, "Name_Head");
        HPDF_Page_MoveTextPos(page, (page_width / 2) - (name_Text_Width / 2), page_height - 70);
        HPDF_Page_ShowText(page, "Name_Head");
        HPDF_Page_EndText(page);
    }

    float wrapTextInRow(const Json::Value &row, vector<vector<string>> &wrapped_lines)
    {
        float max_cell_height = 0;
        for (int i = 0; i < row.size(); ++i)
        {
            string cell_content = row[i].asString();
            float cell_width = (i == 0) ? key_col_width : value_col_width;
            vector<string> wrapped = TextWrapper::wrapText(cell_content, font, font_size, cell_width - 2 * cell_padding, page);
            for (int i = 0; i < wrapped.size(); i++)
            {
                if (wrapped[i].empty())
                {
                    wrapped.erase(wrapped.begin() + i);
                }
            }
            wrapped_lines.push_back(wrapped);
            float cell_height = (wrapped.size() * line_height) + 2 * cell_padding;
            if (cell_height > max_cell_height)
            {
                max_cell_height = cell_height;
            }
        }
        return max_cell_height;
    }

    void drawRow(const Json::Value &row, const vector<vector<string>> &wrapped_lines, float y_position, float max_cell_height)
    {
        float x_position = margin;
        HPDF_Font font_bold = HPDF_GetFont(pdf, "Helvetica-Bold", NULL); // Load bold font
        HPDF_Font font_regular = HPDF_GetFont(pdf, "Helvetica", NULL);   // Regular font

        for (int i = 0; i < row.size(); ++i)
        {
            const auto &wrapped = wrapped_lines[i];
            float cell_width = (i == 0) ? key_col_width : value_col_width;

            // Draw the cell borders
            HPDF_Page_SetLineWidth(page, 1.0);
            HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
            HPDF_Page_Stroke(page);

            // Adjust text position within the cell
            float text_y_position = y_position - cell_padding - line_height;

            for (const auto &line : wrapped)
            {
                HPDF_Page_BeginText(page);

                // Check if this is the status column (assuming it's in the 2nd column, index 1)
                if (i == 1 && (line == " pass" || line == " fail" || line == " Pass" || line == " Fail"))
                {
                    // Apply green color and bold font for "pass"
                    if (line == " pass" || line == " Pass")
                    {
                        HPDF_Page_SetRGBFill(page, 0, 0.5, 0);                  // Green
                        HPDF_Page_SetFontAndSize(page, font_bold, font_size); // Bold
                    }
                    // Apply red color and bold font for "fail"
                    else if (line == " fail" || line == " Fail")
                    {
                        HPDF_Page_SetRGBFill(page, 0.5, 0, 0);                  // Red
                        HPDF_Page_SetFontAndSize(page, font_bold, font_size); // Bold
                    }
                }
                else
                {
                    // Normal text (default black and regular font)
                    HPDF_Page_SetRGBFill(page, 0, 0, 0);                     // Black
                    HPDF_Page_SetFontAndSize(page, font_regular, font_size); // Regular font
                }

                // Write the text line in the cell
                HPDF_Page_MoveTextPos(page, x_position + cell_padding, text_y_position);
                HPDF_Page_ShowText(page, line.c_str());

                HPDF_Page_EndText(page);

                text_y_position -= line_height; // Move to the next line within the cell
            }

            x_position += cell_width; // Move to the next column
        }
    }
};

// Class to handle reading JSON data
class JSONReader
{
public:
    static Json::Value readJson(const string &filename)
    {
        ifstream file(filename, ifstream::binary);
        if (!file.is_open())
        {
            throw runtime_error("Failed to open file: " + filename);
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        string errors;

        if (!Json::parseFromStream(builder, file, &root, &errors))
        {
            throw runtime_error("Failed to parse JSON: " + errors);
        }

        return root;
    }
};

int main()
{
    try
    {
        string input_filename = "data.json";
        string pdf_filename = "output.pdf";

        Json::Value data = JSONReader::readJson(input_filename);
        PDFDocument pdf_doc(pdf_filename);
        pdf_doc.generatePDF(data);

        cout << "PDF created successfully: " << pdf_filename << endl;
    }
    catch (const exception &ex)
    {
        cerr << "Error: " << ex.what() << endl;
    }

    return 0;
}