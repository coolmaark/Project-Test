#include <iostream>
#include <jsoncpp/json/json.h>
#include <sstream>

int main() {
    std::string jsonData = R"({"cur": {"cmd_name": "Cell A2", "data": "hellosdkadofkef", "range": "Lorem ipsum, dolor sit amet consectetur adipisicing elit. Unde, consectetur? Iste unde qui veniam earum explicabo omnis aliquid error nesciunt quae itaque hic, sapiente incidunt, neque inventore similique at doloribus?", "status": "Pass"}})";

    Json::Value newData;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream ss(jsonData);

    if (!Json::parseFromStream(reader, ss, &newData, &errs)) {
        std::cerr << "Error parsing JSON data: " << errs << "\n";
        return 1;
    }

    if (!newData.isMember("cur")) {
        std::cerr << "Error: 'cur' key not found in JSON data\n";
        return 1;
    }

    Json::Value cmdData = newData["cur"];
    std::cout << "Parsed cmd_name: " << cmdData["cmd_name"].asString() << "\n";

    return 0;
}
