/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include "include/wireguard.h"

struct wgallowedip {
	uint16_t family;
	union {
		struct in_addr ip4;
		struct in6_addr ip6;
	};
	uint8_t cidr;
	struct wgallowedip *next_allowedip;
};

enum {
	WGPEER_REMOVE_ME = 1U << 0,
	WGPEER_REPLACE_ALLOWEDIPS = 1U << 1,
	WGPEER_HAS_PUBLIC_KEY = 1U << 2,
	WGPEER_HAS_PRESHARED_KEY = 1U << 3,
	WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL = 1U << 4
};

struct wgpeer {
	uint32_t flags;

	uint8_t public_key[WG_KEY_LEN];
	uint8_t preshared_key[WG_KEY_LEN];

	union {
		struct sockaddr addr;
		struct sockaddr_in addr4;
		struct sockaddr_in6 addr6;
	} endpoint;

	struct timespec last_handshake_time;
	uint64_t rx_bytes, tx_bytes;
	uint16_t persistent_keepalive_interval;

	struct wgallowedip *first_allowedip, *last_allowedip;
	struct wgpeer *next_peer;
};

enum {
	WGDEVICE_REPLACE_PEERS = 1U << 0,
	WGDEVICE_HAS_PRIVATE_KEY = 1U << 1,
	WGDEVICE_HAS_PUBLIC_KEY = 1U << 2,
	WGDEVICE_HAS_LISTEN_PORT = 1U << 3,
	WGDEVICE_HAS_FWMARK = 1U << 4
};

struct wgdevice {
	char name[IFNAMSIZ];
	uint32_t ifindex;

	uint32_t flags;

	uint8_t public_key[WG_KEY_LEN];
	uint8_t private_key[WG_KEY_LEN];

	uint32_t fwmark;
	uint16_t listen_port;

	struct wgpeer *first_peer, *last_peer;
};

#define for_each_wgpeer(__dev, __peer) for ((__peer) = (__dev)->first_peer; (__peer); (__peer) = (__peer)->next_peer)
#define for_each_wgallowedip(__peer, __allowedip) for ((__allowedip) = (__peer)->first_allowedip; (__allowedip); (__allowedip) = (__allowedip)->next_allowedip)

static inline void free_wgdevice(struct wgdevice *dev)
{
	if (!dev)
		return;
#if 0 /* Fix C99 mode */
	for (struct wgpeer *peer = dev->first_peer, *np = peer ? peer->next_peer : NULL; peer; peer = np, np = peer ? peer->next_peer : NULL) {
		for (struct wgallowedip *allowedip = peer->first_allowedip, *na = allowedip ? allowedip->next_allowedip : NULL; allowedip; allowedip = na, na = allowedip ? allowedip->next_allowedip : NULL)
			free(allowedip);
		free(peer);
	}
#else
	struct wgpeer *peer, *np;
	for (peer = dev->first_peer, np = peer ? peer->next_peer : NULL; peer; peer = np, np = peer ? peer->next_peer : NULL) {
		struct wgallowedip *allowedip, *na;
		for (allowedip = peer->first_allowedip, na = allowedip ? allowedip->next_allowedip : NULL; allowedip; allowedip = na, na = allowedip ? allowedip->next_allowedip : NULL)
			free(allowedip);
		free(peer);
	}
#endif
	free(dev);
}

#endif
