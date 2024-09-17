#ifndef UTILITIES_H
#define UTILITIES_H

#include <hpdf.h>

// Error handler for Haru PDF library
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data);

#endif // UTILITIES_H
