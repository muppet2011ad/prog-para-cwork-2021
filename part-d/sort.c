#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINES_ARR_LEN 5 // Size interval for the lines array
#define BUFFER_SIZE 1024 // Max line length when reading in data

char **read_lines (FILE* src, char ***lines, int *num_lines, int *max_lines);
void out_lines (FILE* dest, char **lines, int num_lines, int reverse);
int str_def_cmp (const void *str1, const void *str2);
int str_num_cmp (const void *str1, const void *str2);
long long get_str_num (char* str, int *trailing_start);
void cleanup(char **lines, int num_lines);
void usage_info();

typedef struct flags {
    unsigned int file_specified : 1;
    unsigned int opt_o : 1;
    unsigned int opt_n : 1;
    unsigned int opt_r : 1;
    unsigned int opt_h : 1;
} flags; // Bitfield to store flags (saves an incredible 16 bytes of memory on my machine)

int main(int argc, char *argv[]) {
    int num_lines = 0;
    int lines_arr_size = LINES_ARR_LEN;
    char **lines = calloc(LINES_ARR_LEN, sizeof(char*)); // Allocate memory to store lines
    if (lines == NULL) { // If we have an issue allocating memory
        fprintf(stderr, "Error: failed to allocate memory when reading in string!"); // Give an error
        exit(1); // And exit
    }

    flags status = {0, 0, 0 ,0, 0}; // Initialises all flags to zero
    const char* opt_o_arg;

    for (int i = 1; i < argc; i++) { // Iterate through arguments
        if (argv[i][0] == '-') { // If it's setting a flag
            switch (argv[i][1]) { // Switch through flags
                default: // If flag not recognised
                    fprintf(stderr, "sort: invalid option -- '%c'\nTry sort -h for more information.\n", argv[i][1]);
                    exit(1);
                    break;
                case 'o':
                    status.opt_o = 1;
                    if (i+1 < argc) {
                        opt_o_arg = argv[i+1];
                        i++;
                    }
                    else{
                        fprintf(stderr, "Specify filename for option -o.\n\n");
                        exit(1);
                    }
                    break;
                case 'n': // Otherwise set flags
                    status.opt_n = 1;
                    break;
                case 'r':
                    status.opt_r = 1;
                    break;
                case 'h':
                    status.opt_h = 1;
                    usage_info();
                    exit(0);
                    break;
            }
        }
        else { // If it's not an argument, then it's a file to use as input
            FILE *newfile = fopen(argv[i], "r"); // Attempt to open the file
            if (newfile == NULL) { // If that fails
                fprintf(stderr, "sort: cannot read: %s\n", argv[i]); // Display error
                exit(1); // Exit
            }
            else { // Should we load the file
                read_lines(newfile, &lines, &num_lines, &lines_arr_size); // Read its contents into memory
                fclose(newfile);
                status.file_specified = 1; // Set status flag
            }
        }
    }

    if (!status.file_specified) { // If we didn't load any input files
        read_lines(stdin, &lines, &num_lines, &lines_arr_size); // Read from stdin
    }

    if (status.opt_n) { // If numerical sort flag is set
        qsort(lines, num_lines, sizeof(char*), str_num_cmp); // Sort using the numeric comparator
    }
    else {
        qsort(lines, num_lines, sizeof(char*), str_def_cmp); // Otherwise sort using the default comparator
    }

    if (status.opt_o) { // If the output flag is set
        FILE *outfile = fopen(opt_o_arg, "w"); // Attempt to load specified file for writing
        if (outfile == NULL) {
            fprintf(stderr, "sort: open failed: %s\n", opt_o_arg); // If we fail, complain and exit
            exit(1);
        }
        else {
            out_lines(outfile, lines, num_lines, status.opt_r); // Output to file
            fclose(outfile); // Close file
        }
    }
    else {
        out_lines(stdout, lines, num_lines, status.opt_r); // If no output flag, write output to stdout
    }
    cleanup(lines, num_lines);
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

void out_lines (FILE* dest, char **lines, int num_lines, int reverse) {
    for (int i = 0; i < num_lines; i++) {
        if (reverse) {
            fprintf(dest, lines[num_lines-(i+1)]); // If reverse, flip the order
        }
        else {
            fprintf(dest, lines[i]); // Otherwise print normally
        }
    }
}

int str_def_cmp (const void *str1, const void *str2) {
    return strcmp(*(char**) str1, *(char**) str2);
}

int str_num_cmp (const void *str1, const void *str2) {
    // Using https://unix.stackexchange.com/a/382805 as a basis for how "sort -n" actually sorts lines as man sort was not helpful
    char *a = *(char**) str1;
    char *b = *(char**) str2; // Casts both void pointers to strings
    int counter_a;
    int counter_b; // Tracks where the first non-numeric character occurs
    long long a_numeric = get_str_num(a, &counter_a);
    long long b_numeric = get_str_num(b, &counter_b); // Gets the numeric values of the strings (per rule 1 on the stackexchange link)
    if (a_numeric < b_numeric) { return -1; }
    else if (a_numeric > b_numeric) { return 1; } // If these values are different, we have the comparison done and can finish here
    else {
        return strcmp(&(a[counter_a]), &(b[counter_b])); // Otherwise compare the rest of the string normally (per rule 2)
    }
}

long long get_str_num (char* str, int *trailing_start) { // Looks for number from start of string to first non-digit char. Returns number if found, 0 otherwise.
   int is_negative = (str[0] == '-'); // Checks for a minus sign
   int counter = 0; // Creates a counter
   char numerical_section[strlen(str)+1]; // String to store numerical part of string (which can't be longer than the original string)
   while (isdigit(str[counter]) || (counter == 0 && is_negative)) { // Whilst we have a digit (or the leading minus sign)
       counter++; // Increment counter
   }
   *trailing_start = counter; // Copy counter into parameter to pass back value
   if (counter == 0 || (is_negative && counter == 1)) { // If we didn't encounter a numeric character
       return 0; // Treat as zero per rule 3 on the stackexchange link
   }
   else {
       strncpy(numerical_section, str, counter); // Copy the leading numeric section into the string
       numerical_section[counter] = '\0'; // Add the null-terminator (strncpy doesn't do this automatically which causes issues)
       return atoll(numerical_section); // Convert this to a long long and return
   }
}

void cleanup(char **lines, int num_lines) {
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
}

void usage_info() {
    printf("Sorts input file(s) (or stdin if none specified). Supports options:\n\t-o [FILENAME]: Directs sorted output to file. stdout used if not set.\n\t-n: Compare according to string numerical value.\n\t-r: Reverse sort order.\n\t-h: Display usage information.\nUsage: ./sort [OPTION] ... [FILE] ...\nI have managed to implement all of this section of the coursework (assuming my understanding of the -n option is sound).\n");
}