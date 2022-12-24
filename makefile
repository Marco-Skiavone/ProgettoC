CC = gcc
CFLAGS = -std=c89 -Wpedantic -O2
#CFLAGS = -std=c89 -pedantic -O0 -g
TARGET = application
OBJS = *.o
SOURCES = *.c
#var = [parametro da inserire su cmd: "make run var=[args]"]

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c
	$(CC) $(OBJS) -o $(TARGET)

all: $(TARGET)

run: $(TARGET)
	./$(TARGET) $(var)

clear: *.o
	rm -f *.o $(TARGET)
