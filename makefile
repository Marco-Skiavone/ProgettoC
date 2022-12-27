CC = gcc
CFLAGS = -std=c89 -Wpedantic -O2
DEVFLAGS = -std=c89 -pedantic
# la definizione di GDBFLAGS serve a permettere l'avvio di gdb
GDBFLAGS = -std=c89 -pedantic -O0 -g
TARGET = application
DEBUG = Dapplication
OBJS = *.o
SOURCES = *.c
#var = [parametro da inserire su cmd: "make run var=[args]"]
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c
	$(CC) $(OBJS) -o $(TARGET)

$(DEBUG): $(SOURCES)
	$(CC) $(DEVFLAGS) $(SOURCES) -D DEBUG -c
	$(CC) $(OBJS) -o $(DEBUG)

run: $(TARGET)
	./$(TARGET) $(var)

debug: $(DEBUG)
	./$(DEBUG) $(var)

clear: $(OBJS)
	rm -f $(OBJS) $(TARGET) $(DEBUG)