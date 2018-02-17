#include "PQSqlConnector.h"
#include <csignal>
#include <iostream>

using namespace std;

string connection_config = "dbname=test user=postgres password=postgres hostaddr=127.0.0.1 port=5432";

int main (int argc, char const* argv[])
{

	signal(SIGPIPE, SIG_IGN); 
	// When lost or failed backend connection happens on Unix-like systems, you may also get a SIGPIPE signal. That signal aborts the program by default, so if you wish to be able to continue after a connection breaks, be sure to disarm this signal.If you're working on a Unix-like system, see the manual page for signal (2) on how to deal with SIGPIPE. The easiest way to make this signal harmless is to make your program ignore it:

    PQSqlConnector sql(connection_config);
    
    if(sql.connectionIsOpen())
        cout<<"Successful"<<endl;
        
    sql.createTable("test");
    sql.createTable("Test");
    
    sql.insertSinglePoint("Test", 10.97, 11.5,13.6);
    sql.insertSinglePoint("test", 10.97, 11.4,13.6);
    
    sql.updateSinglePoint("Test", 10.97,11.4, 22);
    sql.updateSinglePoint("test", 1,2,3);
    
    sql.deleteSinglePoint("Test", 1,2);
    
    sql.dropTable("test");
    sql.disconnectDB();
        
    
    return 0;
}
