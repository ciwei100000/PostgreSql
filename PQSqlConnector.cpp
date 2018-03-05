#include <iostream>
#include <algorithm>
#include "PQSqlConnector.h"

const bool DEBUG = false; //used only for debug;
const bool VERBOSE = false; //disable notification;

using namespace std;

const int CONNECTION_FAILURE_RETRY = 10; //Time to retry in case of connection failure
const std::string DATA_TYPE_ID = "INT"; //Data Type for field "X"
const std::string DATA_TYPE_X = "real"; //Data Type for field "X"
const std::string DATA_TYPE_Y = "real"; //Data Type for field "Y"
const std::string DATA_TYPE_Z = "real"; //Data Type for field "Z"
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

void PQSqlConnector::keepConnectionAlive()
{
	non_trans_query("SELECT 1");
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
		    	"ID "+ DATA_TYPE_ID + " PRIMARY KEY" + ", "
			    "X "+ DATA_TYPE_X + ", "
			    "Y "+ DATA_TYPE_Y + ", "
			    "Z "+ DATA_TYPE_Z +
			    //"PRIMARY KEY (X,Y) "
			    ");"; 
			//This defines the SQL to create the table with name of [table_name]
			//Primary Key is (X,Y) and field Z should not be Null
			//Please refer to https://www.postgresql.org/docs/9.6/static/ddl-constraints.html
			//For more details
			
		    trans_query(query_create_table);
		    
		    if (VERBOSE)
		    {
		    	std::cout<<"Table " << table_name<< " has been successfully created."<<std::endl;
		    }
		    
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
        if (VERBOSE)
        {
        	std::cout<<"Table " << table_name_input<< " has been successfully dropped."<<std::endl;
        }        
        return 0;
        
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::insertSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input)
{
    try
    {
        std::string value_tmp = to_string(ID)+"," 
        					  + to_string(X_input)+","
        					  + to_string(Y_input)+","
                              + to_string(Z_input);
        
        std::string query_insert = "INSERT INTO " + table_name_input + 
                                   " (ID, X, Y, Z) VALUES ("+ value_tmp +  ");";

        trans_query(query_insert);
        
        if (VERBOSE)
        {
        	std::cout<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input 
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

bool PQSqlConnector::insertPointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input) //Add one insertPoint to operation Queue;
{

	try
	{
		std::string value_tmp = to_string(ID)+"," 
        					  + to_string(X_input)+","
        					  + to_string(Y_input)+","
                              + to_string(Z_input);
        
        std::string query_insert = "INSERT INTO " + table_name_input + 
                                   " (ID, X, Y, Z) VALUES ("+ value_tmp +  ");" + "\n";
                                   
        this->queue += query_insert;
        
        if (VERBOSE)
        {
        	cout<<"Successfully add insertPoint ( "<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input
        		<<" ) operation to queue"<<endl;
        }
        
        return 0;	
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }	
		
}

bool PQSqlConnector::updateSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input)
{
    try
    {
        pqxx::result record_exists = non_trans_query("SELECT X,Y FROM "+ table_name_input +
        											 " WHERE ID = " + to_string(ID) +
                                                     //" WHERE X = " + to_string(X_input) +
                                                     //" AND Y= " + to_string(Y_input) + 
                                                     ";");
        if (record_exists.empty())
        {
        	if (VERBOSE)
        	{
        		std::cout<<"Point ("<<ID<<")"<<"does not exist, will insert it"<<std::endl;
        	}
            
            return insertSinglePoint(table_name_input,ID, X_input,Y_input,Z_input);
        }
        
        std::string query_update = "UPDATE " + table_name_input + 
                                   " SET X = "+ to_string(X_input) + "," +
                                   " Y= "+ to_string(Y_input) + "," +
                                   " Z= " + to_string(Z_input) + 
                                   " WHERE ID= " + to_string(ID) +
                                   ";";
        trans_query(query_update);
        
        if (VERBOSE)
        {
        	std::cout<<"ID: "<<ID<<" X: "<< X_input <<" Y: "<< Y_input <<" Z: "<< Z_input 
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

bool PQSqlConnector::updatePointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input) // Add one updatePoint to operation Queue;
{

	try
	{
		std::string query_update = "UPDATE " + table_name_input + 
                                   " SET X = "+ to_string(X_input) + "," +
                                   " Y= "+ to_string(Y_input) + "," +
                                   " Z= " + to_string(Z_input) + 
                                   " WHERE ID= " + to_string(ID) +
                                   ";" + "\n";
        
    	this->queue += query_update;
    
    	if (VERBOSE)
        {
        	cout<<"Successfully add updatePoint ( "<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input
        		<<" ) operation to queue"<<endl;
        }
        
        return 0;
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }   
}

bool PQSqlConnector::updatePointQueue(const std::string& table_name_input, const vector<float>& values)
{
	try
	{
		std::string query_update = "UPDATE " + table_name_input + 
                                   " SET X = tmp.X,"
                                   " Y= tmp.Y,"
                                   " Z= tmp.Z"
                                   " FROM (" + "VALUES ";
		
		for (unsigned int i = 0; i < values.size(); i += 4)
		{
			if ((values.size() - i) > 3)
			{
				query_update += "(" + 
				                to_string((int) values[i]) + "," + 
				                to_string(values[i+1]) + "," +
				                to_string(values[i+2]) + "," +
				                to_string(values[i+3]) + ")";
				
				if ((values.size() - i) > 4)
				{
					query_update += ",";
				}
			}
		}
                                   
        query_update += ") AS tmp(ID,X,Y,Z)"
                        " WHERE " + table_name_input + 
                        ".ID=tmp.ID;" 
                        "\n";
        
    	this->queue += query_update;
        
        return 0;
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::deleteSinglePoint(const std::string& table_name_input, const int& ID)
{
    try
    {
    	std::string query_delete = "DELETE FROM " + table_name_input +
    							   " WHERE ID= " + to_string(ID) +
                                   ";";
                                   
        trans_query(query_delete);
        
        std::cout<<"ID: "<<ID
		            << " successfully deleted from " << table_name_input<<std::endl;
		return 0;		            
		
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

bool PQSqlConnector::deletePointQueue(const std::string& table_name_input, const int& ID)
{

	try
	{
		std::string query_delete = "DELETE FROM " + table_name_input +
    							   " WHERE ID= " + to_string(ID) +
                                   ";" + "\n";
    	this->queue += query_delete;
    	
    	if (VERBOSE)
        {
        	cout<<"Successfully add deletePoint ( ""ID: "<<ID
        		<<" ) operation to queue"<<endl;
        }
        
        return 0;
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }                    
}

bool PQSqlConnector::deletePointQueue(const std::string& table_name_input, const vector<int>& ids)
{
	try
	{
		std::string query_delete = "DELETE FROM " + table_name_input +
    							   " WHERE ID IN (";
		for (unsigned int i = 0; i < ids.size(); i += 1)
		{
			query_delete += to_string(ids[i]);
			if (i != ids.size()-1)
			{
				query_delete += ",";
			}
		}
		query_delete += ");\n";
						
		this->queue += query_delete;
		
		return 0;
	
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
	

}

bool PQSqlConnector::commitQueue()
//Commit and empty the operations in the operation queue
{
	try
	{
		if (!this->queue.empty())
		{
			string query_commit = "Begin;\n" + this->queue + "End;";
	
			trans_query(query_commit);
			
			this->queue = "";
		}
		return 0;
		
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        this->queue = "";
        return 1;
    }	
}

bool PQSqlConnector::clearQueue()
//empty the operation queue
{
	this->queue = "";
	return 0;
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
        	return handle_broken_connection_nontrans(query);
        }
        
        return pqxx::result();           
    }
}

bool PQSqlConnector::trans_query(const std::string &query)
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
	    return 0; 
    }
    catch (const pqxx::pqxx_exception &e)
    {
        std::cerr << e.base().what() << std::endl;
        const pqxx::sql_error *sqlerr=dynamic_cast<const pqxx::sql_error*>(&e.base());
        const pqxx::broken_connection *brk_conn=dynamic_cast<const pqxx::broken_connection*>(&e.base());
        
        if (brk_conn)
        {
        	return handle_broken_connection_trans(query);
        } 
        if (sqlerr) 
        {
        	std::cerr << "Query was: " << sqlerr->query() << std::endl;
        }
        return 1;                 
    }

}

bool PQSqlConnector::handle_broken_connection_trans(const std::string &query)
{
    if (failure_time > CONNECTION_FAILURE_RETRY)
    {
        failure_time = 0;
        return 1;
    }
    failure_time++;
        
    return trans_query(query);
}

pqxx::result PQSqlConnector::handle_broken_connection_nontrans(const std::string &query)
{
	if (failure_time > CONNECTION_FAILURE_RETRY)
    {
        failure_time = 0;
        return pqxx::result();
    }
    failure_time++;
        
    return non_trans_query(query);
}

