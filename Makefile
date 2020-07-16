CC=g++
TARGET=chip8
SOURCES=$(wildcard *.cpp)
OBJFILES=$(SOURCES:.cpp=.o)
CPPFLAGS=-lsfml-graphics -lsfml-window -lsfml-system

.PHONY : clean

$(TARGET):$(OBJFILES)
	$(CC) $(CPPFLAGS) $^ -o $@

%.o:%.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)