# Project-Test
sudo apt-get install libhpdf-dev
g++ -o generate_files generate_files.cpp -lhpdf
./generate_files


git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
.\bootstrap-vcpkg.bat # Windows

wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -P include/nlohmann/
