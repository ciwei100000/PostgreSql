#ifndef __PQC_H_
#define __PQC_H_

#include <pqxx/pqxx>

using namespace pqxx;

class PQSqlConnector
{
private:
    std::string connstring; //store connection configuration
    connection* conn; // store current
    uint failure_time // Query Failure Time
    
    result non_trans_query(const std::string &query); // for Non-transactional Query
    void trans_query(const std::string &query); // for Transactional Query
    bool handle_broken_connection(const int& query_case, const std::string &query); //for Handling Connection Failure 
public:

    PQSqlConnector(const std::string& connstring_input = "")
    {}
    ~PQSqlConnector()
    {}
    
    bool connectionIsOpen(); // Check if the connection to the databse is open at the moment
     
    bool createTable(const std::string& table_name_input); // create a table
     
    bool dropTable(const std::string& table_name_input); // delete a table
     
    bool insertPoint(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input); //insert the coordinates
     
    bool updatePoint(const std::string& table_name_input, const double& X_input, const double& Y_input, const double& Z_input); // update the coordinates
     
    bool deletePoint(const std::string& table_name_input, const double& X_input, const double& Y_input);
    // delete the point; 
    void disconnectDB(void);// explictly disconnect the database.
     
}

#endif
