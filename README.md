# PostgreSql

---------Install the postgresql server and create a new database "test" for testing

$sudo apt-get install postgresql  //install postgresql server 

sudo -u postgres psql             //log in postgresql client

create database test;            //create a database called "test"

\password                       //set password for postgres to log on server

password: postgres                // set password to postgres

\q

$sudo vim /etc/postgresql/9.5/main/pg_hba.conf

----------change line------------

local   all             postgres                                peer

----------to---------------------

local   all             postgres                                md5

----------save and exit----------

$ sudo service postgresql restart   //restart the server

----------done-------------------

-----------To Manually Drop a table--------

$sudo -u postgres psql
[your su password]
password: postgres

\c test

drop table [table name];

Two test cases: test1.cpp test2.cpp

To use test case I, modify the SRCS in Makefile:

SRCS = PQSqlConnector.cpp test1.cpp

To use test case II, modify the SRCS in Makefile:

SRCS = PQSqlConnector.cpp test2.cpp

Then,

make

Execute:

./test


