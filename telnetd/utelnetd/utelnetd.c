/* utelnetd.c
 *
 * Simple telnet server
 *
 * Artur Bajor, Centrum Informatyki ROW
 * <abajor@cirow.pl>
 *
 * Joerg Schmitz-Linneweber, Aston GmbH
 * <schmitz-linneweber@aston-technologie.de>
 *
 * Vladimir Oleynik
 * <dzo@simtreas.ru>
 *
 * Robert Schwebel, Pengutronix
 * <r.schwebel@pengutronix.de>
 * 
 * Bjorn Wesen, Axis Communications AB 
 * <bjornw@axis.com>
 * 
 * Sepherosa Ziehau
 * <sepherosa@myrealbox.com>
 *
 * Stephan Olbrich
 * <reader42@gmx.de>
 * 
 * This file is distributed under the GNU General Public License (GPL),
 * please see the file LICENSE for further information.
 * 
 * ---------------------------------------------------------------------------
 * (C) 2000, 2001, 2002, 2003, 2010 by the authors mentioned above
 * ---------------------------------------------------------------------------
 *
 * The telnetd manpage says it all:
 *
 *   Telnetd operates by allocating a pseudo-terminal device (see pty(4))  for
 *   a client, then creating a login process which has the slave side of the
 *   pseudo-terminal as stdin, stdout, and stderr. Telnetd manipulates the
 *   master side of the pseudo-terminal, implementing the telnet protocol and
 *   passing characters between the remote client and the login process.
 */

/* configuration - we'll separate this out later */
#define USE_SYSLOG 	1
#define USE_ISSUE	1


#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

/* we need getpty() */
#define __USE_GNU 1
#define __USE_XOPEN 1
#include <stdlib.h>
#undef __USE_GNU
#undef __USE_XOPEN

#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#ifdef USE_SYSLOG
#include <syslog.h>
#endif

#include <termios.h>

#ifdef BSD
#define XTABS OXTABS
#define getpt() posix_openpt(O_RDWR|O_NOCTTY)
#endif

#ifdef DEBUG
#define TELCMDS
#define TELOPTS
#endif

#include <arpa/telnet.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <net/if.h>

#define BUFSIZE 4000
#ifdef USE_ISSUE
#define ISSUE_FILE "/mnt/ext1/games/issue.txt"
#endif

#define MIN(a,b) ((a) > (b) ? (b) : (a))

static char *loginpath = NULL;

/* shell name and arguments */

static char *argv_init[] = {NULL, NULL, NULL};

/* structure that describes a session */

struct tsession {
	struct tsession *next;
	int sockfd, ptyfd_read, ptyfd_write;
	int shell_pid;
	/* two circular buffers */
	char *buf1, *buf2;
	int rdidx1, wridx1, size1;
	int rdidx2, wridx2, size2;
};

#ifdef DEBUG
#define DEBUG_OUT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_OUT(...)
//static inline void DEBUG_OUT(const char *format, ...) {};
#endif

/*

   This is how the buffers are used. The arrows indicate the movement
   of data.

   +-------+     wridx1++     +------+     rdidx1++     +----------+
   |       | <--------------  | buf1 | <--------------  |          |
   |       |     size1--      +------+     size1++      |          |
   |  pty  |                                            |  socket  |
   |       |     rdidx2++     +------+     wridx2++     |          |
   |       |  --------------> | buf2 |  --------------> |          |
   +-------+     size2++      +------+     size2--      +----------+

   Each session has got two buffers.

*/

static int maxfd;

static struct tsession *sessions;

/* 
 * This code was ported from a version which was made for busybox. 
 * So we have to define some helper functions which are originally 
 * available in busybox...
 */  

void show_usage(void)
{
	printf("Usage: telnetd [-p port] [-i interface] [-l loginprogram] [-d]\n");
	printf("\n");
	printf("   -p port          specify the tcp port to connect to\n");
	printf("   -i interface     specify the network interface to listen on\n");
	printf("                    (default: all interfaces)\n");
	printf("   -l loginprogram  program started by the server\n");
	printf("   -d               daemonize\n");
	printf("\n");         
	exit(1);
}

void perror_msg_and_die(char *text)
{
	fprintf(stderr,text);
	exit(1);
}

