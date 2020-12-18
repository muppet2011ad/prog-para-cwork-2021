//You can include any of headers defined in the C11 standard here. They are:
//assert.h, complex.h, ctype.h, errno.h, fenv.h, float.h, inttypes.h, iso646.h, limits.h, locale.h, math.h, setjmp.h, signal.h, stdalign.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, stdio.h, stdlib.h, stdnoreturn.h, string.h, tgmath.h, threads.h, time.h, uchar.h, wchar.h or wctype.h
//You may not include any other headers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include"connect4.h"

#define TOKENS_IN_INT (sizeof(int)*4)
#define IN_RUN(run, i, j) ((run[0] == i && run[1] == j) || (run[2] == i && run[3] == j) || (run[4] == i && run[5] == j) || (run[6] == i && run[7] == j))

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
void get_win_run(win w, int *run, int width);
void validate_pointer(void *ptr, int errtype);
void error(int type);
char get_val(board u, int row, int col);
void set_val(board u, int row, int col, char token);

struct board_structure {
    unsigned int *grid; // Array of strings to store board data
    int width;
    int height; // Stores bounds of board
};

board setup_board(){
    board new_board = calloc (1, sizeof(struct board_structure)); // Alloc memory and zero all bytes (makes it easy to distinguish between a board populated with data and one that isn't)
    validate_pointer(new_board, 0);
    return new_board;
}

void cleanup_board(board u){
    validate_pointer(u, 6);
    if (!(u->grid == NULL)) { // Only try to free the grid data if it's been assigned, otherwise we'll segfault trying to free NULL
        free(u->grid); // Free the grid array itself
    }
    free(u); // Free the struct
}

void read_in_file(FILE *infile, board u) {
    validate_pointer(infile, 3);
    validate_pointer(u, 6); // Validate both pointers given
    u->height = 0;
    u->width = -1; // Initialise values for height and width
    u->grid = malloc(sizeof(int)); // Allocate initial memory for the grid
    validate_pointer(u->grid, 0); // Validate the memory allocation
    int members = 0; // Initially we store no tokens
    int max_members = TOKENS_IN_INT; // The maximum for one integer is the macro TOKENS_IN_INT
    char input_buffer[514]; // Create an input buffer (512 columns + \n + \0)
    while (fgets(input_buffer, 514, infile)) { // Read until we hit EOF
        if (u->width == -1) { // If we haven't set the width yet
            u->width = strlen(input_buffer); // Get the length of the line
            if (input_buffer[u->width-1] == '\n') { u->width--; } // Make sure to not include the \n in our count
        }
        else { // If we have set the width, we should check against it
            int len = strlen(input_buffer); // Grab the length
            if ((input_buffer[len-1] == '\n' && len-1 != u->width) || (input_buffer[len-1] == '\0' && len != u->width)) { error(5); } 
            // Slightly messy condition but will throw an error if the length of the line is different
        }
        members += u->width; // Increment our counter of members to reflect how many we need to store
        if (members > max_members) { // If we need more room
            u->grid = realloc(u->grid, ((members/TOKENS_IN_INT) + (members%TOKENS_IN_INT != 0))*sizeof(int)); // Realloc based on how much room we need
            validate_pointer(u->grid, 0); // Validate that worked
            max_members = ((members/TOKENS_IN_INT) + (members%TOKENS_IN_INT != 0))*TOKENS_IN_INT; // Calculate the new capacity of the grid
        }
        for (int j = 0; j < u->width; j++) { // Iterate through the line we've read
            set_val(u, u->height, j, tolower(input_buffer[j])); // Copy each character into the grid
        }
        u->height++; // Increment our counters
    }
}

