//You can include any of headers defined in the C11 standard here. They are:
//assert.h, complex.h, ctype.h, errno.h, fenv.h, float.h, inttypes.h, iso646.h, limits.h, locale.h, math.h, setjmp.h, signal.h, stdalign.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, stdio.h, stdlib.h, stdnoreturn.h, string.h, tgmath.h, threads.h, time.h, uchar.h, wchar.h or wctype.h
//You may not include any other headers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"connect4.h"

typedef struct win_structure {
    int real_row_start;
    unsigned int real_col_start : 9; // Max 512 columns so will never need more than 512 indices
    unsigned int direction : 2; // 0: horizontal, 1: diagonal y = x, 2: vertical, 3: diagonal y = -x
    unsigned int is_win : 1;
} win;

char **read_lines (FILE* src, int *num_lines);
void resolve_gravity_single(board u, int real_row, int real_col);
void resolve_gravity_above(board u, int real_row, int real_col);
int get_real_row(board u, int original_row);
win find_win(board u, char player);
int real_modulo(int a, int b);
board copy_board(board u);
void validate_pointer(void *ptr, int errtype);
void error(int type);

struct board_structure {
    char **grid; // Array of strings to store board data
    int width;
    int height; // Stores bounds of board
};

board setup_board(){
    board new_board = calloc (1, sizeof(struct board_structure)); // Alloc memory and zero all bytes (makes it easy to distinguish between a board populated with data and one that isn't)
    validate_pointer(new_board, 0);
    return new_board;
}

void cleanup_board(board u){
    if (!(u->grid == NULL)) { // Only try to free the grid data if it's been assigned, otherwise we'll segfault trying to free NULL
        for (int i = 0; i < u->height; i++) {
            free(u->grid[i]); // Free each row
        }
        free(u->grid); // Free the grid array itself
    }
    free(u); // Free the struct
}

void read_in_file(FILE *infile, board u){
    int rows = 0;
    char **lines = read_lines(infile, &rows); // Read in the lines from the file given
    u->grid = lines; // Copy pointer to lines into board
    u->height = rows;
    u->width = strlen(lines[0]); // Work out the bounds of the board
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
    int num_o; // Create counters for each player
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) {
            if (u->grid[i][j] == 'o') { num_o++; }
            else if (u->grid[i][j] == 'x') { num_x++; } // Increment counters accordingly
        }
    }
    if (num_x <= num_o) { return 'x'; } // Make comparison - x goes first so should be returned if equal
    else { return 'o'; }
}

char current_winner(board u){
    win x_win = find_win(u, 'x');
    win o_win = find_win(u, 'o');
    if (x_win.is_win && o_win.is_win) { return 'd'; }
    else if (x_win.is_win) { return 'x'; }
    else if (o_win.is_win) { return 'o'; }
    else { return '.'; }
}

struct move read_in_move(board u){
    int col;
    int row;
    printf("Player %c enter column to place your token: ",next_player(u)); //Do not edit this line
    if (scanf("%d", &col) != 1) { error(4); }
    printf("Player %c enter row to rotate: ",next_player(u)); // Do not edit this line
    if (scanf("%d", &row) != 1) { error(4); }
    struct move new_move = {col, row};
    return new_move;
}

int is_valid_move(struct move m, board u){
    int real_col = m.column - 1;
    int real_row = get_real_row(u, m.row); // Convert the move data to indices 
    if (real_row >= u->height) { return 0; } // Check bound on row. No need to check if below zero since -1 indicates no rotation (as m.row was 0)
    if (real_col >= u->width || real_col < 0) { return 0; } // Check bounds on column. Must be between 0 and the max index, which is width-1
    if (u->grid[0][real_col] != '.') { return 0; } // Check that there's actually room to place the new token at the top of the grid
    return 1; // If it passes all these tests, it's a winning move so return 1
}

char is_winning_move(struct move m, board u){
    if (!is_valid_move(m, u)) { error(2); }
    board test_board = copy_board(u); // Create a board for testing the move so we don't accidentally ruin the game board
    play_move(m, test_board); // Play the move on the test board
    char winner = current_winner(test_board);
    cleanup_board(test_board);
    return winner;
}

void play_move(struct move m, board u){
    if (!is_valid_move(m, u)) { error(2); }
    char token = next_player(u); // Gets the token that will be played
    u->grid[0][m.column-1] = token; // Place it in the top row
    resolve_gravity_single(u, 0, m.column-1); // Apply gravity to let it fall
    if (m.row == 0) { return; } // If m.row was zero then there is no need to rotate
    int real_row = get_real_row(u, m.row); // Otherwise get the array index for the move's row
    if (m.row > 0) { // If rotating to the right
        char last = u->grid[real_row][u->width-1]; // Rightmost column becomes leftmost so start by copying that
        for (int i = 0; i < u->width; i++) { // Iterate across the row left to right
            char tmp = u->grid[real_row][i];
            u->grid[real_row][i] = last;
            last = tmp; // Swap the item in last with the ith column
            resolve_gravity_above(u, real_row, i); // Apply gravity on the column
        }
    }
    else { // If rotating to the right, the process is the same but right-to-left
        char last = u->grid[real_row][0];
        for (int i = u->width-1; i >= 0; i--) {
            char tmp = u->grid[real_row][i];
            u->grid[real_row][i] = last;
            last = tmp;
            resolve_gravity_above(u, real_row, i);
        } 
    }
}

//You may put additional functions here if you wish.

