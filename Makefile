CXX = gcc

LIBS = -pthread

FILES = threads.c func.c

TARGET = threads

all: $(TARGET)

$(TARGET):
	@echo "\nGeneration of " $(TARGET)
	$(CXX) $(FILES) -o $(TARGET) $(LIBS)

clean:
	@echo "\n Cleaning current directory"
	rm -f *.o *~
