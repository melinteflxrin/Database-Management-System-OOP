#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "Column.h"
#include "Row.h"
#include "TableNames.h"
#include "Table.h"
#include "Index.h"
#include "IndexManager.h"

using namespace std;

class Database;

class HelpersForSelectCommands {
public:
	static bool rowMatchesCondition(const Row& row, const Column& conditionColumn, int conditionColumnIndex, const std::string& value) {
		// Assuming the Row class has a method to get the value of a column by index
		std::string rowValue = row.getTextData(conditionColumnIndex);

		// Compare the row value with the given value
		return rowValue == value;
	}
	static void printRow(const Row& row, const Table* table, const int* columnIndexes, int noColumns, const int* maxWidth) {
		for (int i = 0; i < noColumns; ++i) {
			int columnIndex = columnIndexes[i];
			std::string value = row.getTextData(columnIndex); // Assuming Row has a getValue method
			std::cout << std::left << std::setw(maxWidth[i]) << value << " | ";
		}
		std::cout << std::endl;
	}
	static int* calculateMaxWidths(const Table* table, const std::string* columnNames, int noColumns, const int* columnIndexes) {
		int* maxWidth = new int[noColumns];

		// Initialize maxWidth with the length of the column names
		for (int i = 0; i < noColumns; ++i) {
			maxWidth[i] = columnNames[i].length();
		}

		// Iterate through each row in the table
		for (int i = 0; i < table->getNoRows(); ++i) {
			const Row& row = table->getRow(i);

			// Update maxWidth based on the length of the data in each column
			for (int j = 0; j < noColumns; ++j) {
				int columnIndex = columnIndexes[j];
				std::string value = row.getTextData(columnIndex); // Assuming Row has a getValue method
				if (value.length() > maxWidth[j]) {
					maxWidth[j] = value.length();
				}
			}
		}

		return maxWidth;
	}
	static bool validateColumn(const Table* table, const std::string& columnName, int& columnIndex) {
		for (int i = 0; i < table->getNoColumns(); ++i) {
			if (table->getColumn(i).getName() == columnName) {
				columnIndex = i;
				return true;
			}
		}
		std::cout << "Error: Column '" << columnName << "' does not exist in table '" << table->getName() << "'." << std::endl;
		return false;
	}
	static bool validateColumns(const Table* table, const std::string* columnNames, int noColumns, int* columnIndexes) {
		for (int i = 0; i < noColumns; ++i) {
			if (!validateColumn(table, columnNames[i], columnIndexes[i])) {
				std::cout << "Error: Column '" << columnNames[i] << "' does not exist in table '" << table->getName() << "'." << std::endl;
				return false;
			}
		}
		return true;
	}
	static void printHeader(const std::string* columnNames, const int* maxWidth, int noColumns) {
		for (int i = 0; i < noColumns; ++i) {
			std::cout << std::left << std::setw(maxWidth[i]) << columnNames[i] << " | ";
		}
		std::cout << std::endl;
	}
	static void printSeparator(const int* maxWidth, int noColumns) {
		for (int i = 0; i < noColumns; ++i) {
			std::cout << std::string(maxWidth[i], '-') << "-+-";
		}
		std::cout << std::endl;
	}
};
class stringUtils {
public:
	static ColumnType parseColumnType(const string& type) {
		//convert string to ColumnType for the column constructor
		if (type == "INT") return INT;
		if (type == "TEXT") return TEXT;
		if (type == "FLOAT") return FLOAT;
		if (type == "BOOLEAN") return BOOLEAN;
		if (type == "DATE") return DATE;
		throw invalid_argument("Invalid column type: " + type);
	}
	static bool parseUnique(const string& unique) {
		//convert string to bool for the column constructor
		if (unique == "UNIQUE") return true;
		return false;
	}
	static void trim(string& str) {
		//find the first non-space character
		size_t start = str.find_first_not_of(" ");
		//find the last non-space character
		size_t end = str.find_last_not_of(" ");

		//if the string is only spaces set it to an empty string
		if (start == string::npos) {
			str = "";
		}
		else {
			//make the trimmed string
			str = str.substr(start, end - start + 1);
		}
	}
	static void splitCommand(const string& command, const string& delimiter, string*& tokens, int& tokenCount) {
		string commandCopy = command;
		trim(commandCopy);
		tokenCount = 0;
		size_t pos = 0;
		string token;
		while ((pos = commandCopy.find(delimiter)) != string::npos) {
			token = commandCopy.substr(0, pos);
			if (!token.empty()) {
				tokenCount++;
			}
			commandCopy.erase(0, pos + delimiter.length());
		}
		if (!commandCopy.empty()) {
			tokenCount++;
		}

		tokens = new string[tokenCount];
		commandCopy = command;
		pos = 0;
		int i = 0;
		while ((pos = commandCopy.find(delimiter)) != string::npos) {
			token = commandCopy.substr(0, pos);
			if (!token.empty()) {
				tokens[i++] = token;
			}
			commandCopy.erase(0, pos + delimiter.length());
		}
		if (!commandCopy.empty()) {
			tokens[i] = commandCopy;
		}
	}
};
class Command {
public:
	virtual void execute(Database& db) = 0;
	virtual ~Command() = default;
};

class Database {          //many tabless
private:
	Table** database = nullptr;
	int noTables = 0;

	TableNames* tableNames = nullptr;

	IndexManager indexManager;

