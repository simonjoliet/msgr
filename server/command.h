#ifndef COMMAND_H
#define COMMAND_H

#include "server.h"

void show_prompt();
int process_command_line(struct t_list* clients);
void display_connected_clients(struct t_list* clients);

#endif
