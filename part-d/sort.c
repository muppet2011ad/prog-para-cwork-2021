#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINES_ARR_LEN 5 // Size interval for the lines array
#define BUFFER_SIZE 1024 // Max line length when reading in data

char **read_lines (FILE* src, char ***lines, int *num_lines, int *max_lines);
void out_lines (FILE* dest, char **lines, int num_lines);
int str_sort_cmp (const void *str1, const void *str2);

typedef struct flags {
    unsigned int file_specified : 1;
    unsigned int opt_o : 1;
    unsigned int opt_n : 1;
    unsigned int opt_h : 1;
} flags; // Bitfield to store flags (saves an incredible 3 bytes of memory on my machine)

int main(int argc, char *argv[]) {
    int num_lines = 0;
    int lines_arr_size = LINES_ARR_LEN;
    char **lines = calloc(LINES_ARR_LEN, sizeof(char*)); // Allocate memory to store lines
    if (lines == NULL) { // If we have an issue allocating memory
        fprintf(stderr, "Error: failed to allocate memory when reading in string!"); // Give an error
        exit(1); // And exit
    }

    flags status = {0, 0, 0 ,0}; // Initialises all flags to zero

    // int file_specified = 0; // Tracks whether flag has been specified
    // int opt_o = 0;
    // int opt_n = 0;
    // int opt_h = 0;

    for (int i = 1; i < argc; i++) { // Iterate through arguments
        if (argv[i][0] == '-') { // If it's setting a flag
            switch (argv[i][1]) { // Switch through flags
                default: // If flag not recognised
                    fprintf(stderr, "Option -%c is not valid.\n\n", argv[i][1]);
                    break;
                case 'o': // TODO: Handle argument of output stream
                    status.opt_o = 1;
                    break;
                case 'n': // Otherwise set flags
                    status.opt_n = 1;
                    break;
                case 'h':
                    status.opt_h = 1;
                    break;
            }
        }
        else { // If it's not an argument, then it's a file to use as input
            FILE *newfile = fopen(argv[i], "r"); // Attempt to open the file
            if (newfile == NULL) { // If that fails
                fprintf(stderr, "Failed to read file %s", argv[i]); // Display error
                exit(1); // Exit
            }
            else { // Should we load the file
                read_lines(newfile, &lines, &num_lines, &lines_arr_size); // Read its contents into memory
                status.file_specified = 1; // Set status flag
            }
        }
    }

    if (!status.file_specified) { // If we didn't load any input files
        read_lines(stdin, &lines, &num_lines, &lines_arr_size); // Read from stdin
    }

    qsort(lines, num_lines, sizeof(char*), str_sort_cmp);
    out_lines(stdout, lines, num_lines);
    return 0;
}

char **read_lines (FILE* src, char ***lines, int *num_lines, int *lines_arr_size) {
    char input_buffer[BUFFER_SIZE]; // Create buffer to receive input
    while (!feof(src)) { // Until we reach EOF
        char *success = fgets(input_buffer, BUFFER_SIZE, src); // Read from input stream
        if (success == NULL) { // Check we haven't just read to EOF
            break; // If we have, stop looping
        }
        int line_length = strlen(input_buffer); // Get the length of the string
        char *new_line = malloc(line_length + 1); // Allocate just enough memory for it (+1 for the null character)
        if (new_line == NULL) { // If memory allocation fails
            fprintf(stderr, "Error: failed to allocate memory when reading in string!");
            exit(1); // Complain and exit
        }
        strcpy(new_line, input_buffer); // Copy from input buffer to the newly allocated memory
        if (*num_lines >= *lines_arr_size) { // If we need to expand the array
            *lines = realloc(*lines, (*lines_arr_size+LINES_ARR_LEN)*sizeof(char*)); // Expand it
            if (lines == NULL) { // Complain and exit if realloc fails
                fprintf(stderr, "Error: failed to allocate memory when reading in string!");
                exit(1);
            }
            *lines_arr_size += LINES_ARR_LEN; // Update with new array size
        }
        (*lines)[*num_lines] = new_line; // Add the new line to the array
        (*num_lines)++; // Increment the counter of items in the array
    }
    return *lines; // Return a pointer to the array
}

void out_lines (FILE* dest, char **lines, int num_lines) {
    for (int i = 0; i < num_lines; i++) {
        fprintf(dest, lines[i]);
    }
}

int str_sort_cmp (const void *str1, const void *str2) {
    return strcmp(*(char**) str1, *(char**) str2);
}