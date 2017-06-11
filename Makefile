OPENCV_LIBS=`pkg-config --libs opencv`

all: main.cpp util.cpp table.cpp display.cpp
	g++ -std=c++11 -O3 -Wno-unused-result -o main main.cpp util.cpp table.cpp display.cpp $(OPENCV_LIBS)
clean:
	rm -f main
