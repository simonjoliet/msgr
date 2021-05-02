#ifndef MESSAGE_H_
#define MESSAGE_H_

struct t_message {
	char* command;
	char* dest;
	char* message;
};

#define COMMAND_SEND	"send"
#define	COMMAND_SENDALL	"sendall"
#define	COMMAND_LIST	"list"

#endif
