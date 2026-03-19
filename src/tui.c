#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include "board.h"

#define moveCursor(x, y) printf("\033[%u;%uH", ((y)+1), ((x)+1))
#define clearScreen() printf("\033[2J")
#define alpha(c,a) ((uint8_t)((uint16_t)(c * a) / 255))
#define _color_(r, g, b, extend) "\033[38;2;%u;%u;%um" extend, (r), (g), (b)
#define setColorA(r, g, b, a) printf(_color_(alpha(r,a), alpha(g,a), alpha(b,a), ""))
#define setColor(r, g, b) printf(_color_(r, g, b,""))
#define printColor(text, r, g, b) printf(_color_(r, g, b,"%s"), (text))
#define printColorA(text, r, g, b, a) printf(_color_(alpha(r,a), alpha(g,a), alpha(b,a), "%s"), (text))
#define resetColor() printf("\033[0m")
#define blockChar "▇"
#define shadeChar "▓"
#define emptyChar " "
#define yOffset 2
#define xOffset 1

void displayBoardAndPiece(board_t* board, Piece_t* piece, char* title){
  clearScreen();
  moveCursor(0, 0);
  printf("%s", title);
  for(int x = 0; x < board->width + xOffset * 2 + 1; x++){
    moveCursor(x, yOffset - 1);
    printf("-");
    moveCursor(x, board->height + yOffset);
    printf("-");
  }
  for(int y = 0; y < board->height; y++){
    moveCursor(0, y + yOffset);
    printf("|");
    moveCursor(board->width + xOffset * 2, y + yOffset);
    printf("|");
  }
  
  for (int i = 0; i < board->width * board->height; i++){
    int x = i % board->width;
    int y = i / board->width;
    moveCursor(x + xOffset, y + yOffset);
    if (board->blocks[i].A != 0){
      if (board->blocks[i].A < 255){
        setColorA(board->blocks[i].R, board->blocks[i].G, board->blocks[i].B, board->blocks[i].A);
      } else {
        setColor(board->blocks[i].R, board->blocks[i].G, board->blocks[i].B);
      }
      printf(blockChar);
    }else{
      printf(emptyChar);
    }
  }
  
  for (int x = 0; piece != NULL && x < piece->width; x++){
    for (int y = 0; y < piece->height; y++){
      block_t block = getBlock(piece,x,y);
      if (block.A != 0){
        if (block.A < 255){
          setColorA(block.R, block.G, block.B, block.A);
        } else {
          setColor(block.R, block.G, block.B);
        }
        moveCursor(x + xOffset + piece->x, y + yOffset + piece->y);
        printf(shadeChar);
      }
    }
  }
  resetColor();
}

void setCursorAtEndOfScreen(board_t* board){
  moveCursor(0, board->height + yOffset  * 3);
}