CC=g++
CFLAGS= -O3 -std=c++11 -Wall
LIBS = -lpq -lpqxx
SRCS = PQSqlConnector.cpp test5.cpp
OBJS = $(SRCS:.cpp=.o)
MAIN = test
DEPS = 
INCLUDES=/usr/lib/

.PHONY: clean

all:    $(MAIN)
	@echo  Simple compiler named $(MAIN) has been compiled

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp PQSqlConnector.h
	$(CC) $(CFLAGS) -c $<  -o $@ 

clean:
	$(RM) *.o *~ $(MAIN)

# DO NOT DELETE THIS LINE -- make depend needs it
