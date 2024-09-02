#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <hpdf.h>

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    std::cerr << "Error: error_no=" << error_no << ", detail_no=" << detail_no << std::endl;
}

std::string run_python_script(const std::string& script_name) {
    std::string result;
    std::string command = "python3 " + script_name;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run Python script" << std::endl;
        return "";
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);
    return result;
}

int main() {
    // Run the Python script and capture its output
    std::string python_output = run_python_script("test.py");

    if (python_output.empty()) {
        std::cerr << "No output from the Python script." << std::endl;
        return 1;
    }

    // Create a new PDF document
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        std::cerr << "Error: cannot create PDF object" << std::endl;
        return 1;
    }

    // Add a new page
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    // Set the font and size
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize(page, font, 12);

    // Set the text position
    float height = HPDF_Page_GetHeight(page);
    float width = HPDF_Page_GetWidth(page);
    float text_pos_y = height - 50;
    float text_pos_x = 50;

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, text_pos_x, text_pos_y, python_output.c_str());
    HPDF_Page_EndText(page);

    // Save the PDF document to a file
    HPDF_SaveToFile(pdf, "output.pdf");

    // Clean up
    HPDF_Free(pdf);

    std::cout << "PDF generated successfully." << std::endl;
    return 0;
}
