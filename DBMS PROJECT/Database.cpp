#include "Database.h"

//DEFAULT CONSTRUCTOR
Database::Database() {
	this->database = nullptr;
	this->noTables = 0;
	this->tableNames = nullptr;
	this->tableNames = new TableNames();
}
//DESTRUCTOR
Database::~Database() {
	for (int i = 0; i < noTables; i++) {
		delete database[i];
	}
	delete[] database;
	delete tableNames;  //i need to delete because i used 'new' in the constructor
}
//GETTERS
int Database::getTableIndex(const string& name) const {
	for (int i = 0; i < noTables; i++) {
		if (database[i]->getName() == name) {
			return i;
		}
	}
	return -1;
}
Table Database::getTableByName(const string& name) const {
	for (int i = 0; i < noTables; i++) {
		if (database[i]->getName() == name) {
			return *database[i]; //return a copy of the Table
		}
	}
	throw runtime_error("Table not found");
}
Index Database::getIndexByColumnAndTable(const string& columnName, const string& tableName) {
	try {
		return indexManager.getIndexByColumnAndTable(columnName, tableName);
	}
	catch (const std::runtime_error& e) {
		throw;
	}
}
//EXECUTE COMMANDS
void Database::addTableToDatabase(const Table& table) {
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
bool Database::tableExists(const string& name) const {
	if (tableNames->nameExists(name)) {
		return true;
	}
	return false;
}
void Database::removeTable(int index) {
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
void Database::dropTable(const string& name, const string& tablesConfigAddress) {
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
void Database::insertValues(const string& name, const string* values, int noValues) {
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
void Database::createIndex(const string& indexName, const string& tableName, const string& columnName) {
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
void Database::deleteRowFromTable(const string& tableName, const string& columnName, const string& value) {
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
			indexManager.clear(indexManager.getIndexByColumnAndTable(colName, tableName).getIndexName()); // Clear the existing index

			// Recreate the index
			for (int rowIndex = 0; rowIndex < table->getNoRows(); rowIndex++) {
				indexManager.addValue(indexManager.getIndexByColumnAndTable(colName, tableName).getIndexName(), table->getRow(rowIndex).getTextData(colIndex), rowIndex);
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
void Database::updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
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
void Database::alterTableAddColumn(const string& tableName, const Column& newColumn) {
	if (!tableExists(tableName)) {
		cout << endl << "Error: Table '" << tableName << "' does not exist.";
		return;
	}

	int tableIndex = getTableIndex(tableName);
	Table* table = database[tableIndex];

	table->addColumn(newColumn);

	cout << endl << "Column '" << newColumn.getName() << "' added to table '" << tableName << "' successfully.";
}
void Database::alterTableDeleteColumn(const string& tableName, const string& columnName) {
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
void Database::dropIndex(const string& indexName) {
	if (!indexManager.indexExistsByIndexName(indexName)) {
		cout << endl << "Error: Index with name '" << indexName << "' does not exist.";
		return;
	}

	indexManager.deleteIndex(indexName);
	cout << endl << "Index with name '" << indexName << "' removed successfully.";
}
void Database::importCSV(const string& tableName, const string& csvDirectory, const string& fileName, char delimiter) {
	// Construct the full file path
	string fullPath = csvDirectory + "/" + fileName;

	// Open the CSV file
	std::ifstream csvFile(fullPath);
	if (!csvFile.is_open()) {
		cout << "Error: Could not open file '" << fullPath << "'." << endl;
		return;
	}

	string line;
	int lineNumber = 0;

	// Read file line by line
	while (getline(csvFile, line)) {
		lineNumber++;

		// Trim leading and trailing whitespace from the line
		size_t start = line.find_first_not_of(" \t\n\r");
		size_t end = line.find_last_not_of(" \t\n\r");
		if (start == string::npos || end == string::npos) {
			continue; // Skip empty or whitespace-only lines
		}
		line = line.substr(start, end - start + 1);

		// Count the number of values in the line (based on the delimiter)
		int noValues = 1; // At least one value per line
		for (size_t i = 0; i < line.length(); i++) {
			if (line[i] == delimiter) {
				noValues++;
			}
		}

		// Create an array to store the parsed values
		string* valueArray = new string[noValues];
		int valueIndex = 0;
		size_t prevPos = 0;

		// Split the line into values based on the delimiter
		for (size_t i = 0; i <= line.length(); i++) {
			if (i == line.length() || line[i] == delimiter) {
				// Extract the substring for this value
				string value = line.substr(prevPos, i - prevPos);

				// Trim whitespace from the value
				size_t valueStart = value.find_first_not_of(" \t\n\r");
				size_t valueEnd = value.find_last_not_of(" \t\n\r");
				if (valueStart != string::npos && valueEnd != string::npos) {
					value = value.substr(valueStart, valueEnd - valueStart + 1);
				}
				else {
					value = ""; // Empty value if only whitespace
				}

				// Store the value in the array
				valueArray[valueIndex++] = value;

				// Update the start position for the next value
				prevPos = i + 1;
			}
		}

		// Attempt to insert the row into the table
		try {
			insertValues(tableName, valueArray, noValues);
		}
		catch (const std::exception& e) {
			cout << "Error on line " << lineNumber << ": " << e.what() << endl;
		}

		// Clean up dynamic memory
		delete[] valueArray;
	}

	// Close the file
	csvFile.close();
}
void Database::showTables() const {
	if (noTables == 0) {
		cout << "No tables found.";
		return;
	}

	cout << "My Tables:";
	for (int i = 0; i < noTables; i++) {
		cout << endl << i + 1 << ". " << database[i]->getName();
	}
}
void Database::showIndexFromTable(const string& tableName) const {
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
void Database::showIndexFromAll() const {
	indexManager.displayAllIndexes();
}
//SAVE & LOAD DATABASE
void Database::saveDatabase(const string& tablesConfigAddress) const {
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
void Database::loadDatabase(const string& tablesConfigAddress, const string& selectCommandsAddress) {
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
				//cout << "Loaded Index Name: " << indexName << endl;

				int tableNameLength;
				inFile.read(reinterpret_cast<char*>(&tableNameLength), sizeof(tableNameLength));
				string indexTableName(tableNameLength, ' ');
				inFile.read(&indexTableName[0], tableNameLength);
				//cout << "Loaded Table Name: " << indexTableName << endl;

				int columnNameLength;
				inFile.read(reinterpret_cast<char*>(&columnNameLength), sizeof(columnNameLength));
				string columnName(columnNameLength, ' ');
				inFile.read(&columnName[0], columnNameLength);
				//cout << "Loaded Column Name: " << columnName << endl;

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
					//cout << "Loaded Value: " << value << endl;

					//position count
					int positionCount;
					inFile.read(reinterpret_cast<char*>(&positionCount), sizeof(positionCount));
					if (inFile.fail()) break;
					//cout << "Loaded Position Count: " << positionCount << endl;

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
						//cout << "Loaded Position: " << position << endl;

						if (!indexManager.addValueToIndex(indexName, value, position)) {
							cerr << "Error: Could not add position to index." << endl;
							break;
						}
					}

					//check for the end marker to check if we reached the end of this index
					int endIndexMarker;
					inFile.read(reinterpret_cast<char*>(&endIndexMarker), sizeof(endIndexMarker));
					if (endIndexMarker == -1) {
						//cout << "End of Index Marker Found for: " << indexName << endl;
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