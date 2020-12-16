#include<stdio.h>
#include"connect4.h"
int main(){
  FILE *infile,*outfile;

  board my_board=setup_board();
  infile=fopen("test_input1.txt","r");
  read_in_file(infile,my_board);
  fclose(infile);

  struct move next_move;
  next_move.column=3;
  next_move.row=4;
  play_move(next_move,my_board);
  next_move.column=2;
  next_move.row=2;
  play_move(next_move,my_board);

  outfile=fopen("test_output1.txt","w");
  write_out_file(outfile,my_board);
  fclose(outfile);

  cleanup_board(my_board);
  return 0;
}
