#ifndef TABLE_H
#define TABLE_H

#include "Column.h"
#include "Row.h"
#include <string>

class Table {
private:
	string name;
	Column** columns;                 // array of pointers to Column objects so i can use their constructors because i kept getting the no default constructor error
	int noColumns;                    //cant use the actual column because i dont have a column constructor inside the table class; thats why Column**
	Row** rows;
	int noRows;

	string getColumnTypeName(ColumnType type) const;
public:
	//DEFAULT CONSTRUCTOR
	Table();
	//CONSTRUCTOR
	Table(const string& name, const Column* columns, int noColumns);
	//COPY CONSTRUCTOR
	Table(const Table& original);
	//ASSIGNMENT OPERATOR  - FOR WHEN I ASSIGN AN OBJECT TO ANOTHER (=)   // table1 = table2;
	Table& operator=(const Table& copy);
	//DESTRUCTOR
	~Table();

	//SETTERS
	void setName(const string& name);
	void setColumns(const Column* columns, int noColumns);

	//GETTERS
	const string& getName() const;
	int getNoColumns() const;
	int getNoRows() const;
	const Column& getColumn(int index) const;
	const Column& getColumn(const string& name) const;
	const ColumnType getColumnType(int index) const;
	const int getColumnSize(int index) const;
	int getColumnIndex(const string& name) const;
	Row& getRow(int index) const;

	//ROWS
	void deleteRow(int index);
	void addRow(const string* values);
	void addRowWithoutPrintMessage(const string* values);

	//COLUMNS
	bool columnExists(const string& name) const;
	bool columnExistsByIndex(int index) const;
	void deleteColumn(const string& name);
	void deleteColumnByIndex(int index);
	void addColumn(const Column& newColumn);

	//DISPLAY
	void describeTable() const;
	void displayTable() const;
};

#endif