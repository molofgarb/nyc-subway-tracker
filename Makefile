ver := -std=c++17

main.exe: main.o
	g++ ${ver} \
	main.o \
	-o main.exe
	make clean

main.o: src/main.cpp
	g++ -c ${ver} src/main.cpp -o main.o

clean: 
	-rm main.o