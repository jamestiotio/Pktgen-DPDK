/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2023 Intel Corporation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <alloca.h>
#include <locale.h>

#include <pktperf.h>

/**
 *
 * packet_rate - Calculate the transmit rate.
 *
 * DESCRIPTION
 * Calculate the number of cycles to wait between sending bursts of traffic.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
packet_rate(port_t *port)
{
    uint64_t link_speed, wire_size, pps, cpb;

    wire_size       = ((((uint64_t)info->pkt_size - RTE_ETHER_CRC_LEN) + PKT_OVERHEAD_SIZE) * 8);
    port->wire_size = wire_size;
    if (port->link.link_speed == 0) {
        port->tx_cycles  = 0;
        port->link_speed = 0;
        port->pps        = 0;
        return;
    }

    link_speed = (uint64_t)port->link.link_speed * Million; /* convert to bit rate */
    pps        = (((link_speed / wire_size) * ((info->tx_rate == 0) ? 1 : info->tx_rate)) / 100);
    pps        = ((pps > 0) ? pps : 1);
    cpb        = (rte_get_timer_hz() / pps) * (uint64_t)info->burst_count; /* Cycles per Burst */

    port->tx_cycles = (info->tx_rate == 0) ? 0 : (uint64_t)port->num_tx_qids * cpb;

    port->link_speed = link_speed;
    port->pps        = pps;

    DBG_PRINT("      Speed:%'4" PRIu64 " Gbit, Bits: %'6" PRIu64 ", PPS: %'12" PRIu64
              ", CPB: %'" PRIu64 "\n",
              link_speed / Billion, wire_size, pps, port->tx_cycles);
}

static __inline__ long
get_rand(long range)
{
    return (random() >> 8) % range;
}

/*
 * IPv4/UDP packet
 * Port Src/Dest       :           1234/ 5678
 * Pkt Type            :           IPv4 / UDP
 * IP  Destination     :           198.18.1.1
 *     Source          :        198.18.0.1/24
 * MAC Destination     :    3c:fd:fe:e4:34:c0
 *     Source          :    3c:fd:fe:e4:38:40
 */
uint8_t *
packet_constructor(lport_t *lport)
{
    port_t *port = lport->port;
    uint8_t *pkt;
    uint16_t len;
    char addr[32];
    char name[RTE_MEMPOOL_NAMESIZE];
    struct rte_ether_hdr *eth;
    struct rte_ipv4_hdr *ipv4;
    struct rte_udp_hdr *udp;
    uint16_t lid, pid, tx_qid;

    lid    = lport->lid;
    pid    = port->pid;
    tx_qid = lport->tx_qid;

    snprintf(name, sizeof(name), "pkt_%u_%u_%u", lid, pid, tx_qid);
    DBG_PRINT(">>> Core %u(%u) Port %u:%u name '%s'\n", lid, rte_lcore_id(), pid, tx_qid, name);

    pkt = rte_zmalloc_socket(name, PKT_BUFF_SIZE, RTE_CACHE_LINE_SIZE, rte_eth_dev_socket_id(pid));
    if (!pkt)
        rte_exit(EXIT_FAILURE, "Failed to allocate memory for packet on %u:%u:%u\n", lid, pid,
                 tx_qid);

    eth  = (struct rte_ether_hdr *)pkt;
    ipv4 = (struct rte_ipv4_hdr *)(pkt + sizeof(struct rte_ether_hdr));
    udp  = (struct rte_udp_hdr *)(pkt + sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));

    for (unsigned int i = 0; i < RTE_ETHER_MAX_LEN; i++)
        pkt[i] = (uint8_t)(32 + (i % (127 - 32)));

    eth->dst_addr.addr_bytes[0] = 0x00;
    eth->dst_addr.addr_bytes[1] = 0xaa;
    eth->dst_addr.addr_bytes[2] = 0xbb;
    eth->dst_addr.addr_bytes[3] = 0xcc;
    eth->dst_addr.addr_bytes[4] = 0xdd;
    eth->dst_addr.addr_bytes[5] = tx_qid;

    memcpy(eth->src_addr.addr_bytes, port->mac_addr.addr_bytes, sizeof(struct rte_ether_addr));
    eth->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

    ipv4->version_ihl     = 0x45;
    ipv4->type_of_service = 0;
    ipv4->total_length =
        rte_cpu_to_be_16(info->pkt_size - sizeof(struct rte_ether_hdr) - RTE_ETHER_CRC_LEN);
    ipv4->packet_id       = rte_cpu_to_be_16(1);
    ipv4->fragment_offset = 0;
    ipv4->time_to_live    = 64;
    ipv4->next_proto_id   = IPPROTO_UDP;
    ipv4->hdr_checksum    = 0;

    snprintf(addr, sizeof(addr), "192.18.0.%u", (uint8_t)(get_rand(200) + 1));
    inet_pton(AF_INET, addr, &ipv4->dst_addr);
    snprintf(addr, sizeof(addr), "192.18.0.%u", (uint8_t)(get_rand(200) + 1));
    inet_pton(AF_INET, addr, &ipv4->src_addr);

    udp->src_port = rte_cpu_to_be_16(get_rand(0xFFFE) + 1);
    udp->dst_port = rte_cpu_to_be_16(get_rand(0xFFFe) + 1);

    len = (uint16_t)(info->pkt_size - sizeof(struct rte_ether_hdr) - sizeof(struct rte_ipv4_hdr) -
                     RTE_ETHER_CRC_LEN);
    udp->dgram_len   = rte_cpu_to_be_16(len);
    udp->dgram_cksum = 0;
    udp->dgram_cksum = rte_ipv4_udptcp_cksum(ipv4, udp);

    ipv4->hdr_checksum = rte_ipv4_cksum(ipv4);

    return pkt;
}
