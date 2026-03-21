#pragma once
#include <stdint.h>

/**
 * color is all colors in a single number
 * R, G, B, A are the individual color channels
 * if A is 0, the block is considered empty
 * color = 0xRRGGBBAA, 
 * with RR being the red channel, 
 * GG being the green channel, 
 * BB being the blue channel,
 * AA being the alpha channel
 */
typedef union block_data {
  uint32_t color;
  struct
  {
    uint8_t A, B, G, R;
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
  int width, height;
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


/**
 * get a block from a piece or board
 * relative to own coordinate system
 */
#define getBlock(piece_board, x, y) ((piece_board)->blocks[(x) + (y) * (piece_board)->width])

/**
 * get the block of a board 
 * relative to the piece
 * (0, 0) is the top left corner of the piece
 */
#define getBlock_p(board, piece, x, y) getBlock((board), (x) + (piece)->x, (y) + (piece)->y)

/**
 * x is relative to the piece, so (0, 0) is the top left corner of the piece
 * the block is considered empty if its alpha channel is 0
 */
#define comparePieceToBoard(piece, board, x, y) ((getBlock((piece), (x), (y)).A != 0) && (getBlock_p((board), (piece), (x), (y)).A != 0))
