#
# Makefile for make
#
# Compiler: gcc
#

PROGS   = ParkingSys
CFLAGST = -D_REENTRANT -Wall
LIBS    = pthread
FILE    = parkingsys

all: $(PROGS)

ParkingSys: phtrdsMsgLyr.o $(FILE).o
	$(CC) $(CFLAGST) -o ParkingSys $(FILE).o phtrdsMsgLyr.o -l $(LIBS)

ParkingSysTrc: phtrdsMsgLyr.o $(FILE)Trc.o
	$(CC) $(CFLAGST) -o ParkingSysTrc $(FILE)Trc.o phtrdsMsgLyr.o -l $(LIBS)

phtrdsMsgLyr.o : phtrdsMsgLyr.c phtrdsMsgLyr.h pMLusrConf.h
	$(CC) $(CFLAGST) -g -c phtrdsMsgLyr.c

parkingsys.o : $(FILE).c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c parkingsys.c

parkingsysTrc.o : $(FILE)Trc.c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c parkingsysTrc.c

clean:
	rm -f $(PROGS) *~ *.o

# CoffeeMachineFSMtrc : phtrdsMsgLyr.o CoffeeMachineTrc.o
# 	$(CC) $(CFLAGST) -o CoffeeMachineFSMtrc CoffeeMachineTrc.o phtrdsMsgLyr.o -l $(LIBS)

# CoffeeMachineTrc.o : CoffeeMachineTrc.c pMLusrConf.h
# 	$(CC) $(CFLAGST) -g -c CoffeeMachineTrc.c

