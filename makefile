CC = gcc
CFLAGS = -std=c89 -Wpedantic -O2
DEVFLAGS = -std=c89 -Wpedantic -O0 -g -D DEBUG
TARGET = application
DEBUG = debugging_app
OBJS = *.o
SOURCES = *.c
#var = [parametro da inserire su cmd: "make run var=[args]"]

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -c
	$(CC) $(OBJS) -o $(TARGET)

# con la definizione di WIP posso eseguire porzioni di codice particolari
$(DEBUG): $(SOURCES)
	$(CC) $(DEVFLAGS) $(SOURCES) -c
	$(CC) $(OBJS) -o $(TARGET)

all: $(TARGET)

run: $(TARGET)
	./$(TARGET) $(var)

debug: $(DEBUG)
	./$(DEBUG) $(var)

clear: *.o
	rm -f *.o $(TARGET)
