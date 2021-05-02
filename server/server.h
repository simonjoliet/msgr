#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


#define MAX_CLIENTS	500


#include "client.h"
#include "list.h"

#include "../message.h"

/*Déclaration des fonctions*/
void srv_prc				(int port);
int  init_connection			(int port);
void fin_connection			(int sock);
int  lit_client				(SOCKET sock, char *buffer);
void write_client			(SOCKET sock, const char *buffer);
void process_message			(struct t_list *clients, struct t_client *client, struct t_message *message);
void send_message_to_all_clients	(struct t_list *clients, struct t_client* client, const char *buffer,char from_server);
void efface_client			(struct t_list *clients, int to_remove);
void kick_tous				(struct t_list *clients);
void kick_client			(struct t_list *clients, char* clientName);
void clear_clients			(struct t_list *clients);
void notify_client			(struct t_list *clients, struct t_client* c, int connected);

#endif

