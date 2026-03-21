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
    uint8_t keys[10] = {0};
    int size = getNextKeys(keys, sizeof(keys) - 1);
    if(size > 0){
      for (int i = 0; i < size; i++){
        printf("%4d %c\n", keys[i],keys[i]);
      }
      printf("----");
      for (int i = 0; i < size; i++){
        switch(keys[i]){
          case 27: //ESC
            printf("esc");
            while((keys[i] <= '0' || keys[i] > '9') && i >= size - 1){
              printf("%c", keys[i]);
              i++;
            }
            printf("%c", keys[i]);
          break;
          default:
            printf("%c", keys[i]);
          break;
        }
      }
      printf("\n");
    }
    usleep(10000); // Sleep for 10 milliseconds to prevent busy waiting
  }
  return 0;
}