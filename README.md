# PostgreSql

Two test cases: test1.cpp test2.cpp

To use test case I, modify the SRCS in Makefile:

SRCS = PQSqlConnector.cpp test1.cpp

To use test case II, modify the SRCS in Makefile:

SRCS = PQSqlConnector.cpp test2.cpp

Then,

make

Execute:

./test


