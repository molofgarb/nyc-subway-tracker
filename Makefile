.PHONY: all clean

ver := -std=c++17

all: main.exe
	make clean

main.exe: main.o
	g++ ${ver} \
	main.o \
	src/nlohmann/json.hpp \
	-o main.exe

main.o: src/main.cpp 
	g++ -c ${ver} src/main.cpp -o main.o

clean: 
	-rm *.o

routes_fix.exe: routes_fix.o
	g++ ${ver} routes_fix.o -o src/misc/routes_fix.exe
	make clean

routes_fix.o: src/misc/routes_fix.cpp
	g++ ${ver} -c src/misc/routes_fix.cpp -o routes_fix.o