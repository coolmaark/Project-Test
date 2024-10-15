#include <hpdf.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp> // nlohmann/json header

using json = nlohmann::json; // Using alias for convenience
using namespace std;

class TextWrapper
{
public:
    static vector<string> wrapText(const string &text, HPDF_Font font, float font_size, float cell_width, HPDF_Page page)
    {
        vector<string> lines;
        string current_line = "";
        HPDF_Page_SetFontAndSize(page, font, font_size);

        // Width of a single character (using a common character for approximation, e.g., 'W')
        float char_width = HPDF_Page_TextWidth(page, "W") / 1.0f;

        istringstream remaining_text(text);
        string word;

        while (remaining_text >> word)
        {
            string test_line = current_line.empty() ? word : current_line + " " + word;

            // If the current line + word fits in the cell width, add the word
            if (HPDF_Page_TextWidth(page, test_line.c_str()) <= cell_width)
            {

                current_line = test_line;
            }
            else
            {
                // Push the current line to the lines vector if it's not empty
                if (!current_line.empty())
                {
                    lines.push_back(current_line);
                }

                // Handle the case where the word does not fit
                while (!word.empty())
                {
                    // Calculate the number of characters that can fit in the cell width
                    size_t chars_to_fit = static_cast<size_t>(cell_width / 5);

                    // Check if the entire word can fit
                    string part = word.substr(0, chars_to_fit);

                    // If part exceeds the cell width, split it character by character
                    while (HPDF_Page_TextWidth(page, part.c_str()) > cell_width && !part.empty())
                    {
                        part.erase(part.end() - 1); // Remove last character until it fits
                    }

                    // Add the part to lines and adjust the word accordingly
                    lines.push_back(part);
                    word.erase(0, part.length()); // Remove the used part from the word
                    current_line.clear();         // Clear current_line for the next iteration
                }
            }
        }

        // If there are any remaining words in the current line, push it to lines
        if (!current_line.empty())
        {
            lines.push_back(current_line);
        }

        return lines;
    }
};

class PDFDocument
{
public:
    PDFDocument(const string &filename)
        : pdf_filename(filename), pdf(HPDF_New(error_handler, nullptr)), current_y_position(0.0f), page_number(1)
    {
        if (pdf == nullptr)
        {
            throw runtime_error("Failed to create PDF object.");
        }
        addNewPage();
    }

    ~PDFDocument()
    {
        HPDF_Free(pdf);
    }