void error_msg_and_die(char *text)
{
	perror_msg_and_die(text);
}


/* 
   Remove all IAC's from the buffer pointed to by bf (recieved IACs are ignored
   and must be removed so as to not be interpreted by the terminal).  Make an
   uninterrupted string of characters fit for the terminal.  Do this by packing
   all characters meant for the terminal sequentially towards the end of bf. 

   Return a pointer to the beginning of the characters meant for the terminal.
   and make *processed equal to the number of characters that were actually
   processed and *num_totty the number of characters that should be sent to
   the terminal.  
   
   Note - If an IAC (3 byte quantity) starts before (bf + len) but extends
   past (bf + len) then that IAC will be left unprocessed and *processed will be
   less than len.
  
   FIXME - if we mean to send 0xFF to the terminal then it will be escaped,
   what is the escape character?  We aren't handling that situation here.

  */
static char *
remove_iacs(unsigned char *bf, int len, int *processed, int *num_totty) {
    unsigned char *ptr = bf;
    unsigned char *totty = bf;
    unsigned char *end = bf + len;
   
    while (ptr < end) {
	if (*ptr != IAC) {
	    *totty++ = *ptr++;
	}
	else {
	    if ((ptr+2) < end) {
		/* the entire IAC is contained in the buffer 
		   we were asked to process. */
		DEBUG_OUT("Ignoring IAC 0x%02x, %s, %s\n", *ptr, TELCMD(*(ptr+1)), TELOPT(*(ptr+2)));
		ptr += 3;
	    } else {
		/* only the beginning of the IAC is in the 
		   buffer we were asked to process, we can't
		   process this char. */
		break;
	    }
	}
    }

    *processed = ptr - bf;
    *num_totty = totty - bf;
    /* move the chars meant for the terminal towards the end of the 
       buffer. */
    return memmove(ptr - *num_totty, bf, *num_totty);
}

/*
static int getpty(char *line)
{
        int p;

        p = getpt();
        if (p < 0) {
                DEBUG_OUT("getpty(): couldn't get pty\n");
                close(p);
                return -1;
        }
        if (grantpt(p)<0 || unlockpt(p)<0) {
                DEBUG_OUT("getpty(): couldn't grant and unlock pty\n");
                close(p);
                return -1;
        }
        DEBUG_OUT("getpty(): got pty %s\n",ptsname(p));
        strcpy(line, (const char*)ptsname(p));

        return(p);
}
*/

static void
send_iac(struct tsession *ts, unsigned char command, int option)
{
	/* We rely on that there is space in the buffer for now.  */
	char *b = ts->buf2 + ts->rdidx2;
	*b++ = IAC;
	*b++ = command;
	*b++ = option;
	ts->rdidx2 += 3;
	ts->size2 += 3;
}


