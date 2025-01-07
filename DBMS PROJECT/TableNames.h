#ifndef TABLENAMES_H
#define TABLENAMES_H

#include <string>

using namespace std;

class TableNames {            //to check if a table name already exists
private:
	string* names;
	int noNames;
public:
	//DESTRUCTOR
	~TableNames();
	//DEFAULT CONSTRUCTOR
	TableNames();
	//ASSIGNMENT OPERATOR
	TableNames& operator=(const TableNames& original);
	//COPY CONSTRUCTOR (for somewhere in the assignment operator for table)
	TableNames(const TableNames& original);

	void addName(const string& name);
	void removeName(const string& name);

	bool nameExists(const string& name) const;
	int getSize() const;
};

#endif