	//remove this later
	static int selectCount;

public:
	//DEFAULT CONSTRUCTOR
	Database() {
		this->database = nullptr;
		this->noTables = 0;
		this->tableNames = nullptr;
		this->tableNames = new TableNames();
	}
	//DESTRUCTOR
	~Database() {
		for (int i = 0; i < noTables; i++) {
			delete database[i];
		}
		delete[] database;
		delete tableNames;  //i need to delete because i used 'new' in the constructor
	}
	//GETTERS
	int getTableIndex(const string& name) const {
		for (int i = 0; i < noTables; i++) {
			if (database[i]->getName() == name) {
				return i;
			}
		}
		return -1;
	}
	Table getTableByName(const string& name) const {
		for (int i = 0; i < noTables; i++) {
			if (database[i]->getName() == name) {
				return *database[i]; //return a copy of the Table
			}
		}
		throw runtime_error("Table not found");
	}
	Index getIndexByColumnAndTable(const string& columnName, const string& tableName) {
		try {
			return indexManager.getIndexByColumnAndTable(columnName, tableName);
		}
		catch (const std::runtime_error& e) {
			throw;
		}
	}
	//--------------------------------------------------
	void addTableToDatabase(const Table& table) {
		Table** newDatabase = new Table * [noTables + 1];

		for (int i = 0; i < noTables; i++) {
			newDatabase[i] = new Table(*database[i]);     //i am copying pointers from database to newDatabase, not the actual Table objects
		}
		newDatabase[noTables] = new Table(table); // add new table at the end

		for (int i = 0; i < noTables; i++) {
			delete database[i];  //delete the table objects
		}
		delete[] database;
		database = newDatabase;
		noTables++;

		tableNames->addName(table.getName());
	}
	bool tableExists(const string& name) const {
		if (tableNames->nameExists(name)) {
			return true;
		}
		return false;
	}
	void removeTable(int index) {
		if (index < 0 || index >= noTables) {
			cout << endl << "Error: Invalid table index.";
			return;
		}

		Table** newDatabase = new Table * [noTables - 1];
		int tempIndex = 0;

		for (int i = 0; i < noTables; i++) {
			if (i != index) {
				newDatabase[tempIndex++] = database[i];
			}
			else {
				delete database[i];
			}
		}

		delete[] database;
		database = newDatabase;
		noTables--;

		tableNames->removeName(database[index]->getName());
	}
	void dropTable(const string& name, const string& tablesConfigAddress) {
		if (!tableExists(name)) {
			cout << "Error: Table '" << name << "' does not exist." << endl;
			return;
		}

		int indexToRemove = getTableIndex(name);
		if (indexToRemove == -1) return;

		//remove all indexes associated with the table
		for (int i = 0; i < indexManager.getNoIndexes(); ++i) {
			Index** indexes = indexManager.getIndexes();
			Index* index = indexes[i];
			if (index->getTableName() == name) {
				indexManager.deleteIndex(index->getIndexName());
				--i; //adjust the index after deletion
			}
		}

		delete database[indexToRemove];

		for (int i = indexToRemove; i < noTables - 1; i++) {
			database[i] = database[i + 1];
		}

		noTables--;
		database[noTables] = nullptr;

		tableNames->removeName(name);

		string filename = tablesConfigAddress + name + ".bin";
		if (filesystem::exists(filename)) {
			filesystem::remove(filename);
		}

		cout << "Table '" << name << "' dropped successfully." << endl;
	}
	void insertValues(const string& name, const string* values, int noValues) {
		// Check if the table exists
		if (!tableExists(name)) {
			cout << "Error: Table '" << name << "' does not exist." << endl;
			return;
		}

		// Get the table index
		int tableIndex = getTableIndex(name);

		// Get the table at our index
		Table* table = database[tableIndex];

		// Validate the number of values matches the number of columns
		if (noValues != table->getNoColumns()) {
			cout << "Error: Number of values does not match the number of columns in the table." << endl;
			return;
		}

		// Attempt to add the row
		if (!table->addRowBool(values)) {
			return; // Exit if row insertion fails
		}

		// Get the ID of the newly added row
		int newRowId = table->getNoRows() - 1;

		// Update indexes only if the row was successfully added
		for (int i = 0; i < noValues; i++) {
			const string& columnName = table->getColumn(i).getName();
			try {
				Index index = indexManager.getIndexByColumnAndTable(columnName, name); // Pass column name and table name
				try {
					indexManager.addValue(index.getIndexName(), values[i], newRowId);
				}
				catch (const invalid_argument& e) {
					cout << "Error: Invalid value for column '" << columnName << "': " << values[i] << endl;
				}
				catch (const out_of_range& e) {
					cout << "Error: Value out of range for column '" << columnName << "': " << values[i] << endl;
				}
			}
			catch (const std::runtime_error& e) {
				// No index for this column, skip updating
			}
		}
	}
	void createIndex(const string& indexName, const string& tableName, const string& columnName) {
		if (indexManager.indexExistsByIndexName(indexName)) {
			cout << endl << "Error: Index '" << indexName << "' already exists.";
			return;
		}

		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		Table* table = database[getTableIndex(tableName)];
		if (!table->columnExists(columnName)) {
			cout << endl << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'.";
			return;
		}

		//check if an index already exists on the specified column in the table
		try {
			Index index = indexManager.getIndexByColumnAndTable(columnName, tableName);
			cout << endl << "Error: An index already exists on column '" << columnName << "' in table '" << tableName << "'.";
			return;
		}
		catch (const std::runtime_error& e) {
		}

		//create the index
		indexManager.createIndex(indexName, tableName, columnName);

		//add values to the index
		int columnIndex = table->getColumnIndex(columnName);
		for (int i = 0; i < table->getNoRows(); ++i) {
			Row& row = table->getRow(i);
			string value = row.getTextData(columnIndex);
			indexManager.addValue(indexName, value, i);
		}

		cout << endl << "Index '" << indexName << "' created successfully on column '" << columnName << "' in table '" << tableName << "'.";
	}
	//--------------------------------------------------
	void deleteRowFromTable(const string& tableName, const string& columnName, const string& value) {
		// Validate table existence
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(tableName);
		if (tableIndex == -1) {
			cout << "Error: Table '" << tableName << "' not found." << endl;
			return;
		}

		Table* table = database[tableIndex];
		int columnIndex = table->getColumnIndex(columnName);
		if (columnIndex == -1) {
			cout << "Error: Column '" << columnName << "' does not exist in table: " << tableName << endl;
			return;
		}

		int* rows = nullptr;
		int rowCount = 0;

		try {
			// Check if there is an index on the column
			Index index = indexManager.getIndexByColumnAndTable(columnName, tableName);

			// Use the index to get the rows
			cout << "Using index for column: " << columnName << endl;
			rowCount = index.getRowCount(value);
			if (rowCount > 0) {
				rows = new int[rowCount];
				index.getKeys(value, rows, rowCount);
			}
		}
		catch (const std::runtime_error& e) {
			// Handle the case where the index is not found
			cout << "Scanning table for column '" << columnName << "'." << endl;

			// No index, scan the table
			rowCount = table->getNoRows();
			rows = new int[rowCount];
			int rowIndex = 0;
			for (int i = 0; i < rowCount; i++) {
				if (table->getRow(i).getTextData(columnIndex) == value) {
					rows[rowIndex++] = i;
				}
			}
			rowCount = rowIndex; // Update count to matched rows
		}

		if (rowCount == 0) {
			cout << "No rows found with " << columnName << " = " << value << endl;
			delete[] rows;
			return;
		}

		// Delete the rows
		for (int i = 0; i < rowCount; i++) {
			int rowToDelete = rows[i];
			table->deleteRow(rowToDelete);
			cout << "Row deleted successfully." << endl;

			// Adjust subsequent row indices in rows array
			for (int k = i + 1; k < rowCount; k++) {
				if (rows[k] > rowToDelete) {
					rows[k]--;
				}
			}
		}

		// Recreate indexes for all columns that had an index
		for (int colIndex = 0; colIndex < table->getNoColumns(); colIndex++) {
			string colName = table->getColumn(colIndex).getName();
			try {
				Index index = indexManager.getIndexByColumnAndTable(colName, tableName);
				index.clear(); // Clear the existing index

				// Recreate the index
				for (int rowIndex = 0; rowIndex < table->getNoRows(); rowIndex++) {
					index.addValue(table->getRow(rowIndex).getTextData(colIndex), rowIndex);
				}
				cout << "Recreated index for column: " << colName << endl;
			}
			catch (const std::runtime_error& e) {
				// No index for this column, skip updating
			}
		}

		// Cleanup
		delete[] rows;
	}
	void updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int setColumnIndex = table->getColumnIndex(setColumnName);
		int whereColumnIndex = table->getColumnIndex(whereColumnName);

		if (setColumnIndex == -1) {
			cout << "Error: Column '" << setColumnName << "' does not exist in table '" << tableName << "'." << endl;
			return;
		}

		if (whereColumnIndex == -1) {
			cout << "Error: Column '" << whereColumnName << "' does not exist in table '" << tableName << "'." << endl;
			return;
		}

		int updatedRows = 0;
		int* rowIds = nullptr;
		int keyCount = 0;

		try {
			// Use index to get rows for whereColumnName
			Index whereIndex = indexManager.getIndexByColumnAndTable(whereColumnName, tableName);
			keyCount = whereIndex.getRowCount(whereValue);
			if (keyCount > 0) {
				rowIds = new int[keyCount];
				whereIndex.getKeys(whereValue, rowIds, keyCount);
			}
			cout << "Using index for column: " << whereColumnName << endl;
		}
		catch (const std::runtime_error& e) {
			// No index, scan the table
			keyCount = table->getNoRows();
			rowIds = new int[keyCount];
			int rowIndex = 0;
			for (int i = 0; i < keyCount; i++) {
				if (table->getRow(i).getTextData(whereColumnIndex) == whereValue) {
					rowIds[rowIndex++] = i;
				}
			}
			keyCount = rowIndex; // Update count to matched rows
			cout << "Scanning table for column '" << whereColumnName << "'." << endl;
		}

		// Update the rows
		for (int i = 0; i < keyCount; i++) {
			Row& row = table->getRow(rowIds[i]);
			row.setStringData(setColumnIndex, setValue);
			updatedRows++;
		}

		// Update the index for the set column if it exists
		try {
			Index index = indexManager.getIndexByColumnAndTable(setColumnName, tableName);
			indexManager.clear(index.getIndexName()); // Clear the existing index

			// Recreate the index
			for (int rowIndex = 0; rowIndex < table->getNoRows(); rowIndex++) {
				indexManager.addValue(index.getIndexName(), table->getRow(rowIndex).getTextData(setColumnIndex), rowIndex);
			}
			cout << "Recreated index for column: " << setColumnName << endl;
		}
		catch (const std::runtime_error& e) {
			// No index for this column, skip updating
		}

		cout << "Updated " << updatedRows << " rows in table '" << tableName << "' by setting " << setColumnName << " to '" << setValue << "' where " << whereColumnName << " is '" << whereValue << "'." << endl;

