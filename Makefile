all: gravis

# Welcher Compiler
CC = gcc

# Installationsverzeichnis
INSTDIR = /usr/local/bin

# Wo sind die eigenen Include-Dateien
INCLUDE = .

# Entwicklungsoptionen
#CFLAGS = -g -Wall -ansi

# Optionen fuer Release
CFLAGS = -O -Wall -ansi

gravis: gravis.o geometrie.o util.o
	$(CC) -o gravis gravis.o geometrie.o util.o -lm

gravis.o: gravis.c gravis.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c gravis.c

geometrie.o: geometrie.c
	$(CC) -I$(INCLUDE) $(CFLAGS) -c geometrie.c

util.o: util.c
	$(CC) -I$(INCLUDE) $(CFLAGS) -c util.c

clean:
	-rm gravis.o geometrie.o util.o

install: gravis
	@if [ -d $(INSTDIR) ]; \
		then \
		cp gravis $(INSTDIR);\
		chmod a+x $(INSTDIR)/gravis;\
		chmod og-w $(INSTDIR)/gravis;\
		echo "Installiert in $(INSTDIR)";\
	else \
		echo "Installationsverzeichnis $(INSTDIR) existiert nicht.";\
	fi
