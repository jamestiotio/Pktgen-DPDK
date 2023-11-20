## PKTPERF example application to verify Rx/Tx performance with multiple cores

The `pktperf` example application is used to verify Rx/Tx performance with mutiple cores or queues. The application is configured from the command line arguments and does not have CLI to configure on the fly.

---
```console
**Copyright &copy; <2023>, Intel Corporation. All rights reserved.**

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

- Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

SPDX-License-Identifier: BSD-3-Clause

pktperf: Created 2023 by Keith Wiles @ Intel.com
```
---

### Overview

The implementation uses multiple Rx/Tx queues per port, which is required to achieve maximum performance for receiving and transmitting frames. If your NIC does not support multiple queues, but does support multiple `virtual functions` per physical port, then you should create multiple `virtual functions` network devices and configure them using the `-m` mapping parameters. A single core can only be attached to one network device or port, which means you can not have a single core processing more then one network device or port.

Having a single core processing more then one network device or port was almost never used and made the code more complicated. This is why the implementation was removed from the `pktperf` application.

### Command line arguments

The command line arguments contain the standard DPDK arguments with the pktperf parameters after the '--' option. Please look at the DPDK documentation for the EAL arguments. The pktperf parameter can be displayed using the -h or --help option after the '--' option.

```console
pktperf [EAL options] -- [-b burst] [-s size] [-r rate] [-d rxd/txd] [-m map] [-T secs] [-P] [-M mbufs] [-v] [-h]
	-b|--burst-count <burst> Number of packets for Rx/Tx burst (default 32)
	-s|--pkt-size <size>     Packet size in bytes (default 64) includes FCS bytes
	-r|--rate <rate>         Packet TX rate percentage 0=off (default 100)
	-d|--descriptors <Rx/Tx> Number of RX/TX descriptors (default 1,024/1,024)
	-m|--map <map>           Core to Port/queue mapping '[Rx-Cores:Tx-Cores].port'
	-T|--timeout <secs>      Timeout period in seconds (default 1 second)
	-P|--no-promiscuous      Turn off promiscuous mode (default On)
	-M|--mbuf-count <count>  Number of mbufs to allocate (default 8,192, max 131,072)
	-v|--verbose             Verbose output
	-h|--help                Print this help
```
### Command line examples

```bash
sudo Builddir/examples/pktperf/pktperf -l 1,2-9,14-21 -a 03:00.0 -a 82:00.0 -- -T 1 -b 32 -s 64 -r 100 -m "2-5:6-9.0" -m "14-17:18-21.1"
```

The `-m` argument defines the core to port mapping `<RxCores:TxCores>.<port>` the `:` (colon) is used to specify the the Rx and Tx cores for the port mapping. Leaving off the ':' is equivalent to running Rx and Tx processing on the specified core(s). When present the left side denotes the core(s) to use for receive processing and the right side denotes the core(s) to use for transmit processing.

### Example console output

```bash
Port    : Rate Statistics per queue (/)
 0 >> Link up at 40 Gbps FDX Autoneg, WireSize 672 Bits, PPS 59,523,809, Cycles/Burst 5,120
  RxQs  :    5,882,600    5,883,576    5,883,280            0 Total:   17,649,456
  TxQs  :    4,471,776    3,619,136    4,395,520    5,248,096 Total:   17,734,528
  TxDrop:    2,676,736    3,534,048    2,759,104    1,906,432 Total:   10,876,320
  NoMBUF:            0            0            0            0 Total:            0
  TxTime:          336          330          351          339 Total:        1,356
  RxMissed: 22,464,432, ierr: 0, oerr: 0, rxNoMbuf: 0
 1 >> Link up at 100 Gbps FDX Autoneg, WireSize 672 Bits, PPS 148,809,523, Cycles/Burst 2,048
  RxQs  :   22,884,464            0   11,233,080   11,442,974 Total:   45,560,518
  TxQs  :   11,183,040   11,183,072   11,183,040   11,183,040 Total:   44,732,192
  TxDrop:    6,528,832    6,522,656    6,536,576    6,521,824 Total:   26,109,888
  NoMBUF:            0            0            0            0 Total:            0
  TxTime:          525          537          543          621 Total:        2,226
  RxMissed: 62,477,021, ierr: 0, oerr: 0, RxNoMbuf: 0

Burst: 32, MBUF Count: 12,864, PktSize:64, Rx/Tx 1,024/1,024, Rate 100%
```