void write_out_file(FILE *outfile, board u){
    validate_pointer(outfile, 3);
    validate_pointer(u, 6); // Validates both pointers passed
    win x_win = find_win(u, 'x'); // Attempt to find a win by x
    int x_win_run[8]; // Prepare an array to store the winning locations
    get_win_run(x_win, x_win_run, u->width); // Work out the run if it exists
    win o_win = find_win(u, 'o');
    int o_win_run[8];
    get_win_run(o_win, o_win_run, u->width); // Same for o
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) { // Iterate through the grid
            char token = get_val(u, i, j); // Get the token at location i,j
            if (token == 'x' && (x_win.is_win && IN_RUN(x_win_run, i, j))) { token = 'X'; }
            else if (token == 'o' && (o_win.is_win && IN_RUN(o_win_run, i, j))) { token = 'O'; } // Check if the token is part of a win - if it is, capitalise it
            fprintf(outfile, "%c", token); // Output the token
        }
        fprintf(outfile, "\n"); // Add a newline to split up lines
    }
    fprintf(outfile, "\n"); // Finish with a final newline
}

char next_player(board u){
    validate_pointer(u, 6);
    int num_x = 0;
    int num_o = 0; // Create counters for each player
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) {
            if (get_val(u, i, j) == 'o') { num_o++; }
            else if (get_val(u, i, j) == 'x') { num_x++; } // Increment counters accordingly
        }
    }
    if (num_x <= num_o) { return 'x'; } // Make comparison - x goes first so should be returned if equal
    else { return 'o'; }
}

char current_winner(board u){
    validate_pointer(u, 6);
    win x_win = find_win(u, 'x');
    win o_win = find_win(u, 'o');
    if (x_win.is_win && o_win.is_win) { return 'd'; }
    else if (x_win.is_win) { return 'x'; }
    else if (o_win.is_win) { return 'o'; }
    else { return '.'; }
}

struct move read_in_move(board u){
    validate_pointer(u, 6);
    int col, row;
    printf("Player %c enter column to place your token: ",next_player(u)); //Do not edit this line
    if (scanf("%d", &col) != 1) { error(4); }
    printf("Player %c enter row to rotate: ",next_player(u)); // Do not edit this line
    if (scanf("%d", &row) != 1) { error(4); }
    struct move new_move = {col, row};
    return new_move;
}

int is_valid_move(struct move m, board u){
    validate_pointer(u, 6);
    int real_col = m.column - 1;
    int real_row = get_real_row(u, m.row); // Convert the move data to indices 
    if ((real_row < 0 || real_row >= u->height) && m.row != 0) { return 0; } // Check bound on row. No need to check if below zero since -1 indicates no rotation (as m.row was 0)
    if (real_col >= u->width || real_col < 0) { return 0; } // Check bounds on column. Must be between 0 and the max index, which is width-1
    if (get_val(u, 0, real_col) != '.') { return 0; } // Check that there's actually room to place the new token at the top of the grid
    return 1; // If it passes all these tests, it's a winning move so return 1
}

char is_winning_move(struct move m, board u){
    validate_pointer(u, 6);
    if (!is_valid_move(m, u)) { error(2); }
    board test_board = copy_board(u); // Create a board for testing the move so we don't accidentally ruin the game board
    play_move(m, test_board); // Play the move on the test board
    char winner = current_winner(test_board);
    cleanup_board(test_board);
    return winner;
}

void play_move(struct move m, board u){
    validate_pointer(u, 6);
    if (!is_valid_move(m, u)) { error(2); }
    char token = next_player(u); // Gets the token that will be played
    set_val(u, 0, m.column-1, token); // Place it in the top row
    resolve_gravity_single(u, 0, m.column-1); // Apply gravity to let it fall
    if (m.row == 0) { return; } // If m.row was zero then there is no need to rotate
    int real_row = get_real_row(u, m.row); // Otherwise get the array index for the move's row
    if (m.row > 0) { // If rotating to the right
        char last = get_val(u, real_row, u->width-1); // Rightmost column becomes leftmost so start by copying that
        for (int i = 0; i < u->width; i++) { // Iterate across the row left to right
            char tmp = get_val(u, real_row, i);
            set_val(u, real_row, i, last);
            last = tmp; // Swap the item in last with the ith column
            resolve_gravity_above(u, real_row, i); // Apply gravity on the column
        }
    }
    else { // If rotating to the right, the process is the same but right-to-left
        char last = get_val(u, real_row, 0);
        for (int i = u->width-1; i >= 0; i--) {
            char tmp = get_val(u, real_row, i);
            set_val(u, real_row, i, last);
            last = tmp;
            resolve_gravity_above(u, real_row, i);
        } 
    }
}

