/*
OpenIO SDS server
Copyright (C) 2014 Worldine, original work as part of Redcurrant
Copyright (C) 2015 OpenIO, modified as part of OpenIO Software Defined Storage

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
*/

#ifndef OIO_SDS__server__network_server_h
# define OIO_SDS__server__network_server_h 1

# include <sys/time.h>
# include <server/slab.h>

# define timespec_sub(a, b, result) do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec, \
	(result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
	if ((result)->tv_nsec < 0) { \
		--(result)->tv_sec, (result)->tv_nsec += 1000000000; \
	} \
} while (0)

struct network_server_s;
struct grid_stats_holder_s;
struct network_client_s;
struct network_transport_s;
struct gba_view_s;

/* To be defined by the application instatiating the transport */
struct transport_client_context_s;

enum {
	RC_ERROR,
	RC_NODATA,
	RC_NOTREADY,
	RC_PROCESSED,
};

struct network_transport_s
{
	/* Associate private data to the  */
	struct transport_client_context_s *client_context;

	void (*clean_context) (
			struct transport_client_context_s*);

	/* Be notified that a piece of data is ready */
	int (*notify_input)  (struct network_client_s *);
	void (*notify_error)  (struct network_client_s *);
	gboolean waiting_for_close;
};

struct network_client_s
{
	int fd;
	enum { CLT_READ=0X01, CLT_WRITE=0X02, CLT_ERROR=0X04 } events;
	struct network_server_s *server;

	struct grid_stats_holder_s *main_stats; /*!< XXX DO NOT USE XXX
	(unless you know what you're doing). This is a direct (and unprotected)
	pointer to the stats_holder of the main server. It is shared among 
	all threads. */

	struct grid_stats_holder_s *local_stats; /*!< Can be safely used
	by any app. This is a pointer to the stats_holder local to the thread
	that is running the current client. */

	gchar local_name[128];
	gchar peer_name[128];
	int flags;
	struct {
		time_t cnx; // coarse (bogo)
		time_t evt_out; // coarse (bogo)
		struct timespec evt_in; // precise
	} time;

	/* Pending input */
	struct data_slab_sequence_s input;
	/* Pending output */
	struct data_slab_sequence_s output;
	/* What to do with pending data */
	struct network_transport_s transport;
	GError *current_error;

	struct network_client_s *prev; /*!< XXX DO NOT USE */
	struct network_client_s *next; /*!< XXX DO NOT USE */
};

/*! Creates a new server
 * @return
 */
struct network_server_s * network_server_init(void);

/*!
 * Changes the maximum number of worker threads that the server can run.
 * This can be done while the server is working.
 *
 * @param srv
 * @param max
 */
void network_server_set_max_workers(struct network_server_s *srv, guint max);

/*! Changes the maximum number of concurrent connections that can be
 * managed by the given server.
 *
 * This can be done while the server is working.
 *
 * @param srv
 * @param max
 */
void network_server_set_maxcnx(struct network_server_s *srv, guint max);

/*! Changes the number of connection backlog that can be
 * used by the given server.
 *
 * This can be done while the server is working.
 *
 * @param srv
 * @param cnx_bl
 */
void network_server_set_cnx_backlog(struct network_server_s *srv,
		guint cnx_bl);

typedef void (*network_transport_factory) (gpointer u,
		struct network_client_s *clt);

/*!
 * @param srv * @param url
 * @param factory
 */
void network_server_bind_host(struct network_server_s *srv,
		const gchar *url, gpointer factory_udata,
		network_transport_factory factory);

void network_server_bind_host_throughput(struct network_server_s *srv,
		const gchar *url, gpointer factory_udata,
		network_transport_factory factory);

void network_server_bind_host_lowlatency(struct network_server_s *srv,
		const gchar *url, gpointer factory_udata,
		network_transport_factory factory);

/*!
 * @param srv
 */
void network_server_close_servers(struct network_server_s *srv);

/*!
 * @param srv
 * @return
 */
GError * network_server_open_servers(struct network_server_s *srv);

/*!
 * @param srv
 * @return
 */
GError * network_server_run(struct network_server_s *srv);

/*!
 * @param srv
 */
void network_server_stop(struct network_server_s *srv);

/*!
 * @param srv
 */
void network_server_clean(struct network_server_s *srv);

/*!
 * @param srv
 * @return
 */
struct grid_stats_holder_s * network_server_get_stats(
		struct network_server_s *srv);

/*!
 * @param srv
 * @return
 */
gint network_server_pending_events(struct network_server_s *srv);

/*!
 * @param srv
 * @return
 */
gdouble network_server_reqidle(struct network_server_s *srv);

/*! "not really precise and not really reliable" clock with a precision at a
 * second. Useful and sufficiant when sub-second precision is not required,
 * e.g. for cache expirations. Does not involve syscalls. */
time_t network_server_bogonow(const struct network_server_s *srv);

/*! Precise monotonic clock measurement. Might involve a syscall. */
void network_server_now(struct timespec *ts);

/* -------------------------------------------------------------------------- */

void network_client_allow_input(struct network_client_s *clt, gboolean v);

void network_client_close_output(struct network_client_s *clt, int now);

int network_client_send_slab(struct network_client_s *client,
		struct data_slab_s *slab);

/* Convenience easy factories ----------------------------------------------- */

static inline void
transport_devnull_factory(gpointer factory_udata,
		struct network_client_s *clt)
{
	(void) factory_udata;
	(void) clt;
}

#endif /*OIO_SDS__server__network_server_h*/