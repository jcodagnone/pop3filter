/*
 * queue.c - data queue
 * $Id: queue.c,v 1.1 2003/01/18 04:28:19 juam Exp $
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

#include <stdlib.h>
#include <string.h>
#include "queue.h"

struct queue
{	size_t lenght;
	void *data;
	struct queue *next;
};

struct queueHead
{	struct queue *head;
	struct queue *tail;
};

queue_t
queue_new(void)
{	queue_t q;

	q = malloc(sizeof(*q));
	if( q == NULL )
		return NULL;
	q->head = NULL;
	q->tail = NULL;

	return q;
}

int queue_is_valid(queue_t q)
{
	return q!=NULL;
}
void
queue_delete(queue_t q)
{	struct queue *qu;

	if( q == NULL )
		return;

	for(qu = q->head ; qu ; qu = qu->next )
	{
		free(qu->data);
		free(qu);
	}
	free(q);
}

int
queue_enqueue(queue_t q, const void *data, size_t len)
{	struct queue *qu;

	if( q == NULL || data == NULL || len == 0)
		return -1;

	qu = malloc(sizeof(*qu));
	if( qu == NULL )
		return -1;

	/* fill cell */
	qu->lenght = len;
	qu->next   = NULL;
	qu->data   = malloc(len);
	if( qu->data == NULL )
	{	free(qu);
		return -1;
	}
	memcpy(qu->data,data,len);

	if( q->head == NULL )
		q->head = qu;
	else
		q->tail->next = qu;
	q->tail = qu;

	return 0;
}


void *
queue_dequeue(queue_t q, size_t *len)
{	struct queue *qu;
	void *r;

	if( q == NULL || len == NULL )
		return NULL;

	qu = q->head;

	if( qu == NULL )
		return NULL;
		
	q->head =  qu->next;
	
	*len = qu->lenght;
	r = qu->data;
	free(qu);

	return r;
}

int
queue_is_empty( queue_t q )
{	int r;

	if( q == NULL )
		r = 1;
	else
		r = q->head == NULL;

	return r;
}
