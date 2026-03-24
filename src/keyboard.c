#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include "keyboard.h"

struct termios orig_termios;

#define MAX_WAIT 30
#define KEY_BUFFER_SIZE 10
#define KEY_READ_SIZE 2
#define KEY_READ_INTERVAL 100
pthread_t* managingThread = NULL;
typedef struct keyData{
  char keys[KEY_BUFFER_SIZE];
  int nextIndex;
} keyBuffer_t;

//controlls pullBuffer and keyBuffer
pthread_mutex_t* keyMutex = NULL;
bool pullBuffer = false;//false to disable pulling, true to enable pulling
keyBuffer_t* keyBuffer = NULL;

int tetrisEncoding[NONE_operation][keysPerOperation] = {0};

void setKeyMapping(int operation, int keyIndex, int key){
  if (operation < 0 || operation >= NONE_operation || keyIndex < 0 || keyIndex >= keysPerOperation || key < 0 || key > NONE_key){
    return;
  }
  tetrisEncoding[operation][keyIndex] = key;
}

void setDefaultKeyMapping() {
  for(int i = 0; i < NONE_operation; i++){
    for(int j = 0; j < keysPerOperation; j++){
      tetrisEncoding[i][j] = NONE_key;
    }
  }
  setKeyMapping(moveLeft, 0, left);
  setKeyMapping(moveLeft, 1, a);
  setKeyMapping(moveRight, 0, right);
  setKeyMapping(moveRight, 1, d);
  setKeyMapping(moveDown, 0, down);
  setKeyMapping(moveDown, 1, s);
  setKeyMapping(dropPiece, 0, up);
  setKeyMapping(dropPiece, 1, w);
  setKeyMapping(rotateClockwise, 0, squareBracketOpen);
  setKeyMapping(rotateClockwise, 1, comma);
  setKeyMapping(rotateCounterClockwise, 0, squareBracketClose);
  setKeyMapping(rotateCounterClockwise, 1, period);
  setKeyMapping(holdPiece, 0, tab);
  setKeyMapping(holdPiece, 1, d);
}

#if KEY_BUFFER_SIZE <= 0
#error "KEY_BUFFER_SIZE must be greater than 0"
#elif KEY_READ_SIZE <= 0
#error "KEY_READ_SIZE must be greater than 0"
#elif KEY_READ_INTERVAL <= 0
#error "KEY_READ_INTERVAL must be greater than 0"
#elif KEY_BUFFER_SIZE < KEY_READ_SIZE
#error "KEY_BUFFER_SIZE must be greater than or equal to KEY_READ_SIZE"
#endif

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void makeSpaceInKeyBuffer(int size){
  int missingSpace = size - (KEY_BUFFER_SIZE - keyBuffer->nextIndex);
  if (missingSpace <= 0){
    return;
  }
  for (int i = 0; i < KEY_BUFFER_SIZE - missingSpace; i++){
    keyBuffer->keys[i] = keyBuffer->keys[i + missingSpace];
  }
  for (int i = KEY_BUFFER_SIZE - missingSpace; i < KEY_BUFFER_SIZE; i++){
    keyBuffer->keys[i] = '\0';
  }
}

void* keyPullingThread(){
  pthread_mutex_lock(keyMutex);
  while (pullBuffer){
    pthread_mutex_unlock(keyMutex);

    usleep(KEY_READ_INTERVAL);
    uint8_t keys[KEY_BUFFER_SIZE];
    int ret = read(STDIN_FILENO, &keys, sizeof(keys));
    if (ret <= 0){
      continue;
    }
    pthread_mutex_lock(keyMutex);
    if (keyBuffer->nextIndex + ret > KEY_BUFFER_SIZE){
      int missingSpace = ret - (KEY_BUFFER_SIZE - keyBuffer->nextIndex);
      for (int i = 0; i < KEY_BUFFER_SIZE - missingSpace; i++){
        keyBuffer->keys[i] = keyBuffer->keys[i + missingSpace];
      }
      for (int i = KEY_BUFFER_SIZE - missingSpace; i < KEY_BUFFER_SIZE; i++){
        keyBuffer->keys[i] = '\0';
      }
      keyBuffer->nextIndex -= missingSpace;
    }
    for (int k = 0; k < ret && keyBuffer->nextIndex < KEY_BUFFER_SIZE; k++, keyBuffer->nextIndex++){
      keyBuffer->keys[keyBuffer->nextIndex] = keys[k];
    }
  }
  pthread_mutex_unlock(keyMutex);
  disableRawMode();
  free(keyBuffer);
  keyBuffer = NULL;
  return NULL;
}

