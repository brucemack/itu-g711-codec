run: prog
	./prog

prog: unit-tests.o codec.o
	g++ -o prog unit-tests.o codec.o

unit-tests.o: ../src/unit-tests.cpp  ../src/itu-g711-codec/codec.h
	g++ -I../src -c ../src/unit-tests.cpp

encode-cmd.o: ../src/encode-cmd.cpp  ../src/itu-g711-codec/codec.h
	g++ -I../src -c ../src/encode-cmd.cpp

deecode-cmd.o: ../src/decode-cmd.cpp  ../src/itu-g711-codec/codec.h
	g++ -I../src -c ../src/decode-cmd.cpp

codec.o: ../src/codec.cpp ../src/itu-g711-codec/codec.h
	g++ -I../src -c ../src/codec.cpp

encode: encode-cmd.o codec.o
	g++ -o encode encode-cmd.o codec.o

decode: decode-cmd.o codec.o
	g++ -o decode decode-cmd.o codec.o
