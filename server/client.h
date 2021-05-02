#ifndef CLIENT_H_
#define CLIENT_H_

#define BUF_SIZE	1024

typedef int SOCKET;

struct t_client {
   SOCKET sock;
   char name[BUF_SIZE];
};

#endif