    void generatePDF(const json &jsonData)
    {
        loadImagesAndText();
        for (const auto &entry : jsonData)
        {
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
    float cell_padding = 10.0f; // Increased padding
    float key_col_width;
    float value_col_width;
    float current_y_position;
    int page_number;

    static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
    {
        cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << endl;
    }

    void addNewPage()
    {
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

    void drawPageBorder()
    {
        float page_width = HPDF_Page_GetWidth(page);
        float page_height = HPDF_Page_GetHeight(page);

        HPDF_Page_SetLineWidth(page, 2.0f);
        HPDF_Page_Rectangle(page, margin - 5.0f, margin - 5.0f,
                            page_width - 2 * margin + 10.0f,
                            page_height - 2 * margin + 10.0f);
        HPDF_Page_Stroke(page);
    }

    void loadImagesAndText()
    {
        loadImage("left_image.jpeg", margin, HPDF_Page_GetHeight(page) - 110.0f);
        loadImage("right_image.jpeg", HPDF_Page_GetWidth(page) - margin - 60.0f, HPDF_Page_GetHeight(page) - 110.0f);

        const string centered_text = "Name_Head";
        float page_width = HPDF_Page_GetWidth(page);
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 20.0f);
        float text_width = HPDF_Page_TextWidth(page, centered_text.c_str());
        float text_x_position = (page_width / 2) - (text_width / 2);
        float text_y_position = HPDF_Page_GetHeight(page) - 70.0f;
        HPDF_Page_MoveTextPos(page, text_x_position, text_y_position);
        HPDF_Page_ShowText(page, centered_text.c_str());
        HPDF_Page_EndText(page);

        current_y_position -= 80.0f; // Space for images and title (adjust as needed)
    }

    void loadImage(const string &filename, float x, float y)
    {
        HPDF_Image img = HPDF_LoadJpegImageFromFile(pdf, filename.c_str());
        if (img != nullptr)
        {
            HPDF_Page_DrawImage(page, img, x, y, 60.0f, 60.0f);
        }
        else
        {
            cerr << "Warning: Could not load image " << filename << endl;
        }
    }

    void drawPageNumber()
    {
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

    float calculateTotalHeightForArray(const vector<string> &array_data)
    {
        float total_height = 0.0f;
        for (const auto &item : array_data)
        {
            vector<string> row_data = {item}; // Treat each array element as a row
            vector<vector<string>> wrapped_lines;
            float cell_height = wrapTextInRow(row_data, wrapped_lines);
            total_height += cell_height;
        }
        return total_height;
    }
    void drawArrayRowWithBorder(const string &key, const vector<string> &array_data, float y_position, float total_cell_height)
    {
        float x_position = margin;
        const float light_pink[] = {1.0f, 0.8f, 0.8f}; // Light pink color

        // Set background color for key column (light pink)
        HPDF_Page_SetRGBFill(page, light_pink[0], light_pink[1], light_pink[2]);
        HPDF_Page_Rectangle(page, x_position, y_position - total_cell_height, key_col_width, total_cell_height); // Key column background
        HPDF_Page_Fill(page);

        // Draw the border around the key column
        HPDF_Page_SetLineWidth(page, 1.0f);
        HPDF_Page_Rectangle(page, x_position, y_position - total_cell_height, key_col_width, total_cell_height); // Key column border
        HPDF_Page_Stroke(page);

        // Print the key (once)
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, font_size);
        HPDF_Page_MoveTextPos(page, x_position + cell_padding, y_position - cell_padding - line_height);
        HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f); // Black color for text
        HPDF_Page_ShowText(page, key.c_str());
        HPDF_Page_EndText(page);

        // Move to the value column position
        x_position += key_col_width;

        // Draw the border around the value column
        HPDF_Page_Rectangle(page, x_position, y_position - total_cell_height, value_col_width, total_cell_height); // Value column border
        HPDF_Page_Stroke(page);

        // Draw the array values inside the value column (no background fill, just text and border)
        float text_y_position = y_position - cell_padding - line_height;
        for (const auto &array_item : array_data)
        {
            HPDF_Page_BeginText(page);
            HPDF_Page_SetFontAndSize(page, font, font_size);
            HPDF_Page_MoveTextPos(page, x_position + cell_padding, text_y_position);
            HPDF_Page_ShowText(page, array_item.c_str());
            HPDF_Page_EndText(page);

            text_y_position -= line_height + cell_padding; // Move down for the next array item
        }
    }

    void drawTableForEntry(const json &entry)
    {
        const vector<string> keys = {"cmd_name", "data", "range", "status", "input_other", "output_other"};

        for (const auto &key : keys)
        {
            if (entry.contains(key))
            {
                vector<string> row_data;
                row_data.push_back(key);

                // Handle string values
                if (entry[key].is_string())
                {
                    row_data.push_back(entry[key].get<string>());

                    // Wrap text and calculate max cell height
                    vector<vector<string>> wrapped_lines;
                    float max_cell_height = wrapTextInRow(row_data, wrapped_lines);

                    // Check if we need a new page due to height
                    if (current_y_position - max_cell_height < margin)
                    {
                        addNewPage();
                    }

                    // Draw the row
                    drawRow(row_data, wrapped_lines, current_y_position, max_cell_height);
                    current_y_position -= max_cell_height; // Move down for the next row
                }
                // Handle array values - print key once, no individual borders for array items, but border for the whole block
                else if (entry[key].is_array())
                {
                    vector<string> array_data;
                    for (const auto &item : entry[key])
                    {
                        array_data.push_back(item.get<string>());
                    }

                    // Calculate total height for all array items
                    float total_cell_height = calculateTotalHeightForArray(array_data);

                    // If current page doesn't have enough space, add a new page
                    if (current_y_position - total_cell_height < margin)
                    {
                        addNewPage();
                    }

                    // Draw the block with a border for the key and the array values
                    drawArrayRowWithBorder(key, array_data, current_y_position, total_cell_height);

                    // Move down after drawing
                    current_y_position -= total_cell_height;
                }
            }
        }

        current_y_position -= 20.0f; // Space between tables
    }

    string formatArray(const json &array)
    {
        string combined;
        for (const auto &item : array)
        {
            // Add each item followed by a newline character
            combined += item.get<string>() + "\n \n";
        }
        return combined;
    }

    float wrapTextInRow(const vector<string> &row, vector<vector<string>> &wrapped_lines)
    {
        float max_cell_height = 0.0f;
        for (size_t i = 0; i < row.size(); ++i)
        {
            const string cell_content = row[i];
            const float cell_width = (i == 0) ? key_col_width : value_col_width;

            // Wrap the text considering newline characters
            vector<string> wrapped = TextWrapper::wrapText(cell_content, font, font_size, cell_width - 2 * cell_padding, page); // Respect padding
            wrapped_lines.push_back(wrapped);

            float cell_height = (wrapped.size() * line_height) + 2 * cell_padding; // Text height + padding
            if (cell_height > max_cell_height)
            {
                max_cell_height = cell_height;
            }
        }
        return max_cell_height;
    }

    void drawRow(const vector<string> &row, const vector<vector<string>> &wrapped_lines, float y_position, float max_cell_height)
    {
        float x_position = margin;
        const float light_pink[] = {1.0f, 0.8f, 0.8f};
        for (size_t i = 0; i < row.size(); ++i)
        {
            const auto &wrapped = wrapped_lines[i];
            const float cell_width = (i == 0) ? key_col_width : value_col_width;
            if (i == 0)
            {
                HPDF_Page_SetRGBFill(page, light_pink[0], light_pink[1], light_pink[2]);
                HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
                HPDF_Page_Fill(page); // Fill the rectangle with light pink
            }
            // Set fill color based on the status
            HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f); // Reset to black for text
            if (i == 3)
            { // Assuming the status is in the fourth column (index 3)
                if (row[i] == "pass" || row[i] == "Pass")
                {
                    HPDF_Page_SetRGBFill(page, 0.0f, 0.5f, 0.0f); // Green
                }
                else if (row[i] == "fail" || row[i] == "Fail")
                {
                    HPDF_Page_SetRGBFill(page, 0.5f, 0.0f, 0.0f); // Red
                }
                HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
                HPDF_Page_Fill(page);                         // Fill the cell with the color
                HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f); // Reset to black for text
            }

            else
            {
                HPDF_Page_SetLineWidth(page, 1.0f);
                HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
                HPDF_Page_Stroke(page);
            }

            float text_y_position = y_position - cell_padding - line_height;

            for (const auto &line : wrapped)
            {
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

void parseJsonFile(const string &filename, json &jsonData)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        throw runtime_error("Could not open JSON file: " + filename);
    }
    file >> jsonData;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <input.json> <output.pdf>" << endl;
        return 1;
    }

    const string json_filename = argv[1];
    const string pdf_filename = argv[2];

    try
    {
        json jsonData;
        parseJsonFile(json_filename, jsonData);

        PDFDocument pdfDocument(pdf_filename);
        pdfDocument.generatePDF(jsonData);
        cout << "PDF generated sucessfully\n";
    }
    catch (const runtime_error &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}