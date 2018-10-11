#ifndef __PQC_H_
#define __PQC_H_

#include <pqxx/pqxx>
#include <sstream>
#include <vector>


const bool DEBUG = false; //used only for debug;
const bool VERBOSE = false; //disable notification;

const int CONNECTION_FAILURE_RETRY = 10; //Time to retry in case of connection failure
const std::string DATA_TYPE_ID = "INT"; //Data Type for field "X"
const std::string DATA_TYPE_X = "real"; //Data Type for field "X"
const std::string DATA_TYPE_Y = "real"; //Data Type for field "Y"
const std::string DATA_TYPE_Z = "real"; //Data Type for field "Z"
//please refer to https://www.postgresql.org/docs/9.5/static/datatype-numeric.html

using namespace pqxx;
using namespace std;


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
     
    bool insertSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); //insert the coordinates of one point
    
    bool insertPointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); //Add one insertSinglePoint to operation Queue;
    
    bool updateSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); // update the coordinates of one point
    
    bool updatePointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); // Add one updateSinglePoint to operation Queue;
    //Warning: This functioin will not check if the point exists. If non-existed point were to be updated, 
    //the database server would simply ingore this operation without returning any error. 
    //You should make sure the point to be updated is actually exist in the database;
    
    bool updatePointQueue(const std::string& table_name_input, const std::vector<float>& values); 
    // Add one updateSinglePoint to operation Queue; 
    // values is a vector in the format of  (x1, y1, z1, id1, x2, y2, z2, id2,...)
    //Warning: This functioin will not check if the point exists. If non-existed point were to be updated, 
    //the database server would simply ingore this operation without returning any error. 
    //You should make sure the point to be updated is actually exist in the database;
    
    bool upsertPointQueue(const std::string& table_name_input, const std::vector<float>& values);
    //Add point if id does not exist or update if id already exists
    //values is a vector in the format of  (x1, y1, z1, id1, x2, y2, z2, id2,...)
     
    bool deleteSinglePoint(const std::string& table_name_input, const int& ID);
    // delete one point; 
    
    bool deletePointQueue(const std::string& table_name_input, const int& ID);
    // Add one deletePoint to operation Queue;
    
    //bool deletePointsArrray(const std::string& table_name_input, const vector);
    
    bool deletePointQueue(const std::string& table_name_input, const std::vector<int>& ids);
    
    bool commitQueue();
    //Commit and empty the operations in the operation queue
    
    bool clearQueue();
    //empty the operation queue
    
    bool disconnectDB(void);// explictly disconnect the database.
     
};

#endif
