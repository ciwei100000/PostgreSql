#include <iostream>
#include <algorithm>
#include "PQSqlConnector.h"

#define LIKELY(condition) __builtin_expect((condition), 1)
#define UNLIKELY(condition) __builtin_expect((condition), 0)

PQSqlConnector::PQSqlConnector(const std::string& connstring_input)
{
    connstring = connstring_input;
    if (UNLIKELY(connstring.empty()))
    {
        conn = new pqxx::connection();
    }
    else
    {
        conn = new pqxx::connection(connstring);
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
	}//try
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
        return 0;
	}//catch
}

void PQSqlConnector::keepConnectionAlive()
{
	non_trans_query("SELECT 1");
}

int PQSqlConnector::createTable(const std::string& table_name_input)
{

    /*
    SQL Command Template:
    
    CREATE TABLE [TABLE NAME] (ID PRIMARY KEY,X,Y,Z);
    
    */
    
    int ret = 0;
    try
    {
        std::string table_name = table_name_input;
        if(table_name.empty()){
            std::cerr<<"Table name is empty"<<std::endl;
            return -1;
        }
        
        //convert TABLE NAME to lower case
        transform(table_name.begin(), table_name.end(), table_name.begin(), ::tolower);
        
        /*
        query to check existence of the table.
        SQL Command Template:
        
        SELECT relname FROM pg_class WHERE relname = [TABLE NAME];
        
        */
        pqxx::result table_exists = non_trans_query("SELECT relname FROM pg_class WHERE relname = '" + table_name + "';");
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
			/*
			This defines the SQL to create the table with name of [table_name]
			Primary Key is (X,Y) and field Z should not be Null
			Please refer to https://www.postgresql.org/docs/9.6/static/ddl-constraints.html
			For more details
			*/
			
		    ret = trans_query(query_create_table);
		    
		    if (VERBOSE)
		    {
		        if (!ret)
		        {
		    	    std::cout<<"Table " << table_name<< " has been successfully created."<<std::endl;
		    	}
		    	else
		    	{
		    	    std::cerr<<"Table " << table_name<< " can not be created."<<std::endl;
		    	}
		    }
		    
	    }//if(table_exists.empty())
	    else if (VERBOSE)
		{
		    std::cout<<"Table found. No action will taken"<<std::endl;
	    }
	    return ret;        
    }//try
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }//catch
}//createTable

int PQSqlConnector::dropTable(const std::string& table_name_input)
{

    /*
    SQL Command Template:
    
    DROP TABLE [TABLE NAME];
    
    */

    try
    {
        int ret = trans_query("DROP TABLE " + table_name_input);
        if (VERBOSE)
        {
            if (!ret)
            {
               std::cout<<"Table " << table_name_input<< " has been successfully dropped."<<std::endl;
        	}
        	else
        	{
        	   std::cerr<<"Table " << table_name_input<< " can not be dropped."<<std::endl;
        	}
        }        
        return ret;
        
    }//try
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }//catch
}//dropTable

int PQSqlConnector::insertSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input)
{

    /*
    SQL Command Template:
    
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ( [id],[x],[y],[z]);
    
    */
    
    try
    {
        std::string value_tmp = std::to_string(ID)+"," 
        					  + std::to_string(X_input)+","
        					  + std::to_string(Y_input)+","
                              + std::to_string(Z_input);
        
        std::string query_insert = "INSERT INTO " + table_name_input + 
                                   " (ID, X, Y, Z) VALUES ("+ value_tmp +  ");";

        int ret = trans_query(query_insert);
        
        if (VERBOSE)
        {
            if (!ret)
            {
                std::cout<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input 
		            << " successfully inserted into " << table_name_input <<std::endl;
            }
            else
            {
                std::cerr<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input 
		            << " cannot be inserted into " << table_name_input <<std::endl;
            }
        	
        }
        
		return ret;
    }//try
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }//catch
}//insertSinglePoint

