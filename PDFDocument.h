#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <json/json.h>
#include <string>

class PDFDocument {
public:
    PDFDocument();
    ~PDFDocument();
    void generatePDFWithTables(const Json::Value& jsonData, const std::string& pdf_filename);
};

#endif // PDFDOCUMENT_H
