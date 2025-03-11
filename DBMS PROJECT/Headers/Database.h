#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include "Table.h"
#include "TableNames.h"
#include "IndexManager.h"

class Database {          //many tabless
private:
	Table** database;
	int noTables;

	TableNames* tableNames;

	IndexManager indexManager;

public:
	//DEFAULT CONSTRUCTOR
	Database();
	//DESTRUCTOR
	~Database();
	//GETTERS
	int getTableIndex(const string& name) const;
	Table getTableByName(const string& name) const;
	Index getIndexByColumnAndTable(const string& columnName, const string& tableName);
	//EXECUTE COMMANDS
	void addTableToDatabase(const Table& table);
	bool tableExists(const string& name) const;
	void removeTable(int index);
	void dropTable(const string& name, const string& tablesConfigAddress);
	void insertValues(const string& name, const string* values, int noValues);
	void createIndex(const string& indexName, const string& tableName, const string& columnName);
	void deleteRowFromTable(const string& tableName, const string& columnName, const string& value);
	void updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue);
	void alterTableAddColumn(const string& tableName, const Column& newColumn);
	void alterTableDeleteColumn(const string& tableName, const string& columnName);
	void dropIndex(const string& indexName);
	void importCSV(const string& tableName, const string& csvDirectory, const string& fileName, char delimiter);
	void showTables() const;
	void showIndexFromTable(const string& tableName) const;
	void showIndexFromAll() const;
	//SAVE & LOAD DATABASE
	void saveDatabase(const string& tablesConfigAddress) const;
	void loadDatabase(const string& tablesConfigAddress, const string& selectCommandsAddress);
};

#endif