int PQSqlConnector::insertPointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input) //Add one insertPoint to operation Queue;
{

    /*
    SQL Command Template:
    
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ( [id1],[x1],[y1],[z1]);
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ( [id2],[x2],[y2],[z2]);
    ...
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ( [idn],[xn],[yn],[zn]);    
    
    */

	try
	{
		std::string value_tmp = std::to_string(ID)+"," 
        					  + std::to_string(X_input)+","
        					  + std::to_string(Y_input)+","
                              + std::to_string(Z_input);
        
        std::string query_insert = "INSERT INTO " + table_name_input + 
                                   " (ID, X, Y, Z) VALUES ("+ value_tmp +  ");" + "\n";
                                   
        this->queue += query_insert;
        
        if (VERBOSE)
        {
        	std::cout<<"Successfully add insertPoint ( "<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input
        		<<" ) operation to queue"<<std::endl;
        }
        
        return 0;	
	}//try
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }	//catch
		
}//insertPointQueue

int PQSqlConnector::updateSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input)
{
    
    /*
    SQL Command Template:
    
    UPDATE [TABLE_NAME] SET X=[x], Y=[y], Z=[z] WHERE [TABLE_NAME].ID = [id];
    
    */
    
    try
    {
        /*
        pqxx::result record_exists = non_trans_query("SELECT X,Y FROM "+ table_name_input +
        											 " WHERE ID = " + std::to_string(ID) +
                                                     //" WHERE X = " + std::to_string(X_input) +
                                                     //" AND Y= " + std::to_string(Y_input) + 
                                                     ";");
        if (record_exists.empty())
        {
        	if (VERBOSE)
        	{
        		std::cout<<"Point ("<<ID<<")"<<"does not exist, will insert it"<<std::endl;
        	}
            
            return insertSinglePoint(table_name_input,ID, X_input,Y_input,Z_input);
        }//if (record_exists.empty())
        */
        
        std::string query_update = "UPDATE " + table_name_input + 
                                   " SET X = "+ std::to_string(X_input) + "," +
                                   " Y= "+ std::to_string(Y_input) + "," +
                                   " Z= " + std::to_string(Z_input) + 
                                   " WHERE ID= " + std::to_string(ID) +
                                   ";";
        int ret = trans_query(query_update);
        
        if (VERBOSE)
        {
            if (!ret)
            {
                std::cout<<"ID: "<<ID<<" X: "<< X_input <<" Y: "<< Y_input <<" Z: "<< Z_input 
		            << " successfully updated for " << table_name_input<<std::endl;
            }
            else
            {
                std::cout<<"ID: "<<ID<<" X: "<< X_input <<" Y: "<< Y_input <<" Z: "<< Z_input 
		            << " cannot be updated for " << table_name_input<<std::endl;
            }
        	
        }
        
		return ret;
    }//try
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }//catch
}//updateSinglePoint

int PQSqlConnector::updatePointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input) // Add one updatePoint to operation Queue;
{

    /*
    SQL Command Template:
    
    UPDATE [TABLE_NAME] SET X=[x1], Y=[y1], Z=[z1] WHERE [TABLE_NAME].ID = [id1];
    UPDATE [TABLE_NAME] SET X=[x2], Y=[y2], Z=[z2] WHERE [TABLE_NAME].ID = [id2];
    ...
    UPDATE [TABLE_NAME] SET X=[xn], Y=[yn], Z=[zn] WHERE [TABLE_NAME].ID = [idn];
    
    */
    
	try
	{
		std::string query_update = "UPDATE " + table_name_input + 
                                   " SET X = "+ std::to_string(X_input) + "," +
                                   " Y= "+ std::to_string(Y_input) + "," +
                                   " Z= " + std::to_string(Z_input) + 
                                   " WHERE ID= " + std::to_string(ID) +
                                   ";" + "\n";
        
    	this->queue += query_update;
    
    	if (VERBOSE)
        {
        	std::cout<<"Successfully add updatePoint ( "<<"ID: "<<ID<<" X: "<< X_input <<"Y: "<< Y_input <<"Z: "<< Z_input
        		<<" ) operation to queue"<<std::endl;
        }
        
        return 0;
	}//try
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }  //catch 
}//updatePointQueue(point)

