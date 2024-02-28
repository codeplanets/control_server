######################################
## error meesage : ' cc1plus: error: unrecognized command line option "-std=c++11" '
## resolved: ' scl enable devtoolset-2 bash '
######################################

.SUFFIXES : .cpp .o

CC = g++
AR = ar
EXP = export
CFLAGS = -c -g

SRCS = ${wildcard src/*.cpp}
OBJS = ${SRCS:src/%.cpp=build/%.o}
HEADS = -Ihead
UTESTLIBS = libs/libgtest.a
MOCKLIBS = libs/libgmock.a
LIBPATH = /usr/lib
HEADERPATH = /usr/include
LIBS = -lmysqlclient -lpthread -lrt

TESTSRCS = ${wildcard srctest/*.cpp}
TESTOBJS = ${TESTSRCS:srctest/%.cpp=build/%.o}

GTESTDIR = /workspaces/control_server/googletest
GMOCKDIR = /workspaces/control_server/googlemock

INCLUDE = -I. ${HEADS} -I${GTESTDIR}/include -I${GMOCKDIR}/include -I${HEADERPATH}

ifeq (${MAKECMDGOALS}, test)
TESTFLAG = -std=c++17 -DUNITTEST ${INCLUDE}
else
TESTFLAG = -std=c++17 ${INCLUDE}
endif

TARGET = controlserver

all : ${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${LIBS}
 
build/%.o : src/%.cpp
	${CC} ${CFLAGS} ${TESTFLAG} -pthread -L${LIBPATH} -L/usr/include/mysql ${LIBS} $< -o $@

build/%.o : srctest/%.cpp
	${CC} ${CFLAGS} ${TESTFLAG} -Isrc -L${LIBPATH} ${LIBS} $< -o $@ ${OBJS}

clean :
	rm -f *.o build/*.o
	rm -f libs/*.a
	rm -f ${TARGET} TEST_*
	rm -f *_ipv4*

