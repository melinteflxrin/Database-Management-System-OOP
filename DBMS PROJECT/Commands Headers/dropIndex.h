#ifndef DROPINDEX_H
#define DROPINDEX_H

#include <string>
#include "../Command.h"
#include "../stringUtils.h"
#include "../Database.h"

using namespace std;

class dropIndex : public Command {
private:
	string indexName;
public:
	dropIndex() {
		this->indexName = "";
	}
	dropIndex(const string& indexName) {
		this->indexName = indexName;
	}
	dropIndex(const dropIndex& di) {
		this->indexName = di.indexName;
	}
	dropIndex& operator=(const dropIndex& di) {
		if (this == &di) {
			return *this;
		}
		this->indexName = di.indexName;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.dropIndex(indexName);
	}
	static dropIndex parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "DROP INDEX "
		if (commandCopy.find("DROP INDEX ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Extract the index name
		string indexName = commandCopy.substr(11);  // 11 is the length of "DROP INDEX "
		stringUtils::trim(indexName);

		if (indexName.empty()) {
			throw invalid_argument("Invalid command format. Index name cannot be empty.");
		}

		// Check for extra arguments
		size_t extraArgsPos = indexName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		return dropIndex(indexName);
	}
};

#endif