ifdef SystemRoot
	RM = del /Q
	EXT = .exe
else
	ifeq ($(shell uname), Linux)
		RM = rm -f
		EXT =
	endif
endif

CC = g++
# -Wall -g
CFLAGS = -std=c++11 -O3 -msse4.2 -fomit-frame-pointer -funroll-loops -lm -I .

all: umsa
	$(CC) $(CFLAGS) -o umsa$(EXT) UnrestrictedMultiShiftAnd.o main.o

umsa: clean
	$(CC) $(CFLAGS) -c UnrestrictedMultiShiftAnd.cpp main.cpp

clean:
	$(RM) UnrestrictedMultiShiftAnd.o main.o umsa$(EXT)
