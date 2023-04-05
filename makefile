CC = gcc
CFLAGS = -O2  #-std=c89 -Wpedantic
# Se la si vuole, la definizione di GDBFLAGS serve a permettere l'avvio di gdb
GDBFLAGS = -std=c89 -Wpedantic -O0 -g
TARGET = application
PORTO = porto
NAVE = nave
#possiamo aggiungere altre librerie qua sotto
OBJS = *lib.o
SOURCES = *.c
#var = [parametro da inserire su cmd: "make run var=[args]"]

$(OBJS): $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -c

$(TARGET): $(OBJS)
	gcc $(OBJS) master.o -o $(TARGET) -lm
	gcc $(OBJS) porto.o -o $(PORTO) -lm
	gcc $(OBJS) nave.o -o $(NAVE) -lm

$(TARGET_ME): $(OBJS_ME)
	gcc $(OBJS_ME) master.o -o $(TARGET_ME) -lm
	gcc $(OBJS_ME) porto.o -o $(PORTO) -lm
	gcc $(OBJS_ME) nave.o -o $(NAVE) -lm

all: $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -c
	gcc $(OBJS) master.o -o $(TARGET) -lm
	gcc $(OBJS) porto.o -o $(PORTO) -lm
	gcc $(OBJS) nave.o -o $(NAVE) -lm

run: $(TARGET)
	./$(TARGET) $(var)

runME: $(TARGET)
	gcc -E $(SOURCES) -D DUMP_ME -o $(TARGET_ME) -lm 
	./$(TARGET_ME) $(var)

clear:
	rm -f *.o $(TARGET) $(DEBUG) $(NAVE) $(PORTO)

ipc:
	ipcrm --all