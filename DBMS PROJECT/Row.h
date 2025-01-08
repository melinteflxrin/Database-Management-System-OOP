#ifndef ROW_H
#define ROW_H

#include <string>

using namespace std;

class Row {
private:
	string* data;
	int noColumns;

public:
	//DESTRUCTOR
	~Row();
	//DEFAULT CONSTRUCTOR
	Row();
	//CONSTRUCTOR
	Row(int noColumns);
	//COPY CONSTRUCTOR
	Row(const Row& original);
	//ASSIGNMENT OPERATOR
	Row& operator=(const Row& original);

	//SETTERS
	void setStringData(int columnIndex, const string& value);
	void setIntData(int columnIndex, const string& value);
	void setFloatData(int columnIndex, const string& value);

	//GETTERS
	int getIntData(int columnIndex) const;
	string getTextData(int columnIndex) const;
	float getFloatData(int columnIndex) const;
};

#endif
