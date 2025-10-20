run: prog
	./prog

prog: unit-tests.o codec.o
	g++ -o prog unit-tests.o codec.o

unit-tests.o: unit-tests.cpp
	g++ -c unit-tests.cpp

encode-cmd.o: encode-cmd.cpp
	g++ -c encode-cmd.cpp

deecode-cmd.o: decode-cmd.cpp
	g++ -c decode-cmd.cpp

codec.o: codec.cpp
	g++ -c codec.cpp

encode: encode-cmd.o codec.o
	g++ -o encode encode-cmd.o codec.o

decode: decode-cmd.o codec.o
	g++ -o decode decode-cmd.o codec.o
