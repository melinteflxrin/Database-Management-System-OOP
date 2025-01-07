#ifndef COLUMN_H
#define COLUMN_H

#include <string>

using namespace std;

enum ColumnType { INT, TEXT, FLOAT, BOOLEAN, DATE };

class Column {
private:
	string name;
	ColumnType type;
	int size;
	string defaultValue;
	bool unique;

	bool isValidInt(const string& value);
	bool isValidFloat(const string& value);

public:
	//SETTERS
	void setName(const string& name);
	void setType(ColumnType type);
	void setSize(int size);
	void setDefaultValue(const string& defaultValue);

	//GETTERS
	const string& getName() const;
	ColumnType getType() const;
	int getSize() const;
	const string& getDefaultValue() const;
	const bool isUnique() const;

	//DEFAULT CONSTRUCTOR
	Column();
	//CONSTRUCTOR
	Column(const string& name, ColumnType type, int size, const string& defaultValue);
	//CONSTRUCTOR FOR UNIQUE
	Column(const string& name, ColumnType type, int size, const string& defaultValue, bool unique);
};

#endif