		// Cleanup
		delete[] rowIds;
	}
	void alterTableAddColumn(const string& tableName, const Column& newColumn) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		table->addColumn(newColumn);

		cout << endl << "Column '" << newColumn.getName() << "' added to table '" << tableName << "' successfully.";
	}
	void alterTableDeleteColumn(const string& tableName, const string& columnName) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		if (!table->columnExists(columnName)) {
			cout << endl << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'.";
			return;
		}

		int columnIndex = table->getColumnIndex(columnName);

		//remove all indexes associated with the column
		for (int i = 0; i < indexManager.getNoIndexes(); ++i) {
			Index** indexes = indexManager.getIndexes();
			Index* index = indexes[i];
			if (index->getTableName() == tableName && index->getColumnName() == columnName) {
				indexManager.deleteIndex(index->getIndexName());
				cout << endl << "Index on column '" << columnName << "' in table '" << tableName << "' removed successfully.";
				--i; //adjust the index after deletion
			}
		}

		table->deleteColumnByIndex(columnIndex);
		cout << endl << "Column '" << columnName << "' deleted from table '" << tableName << "' successfully.";
	}
	void dropIndex(const string& indexName) {
		if (!indexManager.indexExistsByIndexName(indexName)) {
			cout << endl << "Error: Index with name '" << indexName << "' does not exist.";
			return;
		}

		indexManager.deleteIndex(indexName);
		cout << endl << "Index with name '" << indexName << "' removed successfully.";
	}
	void showTables() const {
		if (noTables == 0) {
			cout << "No tables found.";
			return;
		}

		cout << "My Tables:";
		for (int i = 0; i < noTables; i++) {
			cout << endl << i + 1 << ". " << database[i]->getName();
		}
	}
	void showIndexFromTable(const string& tableName) const {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table: " << "'" << tableName << "'" << " does not exist.";
			return;
		}

		bool found = false;
		for (int i = 0; i < indexManager.getNoIndexes(); ++i) {
			Index** indexes = indexManager.getIndexes();
			Index* index = indexes[i];
			if (index->getTableName() == tableName) {
				cout << "Index Name: " << index->getIndexName() << ", Column Name: " << index->getColumnName() << "\n";
				index->displayIndex();
				found = true;
			}
		}
		if (!found) {
			cout << "Error: No indexes found for table '" << tableName << "'." << endl;
		}
	}
	void showIndexFromAll() const {
		indexManager.displayAllIndexes();
	}
	//ALTA CLASA PT ASTEA 2
	void printHelpMenu() {
		string commands[] = {
			"CREATE TABLE",
			"DESCRIBE TABLE",
			"DROP TABLE",
			"SELECT ALL",
			"INSERT INTO VALUES",
			"DELETE FROM WHERE",
			"SELECT WHERE",
			"SELECT",
			"UPDATE TABLE",
			"ALTER TABLE ADD COLUMN",
			"ALTER TABLE DROP COLUMN",
			"CREATE INDEX",
			"DROP INDEX",
			"SHOW TABLES",
			"SHOW INDEX FROM",
			"SHOW INDEX FROM ALL",
			"clear",
			"exit"
		};

		string descriptions[] = {
			" Create a new table with specified columns and constraints.",
			" Show the structure of an existing table.",
			" Delete an existing table from the database.",
			" Select all rows from a table.",
			" Insert a new row into a table.",
			" Delete rows from a table based on a condition.",
			" Select specific columns from a table based on a condition.",
			" Select specific columns from a table without conditions.",
			" Update existing rows in a table based on a condition.",
			"Add a new column to an existing table.",
			"Drop an existing column from a table.",
			"Create an index on a specified column of a table.",
			"Drop an existing index from a table.",
			"Show all the tables in the database.",
			"Show indexes associated with a specific table.",
			"Show all indexes in the database.",
			"Clear the console screen.",
			"Exit the program."
		};

		cout << "Available Commands:\n";
		cout << "----------------------------------\n";

		for (int i = 0; i < 18; ++i) {
			cout << i + 1 << ". " << commands[i];
			int spaces = 30 - commands[i].length();
			for (int j = 0; j < spaces; ++j) {
				cout << " "; // fill the space to align descriptions
			}
			cout << descriptions[i] << "\n\n";
		}

		cout << "Type 'help 2' to see each command's syntax.\n";
	}
	void printSyntaxMenu() {
		std::cout << "========================== HELP MENU ==========================\n";
		std::cout << "1. CREATE TABLE table_name (column_name, column_type, size, default_value)\n";
		std::cout << "   - Creates a new table with the specified columns.\n";
		std::cout << "\n";
		std::cout << "2. DESCRIBE table_name\n";
		std::cout << "   - Displays the structure of the specified table.\n";
		std::cout << "\n";
		std::cout << "3. DROP TABLE table_name\n";
		std::cout << "   - Deletes the specified table.\n";
		std::cout << "\n";
		std::cout << "4. SELECT ALL FROM table_name\n";
		std::cout << "   - Retrieves all rows from the specified table.\n";
		std::cout << "\n";
		std::cout << "5. INSERT INTO table_name VALUES (value1, value2, ...)\n";
		std::cout << "   - Inserts new values into the specified table.\n";
		std::cout << "\n";
		std::cout << "6. DELETE FROM table_name WHERE condition\n";
		std::cout << "   - Deletes rows from the table matching the condition.\n";
		std::cout << "\n";
		std::cout << "7. SELECT column1, column2, ... FROM table_name WHERE condition\n";
		std::cout << "   - Retrieves specific columns from the table based on the condition.\n";
		std::cout << "\n";
		std::cout << "8. SELECT column1, column2, ... FROM table_name\n";
		std::cout << "   - Retrieves specific columns from the table.\n";
		std::cout << "\n";
		std::cout << "9. UPDATE table_name SET column1 = value1, column2 = value2, ... WHERE condition\n";
		std::cout << "   - Updates rows in the table that match the condition.\n";
		std::cout << "\n";
		std::cout << "10. ALTER TABLE table_name ADD (column_name, column_type, size, default_value)\n";
		std::cout << "    - Adds a new column to the specified table.\n";
		std::cout << "\n";
		std::cout << "11. ALTER TABLE table_name DROP COLUMN column_name\n";
		std::cout << "    - Removes a column from the specified table.\n";
		std::cout << "\n";
		std::cout << "12. CREATE INDEX index_name ON table_name (column_name)\n";
		std::cout << "    - Creates an index on the specified column in the table.\n";
		std::cout << "\n";
		std::cout << "13. DROP INDEX index_name\n";
		std::cout << "    - Deletes the specified index.\n";
		std::cout << "\n";
		std::cout << "14. SHOW TABLES\n";
		std::cout << "    - Lists all the tables in the database.\n";
		std::cout << "\n";
		std::cout << "15. SHOW INDEX FROM table_name\n";
		std::cout << "    - Displays all indices on the specified table.\n";
		std::cout << "\n";
		std::cout << "16. SHOW INDEX FROM ALL\n";
		std::cout << "    - Displays all indices across all tables.\n";
		std::cout << "\n";
		std::cout << "17. clear\n";
		std::cout << "    - Clears the console screen.\n";
		std::cout << "\n";
		std::cout << "18. exit\n";
		std::cout << "    - Exits the program.\n";
		std::cout << "================================================================\n";
	}
	//--------------------------------------------------
	//refactor these with helper function later
	void saveDatabase(const string& tablesConfigAddress) const {
		for (int i = 0; i < noTables; i++) {
			const Table& table = *database[i];
			string filename = tablesConfigAddress + table.getName() + ".bin";
			ofstream outFile(filename, ios::binary);
			if (!outFile) {
				cout << endl << "Error: Could not create file " << filename;
				continue;
			}

			//write the table structure to the file
			int nameLength = table.getName().length();
			outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
			outFile.write(table.getName().c_str(), nameLength);

			int noColumns = table.getNoColumns();
			outFile.write(reinterpret_cast<const char*>(&noColumns), sizeof(noColumns));
			for (int j = 0; j < noColumns; ++j) {
				const Column& column = table.getColumn(j);
				const string& columnName = column.getName();
				int columnNameLength = columnName.length();
				outFile.write(reinterpret_cast<const char*>(&columnNameLength), sizeof(columnNameLength));
				outFile.write(columnName.c_str(), columnNameLength);

				ColumnType columnType = column.getType();
				outFile.write(reinterpret_cast<const char*>(&columnType), sizeof(columnType));

				int columnSize = column.getSize();
				outFile.write(reinterpret_cast<const char*>(&columnSize), sizeof(columnSize));

				const string& defaultValue = column.getDefaultValue();
				int defaultValueLength = defaultValue.length();
				outFile.write(reinterpret_cast<const char*>(&defaultValueLength), sizeof(defaultValueLength));
				outFile.write(defaultValue.c_str(), defaultValueLength);

				bool unique = column.isUnique();
				outFile.write(reinterpret_cast<const char*>(&unique), sizeof(unique));
			}

			//write the table data to the file
			int noRows = table.getNoRows();
			outFile.write(reinterpret_cast<const char*>(&noRows), sizeof(noRows));
			for (int j = 0; j < noRows; ++j) {
				const Row& row = table.getRow(j);
				for (int k = 0; k < noColumns; ++k) {
					const string& value = row.getTextData(k);
					int valueLength = value.length();
					outFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
					outFile.write(value.c_str(), valueLength);
				}
			}

			//write the indexes to the file
			for (int j = 0; j < indexManager.getNoIndexes(); ++j) {
				Index** indexes = indexManager.getIndexes();
				Index* index = indexes[j];
				if (index->getTableName() == table.getName()) {
					//write index name, table name, and column name
					int indexNameLength = index->getIndexName().length();
					outFile.write(reinterpret_cast<const char*>(&indexNameLength), sizeof(indexNameLength));
					outFile.write(index->getIndexName().c_str(), indexNameLength);
					cout << "Index Name: " << index->getIndexName() << endl;

					int tableNameLength = index->getTableName().length();
					outFile.write(reinterpret_cast<const char*>(&tableNameLength), sizeof(tableNameLength));
					outFile.write(index->getTableName().c_str(), tableNameLength);
					cout << "Table Name: " << index->getTableName() << endl;

					int columnNameLength = index->getColumnName().length();
					outFile.write(reinterpret_cast<const char*>(&columnNameLength), sizeof(columnNameLength));
					outFile.write(index->getColumnName().c_str(), columnNameLength);
					cout << "Column Name: " << index->getColumnName() << endl;

					// Reverse the list before saving
					index->reverseListInPlace();

					// Create an iterator to iterate over ValueNode list
					Index::Iterator iter = index->createIterator();

					// Write values and their positions
					std::string value;
					int* positions = nullptr;
					int positionCount = 0;

					while (iter.hasNext()) {
						// Get the current value and positions
						value = iter.currentValue();
						positions = iter.currentPositions(positionCount);  // Get positions for the current value

						// Write the value
						int valueLength = value.length();
						outFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
						outFile.write(value.c_str(), valueLength);
						std::cout << "Value: " << value << std::endl;

						// Write position count
						outFile.write(reinterpret_cast<const char*>(&positionCount), sizeof(positionCount));
						std::cout << "Position Count: " << positionCount << std::endl;

						// Write positions
						for (int i = 0; i < positionCount; ++i) {
							outFile.write(reinterpret_cast<const char*>(&positions[i]), sizeof(positions[i]));
							std::cout << "Position: " << positions[i] << std::endl;
						}

						// Cleanup: delete positions array to avoid memory leak
						delete[] positions;

						// Move to the next ValueNode in the iteration
						iter.next();
					}

					//write an end marker to indicate the end of the current index
					int endIndexMarker = -1;
					outFile.write(reinterpret_cast<const char*>(&endIndexMarker), sizeof(endIndexMarker));
					cout << "End of Index Marker Written: " << endIndexMarker << endl;
				}
			}

			outFile.close();
			cout << endl << "Database saved successfully." << endl;
		}
	}
	void loadDatabase(const string& tablesConfigAddress, const string& selectCommandsAddress) {
		//clear the current database
		for (int i = 0; i < noTables; ++i) {
			removeTable(0);
		}

		//clear the contents of the select_commands folder
		for (const auto& entry : filesystem::directory_iterator(selectCommandsAddress)) {
			filesystem::remove_all(entry.path());
		}

		//iterate over the files in the directory where the tables are saved
		for (const auto& entry : filesystem::directory_iterator(tablesConfigAddress)) {
			if (entry.path().extension() == ".bin") {
				ifstream inFile(entry.path(), ios::binary);
				if (!inFile) {
					cout << "Error: Could not open file " << entry.path() << endl;
					continue;
				}

				//read the table structure from the file
				int nameLength;
				inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
				string tableName(nameLength, ' ');
				inFile.read(&tableName[0], nameLength);

				int noColumns;
				inFile.read(reinterpret_cast<char*>(&noColumns), sizeof(noColumns));
				Column* columns = new Column[noColumns];
				for (int j = 0; j < noColumns; ++j) {
					int columnNameLength;
					inFile.read(reinterpret_cast<char*>(&columnNameLength), sizeof(columnNameLength));
					string columnName(columnNameLength, ' ');
					inFile.read(&columnName[0], columnNameLength);

					ColumnType columnType;
					inFile.read(reinterpret_cast<char*>(&columnType), sizeof(columnType));

					int columnSize;
					inFile.read(reinterpret_cast<char*>(&columnSize), sizeof(columnSize));

					int defaultValueLength;
					inFile.read(reinterpret_cast<char*>(&defaultValueLength), sizeof(defaultValueLength));
					string defaultValue(defaultValueLength, ' ');
					inFile.read(&defaultValue[0], defaultValueLength);

					bool unique;
					inFile.read(reinterpret_cast<char*>(&unique), sizeof(unique));

					try {
						columns[j] = Column(columnName, columnType, columnSize, defaultValue, unique);
					}
					catch (const invalid_argument& e) {
						cout << endl << e.what();
					}
				}

				Table table(tableName, columns, noColumns);
				delete[] columns;

				//read the table data from the file
				int noRows;
				inFile.read(reinterpret_cast<char*>(&noRows), sizeof(noRows));
				for (int j = 0; j < noRows; ++j) {
					string* values = new string[noColumns];
					for (int k = 0; k < noColumns; ++k) {
						int valueLength;
						inFile.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
						string value(valueLength, ' ');
						inFile.read(&value[0], valueLength);
						values[k] = value;
					}
					table.addRowWithoutPrintMessage(values);
					delete[] values;
				}

				addTableToDatabase(table);

				//read the index information from the file
				while (inFile.peek() != EOF) {
					//read index metadata
					int indexNameLength;
					inFile.read(reinterpret_cast<char*>(&indexNameLength), sizeof(indexNameLength));
					string indexName(indexNameLength, ' ');
					inFile.read(&indexName[0], indexNameLength);
					cout << "Loaded Index Name: " << indexName << endl;

					int tableNameLength;
					inFile.read(reinterpret_cast<char*>(&tableNameLength), sizeof(tableNameLength));
					string indexTableName(tableNameLength, ' ');
					inFile.read(&indexTableName[0], tableNameLength);
					cout << "Loaded Table Name: " << indexTableName << endl;

					int columnNameLength;
					inFile.read(reinterpret_cast<char*>(&columnNameLength), sizeof(columnNameLength));
					string columnName(columnNameLength, ' ');
					inFile.read(&columnName[0], columnNameLength);
					cout << "Loaded Column Name: " << columnName << endl;

					//create the index
					indexManager.createIndex(indexName, indexTableName, columnName);

					//read values and positions for this index
					while (true) {
						//read value length and value
						int valueLength;
						inFile.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
						if (inFile.fail()) break; //exit loop if read failed or EOF reached unexpectedly

						string value(valueLength, ' ');
						inFile.read(&value[0], valueLength);
						if (inFile.fail()) break;
						cout << "Loaded Value: " << value << endl;

						//position count
						int positionCount;
						inFile.read(reinterpret_cast<char*>(&positionCount), sizeof(positionCount));
						if (inFile.fail()) break;
						cout << "Loaded Position Count: " << positionCount << endl;

						if (positionCount < 0) {
							cerr << "Error: Invalid position count for value '" << value << "' in index '" << indexName << "'." << endl;
							break; //abort reading this index to avoid corruption
						}

						//read positions
						for (int i = 0; i < positionCount; ++i) {
							int position;
							inFile.read(reinterpret_cast<char*>(&position), sizeof(position));
							if (inFile.fail()) {
								cerr << "Error: Unexpected EOF or failure while reading positions for value '" << value << "'." << endl;
								break;
							}
							cout << "Loaded Position: " << position << endl;

							if (!indexManager.addValueToIndex(indexName, value, position)) {
								cerr << "Error: Could not add position to index." << endl;
								break;
							}
						}

						//check for the end marker to check if we reached the end of this index
						int endIndexMarker;
						inFile.read(reinterpret_cast<char*>(&endIndexMarker), sizeof(endIndexMarker));
						if (endIndexMarker == -1) {
							cout << "End of Index Marker Found for: " << indexName << endl;
							break; //exit loop when end marker for this index is found
						}
						else {
							//if the marker isnt found -> reset file position or handle error
							inFile.seekg(-static_cast<std::streamoff>(sizeof(endIndexMarker)), ios::cur); //move the pointer back by one marker size
						}

						//ensure no data corruption
						if (inFile.fail()) {
							cerr << "Error: Data corruption detected while reading index '" << indexName << "'." << endl;
							break;
						}
					}
				}
				inFile.close();
			}
		}
		cout << endl << "Database loaded successfully." << endl;
	}
};
int Database::selectCount = 0;

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
class describeTable : public Command {
private:
	string name;
public:
	describeTable() {
		this->name = "";
	}
	describeTable(const string& name) {
		this->name = name;
	}
	describeTable(const describeTable& dt) {
		this->name = dt.name;
	}
	describeTable& operator=(const describeTable& dt) {
		if (this == &dt) {
			return *this;
		}
		this->name = dt.name;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		if (!db.tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		try {
			//get a copy of the table from the database
			Table table = db.getTableByName(name);

			table.describeTable();
		}
		catch (const runtime_error& e) {
			cout << endl << "Error: " << e.what();
		}
	}
	static describeTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "DESCRIBE "
		if (commandCopy.find("DESCRIBE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//get the table name
		string tableName = commandCopy.substr(9);  // 9 is the length of "DESCRIBE " with a space after
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		//return a new describeTable object
		return describeTable(tableName);
	}
};
class dropTable : public Command {
private:
	string name;
	string tablesConfigAddress;
public:
	dropTable() {
		this->name = "";
		this->tablesConfigAddress = "";
	}
	dropTable(const string& name, const string& tablesConfigAddress) {
		this->name = name;
		this->tablesConfigAddress = tablesConfigAddress;
	}
	dropTable(const dropTable& dt) {
		this->name = dt.name;
		this->tablesConfigAddress = dt.tablesConfigAddress;
	}
	dropTable& operator=(const dropTable& dt) {
		if (this == &dt) {
			return *this;
		}
		this->name = dt.name;
		this->tablesConfigAddress = dt.tablesConfigAddress;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.dropTable(name, tablesConfigAddress);
	}
	static dropTable parseCommand(const string& command, const string& tablesConfigAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "DROP TABLE "
		if (commandCopy.find("DROP TABLE ") != 0) {
			if (commandCopy.find("DROP TABLE") == 0) {
				cout << endl << "Invalid command format.";
			}
			else {
				cout << endl << "Invalid command format.";
			}
			throw invalid_argument("Invalid command format.");
		}

		// Get the table name
		string tableName = commandCopy.substr(11);  // 11 is the length of "DROP TABLE " with a space after
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			cout << endl << "Invalid command format. Too few arguments.";
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		// Check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			cout << endl << "Invalid command format. Too many arguments.";
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		return dropTable(tableName, tablesConfigAddress);
	}
};
class InsertCommand : public Command {
private:
	string tableName;
	string* values;
	int noValues;

public:
	InsertCommand() : tableName(""), values(nullptr), noValues(0) {}
	InsertCommand(const string& tableName, const string* values, int noValues)
		: tableName(tableName), values(new string[noValues]), noValues(noValues) {
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = values[i];
		}
	}
	InsertCommand(const InsertCommand& other)
		: tableName(other.tableName), values(new string[other.noValues]), noValues(other.noValues) {
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = other.values[i];
		}
	}
	InsertCommand& operator=(const InsertCommand& other) {
		if (this == &other) {
			return *this;
		}

		delete[] values;

		tableName = other.tableName;
		noValues = other.noValues;
		values = new string[noValues];
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = other.values[i];
		}

