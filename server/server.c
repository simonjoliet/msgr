#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "command.h"
#include "list.h"

/*Programme principal*/
void srv_prc(int port) {
	SOCKET sock = 0;
	char buffer[BUF_SIZE];
	int max = 0;

	struct t_list* clients;
	clients = malloc(sizeof(clients));

	/*On crée une nouvelle connection en utilisant le numéro de port passé en paramètre*/
	sock = init_connection(port);
	max = sock;

	fd_set rdfs;
	printf("> ");
	fflush(stdout);

	/*Boucle principale*/
	while(1) {
	/*On initialise la structure*/
	FD_ZERO(&rdfs);

	/*On ajoute STDIN (clavier) à la structure de descripteurs de socket*/
	FD_SET(STDIN_FILENO, &rdfs);

	/*On ojoute le socket du serveur à la structure de descripteurs de socket*/
	FD_SET(sock, &rdfs);

	/*On ajoute les sockets de chaque clients connectés*/
	struct t_list_item* current = clients->first;      
	while (current != NULL) {
	FD_SET(current->client->sock, &rdfs);
	current = current->next;
	}

	/*Si une erreur intervient à l'appel de select()*/
	if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1) {
		perror("select()");
		exit(errno);
	}

	/*Si un signal est envoyé par le clavier*/
	if(FD_ISSET(STDIN_FILENO, &rdfs)) {
		if (process_command_line(clients) != 0)
		break;
	}
	else if(FD_ISSET(sock, &rdfs)) {
		/*Si quelqu'un se connecte sur notre premier socket, c'est forcément un nouveau client*/
		SOCKADDR_IN csin = { 0 };
		size_t sinsize = sizeof csin;
		int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);

		/*Si le socket est valide, on accepte le client*/
		if(csock == SOCKET_ERROR) {
			perror("accept()");
			continue;
		}

		/*Une fois accepté, le client nous retourne son nom*/
		if(lit_client(csock, buffer) == -1) {
			/*Si la fonction échoue, on quitte la boucle*/
			continue;
		}


		int pseudoSuffix = contains_name(clients, buffer);

		/*On vérifie qu'aucun client ne porte*/
		if (pseudoSuffix > 0) {
			char pseudo[255];
			sprintf(pseudo, "%s%d", buffer, pseudoSuffix);
			strcpy(buffer, pseudo);		
		}

		write_client(csock, buffer);

		/*On calcule la nouvelle taille du déscripteur de socket*/
		max = csock > max ? csock : max;

		FD_SET(csock, &rdfs);

		struct t_client* c = NULL;
		c = malloc(sizeof(struct t_client));
		c->sock = csock;
		strncpy(c->name, buffer, BUF_SIZE - 1);
		add_item(clients, c);
		notify_client(clients, c, 1);
	}
	else {
		struct t_list_item* current = clients->first;
		while(current != NULL) {
			struct t_client* client = current->client;

			/*Un client connu nous contacte*/
			if(FD_ISSET(client->sock, &rdfs)) {
				int c = lit_client(client->sock, buffer);

				/*Si le message reçu de client est nul, celui-ci s'est déconnecté*/
				if(c == 0) {
					/*Alors on ferme son socket*/
					close(client->sock);
					notify_client(clients, client, 0);

					/*Et on retire le client de notre liste*/
					remove_item(clients, client);
				}
				else {
					/*Sinon, le message du client n'est pas nul, il veut donc nous transmettre quelque chose*/
					struct t_message* msg = NULL;
					char* originalMessage = NULL;
					char *token = NULL;
					char delimiter[] = " ";

					originalMessage = malloc(sizeof(char) * strlen(buffer));
					strcpy(originalMessage, buffer);
					printf("original = %s\n", originalMessage);
					msg = malloc(sizeof(struct t_message));

					token = strtok(buffer, delimiter);
					if (token != NULL) {
						msg->command = malloc(sizeof(char) * strlen(token));
						strcpy(msg->command, token);

						printf("%s\n",msg->command);
						token = strtok(NULL, delimiter);
						if (token != NULL) {
							char *message = strstr(originalMessage, token);

							msg->dest = malloc(sizeof(char) * strlen(token));
							strcpy(msg->dest, token);
							//printf("%s\n", message);
							if (strlen(message) > 0)
								message += strlen(token);
							if (strlen(message) > 0)
								message += 1;
							
							msg->message = malloc(sizeof(char) * strlen(message));
							strcpy(msg->message, message);
						}

						process_message(clients, client, msg);
			  		}
					//send_message_to_all_clients(clients, client, buffer, 0);
					//free(originalMessage);
					//free(msg->command);
					//free(msg->dest);
					//free(msg->message);
					//free(msg);
				}
				break;
			}
			current = current->next;
		}         
	}
}

	/*Lorsque l'on termine la boucle, on libère les ressources*/
	clear_clients(clients);
	clear_list(clients);
	free(clients);

	/*Et on termine la connection*/
	fin_connection(sock);
}

