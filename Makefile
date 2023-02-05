.PHONY: all clean

ver := -std=c++17

all: 
	make clean

main.exe: main.o
	g++ ${ver} \
	main.o \
	src/nlohmann/json.hpp \
	-o main.exe

main.o: src/main.cpp 
	g++ -c ${ver} src/main.cpp -o main.o

clean: 
	-rm main.o
	-rm json.o