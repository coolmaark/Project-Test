#include <hpdf.h>
#include <iostream>
#include "Utilities.h"  // Make sure to include your header file

using namespace std;

// Error handler for Haru PDF library
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    cerr << "ERROR: " << error_no << ", DETAIL: " << detail_no << endl;
}
