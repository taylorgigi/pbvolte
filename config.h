#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <stdint.h>

#define DEVICE_NAME_LEN    16

#define MODE_LIVE          1
#define MODE_OFFLINE       2

typedef struct mode_live {
	uint16_t count;
	char **device;
} mode_live;

typedef struct mode_offline {
	char *dir;
	uint16_t repeat;
} mode_offline;

typedef struct config_xdr {
	const char *dir;
	const char *surffix;
} config_xdr;

typedef struct config_pool {
	uint32_t pkt;
    uint32_t ipfrag;
    uint32_t sctpfrag;
    uint32_t tcpfrag;
    uint32_t diameter;
    uint32_t gtpv2;
    uint32_t sip;
    uint32_t rtp;
} config_pool;

typedef struct config_hash {
    uint32_t ipfrag;
    uint32_t sctpfrag;
    uint32_t tcpfrag;
    uint32_t diameter;
    uint32_t gtpv2;
    uint32_t sip;
    uint32_t rtp;
} config_hash;

typedef struct config_timeout {
    uint32_t ipfrag;
    uint32_t sctpfrag;
    uint32_t tcpfrag;
    uint32_t diameter;
    uint32_t gtpv2;
    uint32_t sip;
    uint32_t rtp;
} config_timeout;

typedef struct global_config {
	uint16_t       mode;
	uint16_t       nb_thr;
    mode_live      live;
	mode_offline   offline;
	config_xdr     xdr;
	config_pool    pool;
	config_hash    hash;
	config_timeout timeout;
} global_config;

extern global_config glb_config;

int load_config(const char *f);

#endif

