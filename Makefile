CXX := g++
CXXFLAGS := -O2 -Wall -std=c++17

SRC := main.cpp 
OUT := a.out

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)