/*	$Id$
	
	Copyright 1996, 1997, 1998, 2002
	        Hekkelman Programmatuur B.V.  All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.
	3. All advertising materials mentioning features or use of this software
	   must display the following acknowledgement:
	   
	    This product includes software developed by Hekkelman Programmatuur B.V.
	
	4. The name of Hekkelman Programmatuur B.V. may not be used to endorse or
	   promote products derived from this software without specific prior
	   written permission.
	
	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
	AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 	
	
	Created: 04/05/98 17:21:22
*/

#include "pe.h"
#include "Sstdio.h"
#include <stdarg.h>
#include <ctype.h>
#include <socket.h>
#include <netdb.h>

struct SOCK *s_open(int sock, const char *type)
{
	struct SOCK *s = (struct SOCK *)malloc(sizeof(struct SOCK));
	
	if (s)
	{
		s->sSocket = sock;
		s->sBufSize = 0;
		s->sType = 3;
		s->sBufContentSize = 0;
		s->sBuffer = NULL;
		s->sIgnoreNextLF = FALSE;
	}
	
	return s;
} /* s_open */

int s_close(struct SOCK *sock)
{
	if ((sock->sType & 2) && sock->sBuffer)
		free(sock->sBuffer);
	free(sock);
	
	return 0;
} /* s_close */

int s_printf(struct SOCK *sock, const char *str, ...)
{
	if (sock->sType & 1)
	{
		va_list vl;
		char msg[1024];
	
		va_start(vl, str);
		vsprintf(msg, str, vl);
		va_end(vl);
	
		return send(sock->sSocket, msg, strlen(msg), 0);
	}
	else
		return B_ERROR;
} /* s_printf */

char *s_gets(char *str, int size, struct SOCK *sock)
{
	char *r = str;
	int done = FALSE;
	
	if ((sock->sType & 2) == 0)
		return NULL;

	size--;			// for the 0 byte
	
	do
	{
		char *bp;

		if (sock->sBuffer == NULL || sock->sBufSize < size)
		{
			char *t = sock->sBuffer;

			sock->sBufSize = max_c(size, 1024);
			sock->sBuffer = (char *)malloc(sock->sBufSize);
			if (! sock->sBuffer) return NULL;
			
			if (sock->sBufContentSize && t)
				memcpy(sock->sBuffer, t, sock->sBufContentSize);
			
			if (t)
				free(t);
		}
		
		bp = sock->sBuffer;
		
		if (*bp == '\n' && sock->sIgnoreNextLF)
		{
			sock->sIgnoreNextLF = FALSE;
			bp++;
			sock->sBufContentSize--;
		}

		while (! done && sock->sBufContentSize > 0 && size > 0)
		{
			char c = *bp;

			sock->sBufContentSize--;
			size--;
			*str++ = *bp++;
			
			if ((c == '\r' || c == '\n') && size > 0)
			{
				str[-1] = '\n';
				
				if (bp[-1] == '\r' && *bp == '\n')
					bp++, sock->sBufContentSize--;
				else if (bp[-1] == '\r')
					sock->sIgnoreNextLF = TRUE;
				
				done = TRUE;
				break;
			}
		}
		
		if (sock->sBufContentSize && (bp != sock->sBuffer))
		{
			memmove(sock->sBuffer, bp, sock->sBufContentSize);
		}
		
		if (! done)
		{
			fd_set fd;  
		    struct timeval tv; 
		    
		    tv.tv_sec = 5; 
		    tv.tv_usec = 0;
			
			FD_ZERO(&fd);
			FD_SET(sock->sSocket, &fd);
		
			if (select(32, &fd, 0, 0, &tv) != -1 && FD_ISSET(sock->sSocket, &fd))
			{
				int r = recv(sock->sSocket,
					sock->sBuffer + sock->sBufContentSize, sock->sBufSize - sock->sBufContentSize, 0);

				if (r <= 0)
					return NULL;

				sock->sBufContentSize += r;
			}
			else
				return NULL;
		}
		
		if (size == 0)
		{
			done = TRUE;
		}
	}
	while (! done);
	
	*str = 0;

	return r;
} /* s_gets */

