#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "client.h"

/* --- 	Programme pricipal ---*/
static void clt_prc(const char *address, int port, const char* pseudo) {
   SOCKET sock = 0; 
   char buffer[BUF_MAXSIZE];
   char name[BUF_MAXSIZE];
   fd_set rdfs;

   /*On créé le socket grace à notre sous programme*/
   sock = init_connection(address, port);
  
   /*On copie le login passé en argument */
   strcpy(name, pseudo);

   /*On contacte une première fois le serveur*/
   write_server(sock, name);

   /*On récupère sa réponse*/
   read_server(sock, buffer);

   strcpy(name, buffer);
   show_prompt(name);

   /*Boucle principale*/
   while(1) {
      /*On initialise la structure*/
      FD_ZERO(&rdfs);

      /*On ajoute STDIN (le clavier) à la structure*/
      FD_SET(STDIN_FILENO, &rdfs);

      /*On ajoute l'identifiant de socket à la structure*/
      FD_SET(sock, &rdfs);

      /*Si une erreur intervient à l'appel de select()*/
      if(select(sock + 1, &rdfs, NULL, NULL, NULL) == -1) {
         perror("select()");
         exit(errno);
      }

      /*Si un signal est envoyé par le clavier*/
      if(FD_ISSET(STDIN_FILENO, &rdfs)) {
         /*On récupère ce qui à été entré*/
         fgets(buffer, BUF_MAXSIZE - 1, stdin);
         if (process_command_line(sock, buffer) != 0)
            break;
            show_prompt(name);
            }
            /*Sinon, quelque chose arrive sur un socket observé par notre structure*/
            else if(FD_ISSET(sock, &rdfs)) {
               int n = read_server(sock, buffer);
               /*Si la chaine reçue par le serveur est nulle, le serveur est hors-ligne*/
               if(n == 0) {
                  printf("$Clt\tServeur hors-ligne !\n");
                  break;
               }
               puts(buffer);
         show_prompt(name);
         }
   }
   end_connection(sock);
}

