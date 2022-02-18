#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include "record.h"
#include "json.h"
#include <time.h>

// This assumes line will have a maximum lenght of 256
#define LINE_SIZE 256

uint32_t jenkins_one_at_a_time_hash(char *key, size_t len)
{
    uint32_t hash, i;
    for (hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

// Returns true if successful
bool process_values(json_value *value, int line, book_t *book)
{
    // Check if its a valid pointer
    if (value)
    {
        // Check if its an object
        if (value->type == json_object)
        {
            record_t *record = NULL;
            record = initialize_record(record);
            //Check for valid record pointer
            if(!record)
            {
                fprintf(stderr, "Error: Couldn't initialize record\n");
                return false;
            }
            // Extract pairs from object
            for (int i = 0; i < value->u.object.length; i++)
            {
                json_object_entry *json_objs = value->u.object.values;
                if (json_objs)
                {
                    record_entry_t record_entry;
                    // Get the key value
                    // sscanf(json_objs[i].name, "%*[^0123456789]%d", &key_val);
                    int key_val = jenkins_one_at_a_time_hash(json_objs[i].name, strlen(json_objs[i].name));
                    record_entry.key = key_val;

                    // Detect the type and store data accordingly
                    switch (json_objs[i].value->type)
                    {
                    case json_boolean:
                        record_entry.type = entry_boolean;
                        record_entry.data.boolean = json_objs[i].value->u.boolean;
                        break;
                    case json_integer:
                        record_entry.type = entry_value;
                        record_entry.data.value = json_objs[i].value->u.integer;
                        break;
                    case json_string:
                        record_entry.type = entry_string;
                        //Use strdup because later on the json_objs pointer will be freed and data will be lost
                        record_entry.data.string = strdup(json_objs[i].value->u.string.ptr);
                        break;
                    default:
                        fprintf(stderr, "Error: Couldn't match type %d. Unsupported.\n", line);
                        fprintf(stderr, "Record line %d, record name %s.\n", line, json_objs[i].name);
                        return false;
                        break;
                    }
                    // Append record entry. This create a new entry in the record data.
                    record_append_entry(record, record_entry);
                }
                else
                {
                    fprintf(stderr, "Error: Null array element at line %d", line);
                    return false;
                }
            }

            if(!book_append_entry(book, record))
            {
                fprintf(stderr, "Couldn't append record to book\n");
            }
            return true;
        }
        else
        {
            fprintf(stderr, "Expected object on line %d\n", line);
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

// Note: Make sure to use streaming instead of reading everything into memory.
int main(int argc, char **argv)
{
    // to store the execution time of code
    double time_spent = 0.0;
    clock_t begin = clock();
 
    // Get file name
    if (argc != 2)
    {
        fprintf(stderr, "Error: Provide file path\n");
        return 1;
    }

    // Check if file exists
    struct stat fstatus;

    if (stat(argv[0], &fstatus) != 0)
    {
        fprintf(stderr, "File %s was not found\n", argv[0]);
        return 1;
    }
    else
    {
        // Open file
        FILE *input_file = fopen(argv[1], "rb");
        if (input_file == NULL)
        {
            fprintf(stderr, "Unable to open file: %s\n", argv[0]);
            return 1;
        }
        else
        {
            // Seek to the beginning of the file
            fseek(input_file, 0, SEEK_SET);

            // Allocate space for the line buffer
            char *fetched_line = malloc(sizeof(char) * LINE_SIZE);

            // Objects from the json library
            json_char *json_str;
            json_value *value;

            // Book struct where all data will be stored
            book_t *book;
            book = initialize_book();
            if(!book)
            {
                fprintf(stderr, "Error: Couldn't initialize book\n");
            }

            // Line count
            size_t line_cnt = 0;

            // Get lines from the file
            while (fscanf(input_file, "%255[^\n]%*c", fetched_line) > 0)
            {
                // Parse the lines
                json_str = (json_char *)fetched_line;
                value = json_parse(json_str, strlen(fetched_line));

                // Check if the string was parsed
                if (value == NULL)
                {
                    fprintf(stderr, "Error while parsing data at line %ld\n", line_cnt + 1);
                    fprintf(stderr, "Error parsing data: >>> %s\n", fetched_line);
                    return 1;
                }
                else
                {
                    // Increment line count to keep track of the current line
                    line_cnt++;
                    // Process data
                    if(!process_values(value, line_cnt, book))
                    {
                        fprintf(stderr, "Error while processing data at line %ld\n", line_cnt);
                    }
                }
                //Free allocated json parser
                json_value_free(value);
            }
            // Process data from book into a tlv encoding
            serialized_t serialized = serialize_book_to_tlv(book);

            if(!serialized.buffer)
            {
                fprintf(stderr, "Error: Couldn't serialize book\n");
                return 1;
            }

            //Open output file
            FILE* output_file = fopen("output.bin", "w+");
            if(!output_file)
            {
                fprintf(stderr, "Error: Couldn't open file for output\n");
                return 1;
            }
            printf("TLV records written to output.bin\n");

            //Print the execution time and processed records
            clock_t end = clock();
            time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
            printf("Records: %ld processed in %f seconds\n", line_cnt, time_spent);

            fwrite(serialized.buffer, sizeof(char), serialized.size, output_file);
            // Free allocated data
            free(fetched_line);
            free_book(book);
            free(serialized.buffer);
            // Close file
            fclose(input_file);
            // Exit
            exit(0);
        }
    }

    return 0;
}
