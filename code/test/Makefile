# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles testlock1 testlock2 testfork1 testfork2 testfork3 testexec1 testexec2 testexec3 testCV1 testCV2 testCV3 testCV4 testCV5

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

testlock1.o: testlock1.c
	$(CC) $(CFLAGS) -c testlock1.c
testlock1: testlock1.o start.o
	$(LD) $(LDFLAGS) start.o testlock1.o -o testlock1.coff
	../bin/coff2noff testlock1.coff testlock1

testlock2.o: testlock2.c
	$(CC) $(CFLAGS) -c testlock2.c
testlock2: testlock2.o start.o
	$(LD) $(LDFLAGS) start.o testlock2.o -o testlock2.coff
	../bin/coff2noff testlock2.coff testlock2


testfork1.o: testfork1.c
	$(CC) $(CFLAGS) -c testfork1.c
testfork1: testfork1.o start.o
	$(LD) $(LDFLAGS) start.o testfork1.o -o testfork1.coff
	../bin/coff2noff testfork1.coff testfork1

testfork2.o: testfork2.c
	$(CC) $(CFLAGS) -c testfork2.c
testfork2: testfork2.o start.o
	$(LD) $(LDFLAGS) start.o testfork2.o -o testfork2.coff
	../bin/coff2noff testfork2.coff testfork2

testfork3.o: testfork3.c
	$(CC) $(CFLAGS) -c testfork3.c
testfork3: testfork3.o start.o
	$(LD) $(LDFLAGS) start.o testfork3.o -o testfork3.coff
	../bin/coff2noff testfork3.coff testfork3

testexec1.o: testexec1.c
	$(CC) $(CFLAGS) -c testexec1.c
testexec1: testexec1.o start.o
	$(LD) $(LDFLAGS) start.o testexec1.o -o testexec1.coff
	../bin/coff2noff testexec1.coff testexec1

testexec2.o: testexec2.c
	$(CC) $(CFLAGS) -c testexec2.c
testexec2: testexec2.o start.o
	$(LD) $(LDFLAGS) start.o testexec2.o -o testexec2.coff
	../bin/coff2noff testexec2.coff testexec2

testexec3.o: testexec3.c
	$(CC) $(CFLAGS) -c testexec3.c
testexec3: testexec3.o start.o
	$(LD) $(LDFLAGS) start.o testexec3.o -o testexec3.coff
	../bin/coff2noff testexec3.coff testexec3

testCV1.o: testCV1.c
	$(CC) $(CFLAGS) -c testCV1.c
testCV1: testCV1.o start.o
	$(LD) $(LDFLAGS) start.o testCV1.o -o testCV1.coff
	../bin/coff2noff testCV1.coff testCV1

testCV2.o: testCV2.c
	$(CC) $(CFLAGS) -c testCV2.c
testCV2: testCV2.o start.o
	$(LD) $(LDFLAGS) start.o testCV2.o -o testCV2.coff
	../bin/coff2noff testCV2.coff testCV2

testCV3.o: testCV3.c
	$(CC) $(CFLAGS) -c testCV3.c
testCV3: testCV3.o start.o
	$(LD) $(LDFLAGS) start.o testCV3.o -o testCV3.coff
	../bin/coff2noff testCV3.coff testCV3

testCV4.o: testCV4.c
	$(CC) $(CFLAGS) -c testCV4.c
testCV4: testCV4.o start.o
	$(LD) $(LDFLAGS) start.o testCV4.o -o testCV4.coff
	../bin/coff2noff testCV4.coff testCV4

testCV5.o: testCV5.c
	$(CC) $(CFLAGS) -c testCV5.c
testCV5: testCV5.o start.o
	$(LD) $(LDFLAGS) start.o testCV5.o -o testCV5.coff
	../bin/coff2noff testCV5.coff testCV5


clean:
	rm -f *.o *.coff
