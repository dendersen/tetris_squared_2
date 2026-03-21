#include "sharedData.h"
#include <stdlib.h>
#include <stdbool.h>
#define boardIToX(i, board) ((i) % (board)->width)
#define boardIToY(i, board) ((i) / (board)->width)

Piece_t* generateRandomPiece();
Piece_t* generatePieceFromBoard(board_t* board);
board_t* generateBoard(int width, int height);
void rotatePieceClockWise(Piece_t* piece);
void rotatePieceCounterClockWise(Piece_t* piece);
bool placePieceOnBoard(board_t* board, Piece_t* piece);
bool checkPieceCollision(board_t* board, Piece_t* piece);

typedef enum pieceType {
  I, O, T, S, Z, J, L,
  maxEnumType
} PieceEnum_t;