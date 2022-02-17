#include "record.h"

//Initializes the memory and returns a pointer
record_t* initialize_record()
{
    //Allocate record
    record_t* record = NULL;
    record = malloc(sizeof(record));
    if(!record)
    {
        //Will return NULL because allocation failed
        return record;
    }

    //Allocate memory for the entries
    record->entries = calloc(sizeof(record_append_entry), INIT_ENTRIES);
    record->n_entries = 0;
    record->allocated = INIT_ENTRIES;

    if(!record->entries)
    {
        //Free record
        free(record);
        //Return NULL because entries couldn't be allocated
        return NULL;
    }
    else
    {
        //Everything was allocated successfully
        return record;
    }
}
//Initializes the memory and returns a pointer
book_t* initialize_book()
{
    //Allocate book
    book_t* book = NULL;
    book = malloc(sizeof(book));
    if(!book)
    {
        //Will return NULL because allocation failed
        return book;
    }

    //Allocate memory for the entries
    book->records = calloc(sizeof(record_t), INIT_ENTRIES);
    book->n_entries = 0;
    book->allocated = INIT_ENTRIES;

    if(!book->records)
    {
        //Free book
        free(book);
        //Return NULL because records couldn't be allocated
        return NULL;
    }
    else
    {
        //Everything was allocated successfully
        return book;
    }
}
//Appends a new entry to a record and allocates the required memory, return false if failed.
bool record_append_entry(record_t* record, record_entry_t entry)
{
    if(record)
    {
        //Check if there is enough space allocated
        if(record->allocated > record->n_entries)
        {
            //Append data
            memcpy(&record->entries[record->n_entries], &entry, sizeof(record_entry_t));
            record->n_entries++;
            return true;
        }
        else
        {
            //Allocate another N(INIT_ENTRIES) entries
            record->allocated += INIT_ENTRIES;
            record->entries = realloc(record->entries, record->allocated);
            //Append data
            memcpy(&record->entries[record->n_entries], &entry, sizeof(record_entry_t));
            record->n_entries++;
            return true;
        }
    }
    else
    {
        return false;
    }
}
//Appends a new entry to a book and allocates the required memory, return false if failed.
bool book_append_entry(book_t* book, record_t* record)
{
    if(book)
    {
        //Check if there is enough space allocated
        if(book->allocated > book->n_entries)
        {
            //Append data
            memcpy(&book->records[book->n_entries], record, sizeof(record_t));
            book->n_entries++;
            return true;
        }
        else
        {
            //Allocate another N(INIT_ENTRIES) entries
            book->allocated += INIT_ENTRIES;
            book->records = realloc(book->records, record->allocated);
            //Append data
            memcpy(&book->records[book->n_entries], record, sizeof(record_entry_t));
            record->n_entries++;
            return true;
        }
    }
    else
    {
        return false;
    }
}
//Free book
void free_book(book_t* book)
{
    if(book)
    {
        for(int i = 0;i < book->allocated;i++)
        {
            record_t* record = &book->records[i];
            //Check if pointer is valid
            if(record)
            {
                free(record->entries);
            }
        }
        free(book->records);
    }
}

//Free record
void free_record(record_t* record)
{
    if(record)
    {
        free(record->entries);
    }
}

//Encode record to tlv. Returns buffer with tlv data.
tlv_data_t* encode_record_to_tlv(record_t* record)
{
    //Check if its a valid pointer
    if(!record)
    {
        return NULL;
    }
    
    //Allocate head of the linked list
    tlv_data_t* head = NULL;
    head = malloc(sizeof(tlv_data_t));

    if(!head)
    {
        fprintf(stderr, "Error: There was an issue allocating tlv_data head\n");
        return NULL;
    }

    //Serialize record into TLV encoding
    //Iterate through every entry and serialize it
    for(int i = 0; i < record->n_entries;i++)
    {
        record_entry_t* entries = record->entries;
        if(entries)
        {
            switch (entries[i].type)
            {
            case entry_boolean:
                //Head has no data, so use head first
                if(head->size == 0)
                {
                    //Char casting is being do to get data from pointer as bytes
                    *head = tlv_encode(TAG_BOOLEAN, sizeof(bool),
                        (char*)&(entries[i].data.boolean), entries[i].key);                    
                }
                else
                {
                    tlv_data_append(head, tlv_encode(TAG_BOOLEAN, sizeof(bool),
                        (char*)&(entries[i].data.boolean), entries[i].key));
                }
                break;
            case entry_value:
                if(head->size == 0)
                {
                    *head = tlv_encode(TAG_INT, sizeof(int32_t),
                        (char*)&(entries[i].data.value), entries[i].key);
                }
                else
                {
                    tlv_data_append(head, tlv_encode(TAG_INT, sizeof(int32_t),
                        (char*)&(entries[i].data.value), entries[i].key));
                }
                break;
            case entry_string:
                if(head->size == 0)
                {
                    *head = tlv_encode(TAG_STRING, strlen(entries[i].data.string),
                        entries[i].data.string, entries[i].key);
                }
                else
                {
                    tlv_data_append(head, tlv_encode(TAG_STRING, strlen(entries[i].data.string),
                        entries[i].data.string, entries[i].key));
                }
                break;
            default:
                fprintf(stderr, "Error: Unknown entry type %d\n", entries[i].type);
                return NULL;
                break;
            }
        }
    }
    return head;
}
//Serialize book to tlv encoding. Returns buffer with tlv data.
serialized_t serialize_book_to_tlv(book_t* book)
{
    tlv_data_t* tlv_data_head = malloc(sizeof(tlv_data_t*));
    tlv_data_head->next = NULL;
    tlv_data_head->size = 0;

    //Get all tlv_data entries
    for(int i = 0;i < book->n_entries;i++)
    {
        //Temporary variable to hold the pointer of the linked list
        tlv_data_t* list_head = encode_record_to_tlv(&book->records[i]);

        if(!list_head)
        {
            fprintf(stderr, "Error while trying to serialize records\n");
            //Return NULL buffer
            //Struct that contains buffer and its size
            serialized_t serialized;
            serialized.buffer = NULL;
            serialized.size = 0;
            return serialized;
        }
        //Set tlv_data_head to the head of the first appended list
        if(tlv_data_head->size == 0)
        {
            tlv_data_head = list_head;
        }
        else
        {
            tlv_data_list_append(tlv_data_head, list_head);
        }

    }

    //Serialize all tlv_entries
    //Calculate required space
    size_t buff_size = tlv_data_get_size(tlv_data_head);
    char* buffer = malloc(sizeof(char) * buff_size);
    //Current buffer offset
    size_t offset = 0;

    //Temporaty head.
    tlv_data_t* head = tlv_data_head;
    while(head)
    {
        memcpy(&buffer[offset], head->data, head->size);
        offset += head->size;
        head = head->next;
    }
    //Free memory
    tlv_data_delete(tlv_data_head);
    
    //Struct that contains buffer and its size
    serialized_t serialized;
    serialized.buffer = buffer;
    serialized.size = offset;

    return serialized;
}

