CC = gcc
CFLAGS = -O2  -std=c89 -pedantic -w
# Se la si vuole, la definizione di GDBFLAGS serve a permettere l'avvio di gdb
GDBFLAGS = -std=c89 -pedantic -w -O0 -g
TARGET = application
TARGET_ME = application_ME
PORTO = porto
NAVE = nave
DEMONE = demone
SOURCES = *.c
var = 9
#var = [parametro da inserire su cmd: "make run var=[args]"]

compila: $(SOURCES)
	gcc $(SOURCES) $(CFLAGS) -c

compila_ME: $(SOURCES)
	gcc $(SOURCES) $(CFLAGS) -D DUMP_ME -c

$(TARGET): compila
	gcc *lib.o master.o -o $(TARGET) -lm
	gcc *lib.o porto.o -o $(PORTO) -lm
	gcc *lib.o nave.o -o $(NAVE) -lm
	gcc queue_lib.o sem_lib.o demone.o -o $(DEMONE) -lm

$(TARGET_ME): compila_ME
	gcc *lib.o master.o -o $(TARGET_ME) -lm
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

runME: $(TARGET_ME)
	./$(TARGET_ME) $(var)

clear:
	rm -f *.o $(TARGET) $(TARGET_ME) $(NAVE) $(PORTO) $(DEMONE) log_*.txt out.txt

ipc: clear
	ipcrm --all
	rm fifo_coda_messaggi