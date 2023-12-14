#!/bin/bash
#
sudo ./Builddir/examples/pktperf/pktperf -l 2,4-9,14-19 -a 03:00.0 -a 82:00.0 -- -r $1 -m "4-6:7-9.0" -m "14-16:17-19.1"
#sudo ./Builddir/examples/pktperf/pktperf -l 2,4-6,14-16 -a 03:00.0 -a 82:00.0 -- -r $1 -m "4-6.0" -m "14-16.1"
#sudo gdb -arg ./Builddir/examples/pktperf/pktperf -l 2,4-6,14-16 -a 03:00.0 -a 82:00.0 -- -r $1 -m "4-6.0" -m "14-16.1"