//You may put additional functions here if you wish.

win find_win(board u, char player) {
    validate_pointer(u, 6);
    // Check horizontal wins
    for (int i = 0; i < u->height; i++) {
        for (int j = 0; j < u->width; j++) {
            char token = get_val(u, i, j);
            if (token == player && get_val(u, i, real_modulo(j+1, u->width)) == token && get_val(u, i, real_modulo(j+2, u->width)) == token && get_val(u, i, real_modulo(j+3, u->width)) == token) {
                return (win) {i, j, 0, 1};
            }
        }
    }
    // Check vertical and diagonal wins
    for (int i = 0; i < u->height-3; i++) {
        for (int j = 0; j < u->width; j++) {
            char token = get_val(u, i, j);
            // Check vertical
            if (token == player && get_val(u, i+1, j) == token && get_val(u, i+2, j) == token && get_val(u, i+3, j) == token) {
                return (win) {i, j, 2, 1};
            }
            // Check diagonal y = -x
            if (token == player && get_val(u, i+1, real_modulo(j+1, u->width)) == token && get_val(u, i+2, real_modulo(j+2, u->width)) == token && get_val(u, i+3, real_modulo(j+3, u->width)) == token) {
                return (win) {i, j, 3, 1};
            }
            // Check diagonal y = x
            if (token == player && get_val(u, i+1, real_modulo(j-1, u->width)) == token && get_val(u, i+2, real_modulo(j-2, u->width)) == token && get_val(u, i+3, real_modulo(j-3, u->width)) == token) {
                return (win) {i, j, 1, 1};
            }
        }
    }
    // If we get here, we didn't find a win for the player
    return (win) {0, 0, 0, 0};
}

void get_win_run(win w, int *run, int width) {
    if (!w.is_win) { return; }
    run[0] = w.real_row_start;
    run[1] = w.real_col_start;
    switch (w.direction) {
        case 0:
            for (int i = 1; i < 4; i++) {
                run[2*i] = w.real_row_start;
                run[2*(i)+1] = real_modulo(w.real_col_start+i, width);
            }
            break;
        case 1:
            for (int i = 1; i < 4; i++) {
                run[2*i] = w.real_row_start+i;
                run[2*(i)+1] = real_modulo(w.real_col_start-i, width);
            }
            break;
        case 2:
            for (int i = 1; i < 4; i++) {
                run[2*i] = w.real_row_start+i;
                run[2*(i)+1] = w.real_col_start;
            }
            break;
        case 3:
            for (int i = 1; i < 4; i++) {
                run[2*i] = w.real_row_start+i;
                run[2*(i)+1] = real_modulo(w.real_col_start+i, width);
            }
            break;
    }
}

int get_real_row(board u, int original_row) { // Converts the user-friendly row numbering (+/-1 is bottom row, +/-n is top) to my row numbering (0 is top row, n-1 is bottom row)
    validate_pointer(u, 6);
    return u->height - abs(original_row);
}

void resolve_gravity_single(board u, int real_row, int real_col) { // Applies the effects of gravity to a SINGLE token
    validate_pointer(u, 6);
    if (real_row == u->height-1) { return; } // If we're at the bottom row, no problem
    if (get_val(u, real_row+1, real_col) != '.') { return; } // If the space below us is occupied, also no problem
    int new_row = real_row;
    while (!(new_row == u->height-1) && !(get_val(u, new_row+1, real_col) != '.')) { new_row++; }; // Count through rows until one of the "no problem" conditions is met
    set_val(u, new_row, real_col, get_val(u, real_row, real_col)); // Copy the token into its new position
    set_val(u, real_row, real_col, '.'); // Leave an empty space where it used to be
}

