#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define CRLF	 "\n"
//#define PORT	 9876

#define BUF_MAXSIZE 1024
#define CLT_MAXSIZE 31
#define IPA_MAXSIZE 25

#include "../message.h"

static void clt_prc		(const char *address, int port, const char* pseudo);
static int init_connection	(const char *address, int port);
static void end_connection	(int sock);
void show_prompt		(char *name);
void send_message		(SOCKET socket, struct t_message* msg);
static int read_server		(SOCKET sock, char *buffer);
static void write_server	(SOCKET sock, const char *buffer);
int process_command_line	(SOCKET socket, char *command);

#endif