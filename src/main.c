#define _GNU_SOURCE
#include "sharedData.h"
#include "board.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "keyboard.h"
#include <unistd.h>

int main(){
  setvbuf(stdout, NULL, _IONBF, 0);
  // srand((unsigned int)time(NULL));
  // Piece_t* piece = generateRandomPiece();
  // board_t* board = generateBoard(10, 20);
  // rotatePieceCounterClockWise(piece);
  // placePieceOnBoard(board, piece);
  // displayBoardAndPiece(board, NULL, "Tetris Squared");
  // setCursorAtEndOfScreen(board);
  enableKeyPull();
  while (1){
    uint8_t keys[10];
    int size = getNextKeys(keys, sizeof(keys));
    for (int i = 0; i < size; i++){
      printf("%c", keys[i]);
    }
    usleep(10000); // Sleep for 10 milliseconds to prevent busy waiting
  }
  return 0;
}
/**
 * left: 91,27,68
 * right: 91,27,67
 * down: 91,27,66
 * up: 91,27,65
 * 
 * a: 97
 * d: 100
 * w: 119
 * s: 115
 */