void resolve_gravity_above(board u, int real_row, int real_col) {
    validate_pointer(u, 6);
    for (int i = real_row; i >= 0; i--) { // A token falling can only affect spaces directly above it, so iterate through those
        if (get_val(u, i, real_col) != '.') {
            resolve_gravity_single(u, i, real_col); // And apply gravity if they contain a token
        }
    }
}

int real_modulo(int a, int b) { // C's modulo operator returns -ve values with -ve inputs, which I don't want since these aren't valid array indices. This function bounds the result to [0, b)
    if (a >= 0) {
        return a % b;
    }
    else {
        return (a % b) + b;
    }
}

board copy_board(board u) {
    validate_pointer(u, 6);
    board v = setup_board();
    v->width = u->width;
    v->height = u->height;
    int members = v->width*v->height;
    v->grid = malloc(((members/TOKENS_IN_INT) + (members%TOKENS_IN_INT != 0))*sizeof(int));
    validate_pointer(v->grid, 0);
    memcpy(v->grid, u->grid, ((members/TOKENS_IN_INT) + (members%TOKENS_IN_INT != 0))*sizeof(int));
    return v;
}

char get_val(board u, int row, int col) {
    int index = ((row*u->width) + col)/TOKENS_IN_INT; // Calculates the index of the integer we need to fetch in the grid
    int bit_index = ((row*u->width) + col)%TOKENS_IN_INT; // Calculates the index of the bit pair within that int we need
    unsigned int stored_bytes = u->grid[index]; // Grabs the int from the grid array
    stored_bytes = stored_bytes >> (((TOKENS_IN_INT-bit_index)*2)-2); // Bit-shifts it until the bit pair we want occupies the least significant bits
    int val = 3 & stored_bytes; // Copy these two bits into val
    if (val == 1) { return 'o'; }
    else if (val == 2) { return 'x'; } // Return a different char depending on this value
    else { return '.'; }
}

void set_val(board u, int row, int col, char token) {
    int index = ((row*u->width) + col)/TOKENS_IN_INT;
    int bit_index = ((row*u->width) + col)%TOKENS_IN_INT;
    unsigned int stored_bytes = u->grid[index]; // Same as getVal up to here
    unsigned int val = 0; // Default value to write is 0 (this means if we are told to write an invalid token, we just write blank)
    if (token == 'o') { val = 1; }
    else if (token == 'x') { val = 2; } // Set val depending on the token given
    val = val << (((TOKENS_IN_INT-bit_index)*2)-2); // Shift val so our two bits are correctly positioned within the whole int
    unsigned int clear_flag = ~(3 << (((TOKENS_IN_INT-bit_index)*2)-2)); // Creates a flag to clear the two bits already stored (everything but the two bits we care about is 1)
    stored_bytes = stored_bytes & clear_flag; // Clear what was already in that bit position
    stored_bytes = stored_bytes | val; // Copy our val bits into the correct position
    u->grid[index] = stored_bytes; // Update the array with the new value
}

void validate_pointer(void *ptr, int errtype) { // Checks if pointer is null, calls error(errtype) if it is
    if (ptr == NULL) { error(errtype); }
    else { return; }
}

void error(int type) {
    const char *messages[] = {  "Failed to allocate memory for a board.", // 0
                                "Failed to allocate memory when reading in string.", // 1
                                "Move is not valid. It is either out of bounds or the space is occupied.", // 2
                                "Could not open file for reading/writing.", // 3
                                "Invalid input - make sure you input an integer.", // 4
                                "Board has inconsistent number of columns.", // 5
                                "Board does not exist." // 6
                            };
    fprintf(stderr, "Error: %s\n", messages[type]);
    exit(1);
}