static struct tsession *
make_new_session(int sockfd)
{
	int pid;
	struct tsession *ts = (struct tsession *)malloc(sizeof(struct tsession));
	int fdes[2];
#ifdef USE_ISSUE
	FILE *fp;
	int chr;
#endif
	ts->buf1 = (char *)malloc(BUFSIZE);
	ts->buf2 = (char *)malloc(BUFSIZE);

	ts->sockfd = sockfd;

	ts->rdidx1 = ts->wridx1 = ts->size1 = 0;
	ts->rdidx2 = ts->wridx2 = ts->size2 = 0;

	/* Got a new connection, set up a tty and spawn a shell.  */

	if (pipe(fdes) < 0)
	{
		fprintf(stderr, "Error pipe\n");
		return 0;
	}
	int parent_inp = fdes[0];
	int child_out  = fdes[1];

	if (pipe(fdes) < 0)
	{
		fprintf(stderr, "Error pipe\n");
		return 0;
	}
	int child_inp  = fdes[0];
	int parent_out = fdes[1];
	
	ts->ptyfd_read  = parent_inp;
	ts->ptyfd_write = parent_out;
	if (parent_inp > maxfd)
		maxfd = parent_inp;
	if (parent_out > maxfd)
		maxfd = parent_out;

	/* Make the telnet client understand we will echo characters so it 
	 * should not do it locally. We don't tell the client to run linemode,
	 * because we want to handle line editing and tab completion and other
	 * stuff that requires char-by-char support.
	 */

	send_iac(ts, DO, TELOPT_ECHO);
	send_iac(ts, DO, TELOPT_LFLOW);
	send_iac(ts, WILL, TELOPT_ECHO);
	send_iac(ts, WILL, TELOPT_SGA);
	


	if ((pid = fork()) < 0) {
		perror("fork");
	}
	if (pid == 0) {
		/* In child.  */
		
		int uid = getuid();
		if (dup2(child_inp, 0) < 0)
		{
			fprintf(stderr, "Error: dup2 (stdin)\n");
			exit(1);
		}
		if (dup2(child_out, 1) < 0)
		{
			fprintf(stderr, "Error: dup2 (stdout)\n");
			exit(1);
		}
		if (dup2(child_out, 2) < 0)
		{
			fprintf(stderr, "Error: dup2 (stderr)\n");
			exit(1);
		}
		setgid(getgid());
		setuid(uid);
		int i;
		for (i = 3; i < getdtablesize(); i++)
			close(i);
		
#ifdef USE_ISSUE
		/* Display ISSUE_FILE */
		if ((fp = fopen(ISSUE_FILE, "r")) != NULL) {
			DEBUG_OUT(" Open & start display %s\n", ISSUE_FILE);
			while ((chr=fgetc(fp)) != EOF) { 
				if (chr == '\n') fputc('\r', stdout);
				fputc(chr, stdout);
			}
			fclose(fp);
		}
#endif
		/* exec shell, with correct argv and env */
		execv(loginpath, argv_init);

	        /* NOT REACHED */
		perror_msg_and_die("execv");
	}

	ts->shell_pid = pid;

	return ts;
}

static void
free_session(struct tsession *ts)
{
	struct tsession *t = sessions;

	/* Unlink this telnet session from the session list.  */
	if(t == ts)
		sessions = ts->next;
	else {
		while(t->next != ts)
			t = t->next;
		t->next = ts->next;
	}

	free(ts->buf1);
	free(ts->buf2);

	kill(ts->shell_pid, SIGKILL);

	wait4(ts->shell_pid, NULL, 0, NULL);

	close(ts->ptyfd_write);
	close(ts->ptyfd_read);
	close(ts->sockfd);

	if(ts->ptyfd_read == maxfd || ts->ptyfd_write == maxfd || ts->sockfd == maxfd)
		maxfd--;
	if(ts->ptyfd_read == maxfd || ts->ptyfd_write == maxfd || ts->sockfd == maxfd)
		maxfd--;
	if(ts->ptyfd_read == maxfd || ts->ptyfd_write == maxfd || ts->sockfd == maxfd)
		maxfd--;

	free(ts);
}

