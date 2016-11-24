  # the compiler: gcc for C program, define as g++ for C++
  CC = g++

  # compiler flags:
  #  -std=c++11 Compile with c++ 11 standards
  CFLAGS=-std=c++11 -Wall

  # the build target executable:
  TARGET=dv

  all: dv

  $(TARGET):$(TARGET).cpp
	$(CC) $(CFLAGS) $(TARGET).cpp -o $(TARGET).o

  clean:
	rm -f *.o $(TARGET)

  run:
	./$(TARGET).o 5500