/*Programme d'initialisation de la connection*/
static int init_connection(const char *address, int port) {
   SOCKET sock = 0;
   SOCKADDR_IN sin = { 0 };
   struct hostent *hostinfo;

   /*Création de la socket*/
   sock = socket(AF_INET, SOCK_STREAM, 0);

   /*Si la création du socket a échouée*/
   if(sock == INVALID_SOCKET) {
      perror("socket()");
      exit(errno);
   }

   /*On effectue une résolution d'adesse*/
   hostinfo = gethostbyname(address);

   /*Si la résolution de nom est impossible*/
   if (hostinfo == NULL) {
      fprintf (stderr, "Unknown host %s.\n", address);
      exit(EXIT_FAILURE);
   }

   /*Si le socket est valide, on initialise la structure sin*/
   sin.sin_addr 	= *(IN_ADDR *) hostinfo->h_addr;
   sin.sin_port 	= htons(port);
   sin.sin_family 	= AF_INET;

   /*On se connecte alors grâce à notre structure*/
   if(connect(sock,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
      perror("connect()");
      exit(errno);
   }

   /*On retourne le socket valide créé par la fonction*/
   return sock;
}

/*Sous programme d'affichage des messages reçus*/
void show_prompt(char *name) {
   printf("%s> ", name);
   fflush(stdout);
}

/*Sous programme de fermeture de connection*/
static void end_connection(int sock) {
   closesocket(sock);
}

/*Sous programme de reception des messages du serveur*/
static int read_server(SOCKET sock, char *buffer) {
   int n = 0;

   /*Si une erreur se produit à la réception*/
   if((n = recv(sock, buffer, BUF_MAXSIZE - 1, 0)) < 0) {
      perror("recv()");
      exit(errno);
   }

   /*On met le dernier caractère du buffer à NULL (\0)*/
   buffer[n] = 0;

   /*On retourne le nombre de caractères reçus*/
   return n;
}

/*Sous programme d'emission des messages au serveur*/
static void write_server(SOCKET sock, const char *buffer) {
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

/*Sous programme de gestion des commandes*/
int process_command_line(SOCKET socket, char *command) {
  struct t_message * msg = NULL;
  char* originalCommand = NULL;
  msg = malloc(sizeof(struct t_message));

  command[strlen(command) - 1] = '\0';
  if (strlen(command) == 0)
  {
	free(originalCommand);
	return 0;
  }

  originalCommand = malloc(sizeof(char) * strlen(command));
  strcpy(originalCommand, command);

  /*Si demande de terminaison du programme*/
  if (strlen(command) == 4 && strncmp(command, "quit", 4) == 0) {
	free(originalCommand);
	return 1;
  }

  /*Si demande de la liste des utilisateurs*/
  if (strlen(command) == 4 && strncmp(command, "list", 4) == 0) {
	msg->command = COMMAND_LIST;
	send_message(socket, msg);
	free(originalCommand);
	return 0;
  }

  char *token = NULL;
  char delimiter[] = " ";
  token = strtok(command, delimiter);

  /*Si demande d'envoi d'un message à un hôte*/
  if (strlen(token) == 4 && strncmp(token, "send", 4) == 0) {
	token = strtok(NULL, delimiter);

	/*Si la syntaxe n'est pas respectée*/
	if (token == NULL) {
		/*On averti l'utilisateur*/
		printf("Usage : send [Recipient] [Message]\n");
	}
	/*Sinon*/
	else {
		char *to = token;
		char *message = strstr(originalCommand, to);

		if (strlen(message) > 0)
			message += strlen(token);
		if (strlen(message) > 0)
			message += 1;
		else
			return 0;

		msg->command = COMMAND_SEND;
		msg->message = message; 
		msg->dest = to;

		/*On envoi le nouveau message au serveur*/
		send_message(socket, msg);

		/*Et on libère la mémoire*/
		free(originalCommand);

		return 0;
	}
  }

  msg->command = COMMAND_SENDALL;
  msg->message = originalCommand;
  msg->dest = NULL;
  send_message(socket, msg);
  free(originalCommand);
  return 0;
}

/*Sous programme d'envoi de message à un hôte*/
void send_message(SOCKET socket, struct t_message* msg) {
	int len = 0;
	char *strMessage = NULL;

	/*Si aucune commande n'est envoyée*/
	if (msg->command == NULL)
		return;
	len += strlen(msg->command);

	/*Si le message est adressé à un utilisateur spécifique*/
	if (msg->dest != NULL) {
		/*On spécifie la taille du message à envoyer +1 (\0)*/
		len += strlen(msg->dest) + 1;
	}
	else {	/*Sinon, le message est envoyé à tous (multi-diffusion)*/
		msg->dest = "*";
	}

	/*Si le message n'est pas nul*/
	if (msg->message != NULL) {
		/*On spécifie la taille du message à envoyer +1 (\0)*/
		len += strlen(msg->message) + 1;
	}
	else {	/*Sinon, le message est initialisé à "*" */
		msg->message = "*";
	}

	/*On réserve une quantité de mémoire égale à la taille total de notre message +1*/
	strMessage = malloc(sizeof(char) * len);

	/*Et on copie la commande dans l'espace ainsi réservé*/
	strcpy(strMessage, msg->command);

	/*Puis, on concatène toutes les informations du message (commande, detinataire et message), séparés par des espaces (" ") */
	strcat(strMessage, " ");

	/*Ajout du destinataire au message*/
	strcat(strMessage, msg->dest);
	strcat(strMessage, " ");

	/*Ajout du corps de message au message*/
	strcat(strMessage, msg->message);

	/*Et enfin, on evoye le message ainsi formé au serveur*/
	write_server(socket, strMessage);

	/*Pour terminer, on libère la mémoire réservé par notre malloc()*/
	free(strMessage);
}

/*Point d'entrée du programme*/
int main(int argc, char **argv) {
	struct hostent *struct_He;
	struct in_addr 	struct_Ad;

	int 		it_NoPort 		= 0;
	char 		sz_UsrLog[CLT_MAXSIZE]	= {0};
	char 		sz_IpAddr[IPA_MAXSIZE]	= {0};


	/*On teste si l'utilisateur a entré le bon nombre d'arguments*/
	if (argc != 4) {	/*Si il n'y a pas 4 arguments*/
		fprintf(stderr, "$Usage : %s [Host] [Port] [Login]\n", argv[0]);
		return EXIT_FAILURE;
	}
	else {	/*Sinon*/
		fprintf(stdout, "$Clt>\t--- msgr v0.1 [:%d]---\n",atoi(argv[2]));
	}

	/*On récupère le nom d'hôte*/
	struct_He = gethostbyname (argv[1]);

	/*On récupère le numéro de port*/
	it_NoPort = atoi(argv[2]);

	/*On récupère le login*/
	sprintf(sz_UsrLog,"%s",argv[3]);

	/*On affiche le nom réel de l'hôte*/
	fprintf(stdout,"$Clt>\tName: \t%s\n", struct_He->h_name);
	
	/*On affiche la résolution de nom*/
	while (*struct_He->h_aliases)
		printf("$Clt>\tAlias: \t%s\n", *struct_He->h_aliases++);

	bcopy(*struct_He->h_addr_list++, (char *) &struct_Ad, sizeof(struct_Ad));
	printf("$Clt>\tAddr:\t%s\n", inet_ntoa(struct_Ad));
	
	/*On copie l'adresse ip dans la variable*/
	sprintf(sz_IpAddr,"%s",inet_ntoa(struct_Ad));

	/*Puis, on appelle le programme principal*/
	clt_prc(sz_IpAddr, it_NoPort, sz_UsrLog);

	return EXIT_SUCCESS;
}

