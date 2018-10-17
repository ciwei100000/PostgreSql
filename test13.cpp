#include "PQSqlConnector.h"
#include <csignal>
#include <iostream>

using namespace std;

const string connection_config = "dbname=test user=postgres password=postgres hostaddr=127.0.0.1 port=5432";

float randomNum();

int main (int argc, char const* argv[])
{

	signal(SIGPIPE, SIG_IGN); 
	// When lost or failed backend connection happens on Unix-like systems, you may also get a SIGPIPE signal. That signal aborts the program by default, so if you wish to be able to continue after a connection breaks, be sure to disarm this signal.If you're working on a Unix-like system, see the manual page for signal (2) on how to deal with SIGPIPE. The easiest way to make this signal harmless is to make your program ignore it
	
	std::vector<float> results;

    PQSqlConnector sql(connection_config);
    
    if(sql.connectionIsOpen())
        cout<<"Successful"<<endl; 
    sql.readPointsVector("test", results);
        
    std::cout<<"num of points is " << results.size()/4<<std::endl;
    std::cout<<"last id is " << results.back()<<std::endl<<std::endl;    
    results.clear();
    
    int start_id = 15;
    sql.readPointsVector("test", results, start_id);
      
    std::cout<<"num of points is " << results.size()/4.0<<std::endl;
    std::cout<<"start id is " << results[3]<<std::endl;
    std::cout<<"last id is " << results.back()<<std::endl<<std::endl;    
    results.clear();
    
    float start_id_f = 17.6;
    sql.readPointsVector("test", results, start_id_f);
      
    std::cout<<"num of points is " << results.size()/4.0<<std::endl;
    std::cout<<"start id (float) is " << results[3]<<std::endl;
    std::cout<<"last id is " << results.back()<<std::endl<<std::endl;
    results.clear();
    
    int limit = 1500;
    
    sql.readPointsVector("test", results, start_id, limit);
      
    std::cout<<"num of points is " << results.size()/4.0<<std::endl;
    std::cout<<"start id is " << results[3]<<std::endl;
    std::cout<<"last id is " << results.back()<<std::endl<<std::endl;    
    results.clear();
    
    sql.readPointsVector("test", results, start_id_f, limit);
      
    std::cout<<"num of points is " << results.size()/4.0<<std::endl;
    std::cout<<"start id (float) is " << results[3]<<std::endl;
    std::cout<<"last id is " << results.back()<<std::endl<<std::endl;
    
    //sql.dropTable("test");
    sql.disconnectDB();
    
        
    
    return 0;
}