int PQSqlConnector::updatePointQueue(const std::string& table_name_input, const std::vector<float>& values)
{

    /*
    SQL Command Template:
    
    UPDATE [TABLE_NAME] SET X=tmp.X, Y=tmp.Y, Z=tmp.Z FROM (VALUES ([id1],[x1],[y1],[z1]),([id2],[x2],[y2],[z2]),...)
     AS tmp(ID,X,Y,Z)
     WHERE [TABLE_NAME].ID = tmp.ID;
    
    */
    
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
				                std::to_string((int) values[i+3]) + "," + 
				                std::to_string(values[i]) + "," +
				                std::to_string(values[i+1]) + "," +
				                std::to_string(values[i+2]) + ")";
				
				if ((values.size() - i) > 4)
				{
					query_update += ",";
				}
			}//if ((values.size() - i) > 3)
		}//for
                                   
        query_update += ") AS tmp(ID,X,Y,Z)"
                        " WHERE " + table_name_input + 
                        ".ID=tmp.ID;" 
                        "\n";
        
    	this->queue += query_update;
        
        return 0;
	}//try
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }//catch
}//updatePointQueue(vector)

int PQSqlConnector::upsertPointQueue(const std::string& table_name_input, const std::vector<float>& values)
{

    /*
    SQL Command Template:
    
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ([id1],[x1],[y1],[z1]),([id2],[x2],[y2],[z2]),... ON CONFLICT(ID)
     UPDATE SET X=excluded.X,Y=excluded.Y,Z=excluded.Z;
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ([id11],[x11],[y11],[z1]),([id2],[x2],[y2],[z2]),... ON CONFLICT(ID)
     UPDATE SET X=excluded.X,Y=excluded.Y,Z=excluded.Z;
    ...
    INSERT INTO [TABLE_NAME] (ID,X,Y,Z) VALUES ([idn1],[xn1],[yn1],[zn1]),([idn2],[xn2],[yn2],[z2n]),... ON CONFLICT(ID)
     UPDATE SET X=excluded.X,Y=excluded.Y,Z=excluded.Z;
    
    */
    
	try
	{
		std::string query_upsert = "INSERT INTO " + table_name_input +
								   " (ID, X, Y, Z) VALUES ";
		
		for (unsigned int i = 0; i < values.size(); i += 4)
		{
			if ((values.size() - i) > 3)
			{
				query_upsert += "(" + 
				                std::to_string((int) values[i+3]) + "," + 
				                std::to_string(values[i]) + "," +
				                std::to_string(values[i+1]) + "," +
				                std::to_string(values[i+2]) + 
				                ")";
				
				if ((values.size() - i) > 4)
				{
					query_upsert += ",";
				}
			}
		}
		
		query_upsert += " ON CONFLICT(ID) DO"
						" UPDATE SET "
						" X = excluded.X,"
						" Y = excluded.Y,"
						" Z = excluded.Z"
						";\n";
		
		this->queue += query_upsert;				
		
		return 0;
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

int PQSqlConnector::deleteSinglePoint(const std::string& table_name_input, const int& ID)
{
	
	/*
	SQL Command Template:
    
    DELETE FROM [TABLE_NAME] WHERE ID = [id];
    
    */
    
    try
    {
    	std::string query_delete = "DELETE FROM " + table_name_input +
    							   " WHERE ID= " + std::to_string(ID) +
                                   ";";
                                   
        int ret = trans_query(query_delete);
        
        if(!ret)
        {
            std::cout<<"ID: "<<ID
		            << " successfully deleted from " << table_name_input<<std::endl;
		}
		else
		{
			std::cerr<<"ID: "<<ID
		            << " cannot be deleted from " << table_name_input<<std::endl;
		}
		return ret;		            
		
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

int PQSqlConnector::deletePointQueue(const std::string& table_name_input, const int& ID)
{

    /*
	SQL Command Template:
    
    DELETE FROM [TABLE_NAME] WHERE ID = [id1];
    DELETE FROM [TABLE_NAME] WHERE ID = [id2];
    ...
    DELETE FROM [TABLE_NAME] WHERE ID = [idn];
    
    */
    
	try
	{
		std::string query_delete = "DELETE FROM " + table_name_input +
    							   " WHERE ID= " + std::to_string(ID) +
                                   ";" + "\n";
    	this->queue += query_delete;
    	
    	if (VERBOSE)
        {
        	std::cout<<"Successfully add deletePoint ( ""ID: "<<ID
        		<<" ) operation to queue"<<std::endl;
        }
        
        return 0;
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }                    
}

int PQSqlConnector::deletePointQueue(const std::string& table_name_input, const std::vector<int>& ids)
{

    /*
	SQL Command Template:
    
    DELETE FROM [TABLE_NAME] WHERE ID IN (id1,id2,...,idn);
    
    */
	try
	{
		std::string query_delete = "DELETE FROM " + table_name_input +
    							   " WHERE ID IN (";
		for (unsigned int i = 0; i < ids.size(); i += 1)
		{
			query_delete += std::to_string(ids[i]);
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
        return -1;
    }
	

}

int PQSqlConnector::commitQueue()
//Commit and empty the operations in the operation queue
{

    int ret = 0;
	try
	{
		if (!this->queue.empty())
		{
			std::string query_commit = "Begin;\n" + this->queue + "End;";
	
			ret = trans_query(query_commit);
			
			this->queue = "";
		}
		return ret;
		
	}
	catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        this->queue = "";
        return -1;
    }	
}

void PQSqlConnector::clearQueue()
//empty the operation queue
{
	this->queue = "";
}

int PQSqlConnector::disconnectDB(void)
{
    try
    {
        conn->disconnect();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

pqxx::result PQSqlConnector::non_trans_query(const std::string &query)
{
    int failure_time = 0;
    while (LIKELY(failure_time <= CONNECTION_FAILURE_RETRY))
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
        }//try
        catch (const pqxx::pqxx_exception &e)
        {
            std::cerr << e.base().what() << std::endl;
            const pqxx::sql_error *sqlerr=dynamic_cast<const pqxx::sql_error*>(&e.base());
            const pqxx::broken_connection *brk_conn=dynamic_cast<const pqxx::broken_connection*>(&e.base());
        
            if (brk_conn)
            {
        	    failure_time++;
        	    continue;
            }
            if (sqlerr) 
            {
        	    std::cerr << "Query was: " << sqlerr->query() << std::endl;        	
            }
            return pqxx::result();
        }//catch
    }//while
    std::cerr<<"[PQSqlConnector::non_trans_query] Maximum Retry Reached"<<std::endl;
    return pqxx::result();
}//non_trans_query

int PQSqlConnector::trans_query(const std::string &query)
{
    int failure_time=0;
	while (LIKELY(failure_time <= CONNECTION_FAILURE_RETRY))
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
        }//try
        catch (const pqxx::pqxx_exception &e)
        {
            std::cerr << e.base().what() << std::endl;
            const pqxx::sql_error *sqlerr=dynamic_cast<const pqxx::sql_error*>(&e.base());
            const pqxx::broken_connection *brk_conn=dynamic_cast<const pqxx::broken_connection*>(&e.base());
        
            if (brk_conn)
            {
            	failure_time++;
        	    continue;
            } 
            if (sqlerr) 
            {
        	    std::cerr << "Query was: " << sqlerr->query() << std::endl;
            }            
            return -1;                 
        }//catch
    }//while
    std::cerr<<"[PQSqlConnector::trans_query] Maximum Retry Reached"<<std::endl;
	return -1;
}//trans_query

