#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "command.h"

int process_command_line(struct t_list* clients) {
	char command[255];
	fgets(command, 254, stdin);
	command[strlen(command) - 1] = '\0';
	if (strlen(command) == 0) {
		show_prompt();
		return 0;
	}

	/*Si l'utilisateur désire quitter*/
	if (strlen(command) == 1 && strncmp(command, "quit", 1) == 0)
	return 1;

	if (strlen(command) == 1 && strncmp(command, "exit", 1) == 0)
	return 1;

	/*Si l'utilisateur désire obtenir la liste des clients connectés*/
	if (strlen(command) == 4 && strncmp(command, "list", 4) == 0) {
		display_connected_clients(clients);
		show_prompt();
		return 0;
	}
	if (strlen(command) == 7 && strncmp(command, "kickall", 7) == 0) {
		kick_tous(clients);
		show_prompt();
		return 0;
	}

	char *token = NULL;
	char delimiter[] = " ";
	token = strtok(command, delimiter);

	if (strlen(token) == 4 && strncmp(token, "kick", 4) == 0) {
		while (token != NULL) {
			token = strtok(NULL, delimiter);
			if (token != NULL)
				kick_client(clients, token);
		}
		show_prompt();
		return 0;
	}

	if (strlen(token) == 4 && strncmp(token, "send", 4) == 0) {
		token = strtok(NULL, delimiter);
		if (token != NULL) {
			struct t_client* client = get_client(clients, token);

			/*On vérifie si le client est connu*/
			if (client == NULL) {
				printf("No client connected.\"%s\"\n", token);
			}
			else {
				char message[255];
				printf("\nMessage : ");
				fflush(stdin);
				fgets(message, 254, stdin);
				write_client(client->sock, message);
			}
	
		}
		show_prompt();
		return 0;  
	}

	if (strlen(token) == 7 && strncmp(token, "sendall", 7) == 0) {
		char message[255];
		if (clients->count == 0) {
			printf("No client connected.\n");
		}
		else {
			printf("\nMessage : ");
			fflush(stdin);
			fgets(message, 254, stdin);
			send_message_to_all_clients(clients, clients->first->client, message, 1); 
		}
		show_prompt();
		return 0;
 	}
	show_prompt();
	return 0;
}


/*Sous programme de notification d'écoute du clavier*/
void show_prompt() {
	printf("> ");
	fflush(stdout);
}

/*Sous programme d'affichage de la liste des clients connectés*/
void display_connected_clients(struct t_list* clients) {
	if (clients->count > 0) {
		printf("Client(s) connected :\n");
		struct t_list_item* current = clients->first;		
		while (current != NULL)
		{
			printf("%s\n", current->client->name);
			current = current->next;
		}
	}
	else {
		printf("No client connected.\n");
	}
}
