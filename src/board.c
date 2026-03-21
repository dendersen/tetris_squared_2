#include "board.h"
#include "sharedData.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//              0xRRGGBBAA
#define I_COLOR 0xFFFF00FF //yellow
#define O_COLOR 0x00FFFFFF //cyan
#define T_COLOR 0xFF00FFFF //purple
#define S_COLOR 0xFF0000FF //red
#define Z_COLOR 0x00FF00FF //green
#define J_COLOR 0xFF69B4FF //pink
#define L_COLOR 0xFF5F1FFF //orange

/**
 * generate a board with the given dimensions, with all blocks empty
 * the board is allocated on the heap, so it must be freed by the caller
 * the blocks are stored in a single array, with a header that points to the start of each column
 * this allows for easy access to the blocks as a 2D array, while still being cache friendly
 * and easy to loop over all pieces in a single loop when needed
 */
board_t* generateBoard(int width, int height){
  board_t* out = malloc(sizeof(board_t));
  out->width = width;
  out->height = height;
  out->blocks = malloc(sizeof(block_t) * width * height);
  for (int i = 0; i < width * height; i++){
    out->blocks[i].color = 0;
  }
  return out;
}

/**
 * create a piece with the given dimensions, with all blocks empty
 * the piece is allocated on the heap, so it must be freed by the caller
 */
Piece_t* createPiece(int width, int height){
  Piece_t* out = malloc(sizeof(Piece_t));
  out->width = width;
  out->height = height;
  out->x = 0;
  out->y = 0;
  out->blocks = malloc(sizeof(block_t) * width * height);
  for (int i = 0; i < width * height; i++){
    out->blocks[i].color = 0;
  }
  return out;
}

/**
 * generate a piece from a piece type, with the corresponding color and shape
 * the piece is allocated on the heap, so it must be freed by the caller
 */
Piece_t* generatePieceFromType(PieceEnum_t pieceType){
  Piece_t* out = NULL;
  switch (pieceType)
  {
  case I:
    out = createPiece(4, 1);
    for (int i = 0; i < 4; i++){
      getBlock(out,i,0).color = I_COLOR;
    }
    break;
  case O:
    out = createPiece(2, 2);
    for (int x = 0; x < 2; x++){
      for (int y = 0; y < 2; y++){
        getBlock(out,x,y).color = O_COLOR;
      }
    }
    break;
  case T:
    out = createPiece(3, 2);
    for (int x = 0; x < 3; x++){
      getBlock(out,x,1).color = T_COLOR;
    }
    getBlock(out,1,0).color = T_COLOR;
    break;
  case S:
    out = createPiece(3, 2);
    for (int x = 1; x < 3; x++){
      getBlock(out,x,0).color = S_COLOR;
    }
    for (int x = 0; x < 2; x++){
      getBlock(out,x,1).color = S_COLOR;
    }
    break;
  case Z:
    out = createPiece(3, 2);
    for (int x = 0; x < 2; x++){
      getBlock(out,x,0).color = Z_COLOR;
    }
    for (int x = 1; x < 3; x++){
      getBlock(out,x,1).color = Z_COLOR;
    }
    break;
  case J:
    out = createPiece(2, 3);
    for (int y = 0; y < 3; y++){
      getBlock(out,1,y).color = J_COLOR;
    }
    getBlock(out,0,2).color = J_COLOR;
    break;
  case L:
    out = createPiece(2, 3);
    for (int y = 0; y < 3; y++){
      getBlock(out,0,y).color = L_COLOR;
    }
    getBlock(out,1,2).color = L_COLOR;
    break;
  default:
    break;
  }
  return out;
}

/**
 * generate a random piece, with a random type
 * the piece is allocated on the heap, so it must be freed by the caller
 */
Piece_t* generateRandomPiece(){
  const int max = maxEnumType;
  int randomIndex = rand() % max;
  return generatePieceFromType((PieceEnum_t)randomIndex);
}

/**
 * deep copy a piece from a board
 * please destroy the board afterwards as it is not bound to the piece
 */
Piece_t* generatePieceFromBoard(board_t* board){
  Piece_t* out = malloc(sizeof(Piece_t));
  memcpy(out, board, sizeof(board_t));
  out->blocks = malloc(sizeof(block_t) * board->width * board->height);
  memcpy(out->blocks, board->blocks, sizeof(block_t) * board->width * board->height);
  return out;
}

/**
 * check if a piece collides with the board, meaning that 
 * there is at least one block in the piece that is not empty and 
 * overlaps with a non-empty block in the board
 */
bool checkPieceCollision(board_t* board, Piece_t* piece){
  for (int x = piece->x; x < piece->x + piece->width; x++){
    for (int y = piece->y; y < piece->y + piece->height; y++){
      if (getBlock(piece, x - piece->x, y - piece->y).A != 0 && getBlock(board, x, y).A != 0){
        return true;
      }
    }
  }
  return false;
}

/**
 * rotate a piece 90 degrees clockwise
 * the piece is modified in place, so the pointer is not changed
 */
void rotatePieceCounterClockWise(Piece_t* piece){
  Piece_t* rotated = createPiece(piece->height, piece->width);
  for (int x = 0; x < piece->width; x++){
    for (int y = 0; y < piece->height; y++){
      getBlock(rotated,rotated->width - y - 1,x).color = getBlock(piece, x, y).color;
    }
  }
  free(piece->blocks);
  piece->blocks = rotated->blocks;
  piece->width = rotated->width;
  piece->height = rotated->height;
  free(rotated);
}

/**
 * rotate a piece 90 degrees clockwise
 * the piece is modified in place, so the pointer is not changed
 */
void rotatePieceClockWise(Piece_t* piece){
  Piece_t* rotated = createPiece(piece->height, piece->width);
  for (int x = 0; x < piece->width; x++){
    for (int y = 0; y < piece->height; y++){
      getBlock(rotated, y, rotated->height - x - 1).color = getBlock(piece, x, y).color;
    }
  }
  free(piece->blocks);
  piece->blocks = rotated->blocks;
  piece->width = rotated->width;
  piece->height = rotated->height;
  free(rotated);
}

/**
 * place a piece on the board, modifying the board in place
 * returns true if there was an overlap, meaning that 
 * there is at least one block in the piece that is not empty and 
 * overlaps with a non-empty block in the board
 * returns false if
 * no collision was detected
 */
bool placePieceOnBoard(board_t* board, Piece_t* piece){
  bool overlap = false;
  for (int x = 0; x < piece->width; x++){
    for (int y = 0; y < piece->height; y++){
      if (getBlock(piece,x,y).A != 0){
        if (getBlock_p(board, piece, x, y).A != 0){
          overlap = true;
        }
        getBlock_p(board, piece, x, y).color = getBlock(piece, x, y).color;
      }
    }
  }
  return overlap;
}