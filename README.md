# DBMS - My SQLite Engine
## Table of contents
1. [Description](#description)<br/>
2. [Commands](#commands)<br/>
3. [File Management](#file-management)<br/>
4. [Demo Video](#demo-video)<br/>
## Description
A lightweight implementation of a database management system, similar to SQLite.<br/>

This project was part of an optional OOP class assignment designed to gain practical experience and see how different classes are combined together to develop a “real” software product, 
using only standard C++ libraries, ***without STL***.<br/>

The application handles basic SQL commands, enables faster lookups using indexes, manages files for data import, stores table structures to maintain the database state, and more.<br/>

## Commands
There are 20 available commands, including a help menu and a syntax menu that provide descriptions and syntax details for all commands.<br/>
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
  Displaying the table structure:
  ```
  DESCRIBE Products
  ```
  ![image](https://github.com/user-attachments/assets/de324f7b-1774-4de6-b8a7-4c1d64c95603)<br/>
  
  Dropping a table (also removes all the indexes associated with that table):
  ```
  DROP TABLE Products
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
    Shows all indexes from a table, and all indexes from all tables, respectively.
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
  
- ### Deleting rows:
  Deleting rows from a table will also update the indexes from all columns which have an index created on them.
  It also uses the WHERE condition to search for data quicker using the index (if the column has an index).
  ```
  DELETE FROM Products WHERE Name=TV
  ```
  ![image](https://github.com/user-attachments/assets/d551a7c9-f9e1-4466-937b-4773020e37cb)

## File Management:
  - ### Start commands:
    The application can receive one or more input text files through the arguments of the main function.<br/>
    For example, if the application is called *DBMS PROJECT.exe*, it can be launched with a command like the following:
    ```
    "DBMS PROJECT.exe" commands1.txt commands2.txt
    ```
    If the text files containing the start commands are not in the same directory as the application, the full path to each file is needed as an argument.<br/>
    Can also be started without recieving any arguments.
    
  - ### Table configuration files:
    All table details such as table structure, data, index information, are stored in configuration binary files. Each table in the database has its own associated binary file, with the same name as the table.<br/>
    The configuration files are used by the application at startup to load the database and all table information.<br/>
    
    Upon exiting the application, all modifications made to the tables during the session are saved, and the binary files are updated.

  - ### Select commands:
    The execution of *SELECT* commands automatically generates text files containing the results of the commands.<br/>
    The filenames are generated automatically, following this format: *SELECT_1.txt, SELECT_2.txt, etc.*

  - ### Importing data from CSV files:
    The application allows the upload of data from CSV files, similar to *INSERT* commands.<br/>
    The delimiter symbol (e.g.```,``` , ```#```, or others) is chosen by the user.<br/>

    Importing CSV files can be done like this:
    ```
    IMPORT INTO Products file_name.csv
    ```
    
  - ### Settings file:
    The program starts by reading configuration details from a ```settings.txt``` file located in the same directory as the application.<br/>

    The file contains the following information (must be in this order to be read correctly):
    - The address of the folder containing the tables' configuration files
    - The address for the output of *SELECT* commands
    - The address of the folder containing *CSV* files that can be imported
    - The delimiter for the *CSV* files

## Demo Video 
Here is a quick video showcasing how commands work and how the program manages files.<br/>
  - https://www.youtube.com/watch?v=-Q0qjhDrciE

