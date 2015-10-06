#ifndef TMATE_PROTOCOL_H
#define TMATE_PROTOCOL_H

 /* 17 and not 16 because the sender does not takes into account envelope size */
#define TMATE_MAX_MESSAGE_SIZE (17*1024)

/* TODO document each msg */

enum tmate_control_out_msg_types {
	TMATE_CTL_AUTH,
	TMATE_CTL_DEAMON_OUT_MSG,
	TMATE_CTL_SNAPSHOT,
	TMATE_CTL_CLIENT_JOIN,
	TMATE_CTL_CLIENT_LEFT,
};

/*
[TMATE_CTL_AUTH, int: ctl_proto_version, string: ip_address, string: pubkey,
                 string: session_token, string: session_token_ro]
[TMATE_CTL_DEAMON_OUT_MSG, object: msg]
[TMATE_CTL_SNAPSHOT, [[int: pane_id, [int: cur_x, int: cur_y], int: mode,
                       [[string: line_utf8, [int: char_attr, ...]], ...], ...], ...]]
[TMATE_CTL_CLIENT_JOIN, int: client_id, string: ip_address, string: pubkey]
[TMATE_CTL_CLIENT_LEFT, int: client_id]
*/

enum tmate_control_in_msg_types {
	TMATE_CTL_DEAMON_FWD_MSG,
	TMATE_CTL_REQUEST_SNAPSHOT,
	TMATE_CTL_PANE_KEYS,
	TMATE_CTL_RESIZE,
};

/*
[TMATE_CTL_DEAMON_FWD_MSG, object: msg]
[TMATE_CTL_REQUEST_SNAPSHOT, int: max_history_lines]
[TMATE_CTL_PANE_KEYS, int: pane_id, string: keys]
[TMATE_CTL_RESIZE, int: sx, int: sy] // sx == -1: no clients
*/

enum tmate_daemon_out_msg_types {
	TMATE_OUT_HEADER,
	TMATE_OUT_SYNC_LAYOUT,
	TMATE_OUT_PTY_DATA,
	TMATE_OUT_EXEC_CMD,
	TMATE_OUT_FAILED_CMD,
	TMATE_OUT_STATUS,
	TMATE_OUT_SYNC_COPY_MODE,
	TMATE_OUT_WRITE_COPY_MODE,
};

/*
[TMATE_OUT_HEADER, int: proto_version, string: version]
[TMATE_OUT_SYNC_LAYOUT, [int: sx, int: sy, [[int: win_id, string: win_name,
			  [[int: pane_id, int: sx, int: sy, int: xoff, int: yoff], ...],
			  int: active_pane_id], ...], int: active_win_id]
[TMATE_OUT_PTY_DATA, int: pane_id, binary: buffer]
[TMATE_OUT_EXEC_CMD, string: cmd]
[TMATE_OUT_FAILED_CMD, int: client_id, string: cause]
[TMATE_OUT_STATUS, string: left, string: right]
[TMATE_OUT_SYNC_COPY_MODE, int: pane_id, [int: backing, int: oy, int: cx, int: cy,
					  [int: selx, int: sely, int: flags],
					  [int: type, string: input_prompt, string: input_str]])
                                          // Any of the array can be []
[TMATE_OUT_WRITE_COPY_MODE, int: pane_id, string: str]
*/

enum tmate_daemon_in_msg_types {
	TMATE_IN_NOTIFY,
	TMATE_IN_PANE_KEY,
	TMATE_IN_RESIZE,
	TMATE_IN_EXEC_CMD,
	TMATE_IN_SET_ENV,
	TMATE_IN_READY,
};

/*
[TMATE_IN_NOTIFY, string: msg]
[TMATE_IN_PANE_KEY, int: key]
[TMATE_IN_RESIZE, int: sx, int: sy] // sx == -1: no clients
[TMATE_IN_EXEC_CMD, int: client_id, string: cmd]
[TMATE_IN_SET_ENV, string: name, string: value]
[TMATE_IN_READY]
*/

#endif
