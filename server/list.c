#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "list.h"

/*Sous programme d'ajout de client*/
void add_item(struct t_list* list, struct t_client* client) {
	struct t_list_item* item = NULL;
	item = malloc(sizeof(struct t_list_item));
	item->client = client;
	item->next = NULL;
	list->count++;

	/*Dans le cas où l'on ajoute un premier client*/
	if (list->first == NULL) {
		list->first = item;
	}
	else {
		struct t_list_item* current = list->first;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = item;
	}
}

/*Sous programme de supression de client*/
void remove_item(struct t_list* list, struct t_client* client) {
	if (list->first == NULL) {
		return;
	}
	else {
		/*Si le client est le premier de la liste*/
		if(list->first->client==client) {
			struct t_list_item* todelete=list->first;
			list->first=todelete->next;
			free(todelete->client);
			free(todelete);
		}
		else {
			struct t_list_item* current = list->first;
			struct t_list_item* prev = NULL;
			while (current != NULL) {	/*S'il s'agit d'un client en particulier*/
				if(current->client==client)
				{
					prev->next=current->next;
					free(current->client);
					free(current);
					break;
				}
				prev = current;				
				current = current->next;
			}	
		}
	}
}

/*Sous programme de remise à zéro de la liste*/
void clear_list(struct t_list* list) {
	if (list == NULL) {
		return;
	}
	else {
		struct t_list_item* current = list->first;
		struct t_list_item* next = NULL;

		/*On efface tous les clients*/		
		while (current!=NULL) {
			next = current->next;			
			free(current->client);
			free(current);
			current = next;		
					
		}
		list->count = 0;
		list->first = NULL;
	}
}

/*Sous programme de récupération des noms*/
int contains_name(struct t_list* list, char* name) {
	int count = 0;
	struct t_list_item *current = list->first;
	while (current != NULL) {
		if (strncmp(current->client->name, name, strlen(current->client->name)) == 0)
		{
			count++;
		} 
		current = current->next;
	}
	return count;
}

/*Sous programme de récupération des structures clients*/
struct t_client* get_client(struct t_list* list, char* name) {
	struct t_list_item *current = list->first;
	while (current != NULL) {
		if (strncmp(current->client->name, name, strlen(current->client->name)) == 0) {
			return current->client;
		}
		current = current->next;
	}
	return NULL;
}