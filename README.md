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
5. DELETE FROM                   
6. UPDATE TABLE                  
7. ALTER ADD                    
8. ALTER DROP                   
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








- ### Creating a table & inserting data:
  Eac
  ```
  CREATE TABLE Products ((ID, INT, 5, 0, UNIQUE),(Name, TEXT, 20, unknown), (Price, FLOAT, 10, 0.0))
  ```
  

