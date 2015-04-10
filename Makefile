FLAGS=-msse4 -std=c++11 -O3
FILE=simsort.cpp
OUT=simsort

all: SIMD

SIMD: 
	g++ $(FILE) -o $(OUT) $(FLAGS)