.PHONY: all clean

ver := -std=c++17
target := main.exe
routes_target := routes_fix.exe

#adjust vars to reflect OS
ifneq (,$(filter $(uname), Linux Darwin)) #Linux or macOS, WIP
target := main
routes_target := routes_fix
endif

exists? := which

all: main.exe
	make clean

${target}: main.o
	g++ ${ver} \
	main.o \
	src/nlohmann/json.hpp \
	-o ${target}

main.o: src/main.cpp 
	g++ -c ${ver} src/main.cpp -o main.o

clean: 
	-rm *.o

# for routes_fix for processing routes.json
routes_fix: ${routes_target}

${routes_target}: routes_fix.o
	g++ ${ver} routes_fix.o -o src/misc/${routes_target}
	make clean

routes_fix.o: src/misc/routes_fix.cpp
	g++ ${ver} -c src/misc/routes_fix.cpp -o routes_fix.o