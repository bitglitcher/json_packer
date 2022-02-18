#ifndef __RECORD_H__
#define __RECORD_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

//Number of initial entries when initializing
#define INIT_ENTRIES 256

//The key field is part of the data on the TLV encoding.
//TLV entry   / Data.......
//+-----+-----+-----+ DATA
//| TAG | LEN | KEY | ...
//+-----+-----+-----+

//TLV TAGS
#define TAG_BOOLEAN 0x00
#define TAG_STRING  0x01
#define TAG_INT     0x02

typedef struct tlv_data_struct
{
    int size;
    char* data;
    struct tlv_data_struct* next;
} tlv_data_t;

typedef struct
{
    size_t size;
    char* buffer;
} serialized_t;

typedef enum
{
    entry_string,
    entry_value,
    entry_boolean
} record_entry_type;


typedef struct {
  int key;
  int type;
  union Data
  {
      char* string;
      int value;
      bool boolean;
  } data;
} record_entry_t;

typedef struct
{
    size_t allocated; //Allocated memory
    size_t n_entries; //entries used
    record_entry_t* entries;
} record_t;

typedef struct
{
    size_t allocated; //Allocated memory
    size_t n_entries; //entries used
    record_t** records;
} book_t;

//Initializes the memory and returns a pointer
record_t* initialize_record();
//Initializes the memory and returns a pointer
book_t* initialize_book();
//Appends a new entry to a record and allocates the required memory, return false if failed.
bool record_append_entry(record_t* record, record_entry_t entry);
//Appends a new entry to a book and allocates the required memory, return false if failed.
bool book_append_entry(book_t* book, record_t* record);
//Free book
void free_book(book_t* book);
//Free record
void free_record(record_t* record);
//Encode record to tlv. Returns buffer with tlv data.
tlv_data_t* encode_record_to_tlv(record_t* record);
//Serialize book to tlv encoding. Returns buffer with tlv data.
serialized_t serialize_book_to_tlv(book_t* book);
//Create tlv entry
tlv_data_t tlv_encode(uint8_t tag, uint8_t len, char* data, int key);
//tlv struct append new entry. Return NULL if not succesful.
//Otherwise it returns the head of the linked list.
tlv_data_t* tlv_data_append(tlv_data_t* head, tlv_data_t tlv_data);
//Concatenates 2 linked lists and returns the resultant list.
tlv_data_t* tlv_data_list_append(tlv_data_t* head, tlv_data_t* list);
//tlv struct delete. Returns true on success
void tlv_data_delete(tlv_data_t* head);
//Returns the size of the linked list
size_t tlv_data_get_size(tlv_data_t* head);
//Print tlv linked list
void print_tlv_data_list(tlv_data_t* head);
//Print entries on a record
void print_record(record_t* record);

#endif