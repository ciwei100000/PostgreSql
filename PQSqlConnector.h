#ifndef __PQC_H_
#define __PQC_H_

#include <pqxx/pqxx>

using namespace pqxx;

class PQSqlConnector
{
private:
    std::string connstring;
    connection* conn;
public:

    PQSqlConnector(const std::string& connstring_input = ""):connstring(connstring_input)
    {}
    ~PQSqlConnector()
    {}
    
    connectDB(const std::string& connstring_input = ""); // Connect to a PostgreSql Database
     
    createTable(const std::string& table_name);
     
    deleteTable(const std::string& table_name);
     
    insertPoint(const std::string& table_name, const double& X, const double& Y, const double& Z);
     
    updatePoint(const std::string& table_name, const double& X, const double& Y, const double& Z);
     
    deletePoint(const std::string& table_name, const double& X, const double& Y);
     
    disconnectDB(void);
     
}

#endif
