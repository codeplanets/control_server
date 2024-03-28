
.SUFFIXES : .cpp .o

CC = g++
AR = ar
EXP = export
CFLAGS = -c -g

SRCS = ${wildcard *.cpp}
OBJS = ${SRCS:%.cpp=%.o}
LIBPATH = /usr/lib
HEADERPATH = /usr/include
LIBS = -lpthread

INCLUDE = -I. -I${HEADERPATH}

TESTFLAG = -std=c++17 ${INCLUDE}

TARGET = initsem

all : ${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${LIBS}

%.o : %.cpp
	${CC} ${CFLAGS} ${TESTFLAG} -pthread -L${LIBPATH} ${LIBS} $< -o $@

clean :
	rm -f *.o
	rm -f ${TARGET}

