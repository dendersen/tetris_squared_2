#pragma once
#include <stdint.h>

/**
 * color is all colors in a single number
 * R, G, B, A are the individual color channels
 * if A is 0, the block is considered empty
 */
typedef union block_data {
  uint32_t color;
  struct
  {
    uint8_t R, G, B, A;
  };
} block_t;

/**
 * blocks is a 2D array of block_t, with dimensions width and height
 * width and height are the dimensions of the board
 * x and y are the position of the piece on the board
 * the blocks are indexed as blocks[x][y], with (0, 0) being the top left corner of the board
 */
typedef struct board_data {
  block_t* blocks;
  int width;
  int height;
} board_t;

/**
 * pieces are represented as a 2D array of block_t, with dimensions width and height
 * width and height are the dimensions of the piece
 * x and y are the position of the piece on the board
 * the blocks are indexed as blocks[x][y], with (0, 0) being the top left corner of the piece
 *
 * it is very intentional that the piece is a board with a position
 * as it allows for easy convertion between the piece and the board
 */
typedef struct piece_data {
  block_t* blocks;
  int width, height;
  int x, y; // position of the piece on the board, (0, 0) is the top left corner of the board
} Piece_t;