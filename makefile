CC = gcc
CFLAGS = -O2  -std=c89 -pedantic -w
# Se la si vuole, la definizione di GDBFLAGS serve a permettere l'avvio di gdb
GDBFLAGS = -std=c89 -pedantic -w -O0 -g
TARGET = application
PORTO = porto
NAVE = nave
DEMONE = demone
SOURCES = *.c
var = 9
#var = [parametro da inserire su cmd: "make run var=[args]"]

compila: $(SOURCES)
	gcc $(SOURCES) $(CFLAGS) -c

#gcc $(SOURCES) $(CFLAGS) -D DEBUG -c

$(TARGET): compila
	gcc *lib.o master.o -o $(TARGET) -lm
	gcc *lib.o porto.o -o $(PORTO) -lm
	gcc *lib.o nave.o -o $(NAVE) -lm
	gcc queue_lib.o sem_lib.o demone.o -o $(DEMONE) -lm

all: $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -c
	gcc *lib.o master.o -o $(TARGET) -lm
	gcc *lib.o porto.o -o $(PORTO) -lm
	gcc *lib.o nave.o -o $(NAVE) -lm
	gcc queue_lib.o sem_lib.o demone.o -o $(DEMONE) -lm

run: $(TARGET)
	./$(TARGET) $(var)

clear:
	rm -f *.o $(TARGET) $(NAVE) $(PORTO) $(DEMONE) log_*.txt out.txt

ipc: clear
	ipcrm --all
	rm fifo_coda_messaggi
	pkill nave
	pkill porto