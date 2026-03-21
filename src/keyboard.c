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

struct termios orig_termios;

#define MAX_WAIT 30
#define KEY_BUFFER_SIZE 4
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