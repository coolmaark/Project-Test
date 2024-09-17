# Project-Test
sudo apt update
sudo apt-get install libhpdf-dev
sudo apt install libjsoncpp-dev
g++ -o my_program generate_files.cpp -I/usr/include/jsoncpp -ljsoncpp -lhpdf
./my_program