/*
OpenIO SDS cluster
Copyright (C) 2014 Worldine, original work as part of Redcurrant
Copyright (C) 2015 OpenIO, modified as part of OpenIO Software Defined Storage

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <metautils/lib/metautils.h>

#include "./agent.h"
#include "./message.h"
#include "./write_message_worker.h"

void 
request_clean(request_t *request)
{
	if (!request)
		return;
	if (request->cmd)
		g_free(request->cmd);
	if (request->arg)
		g_free(request->arg);
	memset(request, 0, sizeof(request_t));
	g_free(request);
}

void
request_cleanup(worker_t *worker)
{
	if (worker) {

		if (worker->data.session) {
			request_clean(worker->data.session);
			worker->data.session = NULL;
		}

		if (worker->data.buffer) {
			g_free(worker->data.buffer);
			worker->data.buffer = NULL;
		}

		worker->clean = NULL;
	}
}

void 
message_clean(message_t *msg)
{
	if (!msg)
		return;
	if (msg->data)
		g_free(msg->data);
	memset(msg, 0, sizeof(message_t));
	g_free(msg);
}

void
message_cleanup(worker_t *worker)
{
	if (worker) {

		if (worker->data.session) {
			message_clean(worker->data.session);
			worker->data.session = NULL;
		}

		if (worker->data.buffer) {
			g_free(worker->data.buffer);
			worker->data.buffer = NULL;
		}

		worker->clean = NULL;
	}
}

int
read_request_from_message(message_t *message, request_t *req, GError **error)
{
	char *data, *space;
	guint32 length;


	if (message == NULL) {
		GSETERROR(error,"Argument message can't be NULL");
		return(0);
	}

	if (req == NULL) {
		GSETERROR(error, "Argument req can't be NULL");
		return(0);
	}

        if (!message->length) {
                GSETERROR(error, "Can't parse empty message");
                return(0);
        }

	length = message->length;
	data = message->data;
	space = g_strstr_len(data, length, " ");
	TRACE2("data=%p space=%p length=%u", data, space, length);

	if (space) {
		int i_length = length;
		char *post_space = space + 1;
		req->cmd = g_strndup(data, space - data);
		if ((post_space - data) < i_length) {
			req->arg_size = i_length - (post_space - data);
			req->arg = g_malloc0(req->arg_size + 1);
			memcpy(req->arg, post_space, req->arg_size);
			req->arg[ req->arg_size ] = '\0';
		}
		else {
			req->arg = NULL;
			req->arg_size = 0;
		}
	}
	else {
		req->cmd = g_strndup(data, length);
		req->arg_size = 0;
		req->arg = NULL;
        }

        DEBUG("Parsed message : cmd[%s] arg[%.*s]", req->cmd, MIN(req->arg_size,32), req->arg);
        return(1);
}

static message_t *
build_message_from_response(response_t *response)
{
	message_t *message;

	message = g_malloc0(sizeof(message_t));
	message->length = sizeof(response->status) + response->data_size;
	message->data = g_malloc0(message->length);

	memcpy(message->data, &(response->status), sizeof(response->status));
	memcpy(message->data+sizeof(response->status), response->data, response->data_size);

	return message;
}

int
__respond (worker_t *worker, int ok, GByteArray *content, GError **error)
{
	response_t response;
	message_t *message;

	(void) error;

	/* init the response */
	response.status = ok ? STATUS_OK : STATUS_ERROR;
	if (content && content->data && content->len>0) {
		response.data = content->data;
		response.data_size = content->len;
	} else {
		response.data = 0;
		response.data_size = 0;
	}

	message = build_message_from_response(&response);
	g_byte_array_free(content, TRUE);

	if (worker->clean)
		worker->clean(worker);
	worker->data.session = message;
	worker->func = write_message_worker;
	worker->clean = message_cleanup;
	return 1;
}

int __respond_message (worker_t *worker, int ok, const char *msg, GError **error)
{
	GByteArray *gba = g_byte_array_new();
	if (msg)
		g_byte_array_append( gba, (const guint8*)msg, strlen(msg));
	else
		g_byte_array_append( gba, (guint8*)"no message", strlen("no message"));
	return __respond(worker,ok,gba,error);
}

int __respond_error(worker_t *worker, GError *e, GError **error)
{
	int rc;
	rc = __respond_message(worker, 0, e?e->message:"unknown error", error);
	g_error_free(e);
	return rc;
}

