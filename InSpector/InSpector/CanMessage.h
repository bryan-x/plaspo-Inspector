#pragma once

// 2016:10:10 02:24:40 NO TS DLC 00 00 00 00 00 00 00 00
typedef struct _CanMessage {
	ns_uint8 ch;
	long id;				/**< Message Id 11/29 bit */
	unsigned int flag;
	unsigned int dlc;		/**< Data Length (0..8) */
	struct tm c;			/**< Current Time */
	unsigned long no;		/**< Message Number */
	ns_uint32 ts;			/**< timestamp */
	ns_uint8 data[8];		/**< Data (0..8) */
} CanMessage;