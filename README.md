# DBMS - My SQLite Engine
## Table of contents
1.[Description](#description)<br/>
2.[Commands](#commands)<br/>
## Description
A lightweight implementation of a database management system, similar to SQLite.<br/>

This project was part of an optional OOP class assignment designed to gain practical experience and see how different classes are combined together to develop a “real” software product.<br/>

The application handles basic SQL commands, enables faster lookups using indexes, manages files for data import, stores table structures to maintain the database state, and more.<br/>

## Commands
There are 20 avaliable commands, including a help menu and a sytanx menu that provide descriptions and syntax details for all commands.<br/>
They allow the user to create and manage tables, insert and update data, create indexes for faster lookups, retrieve data with select commands, and more.<br/>
```
  Basic Operations:
1. CREATE TABLE                 
2. DESCRIBE TABLE                
3. DROP TABLE                   

  Data Manipulation:
4. INSERT                       
5. DELETE WHERE                  
6. UPDATE TABLE                  
7. ALTER TABLE ADD COLUMN                 
8. ALTER TABLE DROP COLUMN                   
9. IMPORT                         

  Index Operations:
10. CREATE INDEX                 
11. DROP INDEX                 

  Select Commands:
12. SELECT ALL                    
13. SELECT ALL WHERE              
14. SELECT COLUMNS                
15. SELECT WHERE                  

  Database Information:
16. SHOW TABLES                   
17. SHOW INDEX FROM TABLE         
18. SHOW INDEX FROM ALL          

  Utility Commands:
19. HELP MENU (help)            
20. SYNTAX MENU (help 2)          
21. clear                        
22. exit                          
```
- ### Creating a table:
  Each column has the following attributes: column_name, type (INT, TEXT or FLOAT), size, default_value, and an optional UNIQUE constraint that ensures all values in the column are different.
  ```
  CREATE TABLE Products ((ID, INT, 5, 0, UNIQUE),(Name, TEXT, 20, unknown), (Price, FLOAT, 10, 0.0))
  ```
- ### Inserting data:
  Values from insert commands are separated by , and they have the same number and order as the table definition.<br/>
  When inserting new data, index information is also updated on all columns from the table that have an index created on them.
  ```
  INSERT INTO Products VALUES (123, Laptop, 3799.99)
  INSERT INTO Products VALUES (124, TV, 1599.99)
  INSERT INTO Products VALUES (125, Laptop, 2850.99)
  ```
- ### Indexes:
  #### Creating an index on a column:
  ```
  CREATE INDEX nameIndex ON Products (Name)
  ```
    #### Index information:
  ```
  SHOW INDEX FROM Products
  SHOW INDEX FROM ALL
  ```
  ![image](https://github.com/user-attachments/assets/e2ba8bc5-2ba6-4a72-aff1-491b3edd1177)<br/>
   #### Dropping an index:
  ```
  DROP INDEX nameIndex
  ```

- ### Select commands:
  #### Searching *without* an index:
  ```
  SELECT ALL FROM Products
  SELECT ID,Name FROM Products
  SELECT ALL FROM Products WHERE ID=123
  ```
  ![image](https://github.com/user-attachments/assets/d381a9b0-a088-48a3-b744-73863a176c01)<br/>
  #### Searching *with* an index (from column 'Name'):
  ```
  SELECT ALL FROM Products WHERE Name=Laptop
  ```
  
  ![image](https://github.com/user-attachments/assets/7dcd3155-533d-485d-881e-ec28585e3e98)<br/>

- ### Update & Alter:
  Updating data on a column that has an index created on it will also update the index on that column.
  ```
  UPDATE Products SET Name=Phone WHERE ID=125
  ```
  ![image](https://github.com/user-attachments/assets/f39b6596-3b3c-4efb-95b1-0dcec788308a)<br/>
  Adding a new column to a table will initialize the existing rows with the new column's default value.
  ```
  ALTER TABLE Products ADD (Weight,FLOAT,5,0)
  ``` 
  ![image](https://github.com/user-attachments/assets/2359194a-50e5-4547-88f2-31f7bc94cef4)<br/>
  Dropping a column:
  ```
  ALTER TABLE Products DROP COLUMN Weight
  ```
  

  



- ## Files:
  import, select com, tables config, start com, delimiter, settings
  
  

  
  

