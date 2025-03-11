#include "../Headers/Index.h"

//node constructor
Index::Node::Node(int pos) : position(pos), next(nullptr) {}

// ValueNode constructor
Index::ValueNode::ValueNode(const string& val) : value(val), positions(nullptr), next(nullptr) {}

// iterator constructor
Index::Iterator::Iterator(ValueNode* start) : current(start) {}

bool Index::Iterator::hasNext() const {
	return current != nullptr;
}

void Index::Iterator::next() {
	if (hasNext()) {
		current = current->next;
	}
}

string Index::Iterator::currentValue() const {
	return current ? current->value : "";
}

//get the positions for the current ValueNode
//return number of positions and an array of position values
int* Index::Iterator::currentPositions(int& count) const {
	if (current == nullptr) {
		count = 0;
		return nullptr;
	}

	Node* pos = current->positions;
	count = 0;
	Node* tempPos = pos;
	while (tempPos) {
		count++;
		tempPos = tempPos->next;
	}

	int* positions = new int[count];
	tempPos = pos;
	for (int i = 0; i < count; ++i) {
		positions[i] = tempPos->position;
		tempPos = tempPos->next;
	}
	return positions;
}

//index constructors
Index::Index() : head(nullptr) {}

Index::Index(const string& indexName, const string& tableName, const string& columnName)
	: indexName(indexName), tableName(tableName), columnName(columnName), head(nullptr) {}

Index::Index(const Index& other) : indexName(other.indexName), tableName(other.tableName), columnName(other.columnName), head(nullptr) {
	if (other.head) {
		head = new ValueNode(other.head->value);
		head->positions = copyNodeList(other.head->positions);

		ValueNode* current = head;
		ValueNode* otherCurrent = other.head->next;
		while (otherCurrent) {
			current->next = new ValueNode(otherCurrent->value);
			current->next->positions = copyNodeList(otherCurrent->positions);
			current = current->next;
			otherCurrent = otherCurrent->next;
		}
	}
}

// index destructor
Index::~Index() {
	clear();
}

void Index::addValue(const string& value, int position) {
	ValueNode* current = head;
	while (current) {
		if (current->value == value) {
			// check if the position already exists
			Node* pos = current->positions;
			while (pos) {
				if (pos->position == position) {
					// position already exists; do nothing
					cout << "Duplicate position ignored for value: " << value
						<< " at position: " << position << endl;
					return;
				}
				pos = pos->next;
			}
			// position does not exist; add it to the list
			Node* newPos = new Node(position);
			// find the last position and append the new position to it
			if (current->positions == nullptr) {
				current->positions = newPos; // if no positions, set as first
			}
			else {
				pos = current->positions;
				while (pos->next) {
					pos = pos->next;
				}
				pos->next = newPos; // append at the end
			}
			cout << "Added position: " << position << " for value: " << value << endl;
			return;
		}
		current = current->next;
	}

	//if value not found, add a new ValueNode
	ValueNode* newValue = new ValueNode(value);
	newValue->positions = new Node(position);  // First position for new value
	newValue->next = head;
	head = newValue;
	cout << "Added new value: " << value << " at position: " << position << endl;
}

void Index::removeValue(const string& value, int position) {
	ValueNode* current = head;
	ValueNode* prev = nullptr;

	while (current) {
		if (current->value == value) {
			Node* pos = current->positions;
			Node* posPrev = nullptr;

			while (pos) {
				if (pos->position == position) {
					if (posPrev) {
						posPrev->next = pos->next;
					}
					else {
						current->positions = pos->next;
					}
					delete pos;

					//if no more positions, remove the value node
					if (!current->positions) {
						if (prev) {
							prev->next = current->next;
						}
						else {
							head = current->next;
						}
						delete current;
					}
					return;
				}
				posPrev = pos;
				pos = pos->next;
			}
		}
		prev = current;
		current = current->next;
	}
}

