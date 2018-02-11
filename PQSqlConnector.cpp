#include <iostream>
#include <algorithm>
#include "PQSqlConnector.h"

const bool DEBUG = false; //used only for debug;
const bool VERBOSE = false; //disable notification;

using namespace std;

const int CONNECTION_FAILURE_RETRY = 10; //Time to retry in case of connection failure
const int NON_TRANSACTION_QUERY = 0; // For handle_broken_connection
const int TRANSACTION_QUERY = 1; // handle_broken_connection
const std::string DATA_TYPE_X = "float(53)"; //Data Type for field "X"
const std::string DATA_TYPE_Y = "float(53)"; //Data Type for field "Y"
const std::string DATA_TYPE_Z = "float(53)"; //Data Type for field "Z"
//please refer to https://www.postgresql.org/docs/9.5/static/datatype-numeric.html


PQSqlConnector::PQSqlConnector(const std::string& connstring_input)
{
    connstring = connstring_input;
    if (connstring.empty())
    {
        conn = new connection();
    }
    else
    {
        conn = new connection(connstring);
    }
}


PQSqlConnector::~PQSqlConnector()
{
    delete conn;
}

bool PQSqlConnector::connectionIsOpen()
{
	try
	{
		if(conn->is_open())
    	{
        	std::cout << "Successfully connected to: " << conn->dbname() << std::endl;
        	return 1;        	
   		}
   		
   		std::cout << "Cannot connected to: " << conn->hostname() << " or "
   				  << conn->dbname() << std::endl;
    	return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
        return 0;
	}
}

