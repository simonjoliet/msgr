#ifndef LIST_H
#define LIST_H

#include "client.h"

struct t_list_item {
   struct t_client* client;
   struct t_list_item* next;
};

struct t_list {
   struct t_list_item* first;
   int count;
};

void add_item(struct t_list* list, struct t_client* client);
void remove_item(struct t_list* list, struct t_client* client);
void clear_list(struct t_list* list);
int contains_name(struct t_list* list, char* name);
struct t_client* get_client(struct t_list* list, char *name);

#endif
