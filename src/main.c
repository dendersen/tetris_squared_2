#include "sharedData.h"
#include "board.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
  setvbuf(stdout, NULL, _IONBF, 0);
  board_t* board = generateBoard(10, 20);
  getBlock(board,0,0).color = 0xFF0000FF;
  getBlock(board,1,0).color = 0x00FF00FF;
  getBlock(board,1,1).color = 0x0000FFFF;
  getBlock(board,0,1).color = 0x000000FF;
  displayBoardAndPiece(board, NULL, "Tetris Squared");
  setCursorAtEndOfScreen(board);
  return 0;
}