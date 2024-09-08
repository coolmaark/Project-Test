#include <hpdf.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// Error handler for Haru PDF library

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << endl;
}

// Function to read data from a file
string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return "";
    }

    ostringstream sstream;
    sstream << file.rdbuf();
    return sstream.str();
}

// Function to split text into lines based on a maximum line width
vector<string> splitTextIntoLines(const string& text, int max_width, HPDF_Font font, HPDF_Page page) {
    vector<string> lines;
    istringstream stream(text);
    string line;
    string current_line;
    int space_width = HPDF_Page_TextWidth(page, " ");
    
    while (getline(stream, line)) {
        istringstream line_stream(line);
        string word;
        
        while (line_stream >> word) {
            int word_width = HPDF_Page_TextWidth(page, word.c_str());
            
            if (current_line.empty()) {
                current_line = word;
            } else {
                int current_line_width = HPDF_Page_TextWidth(page, current_line.c_str()) + space_width + word_width;
                
                if (current_line_width > max_width) {
                    lines.push_back(current_line);
                    current_line = word;
                } else {
                    current_line += " " + word;
                }
            }
        }
        
        if (!current_line.empty()) {
            lines.push_back(current_line);
            current_line.clear();
        }
    }
    
    return lines;
}

// Function to generate a PDF and a text data file
void generateFiles(const string& text, const string& pdf_filename, const string& text_filename) {
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        cerr << "Failed to create PDF object." << endl;
        return;
    }

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize(page, font, 12);

    int max_width = 500; // Adjust as needed
    vector<string> lines = splitTextIntoLines(text, max_width, font, page);

    HPDF_Page_BeginText(page);
    HPDF_Page_MoveTextPos(page, 50, 750); // Adjust starting position

    for (const string& line : lines) {
        HPDF_Page_ShowText(page, line.c_str());
        HPDF_Page_MoveTextPos(page, 0, -18); // Move down for the next line (adjust line height as needed)
    }

    HPDF_Page_EndText(page);
    HPDF_SaveToFile(pdf, pdf_filename.c_str());
    HPDF_Free(pdf);

    cout << "PDF created successfully: " << pdf_filename << endl;

    ofstream text_file(text_filename);
    if (text_file.is_open()) {
        text_file << text;
        text_file.close();
        cout << "Text data file created successfully: " << text_filename << endl;
    } else {
        cerr << "Failed to create text data file." << endl;
    }
}

int main() {
    string input_filename = "input.txt";  // The file containing raw data
    string pdf_filename = "output.pdf";   // The output PDF file
    string text_filename = "data.txt";    // The output text data file

    string data = readFile(input_filename);

    if (!data.empty()) {
        generateFiles(data, pdf_filename, text_filename);
    } else {
        cerr << "No data read from file." << endl;
    }

    return 0;
}
