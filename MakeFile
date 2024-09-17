CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lhpdf -ljsoncpp

SRCS = main.cpp PDFDocument.cpp JSONParser.cpp Utilities.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = output

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
