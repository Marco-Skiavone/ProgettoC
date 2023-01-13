CC = gcc
CFLAGS = -std=c89 -pedantic -O2
DEVFLAGS = -std=c89 -pedantic
# la definizione di GDBFLAGS serve a permettere l'avvio di gdb
GDBFLAGS = -std=c89 -pedantic -O0 -g
TARGET = application
PORTO = porto
NAVE = nave
DEBUG = Dapplication
#possiamo aggiungere altre librerie qua sotto
OBJS = *lib.o
SOURCES = *.c
#var = [parametro da inserire su cmd: "make run var=[args]"]

$(OBJS): $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -c

$(TARGET): $(OBJS)
	gcc $(OBJS) master.o -o $(TARGET)

$(DEBUG): $(SOURCES)
	gcc $(DEVFLAGS) $(SOURCES) -D DEBUG -c
	gcc $(OBJS) master.o -o $(DEBUG)
	$(PORTO)
	$(NAVE)

all: $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -c
	gcc $(OBJS) master.o -o $(TARGET) -lm
	gcc $(OBJS) porto.o -o $(PORTO) -lm
	gcc $(OBJS) nave.o -o $(NAVE) -lm

run: $(TARGET) $(PORTO) $(NAVE)
	./$(TARGET) $(var)

debug: $(DEBUG)
	./$(DEBUG) $(var)

clear:
	rm -f *.o $(TARGET) $(DEBUG) $(NAVE) $(PORTO)