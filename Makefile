CXX := g++
CXXFLAGS := -g -O2 -std=c++17 -Wall

SRC := main.cpp 
OUT := a.out

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
