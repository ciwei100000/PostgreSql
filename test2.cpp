#include "PQSqlConnector.h"
#include <csignal>
#include <iostream>
#include <chrono>
#include <random>

using namespace std;

string connection_config = "dbname=test user=postgres password=postgres hostaddr=127.0.0.1 port=5432";

double randomNum();

int main (int argc, char const* argv[])
{

	signal(SIGPIPE, SIG_IGN); 
	// When lost or failed backend connection happens on Unix-like systems, you may also get a SIGPIPE signal. That signal aborts the program by default, so if you wish to be able to continue after a connection breaks, be sure to disarm this signal.If you're working on a Unix-like system, see the manual page for signal (2) on how to deal with SIGPIPE. The easiest way to make this signal harmless is to make your program ignore it
	
	double X,Y,Z;

    PQSqlConnector sql(connection_config);
    
    if(sql.connectionIsOpen())
        cout<<"Successful"<<endl;
        
    sql.createTable("test");
    
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    
    for(uint i = 0; i<10000; i++){
    	X = (double)(rand()/(double)RAND_MAX);
        Y = (double)(rand()/(double)RAND_MAX);
        Z = (double)(rand()/(double)RAND_MAX);
    	
    	//sql.keepConnectionAlive();
    	sql.insertSinglePoint("test", X,Y,Z);
    }
    
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    
    cout<<"Consumer Time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
    	<<" ms"<<endl;
        
        
    sql.dropTable("test");
    sql.disconnectDB();
    
        
    
    return 0;
}

double randomNum()
{

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator (seed);
	std::uniform_real_distribution<double> distribution (-100.0,100.0);
	return distribution(generator);
	
}