void enableKeyPull(){
  enableRawMode();
  keyMutex = malloc(sizeof(*keyMutex));
  pthread_mutex_init(keyMutex, NULL);
  keyBuffer = malloc(sizeof(*keyBuffer));
  managingThread = malloc(sizeof(*managingThread));
  pullBuffer = true;
  pthread_create(managingThread, NULL, (void* (*)(void*))keyPullingThread, NULL);
}

int getNextKeys(uint8_t* buffer, int size){
  pthread_mutex_lock(keyMutex);
  int keysToRead = size < keyBuffer->nextIndex ? size : keyBuffer->nextIndex;
  memcpy(buffer, keyBuffer->keys, keysToRead);
  keyBuffer->nextIndex -= keysToRead;
  for (int i = 0; i < keyBuffer->nextIndex; i++){
    keyBuffer->keys[i] = keyBuffer->keys[i + keysToRead];
  }
  for (int i = keyBuffer->nextIndex; i < KEY_BUFFER_SIZE; i++){
    keyBuffer->keys[i] = '\0';
  }
  pthread_mutex_unlock(keyMutex);
  return keysToRead;
}

/**
 * Returns the number of bytes used in the rawKeyDataBuff for the ESC sequence. 
 * If the first byte is not an ESC sequence, returns 0 and does not modify customKeyBuff.
 * otherwise does not count the ESC byte in the used bytes and sets customKeyBuff to the corresponding customKey value for the ESC sequence.
 */
uint8_t processESC(uint8_t* customKeyBuff, uint8_t* rawKeyDataBuff, int size){
  if(customKeyBuff == NULL || rawKeyDataBuff == NULL || size <= 0){
    return 0;
  }
  uint8_t used = 0;
  if(rawKeyDataBuff[0] == 0x1B){
    if (size > 1 && rawKeyDataBuff[1] == '['){
      used++;
      if (size > 2){
        switch (rawKeyDataBuff[2]) {
        case 'A':
          *customKeyBuff = up;
          break;
        case 'B':
          *customKeyBuff = down;
          break;
        case 'C':
          *customKeyBuff = right;
          break;
        case 'D':
          *customKeyBuff = left;
          break;
        default:
          return 1;
          break;
        }
        used++;
      }
    }
  }
  return used;
}

int rawKeyToCustomKey(uint8_t* rawKeyDataBuff, uint8_t* customKeyBuff, int size){
  if (size <= 0){
    return 0;
  }
  int written = 0;
  for (int i = 0; i < size; i++){
    if (rawKeyDataBuff[i] >= 'a' && rawKeyDataBuff[i] <= 'z'){
      customKeyBuff[i] = rawKeyDataBuff[i] - 'a' + a;
      written++;
      continue;
    }
    if (rawKeyDataBuff[i] >= 'A' && rawKeyDataBuff[i] <= 'Z'){
      customKeyBuff[i] = rawKeyDataBuff[i] - 'A' + a;
      written++; 
      continue;
    }
    switch (rawKeyDataBuff[i]) {
    case ' ':
      customKeyBuff[i] = space;
      written++;
      break;
    case '-':
      customKeyBuff[i] = dash;
      written++;
      break;
    case ',':
      customKeyBuff[i] = comma;
      written++;
      break;
    case '.':
      customKeyBuff[i] = period;
      written++;
      break;
    case '/':
      customKeyBuff[i] = slash;
      written++;
      break;
    case ';':
      customKeyBuff[i] = semicolon;
      written++;
      break;
    case '"':
      customKeyBuff[i] = quote;
      written++;
      break;
    case '|':
      customKeyBuff[i] = vert;
      written++;
      break;
    case '\\':
      customKeyBuff[i] = backslash;
      written++;
      break;
    case 0x1B:
      int temp = processESC(&customKeyBuff[i],&rawKeyDataBuff[i],size - i);
      if (temp != 0)      {
        written++;
      }
      i += temp - 1;
      break;
    default:
      customKeyBuff[i] = NONE_key;
      break;
    }
  }
  return written;
}

int getOperationArray(uint8_t* keys, uint8_t buffSize){
  uint8_t rawKeyData[KEY_BUFFER_SIZE + 1];
  getNextKeys(rawKeyData, sizeof(rawKeyData));
  int keysFound = rawKeyToCustomKey(keys, &rawKeyData, sizeof(rawKeyData) / sizeof(*rawKeyData));
  //TODO customKeyToOperation()
}
