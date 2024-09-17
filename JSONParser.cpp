#include "JSONParser.h"
#include <fstream>
#include <iostream>

Json::Value JSONParser::readJson(const std::string& filename) {
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return Json::nullValue;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;

    if (!Json::parseFromStream(builder, file, &root, &errors)) {
        std::cerr << "Failed to parse JSON: " << errors << std::endl;
        return Json::nullValue;
    }

    return root;
}
