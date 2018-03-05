#include "PQSqlConnector.h"
#include <csignal>
#include <iostream>
#include <vector>

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
    
    sql.insertPointQueue("Test", 1, 10.97, 11.5,13.6);
    sql.insertPointQueue("test", 2, 10.97, 11.4,13.6);
    
    sql.insertPointQueue("test", 3, 1,2,3);
    sql.insertPointQueue("test", 4, 1,2,3);
    sql.insertPointQueue("test", 5, 1,2,3);
    
    float u[] = {1.0, 3.5, 4,3,10.97,11.4,21,2};
    vector<float> updatearray(u,u+8);
    sql.updatePointQueue("test", updatearray);
    
    int n[] = {5, 6};
    vector<int> deletearray(n,n+3);
    
    sql.deletePointQueue("Test", deletearray);
    
    sql.commitQueue();
    
    //sql.dropTable("test");
    sql.disconnectDB();
        
    
    return 0;
}
