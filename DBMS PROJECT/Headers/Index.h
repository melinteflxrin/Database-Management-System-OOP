#ifndef INDEX_H
#define INDEX_H

#include <iostream>
#include <string>

using namespace std;

class Index {
private:
	struct Node {             //node for linked list to store positions
		int position;         //position of the value in the column
		Node* next;
		Node(int pos);
	};

	struct ValueNode { 	  //node for linked list to store values
		string value;
		Node* positions;
		ValueNode* next;
		ValueNode(const string& val);
	};

	string indexName;
	string tableName;
	string columnName;
	ValueNode* head;    //head of the linked list for values

public:
	//internal Iterator class for safe access to ValueNode
	class Iterator {
	private:
		ValueNode* current;
	public:
		Iterator(ValueNode* start);
		bool hasNext() const;
		void next();
		string currentValue() const;
		int* currentPositions(int& count) const;
	};

	Index();
	Index(const string& indexName, const string& tableName, const string& columnName);
	Index(const Index& other);
	~Index();

	void addValue(const string& value, int position);
	void removeValue(const string& value, int position);
	int getRowCount(const string& value) const;
	void getKeys(const string& value, int* rows, int& rowCount) const;
	void findValue(const string& value) const;
	void displayIndex() const;
	void reverseListInPlace();
	string getIndexName() const;
	string getColumnName() const;
	string getTableName() const;
	ValueNode* getHead() const;
	Iterator createIterator();
	void clear();

private:
	//helper function for copy constructor
	Node* copyNodeList(Node* node);
};

#endif // INDEX_H