/*
 * queue.c - implements an special type of queue. 
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

/**
 * special type of queue: you enqueue data stream, and you dequeue blocks of
 * of that data (of BLOCKSIZE size)
 */

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <trace.h>
#include "queue.h"


#define IS_QUEUE(t) (t!=NULL)

enum {
	DEQUEUE_BLOCK = 4096
};


/** queue node */
struct node
{	void *data;
	size_t length;
	struct node *next;
};

/** Concrete Data Type */
struct queueHead
{	struct node *head;      /**< the queue's  head */
	struct node *tail;      /**< the queue's  end  */
};

queue_t
queue_new(void)
{	queue_t q;

	q = malloc(sizeof(*q));
	if( q )
	{	q->head = NULL;
		q->tail = NULL;
	}
	
	return q;
}

int
queue_is_valid(queue_t q)
{
	return q!=NULL;
}

void
queue_destroy(queue_t q)
{	struct node *node, *next;

	if( IS_QUEUE(q) )
	{
		for( node = q->head ; node ; node = next )
		{	next = node->next;
			free(node->data);
			free(node);
		}
		free(q);
	}
}

int
queue_enqueue(queue_t q, const void *data, size_t len)
{	struct node *node;
	int ret = 0;
	
	if( IS_QUEUE(q) && data && len )
	{ 	node = malloc(sizeof(*node));
		if( node == NULL )
			ret = -1;
		else
		{	node->length = len;
			node->data   = malloc(len);
			node->next   = NULL;
			
			if( node->data )
			{ 	memcpy(node->data, data, len); 
				if( q->head == NULL )
					q->head = node;
				else
					q->tail->next = node;
				q->tail = node;
			}
			else
			{	free(node);
				ret = -1;
			}

		}
	}
	else
		ret = -1;
		
	return ret;
}


/* how many blocks i need to use to get max characters? */
static int
get_needed_nodes_to_fill_block( queue_t q, unsigned max, unsigned *real)
{	struct node *node;
	unsigned i;
	unsigned count = 0;

	assert(q);
	node = q->head;
	assert(q->head);

	for( i=0 ; node &&  count + node->length < max ; i++)
	{	count += node->length;
		node = node->next; 
	}

	/* return at least one item (yeah. greater than top) */
	if( i == 0 )
	{	count = node->length; 
		i = 1;
	}

	if( real )
		*real = count;
		
	return i;
}

void *
queue_block_dequeue(queue_t q, size_t *len, size_t block)
{	struct node *node;
	void *ret;
	unsigned i = 0;

	if( IS_QUEUE(q) && len && q->head )
	{	i = get_needed_nodes_to_fill_block(q, 4096, len);

		assert(i);
		if( i == 1 )
		{ 	node = q->head;
			q->head =  node->next;
			if( q->head == NULL )
				q->tail = NULL;
			ret = node->data;
			free(node);
		}
		else
		{	char *s;
			
			s = ret = malloc(*len);
			
			for( ; i>0 ; i-- )
			{	node = q->head;

				if( node )
				{ 	q->head = node->next; 
					memcpy(s, node->data, node->length);
					s += node->length;

					free(node->data);
					free(node);
				}
				else
					rs_log_warning("dequeue: node is null");
			}
		}
	}
	else
		ret = NULL;
		
	return ret;
}

int
queue_is_empty( queue_t q )
{ 
	return IS_QUEUE(q) ? q->head == NULL : 1;
}
