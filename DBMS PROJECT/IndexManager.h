#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H

#include <string>
#include <iostream>
#include "Index.h"

class IndexManager {
private:
	Index** indexes;
	int noIndexes;

public:
	IndexManager();
	~IndexManager();

	void addValue(const string& indexName, const string& value, int position);
	void removeValue(const string& indexName, const string& value, int position);
	void clear(const string& indexName);
	void createIndex(const std::string& indexName, const std::string& tableName, const std::string& columnName);
	void deleteIndex(const std::string& indexName);
	bool addValueToIndex(const std::string& indexName, const std::string& value, int position);
	Index getIndex(const std::string& indexName) const;
	Index getIndexByColumnAndTable(const std::string& columnName, const std::string& tableName) const;
	Index** getIndexes() const;
	bool indexExistsByIndexName(const std::string& indexName) const;
	void displayAllIndexes() const;
	int getNoIndexes() const;
};

#endif // INDEXMANAGER_H