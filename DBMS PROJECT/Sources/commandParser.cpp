#include "../Headers/commandParser.h"

CommandMapping commandParser::commandMappings[] = {
	{{"CREATE", "TABLE", nullptr}, &commandParser::handleCreateTable},
	{{"DESCRIBE", nullptr , nullptr}, &commandParser::handleDescribeTable},
	{{"ALTER", "DROP", nullptr}, &commandParser::handleAlterDropColumn},
	{{"DROP", "TABLE", nullptr}, &commandParser::handleDropTable},
	{{"INSERT", "INTO", nullptr}, &commandParser::handleInsertCommand},
	{{"CREATE", "INDEX", nullptr}, &commandParser::handleCreateIndex},
	{{"DELETE", nullptr, nullptr}, &commandParser::handleDeleteWhere},
	{{"UPDATE", nullptr , nullptr}, &commandParser::handleUpdateTable},
	{{"ALTER", "ADD", nullptr}, &commandParser::handleAlterAddColumn},
	{{"DROP", "INDEX", nullptr}, &commandParser::handleDropIndex},
	{{"IMPORT", nullptr, nullptr}, &commandParser::handleImportCsv},
	//--------------------------------------------------
	{{"SELECT", "ALL", "WHERE"}, &commandParser::handleSelectAllWhere},
	{{"SELECT", "ALL" , nullptr}, &commandParser::handleSelectAll},
	{{"SELECT", "WHERE" , nullptr}, &commandParser::handleSelectWHERE},
	{{"SELECT", nullptr , nullptr}, &commandParser::handleSelectColumns},
	//--------------------------------------------------
	{{"SHOW", "TABLES", nullptr}, &commandParser::handleShowTables},
	{{"SHOW", "INDEX", "ALL"}, &commandParser::handleShowIndexFromAll},
	{{"SHOW", "INDEX", nullptr}, &commandParser::handleShowIndexFromTable},
	//--------------------------------------------------
	{{"help", "2", nullptr}, &commandParser::handleSyntaxMenu},
	{{"help", nullptr, nullptr}, &commandParser::handleHelpMenu},
	{{"clear", nullptr, nullptr}, &commandParser::handleClearDisplay},
	{{nullptr, nullptr, nullptr}, nullptr}  // end marker for the array
};

commandParser::commandParser(const std::string& selectCommandsAddress, const std::string& tablesConfigAddress, const std::string& csvFilesDirectory, char csvDelimiter) {
	this->selectCommandsAddress = selectCommandsAddress;
	this->tablesConfigAddress = tablesConfigAddress;
	this->csvFilesDirectory = csvFilesDirectory;
	this->csvDelimiter = csvDelimiter;
}

Command* commandParser::handleCreateTable(const std::string& command) {
	return new createTable(createTable::parseCommand(command));
}

Command* commandParser::handleDescribeTable(const std::string& command) {
	return new describeTable(describeTable::parseCommand(command));
}

Command* commandParser::handleDropTable(const std::string& command) {
	return new dropTable(dropTable::parseCommand(command, tablesConfigAddress));
}

Command* commandParser::handleInsertCommand(const std::string& command) {
	return new InsertCommand(InsertCommand::parseCommand(command));
}

Command* commandParser::handleCreateIndex(const std::string& command) {
	return new createIndex(createIndex::parseCommand(command));
}

Command* commandParser::handleDeleteWhere(const std::string& command) {
	return new deleteWhere(deleteWhere::parseCommand(command));
}

Command* commandParser::handleUpdateTable(const std::string& command) {
	return new updateTable(updateTable::parseCommand(command));
}

Command* commandParser::handleAlterAddColumn(const std::string& command) {
	return new alterAddColumn(alterAddColumn::parseCommand(command));
}

Command* commandParser::handleAlterDropColumn(const std::string& command) {
	return new alterDropColumn(alterDropColumn::parseCommand(command));
}

Command* commandParser::handleDropIndex(const std::string& command) {
	return new dropIndex(dropIndex::parseCommand(command));
}

Command* commandParser::handleImportCsv(const std::string& command) {
	return new importCSV(importCSV::parseCommand(command, csvFilesDirectory, csvDelimiter));
}

//--------------------------------------------------

Command* commandParser::handleSelectAll(const std::string& command) {
	return new selectAll(selectAll::parseCommand(command, selectCommandsAddress));
}

Command* commandParser::handleSelectWHERE(const std::string& command) {
	return new selectWHERE(selectWHERE::parseCommand(command, selectCommandsAddress));
}

Command* commandParser::handleSelectColumns(const std::string& command) {
	return new selectColumns(selectColumns::parseCommand(command, selectCommandsAddress));
}

Command* commandParser::handleSelectAllWhere(const std::string& command) {
	return new selectAllWhere(selectAllWhere::parseCommand(command, selectCommandsAddress));
}

//--------------------------------------------------

Command* commandParser::handleShowTables(const std::string& command) {
	return new showTables(showTables::parseCommand(command));
}

Command* commandParser::handleShowIndexFromTable(const std::string& command) {
	return new showIndexFromTable(showIndexFromTable::parseCommand(command));
}

Command* commandParser::handleShowIndexFromAll(const std::string& command) {
	return new showIndexFromAll(showIndexFromAll::parseCommand(command));
}

//--------------------------------------------------

Command* commandParser::handleHelpMenu(const std::string& command) {
	return new helpMenu(helpMenu::parseCommand(command));
}

Command* commandParser::handleSyntaxMenu(const std::string& command) {
	return new syntaxMenu(syntaxMenu::parseCommand(command));
}

Command* commandParser::handleClearDisplay(const std::string& command) {
	return new clearDisplay(clearDisplay::parseCommand(command));
}

Command* commandParser::handleCommand(const std::string& command) {
	for (int i = 0; commandMappings[i].commandKeywords[0] != nullptr; i++) {
		bool match = true;
		for (int j = 0; j < 3 && commandMappings[i].commandKeywords[j] != nullptr; j++) {
			if (command.find(commandMappings[i].commandKeywords[j]) == std::string::npos) {
				match = false;
				break;
			}
		}
		if (match) {
			// pass the command to the member function pointer
			return (this->*commandMappings[i].handlerFunction)(command);
		}
	}
	throw std::invalid_argument("Unknown command.");
}