#
# usuario/Makefile
#	Makefile de programas de usuario
#
# NOTA: EN LA VERSI�N INICIAL S�LO COMPILAR�N CORRECTAMENTE LOS PROGRAMAS
# init, excep_arit, excep_mem y simplon, YA QUE LOS DEM�S USAN LLAMADAS QUE
# NO TIENE QUE IMPLEMENTAR EL ALUMNO 
#


MAKEFLAGS=-k
INCLUDEDIR=include
LIBDIR=lib

BIBLIOTECA=$(LIBDIR)/libserv.a

CC=cc
CFLAGS=-Wall -fPIC -Werror -g -I$(INCLUDEDIR)

PROGRAMAS=init excep_arit excep_mem simplon idproceso Pidproceso

all: biblioteca $(PROGRAMAS)

biblioteca:
	cd lib; make

init.o: $(INCLUDEDIR)/servicios.h
init: init.o $(BIBLIOTECA)
	$(CC) -shared -o $@ init.o -L$(LIBDIR) -lserv

excep_arit.o: $(INCLUDEDIR)/servicios.h
excep_arit: excep_arit.o $(BIBLIOTECA)
	$(CC) -shared -o $@ excep_arit.o -L$(LIBDIR)  -lserv 

excep_mem.o: $(INCLUDEDIR)/servicios.h
excep_mem: excep_mem.o $(BIBLIOTECA)
	$(CC) -shared -o $@ excep_mem.o -L$(LIBDIR) -lserv 

simplon.o: $(INCLUDEDIR)/servicios.h
simplon: simplon.o $(BIBLIOTECA)
	$(CC) -shared -o $@ simplon.o -L$(LIBDIR) -lserv 

idproceso.o: $(INCLUDEDIR)/servicios.h
idproceso: idproceso.o $(BIBLIOTECA)
	 $(CC) -shared -o $@ idproceso.o -L$(LIBDIR) -lserv 
Pidproceso.o: $(INCLUDEDIR)/servicios.h
Pidproceso: Pidproceso.o $(BIBLIOTECA)
	$(CC) -shared -o $@ Pidproceso.o -L$(LIBDIR) -lserv
clean:
	rm -f *.o $(PROGRAMAS)
	cd lib; make clean

