#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <json/json.h>
#include <string>

class JSONParser {
public:
    Json::Value readJson(const std::string& filename);
};

#endif // JSONPARSER_H
