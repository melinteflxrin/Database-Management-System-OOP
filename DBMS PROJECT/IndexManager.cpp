#include "IndexManager.h"

IndexManager::IndexManager() : indexes(nullptr), noIndexes(0) {}

IndexManager::~IndexManager() {
	for (int i = 0; i < noIndexes; ++i) {
		delete indexes[i];
	}
	delete[] indexes;
}

void IndexManager::addValue(const string& indexName, const string& value, int position) {
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getIndexName() == indexName) {
			indexes[i]->addValue(value, position);
			return;
		}
	}
}

void IndexManager::removeValue(const string& indexName, const string& value, int position) {
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getIndexName() == indexName) {
			indexes[i]->removeValue(value, position);
			return;
		}
	}
}

void IndexManager::clear(const string& indexName) {
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getIndexName() == indexName) {
			indexes[i]->clear();
			return;
		}
	}
}

void IndexManager::createIndex(const std::string& indexName, const std::string& tableName, const std::string& columnName) {
	Index** newIndexes = new Index * [noIndexes + 1];
	for (int i = 0; i < noIndexes; ++i) {
		newIndexes[i] = indexes[i];
	}
	newIndexes[noIndexes] = new Index(indexName, tableName, columnName);
	delete[] indexes;
	indexes = newIndexes;
	noIndexes++;
}

void IndexManager::deleteIndex(const std::string& indexName) {
	int indexToDelete = -1;
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getIndexName() == indexName) {
			indexToDelete = i;
			break;
		}
	}
	if (indexToDelete == -1) {
		std::cout << "Index: " << indexName << " not found.\n";
		return;
	}

	Index** newIndexes = new Index * [noIndexes - 1];
	for (int i = 0, j = 0; i < noIndexes; ++i) {
		if (i != indexToDelete) {
			newIndexes[j++] = indexes[i];
		}
		else {
			delete indexes[i];
		}
	}
	delete[] indexes;
	indexes = newIndexes;
	noIndexes--;
}

bool IndexManager::addValueToIndex(const string& indexName, const string& value, int position) {
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getIndexName() == indexName) {
			//directly add the value and position to the index
			indexes[i]->addValue(value, position);
			return true; // successfully added
		}
	}

	// if index not found
	cerr << "Error: Index '" << indexName << "' not found." << endl;
	return false;
}

Index IndexManager::getIndex(const string& indexName) const {
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getIndexName() == indexName) {
			return *indexes[i]; // return a copy of the Index object
		}
	}
	throw runtime_error("Index not found");
}

Index IndexManager::getIndexByColumnAndTable(const std::string& columnName, const std::string& tableName) const {
	for (int i = 0; i < noIndexes; ++i) {
		if (indexes[i]->getColumnName() == columnName && indexes[i]->getTableName() == tableName) {
			return *indexes[i];  //return a copy
		}
	}
	throw std::runtime_error("Index not found");
}

//getter function to access a copy of the indexes array
Index** IndexManager::getIndexes() const {
	if (noIndexes == 0) {
		return nullptr;
	}

	//create a new array to hold the copies of the indexes
	Index** indexesCopy = new Index * [noIndexes];
	for (int i = 0; i < noIndexes; ++i) {
		indexesCopy[i] = new Index(*indexes[i]); //use copy constructor
	}

	return indexesCopy;
}

bool IndexManager::indexExistsByIndexName(const std::string& indexName) const {
	try {
		getIndex(indexName);
		return true;
	}
	catch (const std::runtime_error&) {
		return false;
	}
}

void IndexManager::displayAllIndexes() const {
	for (int i = 0; i < noIndexes; ++i) {
		cout << " '" << indexes[i]->getIndexName() << "' created on: " << indexes[i]->getTableName() << " (" << indexes[i]->getColumnName() << ")\n";
	}
}

int IndexManager::getNoIndexes() const {
	return noIndexes;
}