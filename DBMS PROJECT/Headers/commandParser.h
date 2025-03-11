#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <string>
#include "Command.h"
#include "stringUtils.h"
#include "Database.h"
#include "../Commands/createTable.h"
#include "../Commands/describeTable.h"
#include "../Commands/dropTable.h"
#include "../Commands/InsertCommand.h"
#include "../Commands/createIndex.h"
#include "../Commands/deleteWhere.h"
#include "../Commands/updateTable.h"
#include "../Commands/alterAddColumn.h"
#include "../Commands/alterDropColumn.h"
#include "../Commands/dropIndex.h"
#include "../Commands/importCSV.h"
#include "../Commands/selectAll.h"
#include "../Commands/selectWHERE.h"
#include "../Commands/selectColumns.h"
#include "../Commands/selectAllWhere.h"
#include "../Commands/showTables.h"
#include "../Commands/showIndexFromTable.h"
#include "../Commands/showIndexFromAll.h"
#include "../Commands/helpMenu.h"
#include "../Commands/syntaxMenu.h"
#include "../Commands/clearDisplay.h"

class commandParser; //declaration ahead of commandParser to avoid errors

// struct to hold command keyword and corresponding handler
struct CommandMapping {
	const char* commandKeywords[3];
	Command* (commandParser::* handlerFunction)(const std::string&);  // member function pointer to command handler
};

class commandParser {
private:
	static CommandMapping commandMappings[];
	string selectCommandsAddress;
	string tablesConfigAddress;
	string csvFilesDirectory;
	char csvDelimiter;

	Command* handleCreateTable(const std::string& command);
	Command* handleDescribeTable(const std::string& command);
	Command* handleDropTable(const std::string& command);
	Command* handleInsertCommand(const std::string& command);
	Command* handleCreateIndex(const std::string& command);
	Command* handleDeleteWhere(const std::string& command);
	Command* handleUpdateTable(const std::string& command);
	Command* handleAlterAddColumn(const std::string& command);
	Command* handleAlterDropColumn(const std::string& command);
	Command* handleDropIndex(const std::string& command);
	Command* handleImportCsv(const std::string& command);
	//--------------------------------------------------
	Command* handleSelectAll(const std::string& command);
	Command* handleSelectWHERE(const std::string& command);
	Command* handleSelectColumns(const std::string& command);
	Command* handleSelectAllWhere(const std::string& command);
	//--------------------------------------------------
	Command* handleShowTables(const std::string& command);
	Command* handleShowIndexFromTable(const std::string& command);
	Command* handleShowIndexFromAll(const std::string& command);
	//--------------------------------------------------
	Command* handleHelpMenu(const std::string& command);
	Command* handleSyntaxMenu(const std::string& command);
	Command* handleClearDisplay(const std::string& command);

public:
	commandParser(const std::string& selectCommandsAddress, const std::string& tablesConfigAddress, const std::string& csvFilesDirectory, char csvDelimiter);
	Command* handleCommand(const std::string& command);
};

#endif // COMMANDPARSER_H