#include "sharedData.h"
#include <stdlib.h>

/**
 * get a block from a piece or board
 * relative to own coordinate system
 */
#define getBlock(piece_board, x, y) piece_board->blocks[x + y * piece_board->width]

/**
 * get the block of a board 
 * relative to the piece
 * (0, 0) is the top left corner of the piece
 */
#define getBlock_p(board, piece, x, y) getBlock(board, x + piece->x, y + piece->y)

/**
 * x is relative to the piece, so (0, 0) is the top left corner of the piece
 * the block is considered empty if its alpha channel is 0
 */
#define comparePieceToBoard(piece, board, x, y) (getBlock(piece, x, y).A != 0 && getBlock_p(board, piece, x, y).A != 0)

Piece_t* generateRandomPiece();
Piece_t* generatePieceFromBoard(board_t* board);
board_t* generateBoard(int width, int height);

typedef enum pieceType {
  I, O, T, S, Z, J, L,
  maxEnumType
} PieceEnum_t;