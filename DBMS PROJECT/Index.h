#ifndef INDEX_H
#define INDEX_H

#include <string>

using namespace std;

class Index {
private:
	string* indexNames;  //this is to check if index w a name already exists
	int noIndexeNames;
	int* indexValues;    //column position in a specific table
	int noIndexValues;
	string* columnNames; //column names to check if there is an index on them
	int noColumnNames;
	string* tableNames;  //table names to check if there is an index on them
	int noTableNames;
public:
	//DESTRUCTOR
	~Index();
	//DEFAULT CONSTRUCTOR
	Index();
	//ASSIGNMENT OPERATOR
	Index& operator=(const Index& original);
	//COPY CONSTRUCTOR
	Index(const Index& original);
	//GETTERS
	const int getNoIndexes() const;
	const string& getIndexName(const int index) const;
	int getIndexValue(const string& columnName, const string& tableName) const;
	string getIndexColumnName(const string& indexName) const;
	string getIndexTableName(const string& indexName) const;

	void addIndex(const string& indexName, int indexValue, const string& columnName, const string& tableName);
	void removeIndex(const string& columnName, const string& tableName);
	void removeIndexByIndexName(const string& indexName);
	void removeIndexByTableName(const string& tableName);

	bool indexExistsByIndexName(const string& indexName) const;
	bool indexExists(const string& columnName, const string& tableName) const;
	bool indexExistsByTableName(const string& tableName) const;

	void showIndexesByTableName(const string& tableName) const;
	void showAllIndexes() const;
};

#endif
