#ifndef __connexionthread__
#define __connexionthread__

#define RCVSIZE 1024
void *connection_handler(void *arg);
char* ackNum(int i);

#endif
