all:
	clang++-3.8 -std=c++11 -I ./amigadrive/include -o amigatool.exe ./amigadrive/src/*.cpp amigatool/main.cpp -lm -lc -lstdc++
