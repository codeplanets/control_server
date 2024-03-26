#pragma once

typedef int ECODE;

#define	EC_SUCCESS				0
#define EC_FAILURE              1
#define EC_NOT_CONNECTED		20000
#define EC_OPEN_FAILURE			20001
#define EC_COPY_FAILURE			20002
#define EC_BIND_FAILURE         20003
#define EC_LISTEN_FAILURE       20004
#define EC_ACCEPT_FAILURE       20005

#define EC_ALREADY_INUSE		20006
#define EC_SYSTEM_ERROR			20007
#define EC_DEVICE_FAILURE		20008
#define EC_NOT_CREATED			20009
#define EC_NOT_READY			20010