int Index::getRowCount(const string& value) const {
	ValueNode* current = head;
	while (current) {
		if (current->value == value) {
			int count = 0;
			Node* pos = current->positions;
			while (pos) {
				count++;
				pos = pos->next;
			}
			return count;
		}
		current = current->next;
	}
	return 0;
}

void Index::getKeys(const string& value, int* rows, int& rowCount) const {
	rowCount = 0;
	ValueNode* current = head;
	while (current) {
		if (current->value == value) {
			Node* pos = current->positions;
			while (pos) {
				rows[rowCount++] = pos->position;
				pos = pos->next;
			}
			return;
		}
		current = current->next;
	}
}

void Index::findValue(const string& value) const {
	ValueNode* current = head;
	while (current) {
		if (current->value == value) {
			cout << "Value: " << value << " found at positions: ";
			Node* pos = current->positions;
			while (pos) {
				cout << pos->position << " ";
				pos = pos->next;
			}
			cout << endl;
			return;
		}
		current = current->next;
	}
	cout << "Value: " << value << " not found in the index.\n";
}

void Index::displayIndex() const {
	ValueNode* current = head;
	while (current) {
		cout << "Value: " << current->value << " -> Positions: ";
		Node* pos = current->positions;
		while (pos) {
			cout << pos->position << " ";
			pos = pos->next;
		}
		cout << std::endl;
		current = current->next;
	}
}

void Index::reverseListInPlace() {
	ValueNode* current = head;
	ValueNode* prev = nullptr;
	ValueNode* next = nullptr;

	while (current) {
		next = current->next;
		current->next = prev;
		prev = current;
		current = next;
	}

	head = prev;
}

string Index::getIndexName() const {
	return indexName;
}

string Index::getColumnName() const {
	return columnName;
}

string Index::getTableName() const {
	return tableName;
}

Index::ValueNode* Index::getHead() const {
	if (!head) return nullptr;

	Index::ValueNode* newHead = new Index::ValueNode(head->value);

	Index::Node* posCurrent = head->positions;
	Index::Node* newPositionsHead = nullptr;
	Index::Node* newPositionsCurrent = nullptr;
	while (posCurrent) {
		Index::Node* newPos = new Index::Node(posCurrent->position);
		if (!newPositionsHead) {
			newPositionsHead = newPos;
		}
		else {
			newPositionsCurrent->next = newPos;
		}
		newPositionsCurrent = newPos;
		posCurrent = posCurrent->next;
	}

	newHead->positions = newPositionsHead;

	Index::ValueNode* current = head->next;
	Index::ValueNode* newCurrent = newHead;
	while (current) {
		Index::ValueNode* newNode = new Index::ValueNode(current->value);

		posCurrent = current->positions;
		newPositionsHead = nullptr;
		newPositionsCurrent = nullptr;
		while (posCurrent) {
			Index::Node* newPos = new Index::Node(posCurrent->position);
			if (!newPositionsHead) {
				newPositionsHead = newPos;
			}
			else {
				newPositionsCurrent->next = newPos;
			}
			newPositionsCurrent = newPos;
			posCurrent = posCurrent->next;
		}

		newNode->positions = newPositionsHead;

		newCurrent->next = newNode;
		newCurrent = newNode;

		current = current->next;
	}

	return newHead;
}

Index::Iterator Index::createIterator() {
	return Iterator(head);
}

void Index::clear() {
	while (head) {
		ValueNode* temp = head;
		head = head->next;

		Node* pos = temp->positions;
		while (pos) {
			Node* tempPos = pos;
			pos = pos->next;
			delete tempPos;
		}
		delete temp;
	}
}

Index::Node* Index::copyNodeList(Node* node) {
	if (!node) return nullptr;
	Node* newHead = new Node(node->position);
	Node* current = newHead;
	Node* otherCurrent = node->next;
	while (otherCurrent) {
		current->next = new Node(otherCurrent->position);
		current = current->next;
		otherCurrent = otherCurrent->next;
	}
	return newHead;
}