int main(int argc, char **argv)
{
	struct sockaddr_in sa;
	int master_fd;
	fd_set rdfdset, wrfdset;
	int selret;
	int on = 1;
	int portnbr = 23;
	int c, ii;
	int daemonize = 0;
	char *interface_name = NULL;
	struct ifreq interface;

	char CR[1];
	CR[0] = (char)13;
	
	
#ifdef USE_SYSLOG
	char *appname;
	appname = strrchr (argv [0], '/');
	if (!appname) appname = argv [0];
	    else appname++;
#endif

	/* check if user supplied a port number */

	for (;;) {
		c = getopt( argc, argv, "i:p:l:hd");
		if (c == EOF) break;
		switch (c) {
			case 'p':
				portnbr = atoi(optarg);
				break;
			case 'i':
				interface_name = strdup(optarg);
				break;
			case 'l':
				loginpath = strdup(optarg);
				break;
			case 'd':
				daemonize = 1;
				break;
			case 'h': 
			default:
				show_usage();
				exit(1);
		}
	}

	if (!loginpath) {
		loginpath = SHELLPATH;
		if (access(loginpath, X_OK) < 0) loginpath = "/bin/sh";
	}
	  
	if (access(loginpath, X_OK) < 0) {
		/* workaround: error_msg_and_die has doesn't understand
 		   variable argument lists yet */
		fprintf(stderr,"\"%s\"",loginpath);
		perror_msg_and_die(" is no valid executable!\n");
	}

	fprintf(stderr, "telnetd: starting\n");
	fprintf(stderr, "  port: %i; interface: %s; login program: %s\n",
		portnbr, (interface_name)?interface_name:"any", loginpath);

	argv_init[0] = loginpath;
	argv_init[1] = "-i";
	sessions = 0;

	/* Grab a TCP socket.  */

	master_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_fd < 0) {
		perror("socket");
		return 1;
	}
	(void)setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* Set it to listen to specified port */

	memset((void *)&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(portnbr);

	/* Set it to listen on the specified interface */

	if (interface_name) {

		strcpy(interface.ifr_name, interface_name);

		/* use ioctl() here as BSD does not have setsockopt() */
		if (ioctl(master_fd, SIOCGIFADDR, &interface) < 0) {
			printf("Please check the NIC you specified with -i option\n");
			perror("ioctl SIOCGFADDR");
			return 1;
		}

		sa.sin_addr = ((struct sockaddr_in *)(&interface.ifr_addr))->sin_addr;
	} else 
		sa.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(master_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(master_fd, 1) < 0) {
		perror("listen");
		return 1;
	}

	if (daemonize) 
	{
		DEBUG_OUT("  daemonizing\n");
		if (daemon(0, 0) < 0) perror_msg_and_die("daemon");
	}

#ifdef USE_SYSLOG
	openlog(appname , LOG_NDELAY | LOG_PID, LOG_DAEMON);	
	syslog(LOG_INFO, "%s (port: %i, ifname: %s, login: %s) startup succeeded\n"\
	    , appname, portnbr, (interface_name)?interface_name:"any", loginpath);
	closelog();
#endif

	maxfd = master_fd;

	do {
		struct tsession *ts;

		FD_ZERO(&rdfdset);
		FD_ZERO(&wrfdset);

		/* select on the master socket, all telnet sockets and their
		 * ptys if there is room in their respective session buffers.
		 */

		FD_SET(master_fd, &rdfdset);

		ts = sessions;
		while (ts) {
			/* buf1 is used from socket to pty
			 * buf2 is used from pty to socket
			 */
			if (ts->size1 > 0) {
				FD_SET(ts->ptyfd_write, &wrfdset);  /* can write to pty */
				//DEBUG_OUT(stderr, "ptyfd_write\n");
			}
			if (ts->size1 < BUFSIZE) {
				FD_SET(ts->sockfd, &rdfdset); /* can read from socket */
				//DEBUG_OUT(stderr, "socket_read\n");
			}
			if (ts->size2 > 0) {
				FD_SET(ts->sockfd, &wrfdset); /* can write to socket */
				//DEBUG_OUT(stderr, "socket_write\n");
			}
			if (ts->size2 < BUFSIZE) {
				FD_SET(ts->ptyfd_read, &rdfdset);  /* can read from pty */
				//DEBUG_OUT(stderr, "ptyfd_read\n");
			}
			ts = ts->next;
		}
		
		// DEBUG_OUT(stderr, "before select ");

		selret = select(maxfd + 1, &rdfdset, &wrfdset, 0, 0);
		
		// DEBUG_OUT(stderr, "after select\n");

		if (!selret)
			break;

		/* First check for and accept new sessions.  */
		if (FD_ISSET(master_fd, &rdfdset)) {
			int fd, salen;

			salen = sizeof(sa);	
			if ((fd = accept(master_fd, (struct sockaddr *)&sa,
					 &salen)) < 0) {
				continue;
			} else {
				/* Create a new session and link it into our active list. */
				struct tsession *new_ts;
#ifdef USE_SYSLOG
				openlog(appname , LOG_NDELAY, LOG_DAEMON);	
				syslog(LOG_INFO, "connection from: %s\n", inet_ntoa(sa.sin_addr));
				closelog();
#endif
				new_ts = make_new_session(fd);
				if (new_ts) {
					new_ts->next = sessions;
					sessions = new_ts;
					if (fd > maxfd)
						maxfd = fd;
//					DEBUG_OUT(stderr, "New Session\n");
				} else {
					close(fd);
				}
			}
		}

		/* Then check for data tunneling.  */

		ts = sessions;
		while (ts) { /* For all sessions...  */
			int maxlen, w, r;
			struct tsession *next = ts->next; /* in case we free ts. */

			if (ts->size1 && FD_ISSET(ts->ptyfd_write, &wrfdset)) {
			    int processed, num_totty;
			    char *ptr;
				/* Write to pty from buffer 1.  */
				
				maxlen = MIN(BUFSIZE - ts->wridx1,
					     ts->size1);
				ptr = remove_iacs(ts->buf1 + ts->wridx1, maxlen, 
					&processed, &num_totty);
		
				/* the difference between processed and num_totty
				   is all the iacs we removed from the stream.
				   Adjust buf1 accordingly. */
				ts->wridx1 += processed - num_totty;
				ts->size1 -= processed - num_totty;

				w = write(ts->ptyfd_write, ptr, num_totty);
//				DEBUG_OUT(stderr, "to pty %d\n", w);
				if (w < 0) {
					perror("write");
					free_session(ts);
					ts = next;
					continue;
				}
				ts->wridx1 += w;
				ts->size1 -= w;
				if (ts->wridx1 == BUFSIZE)
					ts->wridx1 = 0;
			}

			if (ts->size2 && FD_ISSET(ts->sockfd, &wrfdset)) {
				/* Write to socket from buffer 2.  */
				maxlen = MIN(BUFSIZE - ts->wridx2,
					     ts->size2);
				//w = write(ts->sockfd, ts->buf2 + ts->wridx2, maxlen);
				int ii;
				for (ii=0; ii<maxlen; ii++)
				{
					if ((int)(*(ts->buf2 + ts->wridx2 + ii)) == 10)
					{
						w = write(ts->sockfd, CR, 1);
					}
					w = write(ts->sockfd, ts->buf2 + ts->wridx2 + ii, 1);
					if (w < 1) {
						perror("write");
						free_session(ts);
						ts = next;
						continue;
					}
				}
				w = maxlen;
//				DEBUG_OUT(stderr, "to socket %d\n", w);
				ts->wridx2 += w;
				ts->size2 -= w;
				if (ts->wridx2 == BUFSIZE)
					ts->wridx2 = 0;
			}

			if (ts->size1 < BUFSIZE && FD_ISSET(ts->sockfd, &rdfdset)) {
				/* Read from socket to buffer 1. */
				maxlen = MIN(BUFSIZE - ts->rdidx1,
					     BUFSIZE - ts->size1);
				r = read(ts->sockfd, ts->buf1 + ts->rdidx1, maxlen);
//				DEBUG_OUT(stderr, "from socket %d\n", r);
				if (!r || (r < 0 && errno != EINTR)) {
					free_session(ts);
					ts = next;
					continue;
				}
				if(!*(ts->buf1 + ts->rdidx1 + r - 1)) {
					r--;
					if(!r)
						continue;
				}
				ts->rdidx1 += r;
				ts->size1 += r;
				if (ts->rdidx1 == BUFSIZE)
					ts->rdidx1 = 0;
			}

			if (ts->size2 < BUFSIZE && FD_ISSET(ts->ptyfd_read, &rdfdset)) {
				/* Read from pty to buffer 2.  */
				maxlen = MIN(BUFSIZE - ts->rdidx2,
					     BUFSIZE - ts->size2);
				r = read(ts->ptyfd_read, ts->buf2 + ts->rdidx2, maxlen);
//				DEBUG_OUT(stderr, "from pty %d\n", r);
				if (!r || (r < 0 && errno != EINTR)) {
					free_session(ts);
					ts = next;
					continue;
				}
				for (ii=0; ii < r; ii++)
				  if (*(ts->buf2 + ts->rdidx2 + ii) == 3)
				    fprintf(stderr, "found <CTRL>-<C> in data!\n");
				ts->rdidx2 += r;
				ts->size2 += r;
				if (ts->rdidx2 == BUFSIZE)
					ts->rdidx2 = 0;
			}

			if (ts->size1 == 0) {
				ts->rdidx1 = 0;
				ts->wridx1 = 0;
			}
			if (ts->size2 == 0) {
				ts->rdidx2 = 0;
				ts->wridx2 = 0;
			}
			ts = next;
		}

	} while (1);

	return 0;
}