bool PQSqlConnector::createTable(const std::string& table_name_input)
{
    try
    {
        std::string table_name = table_name_input;
        if(table_name.empty()){
            std::cerr<<"Table name is empty"<<std::endl;
            return 1;
        }
        
        transform(table_name.begin(), table_name.end(), table_name.begin(), ::tolower);
        
        pqxx::result table_exists = non_trans_query("SELECT relname FROM pg_class WHERE relname = '" + table_name + "';");
        //query to check existence of the table.
        
        if(table_exists.empty())
	    {
		    if (VERBOSE)
		    {
		    	std::cout<<"Table not found. It will be created."<<std::endl;
		    }		    

		    std::string query_create_table = "CREATE TABLE " + table_name + "("
			    "X "+ DATA_TYPE_X + ", "
			    "Y "+ DATA_TYPE_Y + ", "
			    "Z "+ DATA_TYPE_Z + " NOT NULL, "
			    "PRIMARY KEY (X,Y) "
			    ");"; 
			//This defines the SQL to create the table with name of [table_name]
			//Primary Key is (X,Y) and field Z should not be Null
			//Please refer to https://www.postgresql.org/docs/9.6/static/ddl-constraints.html
			//For more details
			
		    trans_query(query_create_table);
		    
		    std::cout<<"Table " << table_name<< " has been successfully created."<<std::endl;
	    }
	    return 0;        
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::dropTable(const std::string& table_name_input)
{
    try
    {
        trans_query("DROP TABLE " + table_name_input);
        std::cout<<"Table " << table_name_input<< " has been successfully dropped."<<std::endl;
        return 0;
        
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::insertPoint(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input)
{
    try
    {
        std::string value_tmp = to_string(X_input)+","+to_string(Y_input)+","+
                                to_string(Z_input);
        
        std::string query_insert = "INSERT INTO " + table_name_input + 
                                   " (X, Y, Z) VALUES ("+ value_tmp +  ");";

        trans_query(query_insert);
        
        if (VERBOSE)
        {
        	std::cout<<"X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input 
		            << " successfully inserted into " << table_name_input <<std::endl;
        }
        
		return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::updatePoint(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input)
{
    try
    {
        pqxx::result record_exists = non_trans_query("SELECT X,Y FROM "+ table_name_input +
                                                     " WHERE X = " + to_string(X_input) +
                                                     " AND Y= " + to_string(Y_input) + 
                                                     ";");
        if (record_exists.empty())
        {
        	if (VERBOSE)
        	{
        		std::cout<<"Point ("<<X_input<<","<<Y_input<<")"<<"does not exist, will insert it"<<std::endl;
        	}
            
            return insertPoint(table_name_input,X_input,Y_input,Z_input);
        }
        
        std::string query_update = "UPDATE " + table_name_input + 
                                   " SET Z = "+ to_string(Z_input) +
                                   " WHERE X= "+ to_string(X_input) + 
                                   " AND Y= " + to_string(Y_input) + 
                                   ";";
        trans_query(query_update);
        
        if (VERBOSE)
        {
        	std::cout<<"X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input 
		            << " successfully updated into " << table_name_input<<std::endl;
        }
        
		return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::deletePoint(const std::string& table_name_input, const double& X_input, const double& Y_input)
{
    try
    {
    	std::string query_delete = "DELETE FROM " + table_name_input +
                                   " WHERE X= "+ to_string(X_input) + 
                                   " AND Y= " + to_string(Y_input) + 
                                   ";";
                                   
        trans_query(query_delete);
        
        std::cout<<"X: "<< X_input <<"Y: "<< Y_input
		            << " successfully deleted from " << table_name_input<<std::endl;
		return 0;		            
		
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::disconnectDB(void)
{
    try
    {
        conn->disconnect();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

pqxx::result PQSqlConnector::non_trans_query(const std::string &query)
{
    try
    {
    
    	if (DEBUG)
    	{
    		//Print the yellow in blue for debug purposes.
        	std::cout << "\033[33mRunning non-transactional query: " << query << "\033[0m" << std::endl;
    	}        

	    pqxx::nontransaction nt(*conn);
	    pqxx::result tq(nt.exec(query));
	    return tq;
    }
    catch (const pqxx::pqxx_exception &e)
    {
        std::cerr << e.base().what() << std::endl;
        const pqxx::sql_error *sqlerr=dynamic_cast<const pqxx::sql_error*>(&e.base());
        const pqxx::broken_connection *brk_conn=dynamic_cast<const pqxx::broken_connection*>(&e.base());
        if (sqlerr) 
        {
        	std::cerr << "Query was: " << sqlerr->query() << std::endl;        	
        }
        
        if (brk_conn)
        {
        	return handle_broken_connection(NON_TRANSACTION_QUERY,query);
        }
        
        return pqxx::result();           
    }
}

void PQSqlConnector::trans_query(const std::string &query)
{
    try
    {
    	if (DEBUG)
    	{
    		//Print the query in blue for debug purposes.
	    	std::cout << "\033[34mRunning transactional query: " << query << "\033[0m" <<std::endl;
    	}       	

	    pqxx::work twerk(*conn);
	    twerk.exec(query);
	    twerk.commit();
	    return; 
    }
    catch (const pqxx::pqxx_exception &e)
    {
        std::cerr << e.base().what() << std::endl;
        const pqxx::sql_error *sqlerr=dynamic_cast<const pqxx::sql_error*>(&e.base());
        const pqxx::broken_connection *brk_conn=dynamic_cast<const pqxx::broken_connection*>(&e.base());
        if (sqlerr) 
        {
        	std::cerr << "Query was: " << sqlerr->query() << std::endl;
        }
        
        if (brk_conn)
        {
        	handle_broken_connection(TRANSACTION_QUERY,query);
        }
        return;           
    }

}

pqxx::result PQSqlConnector::handle_broken_connection(const int& query_case, const std::string &query)
{
    if (failure_time > CONNECTION_FAILURE_RETRY)
    {
        failure_time = 0;
        return pqxx::result();
    }
    failure_time++;
    switch(query_case)
    {
        case NON_TRANSACTION_QUERY: return non_trans_query(query);
        case TRANSACTION_QUERY    : trans_query(query);        							
    } 
    
    return pqxx::result();  
}

