# json_packer
The task is to read data in JSON format and using dictionary encoding write a compressed stream to disk.

The program reads json records in the following format are parsed and then the key is hashed to identify them.
```
{"key":"string", "key":66, "key":true}
```
The parsed data encoded into TLV and stored into a file called `output.bin`.

Data is stored in the following format.
```
+-----+-----+-----+------+
| TAG | LEN | KEY | DATA ...
+-----+-----+-----+------+
```
Tag is data type identifier.
Len encodes the lenght of the data. Up to 256 bytes.
Key encodes the hashed key from the json record.
Data is a variable lenght field to store data.

The JSON parser libray that is used in this project.
https://github.com/json-parser/json-parser

## Compile
Compile the code in GNU/Linux with the following comand.
```
make
````

### Generate Data
To generate data for the program to ingest run the python script called `gen_input.py`
```
python3 gen_input.py
```
By default the script generates 256 record entries. 
```
gen_json(256)
```

### Check for memory leaks or memory related issues.
```
valgrind --tool=memcheck --leak-check=full ./json_packer input.json
```
Function which are shown as `definitely lost in loss` are shown like that because they return pointers. Those pacticular pointers are later freed on the main function.