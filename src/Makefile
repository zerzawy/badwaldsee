#-*-makefile-*-
#######################################################################
#
#  Copyright 2014 Kurt Zerzawy
#  GPL
#
#  $Id: Makefile.in 929 2013-01-16 19:48:14Z kurt $
#  $Author: kurt $
#  $Date: $
#  
#######################################################################

VPATH           = ../src

binaries = stw
# HWTest HWTest2
libraries =
common =

#CFLAGS = -D DOMINO55
CFLAGS = -D SPDR60
LINK = gcc
.PHONY: compile export
LIBS=-lbcm2835
all: compile export

compile: $(binaries)

# using dependencies to library files not necessary as they are requested
# to be unchanged

#HWTest: HWTest.o HW.o $(addsuffix .o, $(common))

#HWTest2: HWTest2.o HW.o inshift.o outshift.o register.o $(addsuffix .o, $(common))

stw	: main.o ap216.o section.o parser/section.o button.o parser/button.o combishuntroute.o parser/combishuntroute.o comm.o duoled.o parser/duoled.o dwarf.o parser/dwarf.o ep16.o hw.o inshift.o parser/inshift.o led.o parser/led.o log.o distant.o parser/distant.o combiroute.o parser/combiroute.o mains.o parser/mains.o outshift.o parser/outshift.o parser/parser.o registers.o parser/registers.o route.o shuntroute.o parser/shuntroute.o route.o parser/route.o  turnout.o parser/turnout.o vars.o parser/vars.o line.o parser/line.o block.o parser/block.o automat.o parser/automat.o $(addsuffix .o, $(common)) 
	$(LINK) -o $@ $^ $(LIBS) -lpthread $(EXTRA_LDFLAGS_SUFFIX)

# Comm.o Taste.o Zwergsignal.o Hauptsignal.o Vorsignal.o RaFa.o Weiche.o

main.o : main.c 

ap216.o : ap216.c ap216.h

section.o : section.c section.h

parser/section.o : parser/section.c parser/section.h

button.o : button.c button.h 

parser/button.o : parser/button.c parser/button.h

comm.o: comm.c comm.h

duoled.o : duoled.c duoled.h 

parser/duoled.o : parser/duoled.c parser/duoled.h

dwarf.o : dwarf.c dwarf.h 

parser/dwarf.o : parser/dwarf.c parser/dwarf.h

ep16 : ep16.c ep16.h

hw.o : hw.c hw.h

inshift.o : inshift.c inshift.h

parser/inshift.o : parser/inshift.c parser/inshift.h

led.o : led.c led.h 

parser/led : parser/led.c parser/led.h

log.o : log.c log.h

outshift.o : outshift.c outshift.h 

parser/outshift.o : parser/outshift.c parser/outshift.h

parser/parser.o : parser/parser.c parser/parser.h

registers.o : registers.c registers.h 

parser/registers.o : parser/registers.c parser/registers.h

shuntroute.o : shuntroute.c shuntroute.h 

parser/shuntroute.o : parser/shuntroute.c parser/shuntroute.h

turnout.o : turnout.c turnout.h 

parser/turnout.o : parser/turnout.c parser/turnout.h

vars.o : vars.h vars.c 

parser/vars.o : parser/vars.c parser/vars.h

line.o : line.h line.c

parser/line.o : parser/line.h parser/line.c

block.o : block.h block.c

parser/block.o : parser/block.h parser/block.c

automat.o : automat.h automat.c

parser/automat.o : parser/automat.h parser/automat.c

section.o : section.h section.c

parser/section.o : parser/section.h parser/section.c

#parser/parser.o : parser/parser.c parser/parser.h
#	$(CC) $(CFLAGS_MODULE) -D__TEST__ -D__PARSER_TEST__ $(CFLAGS_INC) -o $@ -c $<  

#registers.o : registers.c registers.h
#	$(CC) $(CFLAGS_MODULE) -D__TEST__ $(CFLAGS_INC) registers.c -o registers.o

#outshift.o : outshift.c outshift.h
#	$(CC) $(CFLAGS_MODULE) -D__TEST__ $(CFLAGS_INC) outshift.c -o outshift.o

#inshift.o : inshift.c inshift.h
#	$(CC) $(CFLAGS_MODULE) -D__TEST__ $(CFLAGS_INC) inshift.c -o inshift.o

#vars.o : vars.c vars.h
#	$(CC) $(CFLAGS_MODULE) -D__TEST__ $(CFLAGS_INC) vars.c -o vars.o

#HW.o : HW.c HW.h test/HW.c
#	$(CC) $(CFLAGS_MODULE) -D__TEST__ $(CFLAGS_INC)  -I /targets/LxNETES/project_unc20/linux/include HW.c  -o HW.o

#Weiche.o : Weiche.c Weiche.h parser/Weiche.c


HWTest.o: HWTest.c $(addsuffix .o, $(common))
#	$(CC) $(CFLAGS_MODULE) $(CFLAGS_INC) -o $@ -c $<


HWTest2.o : HWTest2.c
#	$(CC) $(CFLAGS_MODULE) $(CFLAGS_INC) -o $@ -c $<


#HW.o : HW.c HW.h test/HW.c
#	$(CC) $(CFLAGS_MODULE) $(CFLAGS_INC) -I /targets/LxNETES/project_unc20/linux/include -c -o $@ $<




clean::
	rm -f core *.o parser/*.o $(binaries) $(addsuffix .gdb, $(binaries))

