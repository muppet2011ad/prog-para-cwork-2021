//You can include any of headers defined in the C11 standard here. They are:
//assert.h, complex.h, ctype.h, errno.h, fenv.h, float.h, inttypes.h, iso646.h, limits.h, locale.h, math.h, setjmp.h, signal.h, stdalign.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, stdio.h, stdlib.h, stdnoreturn.h, string.h, tgmath.h, threads.h, time.h, uchar.h, wchar.h or wctype.h
//You may not include any other headers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"connect4.h"

char **read_lines (FILE* src, int *num_lines);

struct board_structure {
    char **grid;
    int width;
    int height;
};

board setup_board(){
    board new_board = calloc (1, sizeof(struct board_structure));
    return new_board;
}

void cleanup_board(board u){
    if (!(u->grid == NULL)) { // Only try to free the grid data if it's been assigned, otherwise we'll segfault trying to free NULL
        for (int i = 0; i < u->height; i++) {
            free(u->grid[i]);
        }
        free(u->grid);
    }
    free(u);
}

void read_in_file(FILE *infile, board u){
    int rows = 0;
    char **lines = read_lines(infile, &rows);
    u->grid = lines;
    u->height = rows;
    u->width = strlen(lines[0]);
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) { // Correct any capitals to lowercase and remove any invalid characters
            if (u->grid[i][j] == 'X') { u->grid[i][j] = 'x'; }
            else if (u->grid[i][j] == 'O') { u->grid[i][j] = 'o'; }
            else if (!(u->grid[i][j] == 'x') && !(u->grid[i][j] == 'o') && !(u->grid[i][j] == '.')) { u->grid[i][j] = '.'; }
        }
    }
}

void write_out_file(FILE *outfile, board u){
//You may put code here
}

char next_player(board u){
    int num_x;
    int num_o;
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) {
            if (u->grid[i][j] == 'o') { num_o++; }
            else if (u->grid[i][j] == 'x') { num_x++; }
        }
    }
    if (num_x <= num_o) { return 'x'; }
    else { return 'o'; }
}

char current_winner(board u){
//You may put code here
}

struct move read_in_move(board u){
//You may put code here
  printf("Player %c enter column to place your token: ",next_player(u)); //Do not edit this line
//You may put code here
  printf("Player %c enter row to rotate: ",next_player(u)); // Do not edit this line
//You may put code here
}

int is_valid_move(struct move m, board u){
//You may put code here
}

char is_winning_move(struct move m, board u){
//You may put code here
}

void play_move(struct move m, board u){
//You may put code here
}

//You may put additional functions here if you wish.


char **read_lines (FILE* src, int *num_lines) { // Mostly copied from my part d with a few alterations since the functionality is very similar
    char **lines = calloc(5, sizeof(char*));
    int lines_arr_size = 5;
    char input_buffer[512]; // Create buffer to receive input
    while (!feof(src)) { // Until we reach EOF
        char *success = fgets(input_buffer, 512, src); // Read from input stream
        if (success == NULL) { // Check we haven't just read to EOF
            break; // If we have, stop looping
        }
        int line_length = strlen(input_buffer); // Get the length of the string
        char *new_line = malloc(line_length); // Allocate just enough memory for it
        if (new_line == NULL) { // If memory allocation fails
            fprintf(stderr, "Error: failed to allocate memory when reading in string!");
            exit(1); // Complain and exit
        }
        strncpy(new_line, input_buffer, line_length); // Copy from input buffer to the newly allocated memory
        new_line[line_length-1] = '\0';
        if (*num_lines >= lines_arr_size) { // If we need to expand the array
            lines = realloc(lines, (lines_arr_size+5)*sizeof(char*)); // Expand it
            if (lines == NULL) { // Complain and exit if realloc fails
                fprintf(stderr, "Error: failed to allocate memory when reading in string!");
                exit(1);
            }
            lines_arr_size += 5; // Update with new array size
        }
        lines[*num_lines] = new_line; // Add the new line to the array
        (*num_lines)++; // Increment the counter of items in the array
    }
    return lines; // Return a pointer to the array
}

int main () {
    board new_board = setup_board();
    FILE *testfile = fopen("test_input1.txt", "r");
    read_in_file(testfile, new_board);
    fclose(testfile);
    return 0;
}