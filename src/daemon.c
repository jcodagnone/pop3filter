/*
 * daemon.c -- some tools related to daemons
 *
 * Copyright (C) 2003 by Juan F. Codagnone <juam@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
   #include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <unistd.h>

#include <trace.h>

int
switch_to(const char *user, const char *group)
{	int ret = 0;
	struct passwd *p;
	struct group *g;
	
	if(  group )
	{	g = getgrnam(group);
		if( g )
			ret = setregid(g->gr_gid, g->gr_gid);
		else
			ret = -1;
	}
	
	
	if( ret != -1 && user )
	{	p = getpwnam(user);
		if( p )
			ret = setreuid(p->pw_uid, p->pw_uid);
		else
			ret = -1;
	}
	
	return ret;
}


extern const char * progname;

#ifdef HAVE_SYSLOGD
 #include <syslog.h>
#endif

void
open_syslogd(void)
{
	#ifdef HAVE_SYSLOGD
		 openlog(progname, LOG_PID, LOG_DAEMON);
	#endif
}

void
close_syslogd(void)
{
	#ifdef HAVE_SYSLOGD
		 closelog();
	 #endif
}

int
hechizar(void)
{	unsigned i;
	int pid;
	int sid;
	
 	for (i=0;i<3;i++) {
		close(i);
		open("/dev/null", O_RDWR);
	}

	open_syslogd();
	rs_trace_to(rs_trace_syslog);
	
	/* ignore sighup */
	signal(SIGHUP, SIG_IGN);
	
	pid =  fork();
	if( pid  == -1 ) {
		rs_log_error("forking: %s", strerror(errno));
		return -1;
	}
	else if( pid == 0 )
		; /* child */
	else
		exit(0);

	/* new session */
	if((sid = setsid()) <0 )
	{       rs_log_error("setsid: %s\n",strerror(errno));
		return -1;
	}
	
	umask(0022);
	chdir("/");

	return 0;
}