/*Sous programme de traitement des messages*/
void process_message(struct t_list *clients, struct t_client *client, struct t_message *message) {
	if (strlen(message->command) == strlen(COMMAND_SENDALL) && strncmp(message->command, COMMAND_SENDALL, strlen(COMMAND_SENDALL)) == 0) {
	send_message_to_all_clients(clients, client, message->message, 0);
	return ;	
	}
	if (strlen(message->command) == strlen(COMMAND_SEND) && strncmp(message->command, COMMAND_SEND, strlen(COMMAND_SEND)) == 0) {
		/*Si le message est addressé à un destinataire*/
		if(message->dest!=NULL) {
			struct t_client* dest=NULL;
			/*On récupère le destinataire dans le liste des clients connectés*/
			dest=get_client(clients,message->dest);

			/*Si le destinataire est connu*/
			if(dest!=NULL)
			{
				write_client(dest->sock, client->name);
				write_client(dest->sock, ">");
				/*On lui transmet ce message*/
				write_client(dest->sock,message->message);
			}
		}
		return ;	
	}	

	if (strlen(message->command) == strlen(COMMAND_LIST) && strncmp(message->command, COMMAND_LIST, strlen(COMMAND_LIST)) == 0) {
		struct t_list_item * current = clients->first;
		while (current != NULL) {
			write_client(client->sock, current->client->name);
			current = current->next;
			if (current != NULL)
 				write_client(client->sock, ",");
		}
	}
}


/*Sous programme d'effacement des clients*/
void clear_clients(struct t_list *clients) {
   struct t_list_item* current = clients->first;
   while (current != NULL) {
      	close(current->client->sock);
	current = current->next;
   }
}


/*Sous programme de libération de la mémoire*/
void kick_tous(struct t_list *clients) {
	struct t_list_item* current = clients->first;
	while (current != NULL) {
		struct t_client* c = current->client;
		close(c->sock);
		current = current->next;
	}
	clear_list(clients);
}

/*Sous programme de suppression d'un client*/
void kick_client(struct t_list *clients, char* clientName) {
	struct t_list_item* current = clients->first;
   	while (current != NULL) {
		struct t_client* c = current->client;
		if (strncmp(c->name, clientName, strlen(c->name)) == 0) {
			close(c->sock);
			remove_item(clients, c);
			break;
		}
		current = current->next;
	}
}

/*Sous programme de notification d'état des client*/
void notify_client(struct t_list *clients, struct t_client* c, int connected) {
	char *message = malloc(sizeof(char) * strlen(c->name));
	strcpy(message, c->name);
	if (connected == 1) {
		/*Si un client se connecte*/		
		message = strcat(message, " connected.\n");
	}
	else {
		/*Si un client se déconnecte*/	
		message = strcat(message, " disconnected.\n");
	}

	printf("%s", message);
	send_message_to_all_clients(clients, c, message, 1);
}

/*Sous programme de multi-diffusion*/
void send_message_to_all_clients(struct t_list *clients, struct t_client* sender, const char *buffer, char from_server) {
   	char message[BUF_SIZE];
   	message[0] = 0;
	struct t_list_item* current = clients->first;
	
	while (current != NULL) {
		struct t_client* client = current->client;

		/*On n'envoie pas le message à l'expéditeur initial*/
	      	if(sender->sock != client->sock) {
				if(from_server == 0) {
						strncpy(message, sender->name, BUF_SIZE - 1);
						strncat(message, " : ", sizeof message - strlen(message) - 1);
				}
				strncat(message, buffer, sizeof message - strlen(message) - 1);
				write_client(client->sock, message);
	      	}
		current = current->next;
	}
}


/*Sous programme d'initialisation de la connection*/
int init_connection(int port) {
   SOCKET sock = 0;
   SOCKADDR_IN sin = { 0 };

   /*Création de la socket*/
   sock = socket(AF_INET, SOCK_STREAM, 0);

   /*Si la création du socket a échouée*/
   if(sock == INVALID_SOCKET) {
      perror("socket()");
      exit(errno);
   }

   /*Si le socket est valide, on initialise la structure sin*/
   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(port);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
      perror("bind()");
      exit(errno);
   }

   /*On écoute le port dans la limite du nombre de clients authorisés*/
   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
      perror("listen()");
      exit(errno);
   }

   /*On retourne le socket ainsi créé*/
   return sock;
}

/*Sous programme de fermeture d'un socket*/
void fin_connection(int sock) {
   close(sock);
}

/*Sous programme de lecture d'un message client*/
int lit_client(SOCKET sock, char *buffer) {
   int n = 0;

   /*Si une erreur se produit à la réception*/
   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   /*On met le dernier caractère du buffer à NULL (\0)*/
   buffer[n] = 0;

   /*On retourne le nombre de caractères reçus*/
   return n;
}

/*Sous programme d'emission des messages aux clients*/
void write_client(SOCKET sock, const char *buffer) {
   if(send(sock, buffer, strlen(buffer), 0) < 0) {
      perror("send()");
      exit(errno);
   }
}


/*Point d'entrée*/
int main(int argc, char **argv) {
	if (argc != 2) {	/*Si il n'y a pas 1 arguments*/
		fprintf(stderr, "$Usage : %s [Port]\n", argv[0]);
		return EXIT_FAILURE;
	}
	else {	/*Sinon*/
		fprintf(stdout, "$Srv>\t--- msgr v0.1 [:%d]---\n",atoi(argv[1]));

		/*On appelle le programme principal*/
		srv_prc(atoi(argv[1]));
	  	return EXIT_SUCCESS;
	}
}