		return *this;
	}
	~InsertCommand() {
		delete[] values;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.insertValues(tableName, values, noValues);
	}
	static InsertCommand parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "INSERT INTO "
		if (commandCopy.find("INSERT INTO ") != 0) {
			throw std::invalid_argument("Invalid command format: Missing 'INSERT INTO'.");
		}

		// Find the position of "VALUES "
		size_t pos = commandCopy.find("VALUES ");
		if (pos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing 'VALUES'.");
		}

		// Get the table name
		string tableName = commandCopy.substr(12, pos - 12);  // 12 is the length of "INSERT INTO " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw std::invalid_argument("Invalid command format: Table name cannot be empty.");
		}

		// Find the position of the first '('
		size_t startPos = commandCopy.find("(");
		if (startPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing '('.");
		}

		// Find the position of the last ')'
		size_t endPos = commandCopy.find_last_of(")");
		if (endPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing ')'.");
		}

		// Get the values part
		string valuesPart = commandCopy.substr(startPos + 1, endPos - startPos - 1);  // from after '(' to before ')'
		stringUtils::trim(valuesPart);

		if (valuesPart.empty()) {
			throw std::invalid_argument("Invalid command format: Values cannot be empty.");
		}

		// Split the values part into individual values
		string* values = nullptr;
		int noValues = 0;
		stringUtils::splitCommand(valuesPart, ",", values, noValues);

		if (noValues == 0) {
			throw std::invalid_argument("Invalid command format: No values specified.");
		}

		// Trim each value individually
		for (int i = 0; i < noValues; i++) {
			stringUtils::trim(values[i]);
		}

		InsertCommand insertCommand(tableName, values, noValues);
		delete[] values;
		return insertCommand;
	}
};
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

