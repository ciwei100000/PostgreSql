#ifndef __PQC_H_
#define __PQC_H_

#include <pqxx/pqxx>
#include <sstream>
#include <vector>


const bool DEBUG = false; //used only for debug;
const bool VERBOSE = false; //disable notification;

const int CONNECTION_FAILURE_RETRY = 100; //Time to retry in case of connection failure
const std::string DATA_TYPE_ID = "INT"; //Data Type for field "X"
const std::string DATA_TYPE_X = "real"; //Data Type for field "X"
const std::string DATA_TYPE_Y = "real"; //Data Type for field "Y"
const std::string DATA_TYPE_Z = "real"; //Data Type for field "Z"
//please refer to https://www.postgresql.org/docs/9.5/static/datatype-numeric.html


class PQSqlConnector
{
private:
    std::string connstring; //store connection configuration
    pqxx::connection* conn; // store current
    unsigned int failure_time; // Query Failure Time
    
    std::string queue; // Operation Queue;
    
     // Template to convert Type to String
    pqxx::result non_trans_query(const std::string &query); // for Non-transactional Query
    int trans_query(const std::string &query); // for Transactional Query

public:

    PQSqlConnector(const std::string& connstring_input = "");
    ~PQSqlConnector();
    
    bool connectionIsOpen(); // Check if the connection to the databse is open at the moment
    
    void keepConnectionAlive(); //used to keep the connection alive from timeout; Execute it every few seconds
    					   //when the idle
     
    int createTable(const std::string& table_name_input); // create a table
     
    int dropTable(const std::string& table_name_input); // delete a table
     
    int insertSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); //insert the coordinates of one point
    
    int insertPointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); //Add one insertSinglePoint to operation Queue;
    
    int updateSinglePoint(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); // update the coordinates of one point
    //Warning: This functioin will not check if the point exists. If non-existed point were to be updated, 
    //the database server would simply ingore this operation without returning any error. 
    //You should make sure the point to be updated is actually exist in the database;
    
    int updatePointQueue(const std::string& table_name_input, const int& ID, const float& X_input, const float& Y_input, const float& Z_input); // Add one updateSinglePoint to operation Queue;
    //Warning: This functioin will not check if the point exists. If non-existed point were to be updated, 
    //the database server would simply ingore this operation without returning any error. 
    //You should make sure the point to be updated is actually exist in the database;
    
    int updatePointQueue(const std::string& table_name_input, const std::vector<float>& values); 
    // Add one updateSinglePoint to operation Queue; 
    // values is a vector in the format of  (x1, y1, z1, id1, x2, y2, z2, id2,...)
    //Warning: This functioin will not check if the point exists. If non-existed point were to be updated, 
    //the database server would simply ingore this operation without returning any error. 
    //You should make sure the point to be updated is actually exist in the database;
    
    int upsertPointQueue(const std::string& table_name_input, const std::vector<float>& values);
    //Add point if id does not exist or update if id already exists
    //values is a vector in the format of  (x1, y1, z1, id1, x2, y2, z2, id2,...)
     
    int deleteSinglePoint(const std::string& table_name_input, const int& ID);
    // delete one point; 
    
    int deletePointQueue(const std::string& table_name_input, const int& ID);
    // Add one deletePoint to operation Queue;
    
    int deletePointQueue(const std::string& table_name_input, const std::vector<int>& ids);
    // Add one deletePoint to operation Queue;
    
    int readPointsVector(const std::string& table_name_input, std::vector<float>& values);
    //read points from the table named [table_name_input],sorted by id ascending
    //output values to a float vector, format: [x1,y1,z1,id1,x2,y2,z2,id2,...]
    
    int readPointsVector(const std::string& table_name_input, std::vector<float>& values, const int& start_id);
    //read points from the table named [table_name_input],starting from [start_id],sorted by id ascending
    //output values to a float vector, format: [x1,y1,z1,id1,x2,y2,z2,id2,...]
    
    int readPointsVector(const std::string& table_name_input, std::vector<float>& values, const float& start_id);
    //read points from the table named [table_name_input], starting from [start_id],sorted by id ascending
    //output values to a float vector, format: [x1,y1,z1,id1,x2,y2,z2,id2,...]
    
    int readPointsVector(const std::string& table_name_input, std::vector<float>& values, const int& start_id, const int& limit);
    //read [limit] of points from the table named [table_name_input],starting from [start_id], sorted by id ascending 
    //output values to a float vector, format: [x1,y1,z1,id1,x2,y2,z2,id2,...]
    
    int readPointsVector(const std::string& table_name_input, std::vector<float>& values, const float& start_id, const int& limit);
    //read [limit] of points from the table named [table_name_input],starting from [start_id], sorted by id ascending 
    //output values to a float vector, format: [x1,y1,z1,id1,x2,y2,z2,id2,...]
    
    int commitQueue();
    //Commit and empty the operations in the operation queue
    
    void clearQueue();
    //empty the operation queue
    
    int disconnectDB(void);// explictly disconnect the database.
     
};

#endif
