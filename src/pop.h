#ifndef _POP_H_
#define _POP_H_


enum cmds
{
	CMD_RETR = 1,		/* command is RETR  */
	CMD_UNKN = 2		/* unknown command; */
};

enum ST_RETR		/* RETR state machine */
{	RT_RESPONSE, 	/* ["-..."] -> RT_END, [*]->RT_BODY */
	RT_BODY,	/* [".\r\n"] -> RT_END | [*] -> RT_BODY */
	RT_END,
	RT_ERR
};

enum {
	PIPE_PAREN_READ,
	PIPE_PAREN_WRITE,
	PIPE_CHILD_READ,
	PIPE_CHILD_WRITE
};

/* struct global
 *	Global data to share across async calls
 */
struct global
{	int local,remote;	/* sockets */
	int fd[4];		/* comunication pipes */
	pid_t pid;		/* child process pid */
	int failed;		/* */
	enum cmds last_cmd;	/* the last pop3 command */
	const char *exec;	/* command to exec to filter */
	enum ST_RETR retr;	/* RETR state machine */
	
};


/* ascii2cmd()
 * 	Given a command 'p' returns the enum cmds equivalent
 */
enum cmds ascii2cmd ( const char *p );
void pop3_error(int socket,const char *fmt,...);
int pop_local_read( struct global  *d, const char *buf);
int pop_remote_read( struct global *d, const char *buf);
int pop_child_read(struct global *d, char *buf,size_t size);


#endif