class selectCommands : public Command {
protected:
	string selectCommandsAddress;
	static int selectCount;
public:
	selectCommands() {
		this->selectCommandsAddress = "";
	}
	selectCommands(const string& selectCommandsAddress) {
		this->selectCommandsAddress = selectCommandsAddress;
	}
	selectCommands(const selectCommands& sc) {
		this->selectCommandsAddress = sc.selectCommandsAddress;
	}
	selectCommands& operator=(const selectCommands& sc) {
		if (this == &sc) {
			return *this;
		}
		this->selectCommandsAddress = sc.selectCommandsAddress;
		return *this;
	}
	//--------------------------------------------------
	virtual void execute(Database& db) = 0;
	virtual ~selectCommands() = default;
};
int selectCommands::selectCount = 0;
class selectAll : public selectCommands {
private:
	string name;
public:
	selectAll() : selectCommands() {
		this->name = "";
	}
	selectAll(const string& name, const string& selectCommandsAddress) : selectCommands(selectCommandsAddress) {
		this->name = name;
	}
	selectAll(const selectAll& sa) : selectCommands(sa.selectCommandsAddress) {
		this->name = sa.name;
	}
	selectAll& operator=(const selectAll& sa) {
		if (this == &sa) {
			return *this;
		}
		this->name = sa.name;
		this->selectCommandsAddress = sa.selectCommandsAddress;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		if (!db.tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		try {
			//get a copy of the table from the database
			Table table = db.getTableByName(name);

			//find a unique file name
			string fileName;
			do {
				selectCount++;
				fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
			} while (filesystem::exists(fileName));

			//redirect cout to a file
			ofstream outFile(fileName);
			if (!outFile) {
				cout << endl << "Error: Could not open file: " << fileName;
				return;
			}
			streambuf* coutBuf = cout.rdbuf(); //save old buffer
			cout.rdbuf(outFile.rdbuf()); //redirect cout to file

			//display table to file
			table.displayTable();

			//restore cout to its original state
			cout.rdbuf(coutBuf);

			//also display on the screen
			table.displayTable();
		}
		catch (const runtime_error& e) {
			cout << endl << "Error: " << e.what();
		}
	}
	static selectAll parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT ALL FROM "
		if (commandCopy.find("SELECT ALL FROM ") != 0) {
			if (commandCopy.find("SELECT ALL FROM") == 0) {
				throw invalid_argument("Invalid command format.");
			}
			else {
				throw invalid_argument("Invalid command format.");
			}
		}

		//find the position of "FROM " and make sure there is a space after it
		size_t pos = commandCopy.find("FROM ") + 5;  // 5 is the length of "FROM " with the space
		if (pos >= commandCopy.length()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//get the table name
		string tableName = commandCopy.substr(pos);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		//return a new selectAll object
		return selectAll(tableName, selectCommandsAddress);
	}
};
class selectWHERE : public selectCommands {
private:
	string tableName;
	string* columnNames;
	int noColumns;
	string conditionColumn;
	string value;
public:
	selectWHERE() : selectCommands(), columnNames(nullptr), noColumns(0) {}
	//param constructor
	selectWHERE(const string& tableName, const string* columnNames, int noColumns, const string& conditionColumn, const string& value, const string& selectCommandsAddress)
		: selectCommands(selectCommandsAddress), tableName(tableName), noColumns(noColumns), conditionColumn(conditionColumn), value(value) {
		this->columnNames = new string[noColumns];
		for (int i = 0; i < noColumns; ++i) {
			this->columnNames[i] = columnNames[i];
		}
	}
	//cpy constructor
	selectWHERE(const selectWHERE& sw)
		: selectCommands(sw.selectCommandsAddress), tableName(sw.tableName), noColumns(sw.noColumns), conditionColumn(sw.conditionColumn), value(sw.value) {
		this->columnNames = new string[sw.noColumns];
		for (int i = 0; i < sw.noColumns; ++i) {
			this->columnNames[i] = sw.columnNames[i];
		}
	}
	//assignment operator
	selectWHERE& operator=(const selectWHERE& sw) {
		if (this == &sw) {
			return *this;
		}
		this->tableName = sw.tableName;
		this->noColumns = sw.noColumns;
		this->conditionColumn = sw.conditionColumn;
		this->value = sw.value;
		this->selectCommandsAddress = sw.selectCommandsAddress;

		delete[] this->columnNames;
		this->columnNames = new string[sw.noColumns];
		for (int i = 0; i < sw.noColumns; ++i) {
			this->columnNames[i] = sw.columnNames[i];
		}
		return *this;
	}
	//destructor
	~selectWHERE() {
		delete[] columnNames;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		selectCount++;

		// Validate table existence
		if (!db.tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		try {
			// Get a copy of the table from the database
			Table table = db.getTableByName(tableName);

			int conditionColumnIndex = table.getColumnIndex(conditionColumn);
			if (conditionColumnIndex == -1) {
				cout << "Error: Column '" << conditionColumn << "' does not exist in table: " << tableName << endl;
				return;
			}

			int* rows = nullptr;
			int rowCount = 0;

			try {
				// Use the method directly from the Database class
				Index index = db.getIndexByColumnAndTable(conditionColumn, tableName);

				// Use the index to get the rows
				cout << "Using index for column: " << conditionColumn << endl;
				rowCount = index.getRowCount(value);
				if (rowCount > 0) {
					rows = new int[rowCount];
					index.getKeys(value, rows, rowCount);
				}
			}
			catch (const std::runtime_error& e) {
				// Handle the case where the index is not found
				cout << "Scanning table for column '" << conditionColumn << "'." << endl;

				// No index, scan the table
				rowCount = table.getNoRows();
				rows = new int[rowCount];
				int rowIndex = 0;
				for (int i = 0; i < rowCount; i++) {
					if (table.getRow(i).getTextData(conditionColumnIndex) == value) {
						rows[rowIndex++] = i;
					}
				}
				rowCount = rowIndex; // Update count to matched rows
			}

			if (rowCount == 0) {
				cout << "No rows found with " << conditionColumn << " = " << value << endl;
				delete[] rows;
				return;
			}

			// Validate column existence and initialize indexes
			int* columnIndexes = new int[noColumns];
			if (!HelpersForSelectCommands::validateColumns(&table, columnNames, noColumns, columnIndexes)) {
				delete[] columnIndexes;
				delete[] rows;
				return;
			}

			// Calculate column widths
			int* maxWidth = HelpersForSelectCommands::calculateMaxWidths(&table, columnNames, noColumns, columnIndexes);

			// Create the file path
			string fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
			ofstream outFile(fileName);
			if (!outFile.is_open()) {
				cout << "Error: Could not open file: " << fileName << endl;
				delete[] columnIndexes;
				delete[] maxWidth;
				delete[] rows;
				return;
			}

			// Redirect cout to a file
			streambuf* coutBuf = cout.rdbuf();
			cout.rdbuf(outFile.rdbuf());

			// Print headers and rows to file
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
			HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			for (int i = 0; i < rowCount; i++) {
				const Row& row = table.getRow(rows[i]);
				HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
			}

			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			// Restore cout
			cout.rdbuf(coutBuf);

			// Print headers and rows to screen
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
			HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			for (int i = 0; i < rowCount; i++) {
				const Row& row = table.getRow(rows[i]);
				HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
			}

			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			// Cleanup
			delete[] columnIndexes;
			delete[] maxWidth;
			delete[] rows;
		}
		catch (const runtime_error& e) {
			cout << "Error: " << e.what() << endl;
		}
	}
	static selectWHERE parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT "
		if (commandCopy.find("SELECT ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//find the position of "FROM "
		size_t fromPos = commandCopy.find("FROM ");
		if (fromPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'FROM'.");
		}

		//make sure there is a space before "FROM "
		if (commandCopy[fromPos - 1] != ' ') {
			throw invalid_argument("Invalid command format. Missing space before 'FROM'.");
		}

		//get the columns part
		string columnsPart = commandCopy.substr(7, fromPos - 8);  // 7 is the length of "SELECT " with a space
		stringUtils::trim(columnsPart);

		if (columnsPart.empty()) {
			throw invalid_argument("Invalid command format. Columns cannot be empty.");
		}

		//count the number of columns
		int noColumns = 1;
		for (char c : columnsPart) {
			if (c == ',') {
				noColumns++;
			}
		}

		//split the columns part into individual columns
		string* columns = new string[noColumns];
		size_t start = 0;
		size_t end = columnsPart.find(',');
		int index = 0;

		while (end != string::npos) {
			columns[index] = columnsPart.substr(start, end - start);
			stringUtils::trim(columns[index]);
			start = end + 1;
			end = columnsPart.find(',', start);
			index++;
		}

		//add the last column
		columns[index] = columnsPart.substr(start);
		stringUtils::trim(columns[index]);

		if (noColumns == 0) {
			delete[] columns;
			throw invalid_argument("Invalid command format. No columns specified.");
		}

		//find the position of "WHERE "
		size_t wherePos = commandCopy.find(" WHERE ");
		if (wherePos == string::npos) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Missing 'WHERE' or missing space before 'WHERE'.");
		}

		//get the table name
		string tableName = commandCopy.substr(fromPos + 5, wherePos - (fromPos + 5));  // 5 is the length of "FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		//get the condition part
		string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE " with spaces
		stringUtils::trim(conditionPart);

		if (conditionPart.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Condition cannot be empty.");
		}

		//split the condition part into column and value
		size_t equalPos = conditionPart.find('=');
		if (equalPos == string::npos) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Missing '=' in condition.");
		}

		string conditionColumn = conditionPart.substr(0, equalPos);
		stringUtils::trim(conditionColumn);
		string conditionValue = conditionPart.substr(equalPos + 1);
		stringUtils::trim(conditionValue);

		if (conditionColumn.empty() || conditionValue.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Condition column or value cannot be empty.");
		}

		selectWHERE selectCommand(tableName, columns, noColumns, conditionColumn, conditionValue, selectCommandsAddress);
		delete[] columns;
		return selectCommand;
	}
};
class selectColumns : public selectCommands {
private:
	string tableName;
	string* columnNames;
	int noColumns;
public:
	selectColumns() : selectCommands() {
		this->tableName = "";
		this->columnNames = nullptr;
		this->noColumns = 0;
	}
	selectColumns(const string& tableName, const string* columnNames, int noColumns, const string& selectCommandsAddress)
		: selectCommands(selectCommandsAddress) {
		this->tableName = tableName;
		this->columnNames = new string[noColumns];
		for (int i = 0; i < noColumns; ++i) {
			this->columnNames[i] = columnNames[i];
		}
		this->noColumns = noColumns;
	}
	selectColumns(const selectColumns& sc) : selectCommands(sc.selectCommandsAddress) {
		this->tableName = sc.tableName;
		this->noColumns = sc.noColumns;
		this->columnNames = new string[sc.noColumns];
		for (int i = 0; i < sc.noColumns; ++i) {
			this->columnNames[i] = sc.columnNames[i];
		}
	}
	selectColumns& operator=(const selectColumns& sc) {
		if (this == &sc) {
			return *this;
		}
		this->tableName = sc.tableName;
		this->selectCommandsAddress = sc.selectCommandsAddress;
		this->noColumns = sc.noColumns;
		delete[] this->columnNames;
		this->columnNames = new string[sc.noColumns];
		for (int i = 0; i < sc.noColumns; ++i) {
			this->columnNames[i] = sc.columnNames[i];
		}
		return *this;
	}
	~selectColumns() {
		delete[] columnNames;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		selectCount++;

		// Validate table existence
		if (!db.tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		// Get a copy of the table from the database
		Table table = db.getTableByName(tableName);

		// Validate column existence and initialize indexes
		int* columnIndexes = new int[noColumns];
		if (!HelpersForSelectCommands::validateColumns(&table, columnNames, noColumns, columnIndexes)) {
			delete[] columnIndexes;
			return;
		}

		// Calculate column widths
		int* maxWidth = HelpersForSelectCommands::calculateMaxWidths(&table, columnNames, noColumns, columnIndexes);

		// Create the file path
		string fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
		ofstream outFile(fileName);
		if (!outFile.is_open()) {
			cout << "Error: Could not open file: " << fileName << endl;
			delete[] columnIndexes;
			delete[] maxWidth;
			return;
		}

		// Redirect cout to a file
		streambuf* coutBuf = cout.rdbuf();
		cout.rdbuf(outFile.rdbuf());

		// Print headers and rows to file
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Print rows
		for (int i = 0; i < table.getNoRows(); i++) {
			const Row& row = table.getRow(i);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Restore cout
		cout.rdbuf(coutBuf);

		// Print headers and rows to screen
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		for (int i = 0; i < table.getNoRows(); i++) {
			const Row& row = table.getRow(i);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Cleanup
		delete[] columnIndexes;
		delete[] maxWidth;
	}
	static selectColumns parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT "
		if (commandCopy.find("SELECT ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//find the position of "FROM "
		size_t pos = commandCopy.find("FROM ");
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'FROM'.");
		}

		//make sure there is a space before "FROM "
		if (commandCopy[pos - 1] != ' ') {
			throw invalid_argument("Invalid command format. Missing space before 'FROM'.");
		}

		//get the columns part
		string columnsPart = commandCopy.substr(7, pos - 8);  // 7 is the length of "SELECT " with a space
		stringUtils::trim(columnsPart);

		if (columnsPart.empty()) {
			throw invalid_argument("Invalid command format. Columns cannot be empty.");
		}

		//count the number of columns
		int noColumns = 1;
		for (char c : columnsPart) {
			if (c == ',') {
				noColumns++;
			}
		}

		//split the columns part into individual columns
		string* columns = new string[noColumns];
		size_t start = 0;
		size_t end = columnsPart.find(',');
		int index = 0;

		while (end != string::npos) {
			columns[index] = columnsPart.substr(start, end - start);
			stringUtils::trim(columns[index]);
			start = end + 1;
			end = columnsPart.find(',', start);
			index++;
		}

		//add the last column
		columns[index] = columnsPart.substr(start);
		stringUtils::trim(columns[index]);

		if (noColumns == 0) {
			delete[] columns;
			throw invalid_argument("Invalid command format. No columns specified.");
		}

		//get the table name
		string tableName = commandCopy.substr(pos + 5);  // 5 is the length of "FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		selectColumns sc(tableName, columns, noColumns, selectCommandsAddress);
		delete[] columns;
		return sc;
	}
};
class selectAllWhere : public selectCommands {
private:
	string tableName;
	string conditionColumn;
	string value;
public:
	selectAllWhere() : selectCommands() {
		this->tableName = "";
		this->conditionColumn = "";
		this->value = "";
	}
	selectAllWhere(const string& tableName, const string& conditionColumn, const string& value, const string& selectCommandsAddress)
		: selectCommands(selectCommandsAddress), tableName(tableName), conditionColumn(conditionColumn), value(value) {}
	selectAllWhere(const selectAllWhere& saw)
		: selectCommands(saw.selectCommandsAddress), tableName(saw.tableName), conditionColumn(saw.conditionColumn), value(saw.value) {}
	selectAllWhere& operator=(const selectAllWhere& saw) {
		if (this == &saw) {
			return *this;
		}
		this->tableName = saw.tableName;
		this->conditionColumn = saw.conditionColumn;
		this->value = saw.value;
		this->selectCommandsAddress = saw.selectCommandsAddress;
		return *this;
	}

	void execute(Database& db) override {
		selectCount++;

		// Validate table existence
		if (!db.tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		// Get a copy of the table from the database
		Table table = db.getTableByName(tableName);

		int conditionColumnIndex = table.getColumnIndex(conditionColumn);
		if (conditionColumnIndex == -1) {
			cout << "Error: Column '" << conditionColumn << "' does not exist in table: " << tableName << endl;
			return;
		}

		int* rows = nullptr;
		int rowCount = 0;

		try {
			// Check if there is an index on the condition column
			Index index = db.getIndexByColumnAndTable(conditionColumn, tableName);

			// Use the index to get the rows
			cout << "Using index for column: " << conditionColumn << endl;
			rowCount = index.getRowCount(value);
			if (rowCount > 0) {
				rows = new int[rowCount];
				index.getKeys(value, rows, rowCount);
			}
		}
		catch (const std::runtime_error& e) {
			// Handle the case where the index is not found
			cout << "Scanning table for column '" << conditionColumn << "'." << endl;

			// No index, scan the table
			rowCount = table.getNoRows();
			rows = new int[rowCount];
			int rowIndex = 0;
			for (int i = 0; i < rowCount; i++) {
				if (table.getRow(i).getTextData(conditionColumnIndex) == value) {
					rows[rowIndex++] = i;
				}
			}
			rowCount = rowIndex; // Update count to matched rows
		}

		if (rowCount == 0) {
			cout << "No rows found with " << conditionColumn << " = " << value << endl;
			delete[] rows;
			return;
		}

		// Get all column names
		int noColumns = table.getNoColumns();
		string* columnNames = new string[noColumns];
		for (int i = 0; i < noColumns; i++) {
			columnNames[i] = table.getColumn(i).getName();
		}

		// Initialize column indexes
		int* columnIndexes = new int[noColumns];
		for (int i = 0; i < noColumns; i++) {
			columnIndexes[i] = i;
		}

		// Calculate column widths
		int* maxWidth = HelpersForSelectCommands::calculateMaxWidths(&table, columnNames, noColumns, columnIndexes);

		// Create the file path
		string fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
		ofstream outFile(fileName);
		if (!outFile.is_open()) {
			cout << "Error: Could not open file: " << fileName << endl;
			delete[] columnNames;
			delete[] columnIndexes;
			delete[] maxWidth;
			delete[] rows;
			return;
		}

		// Redirect cout to a file
		streambuf* coutBuf = cout.rdbuf();
		cout.rdbuf(outFile.rdbuf());

		// Print headers and rows to file
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		for (int i = 0; i < rowCount; i++) {
			const Row& row = table.getRow(rows[i]);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Restore cout
		cout.rdbuf(coutBuf);

		// Print headers and rows to screen
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		for (int i = 0; i < rowCount; i++) {
			const Row& row = table.getRow(rows[i]);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Cleanup
		delete[] columnNames;
		delete[] columnIndexes;
		delete[] maxWidth;
		delete[] rows;
	}
	static selectAllWhere parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "SELECT ALL "
		if (commandCopy.find("SELECT ALL ") != 0) {
			cout << endl << "Invalid command format.";
			return selectAllWhere();
		}

		// Find the position of "FROM "
		size_t fromPos = commandCopy.find("FROM ");
		if (fromPos == string::npos) {
			cout << endl << "Invalid command format. Missing 'FROM'.";
			return selectAllWhere();
		}

		// Make sure there is a space before "FROM "
		if (commandCopy[fromPos - 1] != ' ') {
			cout << endl << "Invalid command format. Missing space before 'FROM'.";
			return selectAllWhere();
		}

		// Get the table name
		size_t wherePos = commandCopy.find(" WHERE ");
		if (wherePos == string::npos) {
			cout << endl << "Invalid command format. Missing 'WHERE' or missing space before 'WHERE'.";
			return selectAllWhere();
		}

		string tableName = commandCopy.substr(fromPos + 5, wherePos - (fromPos + 5));  // 5 is the length of "FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			cout << endl << "Invalid command format. Table name cannot be empty.";
			return selectAllWhere();
		}

		// Get the condition part
		string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE " with spaces
		stringUtils::trim(conditionPart);

		if (conditionPart.empty()) {
			cout << endl << "Invalid command format. Condition cannot be empty.";
			return selectAllWhere();
		}

		// Split the condition part into column and value
		size_t equalPos = conditionPart.find('=');
		if (equalPos == string::npos) {
			cout << endl << "Invalid command format. Missing '=' in condition.";
			return selectAllWhere();
		}

		string conditionColumn = conditionPart.substr(0, equalPos);
		stringUtils::trim(conditionColumn);
		string conditionValue = conditionPart.substr(equalPos + 1);
		stringUtils::trim(conditionValue);

		if (conditionColumn.empty() || conditionValue.empty()) {
			cout << endl << "Invalid command format. Condition column or value cannot be empty.";
			return selectAllWhere();
		}

		return selectAllWhere(tableName, conditionColumn, conditionValue, selectCommandsAddress);
	}
};

class commandParser; //declaration ahead of commandParser to avoid errors

// struct to hold command keyword and corresponding handler
struct CommandMapping {
	const char* commandKeywords[3];
	Command* (commandParser::* handlerFunction)(const string&);  // member function pointer to command handler
};

class commandParser {
private:
	static CommandMapping commandMappings[];
	string selectCommandsAddress;
	string tablesConfigAddress;

	Command* handleCreateTable(const string& command) {
		return new createTable(createTable::parseCommand(command));
	}
	Command* handleDescribeTable(const string& command) {
		return new describeTable(describeTable::parseCommand(command));
	}
	Command* handleDropTable(const string& command) {
		return new dropTable(dropTable::parseCommand(command, tablesConfigAddress));
	}
	Command* handleInsertCommand(const string& command) {
		return new InsertCommand(InsertCommand::parseCommand(command));
	}
	Command* handleCreateIndex(const string& command) {
		return new createIndex(createIndex::parseCommand(command));
	}
	Command* handleSelectAll(const string& command) {
		return new selectAll(selectAll::parseCommand(command, selectCommandsAddress));
	}
	Command* handleSelectWHERE(const string& command) {
		return new selectWHERE(selectWHERE::parseCommand(command, selectCommandsAddress));
	}
	Command* handleSelectColumns(const string& command) {
		return new selectColumns(selectColumns::parseCommand(command, selectCommandsAddress));
	}
	Command* handleSelectAllWhere(const string& command) {
		return new selectAllWhere(selectAllWhere::parseCommand(command, selectCommandsAddress));
	}
	// add more here

public:
	commandParser(const string& selectCommandsAddress, const string& tablesConfigAddress) {
		this->selectCommandsAddress = selectCommandsAddress;
		this->tablesConfigAddress = tablesConfigAddress;
	}
	Command* handleCommand(const string& command) {
		for (int i = 0; commandMappings[i].commandKeywords[0] != nullptr; i++) {
			bool match = true;
			for (int j = 0; j < 3 && commandMappings[i].commandKeywords[j] != nullptr; j++) {
				if (command.find(commandMappings[i].commandKeywords[j]) == string::npos) {
					match = false;
					break;
				}
			}
			if (match) {
				// pass the command to the member function pointer
				return (this->*commandMappings[i].handlerFunction)(command);
			}
		}
		throw invalid_argument("Unknown command.");
	}
};

CommandMapping commandParser::commandMappings[] = {
	{{"CREATE", "TABLE", nullptr}, &commandParser::handleCreateTable},
	{{"DESCRIBE", nullptr , nullptr}, &commandParser::handleDescribeTable},
	{{"DROP", "TABLE", nullptr}, &commandParser::handleDropTable},
	{{"INSERT", "INTO", nullptr}, &commandParser::handleInsertCommand},
	{{"CREATE", "INDEX", nullptr}, &commandParser::handleCreateIndex},
	//--------------------------------------------------
	{{"SELECT", "ALL", "WHERE"}, &commandParser::handleSelectAllWhere},
	{{"SELECT", "ALL" , nullptr}, &commandParser::handleSelectAll},
	{{"SELECT", "WHERE" , nullptr}, &commandParser::handleSelectWHERE},
	{{"SELECT", nullptr , nullptr}, &commandParser::handleSelectColumns},
	// add more command mappings here
	{{nullptr, nullptr, nullptr}, nullptr}  // end marker for the array
};

class Commands { //doar daca e comanda scrisa cum trebuie
	//VERIFIC DACA ARE FORMATU BUN LA COMANDA SI GENEREAZA UN OBIECT SPECIFIC COMENZII INTRODUSE
	//NIMIC LA MEMBRI
private:
	Database* db = nullptr;
	//HELPER FUNCTIONS----------------------------------
	ColumnType parseColumnType(const string& type) {
		//convert string to ColumnType for the column constructor
		if (type == "INT") return INT;
		if (type == "TEXT") return TEXT;
		if (type == "FLOAT") return FLOAT;
		if (type == "BOOLEAN") return BOOLEAN;
		if (type == "DATE") return DATE;
		throw invalid_argument("Invalid column type: " + type);
	}
	bool parseUnique(const string& unique) {
		//convert string to bool for the column constructor
		if (unique == "UNIQUE") return true;
		return false;
	}
	void trim(string& str) {
		//find the first non-space character
		size_t start = str.find_first_not_of(" ");
		//find the last non-space character
		size_t end = str.find_last_not_of(" ");

		//if the string is only spaces set it to an empty string
		if (start == string::npos) {
			str = "";
		}
		else {
			//make the trimmed string
			str = str.substr(start, end - start + 1);
		}
	}
	void splitCommand(const string& command, const string& delimiter, string*& tokens, int& tokenCount) {
		string commandCopy = command;
		trim(commandCopy);
		tokenCount = 0;
		size_t pos = 0;
		string token;
		while ((pos = commandCopy.find(delimiter)) != string::npos) {
			token = commandCopy.substr(0, pos);
			if (!token.empty()) {
				tokenCount++;
			}
			commandCopy.erase(0, pos + delimiter.length());
		}
		if (!commandCopy.empty()) {
			tokenCount++;
		}

		tokens = new string[tokenCount];
		commandCopy = command;
		pos = 0;
		int i = 0;
		while ((pos = commandCopy.find(delimiter)) != string::npos) {
			token = commandCopy.substr(0, pos);
			if (!token.empty()) {
				tokens[i++] = token;
			}
			commandCopy.erase(0, pos + delimiter.length());
		}
		if (!commandCopy.empty()) {
			tokens[i] = commandCopy;
		}
	}
private:
	void stringCommandDeleteFromWhere(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "DELETE FROM "
			if (commandCopy.find("DELETE FROM ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of "WHERE "
			size_t pos = commandCopy.find("WHERE ");
			if (pos == string::npos) {
				cout << endl << "Invalid command format. Missing 'WHERE'.";
				return;
			}

			//make sure there is a space before "WHERE "
			if (commandCopy[pos - 1] != ' ') {
				cout << endl << "Invalid command format. Missing space before 'WHERE'.";
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(12, pos - 13);  // 12 is the length of "DELETE FROM " with a space
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//get the where part
			string wherePart = commandCopy.substr(pos + 6);  // 6 is the length of "WHERE " with a space
			trim(wherePart);

			if (wherePart.empty()) {
				cout << endl << "Invalid command format. Where part cannot be empty.";
				return;
			}

			//split the where part into individual parts
			string* whereParts = nullptr;
			int noParts = 0;
			splitCommand(wherePart, "=", whereParts, noParts);

			if (noParts != 2) {
				cout << endl << "Invalid command format. Invalid where part.";
				delete[] whereParts;
				return;
			}

			//get the column name
			string columnName = whereParts[0];
			trim(columnName);

			if (columnName.empty()) {
				cout << endl << "Invalid command format. Column name cannot be empty.";
				delete[] whereParts;
				return;
			}

			//get the value
			string value = whereParts[1];
			trim(value);

			if (value.empty()) {
				cout << endl << "Invalid command format. Value cannot be empty.";
				delete[] whereParts;
				return;
			}

			db->deleteRowFromTable(tableName, columnName, value);

			delete[] whereParts;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandUpdateTable(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "UPDATE "
			if (commandCopy.find("UPDATE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " SET "
			size_t setPos = commandCopy.find(" SET ");
			if (setPos == string::npos) {
				cout << endl << "Invalid command format. Missing 'SET'.";
				return;
			}

			//find the position of " WHERE "
			size_t wherePos = commandCopy.find(" WHERE ");
			if (wherePos == string::npos) {
				cout << endl << "Invalid command format. Missing 'WHERE'.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(7, setPos - 7);  // 7 is the length of "UPDATE "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//extract the set part
			string setPart = commandCopy.substr(setPos + 5, wherePos - (setPos + 5));  // 5 is the length of " SET "
			trim(setPart);

			if (setPart.empty()) {
				cout << endl << "Invalid command format. Set part cannot be empty.";
				return;
			}

			//split the set part into column and value
			size_t equalPos = setPart.find('=');
			if (equalPos == string::npos) {
				cout << endl << "Invalid command format. Missing '=' in set part.";
				return;
			}

			string setColumnName = setPart.substr(0, equalPos);
			trim(setColumnName);
			string setValue = setPart.substr(equalPos + 1);
			trim(setValue);

			if (setColumnName.empty() || setValue.empty()) {
				cout << endl << "Invalid command format. Set column or value cannot be empty.";
				return;
			}

			//extract the condition part
			string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE "
			trim(conditionPart);

			if (conditionPart.empty()) {
				cout << endl << "Invalid command format. Condition cannot be empty.";
				return;
			}

			//split the condition part into column and value
			size_t conditionEqualPos = conditionPart.find('=');
			if (conditionEqualPos == string::npos) {
				cout << endl << "Invalid command format. Missing '=' in condition part.";
				return;
			}

			string whereColumnName = conditionPart.substr(0, conditionEqualPos);
			trim(whereColumnName);
			string whereValue = conditionPart.substr(conditionEqualPos + 1);
			trim(whereValue);

			if (whereColumnName.empty() || whereValue.empty()) {
				cout << endl << "Invalid command format. Condition column or value cannot be empty.";
				return;
			}

			db->updateTable(tableName, setColumnName, setValue, whereColumnName, whereValue);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandAlterTableAddColumn(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "ALTER TABLE "
			if (commandCopy.find("ALTER TABLE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " ADD "
			size_t addPos = commandCopy.find(" ADD ");
			if (addPos == string::npos) {
				cout << endl << "Invalid command format.";
				return;
			}

			//check for '(' after " ADD "
			size_t openParenPos = commandCopy.find("(", addPos);
			if (openParenPos == string::npos) {
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(12, addPos - 12);  // 12 is the length of "ALTER TABLE "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//extract the column definition part
			string columnDef = commandCopy.substr(openParenPos + 1);  // 1 is the length of "("
			if (columnDef.back() != ')') {
				cout << endl << "Invalid command format. Missing ')'.";
				return;
			}
			columnDef.pop_back();  //remove the closing parenthesis
			trim(columnDef);

			//split the column definition into parts
			string columnParts[4];
			int partIndex = 0;
			size_t start = 0;
			size_t end = columnDef.find(',');

			while (end != string::npos && partIndex < 4) {
				columnParts[partIndex++] = columnDef.substr(start, end - start);
				start = end + 1;
				end = columnDef.find(',', start);
			}
			if (partIndex < 4) {
				columnParts[partIndex++] = columnDef.substr(start);
			}

			if (partIndex != 4) {
				cout << endl << "Invalid command format. Column definition must have 4 parameters.";
				return;
			}

			//trim each part
			for (int i = 0; i < 4; i++) {
				trim(columnParts[i]);
			}

			//extract column details
			string columnName = columnParts[0];
			string columnTypeStr = columnParts[1];
			int columnSize = stoi(columnParts[2]);
			string defaultValue = columnParts[3];

			ColumnType columnType;
			if (columnTypeStr == "INT") {
				columnType = INT;
			}
			else if (columnTypeStr == "TEXT") {
				columnType = TEXT;
			}
			else if (columnTypeStr == "FLOAT") {
				columnType = FLOAT;
			}
			else if (columnTypeStr == "BOOLEAN") {
				columnType = BOOLEAN;
			}
			else if (columnTypeStr == "DATE") {
				columnType = DATE;
			}
			else {
				cout << endl << "Invalid column type.";
				return;
			}

			Column newColumn(columnName, columnType, columnSize, defaultValue);

			db->alterTableAddColumn(tableName, newColumn);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandAlterTableDropColumn(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "ALTER TABLE "
			if (commandCopy.find("ALTER TABLE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " DROP COLUMN "
			size_t dropPos = commandCopy.find(" DROP COLUMN ");
			if (dropPos == string::npos) {
				cout << endl << "Invalid command format.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(12, dropPos - 12);  // 12 is the length of "ALTER TABLE "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//extract the column name
			string columnName = commandCopy.substr(dropPos + 13);  // 13 is the length of " DROP COLUMN "
			trim(columnName);

			if (columnName.empty()) {
				cout << endl << "Invalid command format. Column name cannot be empty.";
				return;
			}

			db->alterTableDeleteColumn(tableName, columnName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandCreateIndex(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "CREATE INDEX "
			if (commandCopy.find("CREATE INDEX ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " ON "
			size_t onPos = commandCopy.find(" ON ");
			if (onPos == string::npos) {
				cout << endl << "Invalid command format. Missing 'ON'.";
				return;
			}

			//ensure there is an index name between "CREATE INDEX " and " ON "
			if (onPos <= 13) {  // 13 is the length of "CREATE INDEX "
				cout << endl << "Invalid command format. Missing index name.";
				return;
			}

			//extract the index name
			string indexName = commandCopy.substr(13, onPos - 13);
			trim(indexName);

			if (indexName.empty()) {
				cout << endl << "Invalid command format. Index name cannot be empty.";
				return;
			}

			//extract the table name and column part
			string tableAndColumnPart = commandCopy.substr(onPos + 4);  // 4 is the length of " ON "
			trim(tableAndColumnPart);

			//find the position of the opening parenthesis '('
			size_t openParenPos = tableAndColumnPart.find('(');
			if (openParenPos == string::npos) {
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//table name
			string tableName = tableAndColumnPart.substr(0, openParenPos);
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//find the position of the closing parenthesis ')'
			size_t closeParenPos = tableAndColumnPart.find(')', openParenPos);
			if (closeParenPos == string::npos) {
				cout << endl << "Invalid command format. Missing closing parenthesis.";
				return;
			}

			//column name
			string columnName = tableAndColumnPart.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
			trim(columnName);

			if (columnName.empty()) {
				cout << endl << "Invalid command format. Column name cannot be empty.";
				return;
			}

			// check for extra arguments after the closing parenthesis
			if (closeParenPos + 1 < tableAndColumnPart.length()) {
				string extraArgs = tableAndColumnPart.substr(closeParenPos + 1);
				trim(extraArgs);
				if (!extraArgs.empty()) {
					cout << endl << "Invalid command format. Too many arguments.";
					return;
				}
			}

			db->createIndex(indexName, columnName, tableName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandDropIndex(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "DROP INDEX "
			if (commandCopy.find("DROP INDEX ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//extract the index name
			string indexName = commandCopy.substr(11);  // 11 is the length of "DROP INDEX "
			trim(indexName);

			if (indexName.empty()) {
				cout << endl << "Invalid command format. Index name cannot be empty.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = indexName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->dropIndex(indexName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandShowTables(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "SHOW TABLES"
			if (commandCopy != "SHOW TABLES") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->showTables();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandShowIndexFromTable(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "SHOW INDEX FROM "
			if (commandCopy.find("SHOW INDEX FROM ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(16);  // 16 is the length of "SHOW INDEX FROM "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = tableName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->showIndexFromTable(tableName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandShowIndexFromAll(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "SHOW INDEX FROM ALL"
			if (commandCopy != "SHOW INDEX FROM ALL") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->showIndexFromAll();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandClearConsole(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "clear"
			if (commandCopy != "clear") {
				cout << endl << "Invalid command format.";
				return;
			}

			system("cls");
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandHelpMenu(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "help"
			if (commandCopy != "help") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->printHelpMenu();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandSyntaxMenu(const string& command) {
		try {
			system("cls");
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "help 2"
			if (commandCopy != "help 2") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->printSyntaxMenu();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
public:
	//DEFAULT CONSTRUCTOR
	Commands() {
		this->db = nullptr;
	}
	//CONSTRUCTOR
	Commands(Database* database) : db(database) {}
};

class FileManager {
public:
	const static string TABLES_CONFIG_ADDRESS;
	const static string SELECT_COMMANDS_ADDRESS;
	const static int MAX_COMMANDS_FILES;
	const static string START_COMMANDS_ADDRESSES[];
public:
	void readStartCommandsFromFiles(const string* filenames, int count, Commands& commands) {
		for (int i = 0; i < count; i++) {
			if (filenames[i].empty()) {
				continue; //skip empty addresses
			}
			ifstream file(filenames[i]);
			if (!file.is_open()) {
				cout << endl << "Error: Could not open file " << filenames[i] << endl;
				continue; //skip to the next file if one can't be opened
			}
			string command;
			while (getline(file, command)) {
				if (!command.empty()) {
					//commands.handleCommand(command, FileManager::TABLES_CONFIG_ADDRESS, FileManager::SELECT_COMMANDS_ADDRESS);
				}
			}
			cout << endl;
			file.close();
		}
	}
};
const int FileManager::MAX_COMMANDS_FILES = 5;
const string FileManager::START_COMMANDS_ADDRESSES[FileManager::MAX_COMMANDS_FILES] = {
	"D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\start_commands\\commands1.txt",
	"D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\start_commands\\commands2.txt"
};

int main() {
	Database db;
	string selectCommandsAddress = "D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\select_commands\\";
	string tablesConfigAddress = "D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\tables_config\\";

	commandParser parser(selectCommandsAddress, tablesConfigAddress);
	string userCommand;

	cout << "Use the 'help' command to view available commands and their syntax." << endl;

	db.loadDatabase(tablesConfigAddress, selectCommandsAddress);

	//read commands from multiple files at the start
	//fm.readStartCommandsFromFiles(FileManager::START_COMMANDS_ADDRESSES, FileManager::MAX_COMMANDS_FILES, commands);

	//continue with console input
	while (true) {
		cout << endl << ">> ";
		getline(cin, userCommand);

		if (userCommand == "exit") {
			break;
		}

		try {
			Command* cmd = parser.handleCommand(userCommand);
			cmd->execute(db);
			delete cmd;
		}
		catch (const invalid_argument& e) {
			cout << e.what() << endl;
		}
		catch (const exception& e) {
			cout << e.what() << endl;
		}
		catch (...) {
			cout << "An error occurred." << endl;
		}
	}

	db.saveDatabase(tablesConfigAddress);

	return 0;
}