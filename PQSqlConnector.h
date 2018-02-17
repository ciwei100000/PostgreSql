#ifndef __PQC_H_
#define __PQC_H_

#include <pqxx/pqxx>
#include <sstream>

using namespace pqxx;

class PQSqlConnector
{
private:
    std::string connstring; //store connection configuration
    connection* conn; // store current
    unsigned int failure_time; // Query Failure Time
    
    std::string queue; // Operation Queue;
    
     // Template to convert Type to String
    result non_trans_query(const std::string &query); // for Non-transactional Query
    bool trans_query(const std::string &query); // for Transactional Query
    bool handle_broken_connection_trans(const std::string &query);
    //for Handling Connection Failure 
    pqxx::result handle_broken_connection_nontrans(const std::string &query);
    //for Handling Connection Failure
public:

    PQSqlConnector(const std::string& connstring_input = "");
    ~PQSqlConnector();
    
    bool connectionIsOpen(); // Check if the connection to the databse is open at the moment
    
    void keepConnectionAlive(); //used to keep the connection alive from timeout; Execute it every few seconds
    					   //when the idle
     
    bool createTable(const std::string& table_name_input); // create a table
     
    bool dropTable(const std::string& table_name_input); // delete a table
     
    bool insertSinglePoint(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input); //insert the coordinates
    
    bool insertPointQueue(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input); //Add one insertPoint to operation Queue;
    
    bool updateSinglePoint(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input); // update the coordinates
    
    bool updatePointQueue(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input); // Add one updatePoint to operation Queue;
    //Warning: This functioin will not check if the point exists. If non-existed point were to be updated, 
    //the database server would simply ingore this operation without returning any error. 
    //You should make sure the point to be updated is actually exist in the database;
     
    bool deleteSinglePoint(const std::string& table_name_input, const double& X_input, const double& Y_input);
    // delete the point; 
    
    bool deletePointQueue(const std::string& table_name_input, const double& X_input, const double& Y_input);
    // Add one deletePoint to operation Queue;
    
    bool commitQueue();
    //Commit and empty the operations in the operation queue
    
    bool clearQueue();
    //empty the operation queue
    
    bool disconnectDB(void);// explictly disconnect the database.
     
};

#endif
