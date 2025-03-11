#ifndef CREATETABLE_H
#define CREATETABLE_H

#include <iostream>
#include <string>
#include "../Headers/Command.h"  //go up one directory
#include "../Headers/Database.h"
#include "../Headers/Column.h"
#include "../Headers/stringUtils.h"

using namespace std;

class createTable : public Command {
private:
	string name;
	Column* columns;
	int noColumns;
public:
	~createTable() {
		delete[] columns;
	}
	createTable() {
		this->name = "";
		this->columns = nullptr;
		this->noColumns = 0;
	}
	createTable(const string& name, Column* columns, int noColumns) {
		this->name = name;
		this->noColumns = noColumns;
		this->columns = new Column[noColumns];
		for (int i = 0; i < noColumns; i++) {
			this->columns[i] = columns[i];
		}
	}
	createTable(const createTable& ct) {
		this->name = ct.name;
		this->noColumns = ct.noColumns;
		this->columns = new Column[ct.noColumns];
		for (int i = 0; i < ct.noColumns; i++) {
			this->columns[i] = ct.columns[i];
		}
	}
	createTable& operator=(const createTable& ct) {
		if (this == &ct) {
			return *this;
		}
		this->name = ct.name;
		this->noColumns = ct.noColumns;
		this->columns = new Column[ct.noColumns];
		for (int i = 0; i < ct.noColumns; i++) {
			this->columns[i] = ct.columns[i];
		}

		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		//check
		if (db.tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " already exists.";
			return;
		}

		//create
		Table* newTable = new Table(name, columns, noColumns);

		//add
		db.addTableToDatabase(*newTable);

		delete newTable;

		cout << endl << "Table " << "'" << name << "'" << " created successfully.";
	}
	static createTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if command starts with "CREATE TABLE "
		if (commandCopy.find("CREATE TABLE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//find the position of the first '('
		size_t pos = commandCopy.find("(");
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '('.");
		}

		//get the table name
		string tableName = commandCopy.substr(13, pos - 13);  // 13 is the length of "CREATE TABLE "
		stringUtils::trim(tableName);

		//check if the table name is empty
		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		//check if the table name contains spaces
		if (tableName.find(' ') != string::npos) {
			throw invalid_argument("Invalid command format. Table name cannot contain spaces.");
		}

		//find the position of the last ')'
		size_t endPos = commandCopy.find_last_of(")");
		if (endPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing ')'.");
		}

		//get the columns part
		string columnsPart = commandCopy.substr(pos + 1, endPos - pos - 1);  // from after '(' to before ')'
		stringUtils::trim(columnsPart);

		if (columnsPart.empty()) {
			throw invalid_argument("Invalid command format. Columns cannot be empty.");
		}

		//count the number of columns
		int noColumns = 0;
		size_t start = 0;
		size_t end = 0;
		while ((end = columnsPart.find("),", start)) != string::npos) {
			noColumns++;
			start = end + 2;
			while (start < columnsPart.length() && isspace(columnsPart[start])) {
				start++;
			}
			if (start < columnsPart.length() && columnsPart[start] == '(') {
				start++;
			}
		}
		noColumns++; //increment for the last column

		if (noColumns == 0) {
			throw invalid_argument("Invalid command format. No columns specified.");
		}

		//split the columns part into individual columns
		string* columns = new string[noColumns];
		start = 0;
		int colIndex = 0;
		while ((end = columnsPart.find("),", start)) != string::npos) {
			string column = columnsPart.substr(start, end - start + 1);
			stringUtils::trim(column);
			columns[colIndex++] = column;
			start = end + 2;
			while (start < columnsPart.length() && isspace(columnsPart[start])) {
				start++;
			}
			if (start < columnsPart.length() && columnsPart[start] == '(') {
				start++;
			}
		}
		string lastColumn = columnsPart.substr(start);
		stringUtils::trim(lastColumn);
		columns[colIndex] = lastColumn;

		Column* tableColumns = new Column[noColumns];
		for (int i = 0; i < noColumns; i++) {
			string column = columns[i];
			stringUtils::trim(column);

			//remove leading and trailing parentheses
			if (!column.empty() && column.front() == '(') column.erase(0, 1);
			if (!column.empty() && column.back() == ')') column.pop_back();

			//split the column into individual parts
			string* columnParts = nullptr;
			int noParts = 0;
			stringUtils::splitCommand(column, ",", columnParts, noParts);

			if (noParts < 4 || noParts > 5) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw invalid_argument("Invalid command format. Invalid column format.");
			}

			//get the column name
			string columnName = columnParts[0];
			stringUtils::trim(columnName);

			if (columnName.empty()) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw invalid_argument("Invalid command format. Column name cannot be empty.");
			}

			//get the column type
			string columnType = columnParts[1];
			stringUtils::trim(columnType);

			ColumnType type;
			try {
				type = stringUtils::parseColumnType(columnType);
			}
			catch (const invalid_argument& e) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw;
			}

			//size
			string columnSize = columnParts[2];
			stringUtils::trim(columnSize);

			if (columnSize.empty()) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw invalid_argument("Invalid command format. Column size cannot be empty.");
			}

			//default value
			string columnDefaultValue = columnParts[3];
			stringUtils::trim(columnDefaultValue);

			//check if column is unique
			bool unique = false;
			if (noParts == 5) {
				string uniquePart = columnParts[4];
				stringUtils::trim(uniquePart);
				if (uniquePart == "UNIQUE") {
					unique = true;
				}
				else {
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					throw invalid_argument("Invalid command format. Invalid column format.");
				}
			}

			//create the column
			tableColumns[i] = Column(columnName, type, stoi(columnSize), columnDefaultValue, unique);
		}

		delete[] columns;
		return createTable(tableName, tableColumns, noColumns);
	}
};

#endif // CREATETABLE_H