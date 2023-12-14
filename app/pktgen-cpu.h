/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_CPU_H_
#define _PKTGEN_CPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * pktgen_config_init - Init the configuration information
 *
 * DESCRIPTION
 * initialize the configuration information
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void pktgen_cpu_init(void);

/**
 *
 * pktgen_page_cfg - Display the CPU page.
 *
 * DESCRIPTION
 * Display the CPU page for a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void pktgen_page_cpu(void);

static inline uint8_t
sct(uint8_t s, uint8_t c, uint8_t t)
{
    lc_info_t *lc;

    for (int i = 0; i < coremap_core_cnt(); i++) {
        lc = coremap_get(i);

        if (lc->s.socket_id == s && lc->s.core_id == c && lc->s.thread_id == t)
            return lc->s.id;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _PKTGEN_CPU_H_ */
