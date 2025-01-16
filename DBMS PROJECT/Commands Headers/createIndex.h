#ifndef CREATEINDEX_H
#define CREATEINDEX_H

#include <string>
#include "../Command.h"
#include "../stringUtils.h"
#include "../Database.h"

using namespace std;

class createIndex : public Command {
private:
	string indexName;
	string tableName;
	string columnName;
public:
	createIndex() {
		this->indexName = "";
		this->tableName = "";
		this->columnName = "";
	}
	createIndex(const string& indexName, const string& tableName, const string& columnName) {
		this->indexName = indexName;
		this->tableName = tableName;
		this->columnName = columnName;
	}
	createIndex(const createIndex& ci) {
		this->indexName = ci.indexName;
		this->tableName = ci.tableName;
		this->columnName = ci.columnName;
	}
	createIndex& operator=(const createIndex& ci) {
		if (this == &ci) {
			return *this;
		}
		this->indexName = ci.indexName;
		this->tableName = ci.tableName;
		this->columnName = ci.columnName;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.createIndex(indexName, tableName, columnName);
	}
	static createIndex parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "CREATE INDEX "
		if (commandCopy.find("CREATE INDEX ") != 0) {
			throw std::invalid_argument("Invalid command format: Missing 'CREATE INDEX'.");
		}

		// Find the position of " ON "
		size_t onPos = commandCopy.find(" ON ");
		if (onPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing 'ON'.");
		}

		// Ensure there is an index name between "CREATE INDEX " and " ON "
		if (onPos <= 13) {  // 13 is the length of "CREATE INDEX "
			throw std::invalid_argument("Invalid command format: Missing index name.");
		}

		// Extract the index name
		string indexName = commandCopy.substr(13, onPos - 13);
		stringUtils::trim(indexName);

		if (indexName.empty()) {
			throw std::invalid_argument("Invalid command format: Index name cannot be empty.");
		}

		// Extract the table name and column part
		string tableAndColumnPart = commandCopy.substr(onPos + 4);  // 4 is the length of " ON "
		stringUtils::trim(tableAndColumnPart);

		// Find the position of the opening parenthesis '('
		size_t openParenPos = tableAndColumnPart.find('(');
		if (openParenPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing '('.");
		}

		// Table name
		string tableName = tableAndColumnPart.substr(0, openParenPos);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw std::invalid_argument("Invalid command format: Table name cannot be empty.");
		}

		// Find the position of the closing parenthesis ')'
		size_t closeParenPos = tableAndColumnPart.find(')', openParenPos);
		if (closeParenPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing closing parenthesis.");
		}

		// Column name
		string columnName = tableAndColumnPart.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
		stringUtils::trim(columnName);

		if (columnName.empty()) {
			throw std::invalid_argument("Invalid command format: Column name cannot be empty.");
		}

		// Check for extra arguments after the closing parenthesis
		if (closeParenPos + 1 < tableAndColumnPart.length()) {
			string extraArgs = tableAndColumnPart.substr(closeParenPos + 1);
			stringUtils::trim(extraArgs);
			if (!extraArgs.empty()) {
				throw std::invalid_argument("Invalid command format: Too many arguments.");
			}
		}

		return createIndex(indexName, tableName, columnName);
	}
};

#endif