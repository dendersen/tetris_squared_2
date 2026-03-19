#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>

struct termios orig_termios;

#define MAX_WAIT 10
#define KEY_BUFFER_SIZE 2
pthread_t* managingThread = NULL;
pthread_mutex_t* keyMutex = NULL;
char* keyBuffer = NULL;
bool pullBuffer = false;

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

char getKey() {
  unsigned char c = 0;
  if (read(STDIN_FILENO, &c, 1) == 1) {
    return c;
  }
  return -1;
}

int keyPullingThread(){
  pthread_mutex_lock(keyMutex);
  int waitTime = 0;
  while (pullBuffer){
    pthread_mutex_unlock(keyMutex);
    char key = getKey();
    if (key != -1){
      waitTime = 0;
      keyBuffer[0] = keyBuffer[1];
      keyBuffer[1] = key;
    }else if (waitTime < MAX_WAIT){
      waitTime += 1;
    } else if (waitTime >= MAX_WAIT){
      waitTime = 0;
      keyBuffer[0] = keyBuffer[1];
      keyBuffer[1] = '\0';
    }
    usleep(20000);
    pthread_mutex_lock(keyMutex);
  }
  return 0;
}

void enableKeyPull(){
  enableRawMode();
  keyMutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(keyMutex, NULL);
  keyBuffer = malloc(sizeof(char) * KEY_BUFFER_SIZE);
  managingThread = malloc(sizeof(pthread_t));
  pullBuffer = true;
  pthread_create(managingThread, NULL, (void* (*)(void*))keyPullingThread, NULL);
}