win find_win(board u, char player) {
    // Check horizontal wins
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) {
            char token = u->grid[i][j];
            if (token == player && u->grid[i][real_modulo(j+1, u->width)] == token && u->grid[i][real_modulo(j+2, u->width)] == token && u->grid[i][real_modulo(j+3, u->width)]) {
                return (win) {i, j, 0, 1};
            }
        }
    }
    // Check vertical and diagonal wins
    for (int i = 0; i < u->height-3; i++) {
        for (int j = 0; j < u->width; j++) {
            char token = u->grid[i][j];
            // Check vertical
            if (token == player && u->grid[i+1][j] == token && u->grid[i+2][j] == token && u->grid[i+3][j] == token) {
                return (win) {i, j, 2, 1};
            }
            // Check diagonal y = -x
            if (token == player && u->grid[i+1][real_modulo(j+1, u->width)] == token && u->grid[i+2][real_modulo(j+2, u->width)] == token && u->grid[i+3][real_modulo(j+3, u->width)] == token) {
                return (win) {i, j, 3, 1};
            }
            // Check diagonal y = x
            if (token == player && u->grid[i+1][real_modulo(j-1, u->width)] == token && u->grid[i+2][real_modulo(j-2, u->width)] == token && u->grid[i+3][real_modulo(j-3, u->width)] == token) {
                return (win) {i, j, 1, 1};
            }
        }
    }
    // If we get here, we didn't find a win for the player
    return (win) {0, 0, 0, 0};
}

int get_real_row(board u, int original_row) { // Converts the user-friendly row numbering (+/-1 is bottom row, +/-n is top) to my row numbering (0 is top row, n-1 is bottom row)
    return u->height - abs(original_row);
}

void resolve_gravity_single(board u, int real_row, int real_col) { // Applies the effects of gravity to a SINGLE token
    if (real_row == u->height-1) { return; } // If we're at the bottom row, no problem
    if (u->grid[real_row+1][real_col] != '.') { return; } // If the space below us is occupied, also no problem
    int new_row = real_row;
    while (!(new_row == u->height-1) && !(u->grid[new_row+1][real_col] != '.')) { new_row++; } // Count through rows until one of the "no problem" conditions is met
    u->grid[new_row][real_col] = u->grid[real_row][real_col]; // Copy the token into its new position
    u->grid[real_row][real_col] = '.'; // Leave an empty space where it used to be
}

void resolve_gravity_above(board u, int real_row, int real_col) {
    for (int i = real_row; i >= 0; i--) {
        if (u->grid[i][real_col] != '.') {
            resolve_gravity_single(u, i, real_col);
        }
    }
}

int real_modulo(int a, int b) {
    if (a >= 0) {
        return a % b;
    }
    else {
        return (a % b) + b;
    }
}

char **read_lines (FILE* src, int *num_lines) { // Mostly copied from my part d with a few alterations since the functionality is very similar
    validate_pointer(src, 3);
    char **lines = calloc(5, sizeof(char*));
    validate_pointer(lines, 1);
    int lines_arr_size = 5;
    char input_buffer[514]; // Create buffer to receive input - 514 as it allows 512 chars + \n + \0
    while (!feof(src)) { // Until we reach EOF
        char *success = fgets(input_buffer, 514, src); // Read from input stream
        if (success == NULL) { // Check we haven't just read to EOF
            break; // If we have, stop looping
        }
        int line_length = strlen(input_buffer); // Get the length of the string
        char *new_line = malloc(line_length); // Allocate just enough memory for it
        validate_pointer(new_line, 1);
        strncpy(new_line, input_buffer, line_length); // Copy from input buffer to the newly allocated memory
        new_line[line_length-1] = '\0';
        if (*num_lines >= lines_arr_size) { // If we need to expand the array
            lines = realloc(lines, (lines_arr_size+5)*sizeof(char*)); // Expand it
            validate_pointer(lines, 1);
            lines_arr_size += 5; // Update with new array size
        }
        lines[*num_lines] = new_line; // Add the new line to the array
        (*num_lines)++; // Increment the counter of items in the array
    }
    return lines; // Return a pointer to the array
}

void display_board(board u) {
    for (int i = 0; i < u->height; i++) {
        printf("%s\n", u->grid[i]);
    }
    printf("\n");
}

board copy_board(board u) {
    board v = setup_board();
    v->width = u->width;
    v->height = u->height;
    v->grid = malloc(sizeof(char*)*v->height);
    validate_pointer(v->grid, 0);
    for (int i = 0; i < v->height; i++) {
        v->grid[i] = malloc(sizeof(char)*v->width);
        validate_pointer(v->grid[i], 0);
        strcpy(v->grid[i], u->grid[i]);
    }
    return v;
}

void validate_pointer(void *ptr, int errtype) {
    if (ptr == NULL) { error(errtype); }
    else { return; }
}

void error(int type) {
    const char *messages[] = { "Failed to allocate memory for a board.", "Failed to allocate memory when reading in string.", "Move is not valid.", "File pointer is null.", "Invalid input."  };
    fprintf(stderr, "Error: %s\n", messages[type]);
    exit(type+1);
}

int main () {
    board new_board = setup_board();
    FILE *testfile = fopen("test_input1.txt", "r");
    read_in_file(testfile, new_board);
    fclose(testfile);
    struct move test = read_in_move(new_board);
    //struct move test = {3, -4};
    //int valid = is_valid_move(test, new_board);
    play_move(test, new_board);
    //printf("%c\n", current_winner(new_board));
    display_board(new_board);
    cleanup_board(new_board);
    return 0;
}