tlv_data_t tlv_encode(uint8_t tag, uint8_t len, char* data, int key)
{
    tlv_data_t tlv_data;

    //Data lenght + tag + key and len fields
    uint8_t total_len = len + 3;
    //Allocate buffer for it
    char* buffer = malloc(total_len * sizeof(char*));
    tlv_data.data = buffer;

    //Copy tag and lenght fields into buffer
    int offset = 0;
    memcpy(buffer + offset, &tag, sizeof(tag));
    offset += sizeof(tag);
    memcpy(buffer + offset, &len, sizeof(len));
    offset += sizeof(len);

    //Copy key field
    memcpy(buffer + offset, &key, sizeof(key));
    offset += sizeof(key);

    //Now copy data if valid
    if(data)
    {
        memcpy(buffer + offset, data, len);
        offset += len;
    }
    
    tlv_data.size = offset;
    tlv_data.next = NULL;
    
    return tlv_data;
}

//tlv struct append new entry. Return NULL if not succesful.
//Otherwise it returns the new tail of the linked list.
tlv_data_t* tlv_data_append(tlv_data_t* head, tlv_data_t tlv_data)
{
    if(!head) return NULL;

    //Find the tail next of the linked list
    tlv_data_t* tail = head;
    while(tail->next) tail = tail->next;

    //Add tlv_data to the tail
    tlv_data_t* new_tlv_data = malloc(sizeof(tlv_data_t));
    if(!new_tlv_data) return NULL;

    //Copy data to new tail
    new_tlv_data->data = tlv_data.data;
    new_tlv_data->size = tlv_data.size;
    
    //Append by joining tail to head
    tail->next = new_tlv_data;

    //Set assign new tail to NULL
    new_tlv_data->next = NULL;

    return tail;
}
//Concatenates 2 linked lists and returns the resultant list.
tlv_data_t* tlv_data_list_append(tlv_data_t* head, tlv_data_t* list)
{
    if(!head) return NULL;

    //Find the tail next of the linked list
    tlv_data_t* tail = head;
    while(tail->next) tail = tail->next;
    
    //Append by joining tail to head
    tail->next = list;

    return tail;
}
//tlv struct delete. Returns true on success
void tlv_data_delete(tlv_data_t* head)
{
    //Iterate though linked list and delete entries
    tlv_data_t* tail = head;
    tlv_data_t* prev = NULL;
    while(tail)
    {
        prev = tail;
        tail = tail->next;
        free(prev);
    }
}
//Returns the size all data in linked list
size_t tlv_data_get_size(tlv_data_t* head)
{
    //Find the tail of the linked list
    size_t size = 0;
    tlv_data_t* tail = head;
    while(tail)
    {
        size += tail->size;
        tail = tail->next;
    }

    return size;
}

// void print_tlv_data_list(tlv_data_t* head)
// {
//     printf("Head address 0x%08x\n", (unsigned int)head);

//     //Iterate through linked list and print elements
//     tlv_data_t* temp = head;

//     while(temp)
//     {
//         printf("Node size 0x%08x\n", temp->size);
//         printf("Node data: ");
//         for(int i = 0;i < temp->size;i++)
//         {
//             printf("%02x ", temp->data[i] & 0xff);
//         }
//         temp = temp->next;
//         printf("\n\n\n\n");
//     }
// }