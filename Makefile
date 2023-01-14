CXX = gcc
CXXFLAGS = -Wall -O2
LDFLAGS = -lSDL
OBJS = c8_cpu.o
EXEC = c8

EXEC: ${OBJS}
	${CXX} -o ${EXEC} ${OBJS} ${LDFLAGS}

c8_cpu.o: c8_cpu.c c8_cpu.h
	${CXX} ${CXXFLAGS} -c c8_cpu.c 

clean:
	rm -rf c8_cpu.o c8