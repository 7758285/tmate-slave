#include "tmate.h"

static int msgpack_write(void *data, const char *buf, unsigned int len)
{
	struct tmate_encoder *encoder = data;

	evbuffer_add(encoder->buffer, buf, len);

	if ((encoder->ev_readable.ev_flags & EVLIST_INSERTED) &&
	    !(encoder->ev_readable.ev_flags & EVLIST_ACTIVE)) {
		event_active(&encoder->ev_readable, EV_READ, 0);
	}

	return 0;
}

void tmate_encoder_init(struct tmate_encoder *encoder)
{
	encoder->buffer = evbuffer_new();
	msgpack_packer_init(&encoder->pk, encoder, &msgpack_write);
}

#define msgpack_pack_string(pk, str) do {		\
	int __strlen = strlen(str);			\
	msgpack_pack_raw(pk, __strlen);			\
	msgpack_pack_raw_body(pk, str, __strlen);	\
} while(0)

/* tmate_encoder may be NULL when replaying a session */
#define pack(what, ...) do {						\
	if (tmate_encoder)						\
		msgpack_pack_##what(&tmate_encoder->pk, __VA_ARGS__);	\
} while(0)

static void __tmate_notify(const char *msg)
{
	pack(array, 2);
	pack(int, TMATE_NOTIFY);
	pack(string, msg);
}

void printflike1 tmate_notify(const char *fmt, ...)
{
	va_list ap;
	char msg[1024];

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	__tmate_notify(msg);
}

static void __tmate_notify_later(evutil_socket_t fd, short what, void *arg)
{
	char *msg = arg;
	__tmate_notify(msg);
}

void printflike2 tmate_notify_later(int timeout, const char *fmt, ...)
{
	struct timeval tv;
	va_list ap;
	char *msg;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	va_start(ap, fmt);
	xvasprintf(&msg, fmt, ap);
	va_end(ap);

	/*
	 * FIXME leaks like crazy when calling tmate_notify_later()
	 * multiple times.
	 */

	evtimer_assign(&tmate_encoder->ev_notify_timer, ev_base,
		       __tmate_notify_later, msg);
	evtimer_add(&tmate_encoder->ev_notify_timer, &tv);
}

static int num_clients(void)
{
	unsigned int i, count = 0;

	for (i = 0; i < ARRAY_LENGTH(&clients); i++) {
		if (ARRAY_ITEM(&clients, i))
			count++;
	}

	return count;
}

static void mate_notify_message(struct client *c, int join)
{
	char buf[100];
	int count;
	static int multi_client;

	count = num_clients();
	if (count > 1)
		multi_client = 1;

	if (multi_client)
		sprintf(buf, " -- %d mate%s %sconnected",
			count,
			count == 1 ? " is" : "s are",
			(join || !count) ? "" : "still ");

	tmate_notify("%s mate has %s the session (%s)%s",
		     multi_client ? "A" : "Your",
		     join ? "joined" : "left",
		     c->ip_address,
		     multi_client ? buf : "");
}

void tmate_notify_client_join(struct client *c)
{
	mate_notify_message(c, 1);
}

void tmate_notify_client_left(struct client *c)
{
	mate_notify_message(c, 0);
}

void tmate_client_resize(u_int sx, u_int sy)
{
	pack(array, 3);
	pack(int, TMATE_CLIENT_RESIZE);
	/* cast to signed, -1 == no clients */
	pack(int, sx);
	pack(int, sy);
}

void tmate_client_pane_key(int pane_id, int key)
{
	/*
	 * We don't specify the pane id because the current active pane is
	 * behind, so we'll let master send the key to its active pane.
	 */

	pack(array, 2);
	pack(int, TMATE_CLIENT_PANE_KEY);
	pack(int, key);
}

static const struct cmd_entry *local_cmds[] = {
	&cmd_bind_key_entry,
	&cmd_unbind_key_entry,
	&cmd_set_option_entry,
	&cmd_set_window_option_entry,
	&cmd_detach_client_entry,
	&cmd_attach_session_entry,
	NULL
};

int tmate_should_exec_cmd_locally(const struct cmd_entry *cmd)
{
	const struct cmd_entry **ptr;

	for (ptr = local_cmds; *ptr; ptr++)
		if (*ptr == cmd)
			return 1;
	return 0;
}

void tmate_client_cmd(int client_id, const char *cmd)
{
	pack(array, 3);
	pack(int, TMATE_CLIENT_EXEC_CMD);
	pack(int, client_id);
	pack(string, cmd);
}

void tmate_client_set_active_pane(int client_id, int win_idx, int pane_id)
{
	char cmd[1024];

	sprintf(cmd, "select-pane -t %d.%d", win_idx, pane_id);
	tmate_client_cmd(client_id, cmd);
}
