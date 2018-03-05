#include "PQSqlConnector.h"
#include <csignal>
#include <iostream>
#include <chrono>
#include <random>

using namespace std;

string connection_config = "dbname=test user=postgres password=postgres hostaddr=127.0.0.1 port=5432";

float randomNum();

int main (int argc, char const* argv[])
{

	signal(SIGPIPE, SIG_IGN); 
	// When lost or failed backend connection happens on Unix-like systems, you may also get a SIGPIPE signal. That signal aborts the program by default, so if you wish to be able to continue after a connection breaks, be sure to disarm this signal.If you're working on a Unix-like system, see the manual page for signal (2) on how to deal with SIGPIPE. The easiest way to make this signal harmless is to make your program ignore it
	
	float X,Y,Z;
	int ID;

    PQSqlConnector sql(connection_config);
    
    if(sql.connectionIsOpen())
        cout<<"Successful"<<endl;
        
    sql.createTable("test");

    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    
    ID = 0;
    X= 0.1;
    Y= 0.1;
    Z= 0.1;
    
    vector<float> insertvalues;
    
    for(uint i = 0; i<100000; i++){
    
		ID++;    
    	X++ ;
        Y++ ;
        Z++ ;
        
    
        insertvalues.push_back(X);
        insertvalues.push_back(Y);
        insertvalues.push_back(Z);
        insertvalues.push_back(ID);
    
    	
    	if(insertvalues.size() % 40000 == 0)
    	{
    		//sql.keepConnectionAlive();
    		sql.upsertPointQueue("test", insertvalues);
    		sql.commitQueue();
    		insertvalues.clear();
    	}
    		
    }
    
    
    
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    
    cout<<"Consumer Time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
    	<<" ms"<<endl;
    
    X = 0.1;
    Y = 0.1;
    Z = 0.1;
    ID = 0;
    
    start = std::chrono::system_clock::now();
    
    vector<float> updatevalues;
    
    for(uint i = 0; i<2500; i++){
    	ID++;
    	X=2 ;
        Y=2 ;
        Z=2 ;
        updatevalues.push_back(X);
        updatevalues.push_back(Y);
        updatevalues.push_back(Z);
        updatevalues.push_back(ID);
           	
    }
    

    sql.upsertPointQueue("test", updatevalues);
    updatevalues.clear();

    sql.commitQueue();
    
    end = std::chrono::system_clock::now();
    
    cout<<"Consumer Time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
    	<<" ms"<<endl;
    	
    ID = 0;
    
    start = std::chrono::system_clock::now();
    for(uint i = 0; i<60000; i+=1){
    
    	
    	vector<int> ids;
    	
    	ids.push_back(ID);

		if (i % 100 == 99)
		{
			sql.deletePointQueue("test",ids);
			ids.clear();
		}
    	
    }	
    
    sql.commitQueue();
    
    end = std::chrono::system_clock::now();
    
    cout<<"Consumer Time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
    	<<" ms"<<endl;
        
        
    //sql.dropTable("test");
    sql.disconnectDB();
    
        
    
    return 0;
}

float randomNum()
{

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator (seed);
	std::uniform_real_distribution<float> distribution (-100.0,100.0);
	return distribution(generator);
	
}
