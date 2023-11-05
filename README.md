
Protean
================================
This folder contains the implementation of the [Protean]([https://arxiv.org/abs/2302.05865](https://ieeexplore.ieee.org/document/10229046)) on The Network Simulator, Version 3.

## Table of Contents:


1) [Building ns-3](#building-ns-3)
2) [Running Protean](#running-protean)



## Building ns-3

The code for the framework and the default models provided
by ns-3 is built as a set of libraries. User simulations
are expected to be written as simple programs that make
use of these ns-3 libraries.

To build the set of default libraries and the example
programs included in this package, you need to use the
tool 'waf'. Detailed information on how to use waf is
included in the file doc/build.txt

However, the real quick and dirty way to get started is to
type the command
```shell
./waf configure --enable-examples
```

followed by

```shell
./waf
```

in the directory which contains this README file. The files
built will be copied in the build/ directory.

The current codebase is expected to build and run on the
set of platforms listed in the [release notes](RELEASE_NOTES)
file.

Other platforms may or may not work: we welcome patches to
improve the portability of the code to these other platforms.

## Running Protean

On recent Linux systems, once you have built ns-3 (with examples
enabled), it should be easy to run the sample programs with the
following command, such as:

```shell
./waf --run simple-global-routing
```

That program should generate a `simple-global-routing.tr` text
trace file and a set of `simple-global-routing-xx-xx.pcap` binary
pcap trace files, which can be read by `tcpdump -tt -r filename.pcap`
The program source can be found in the examples/routing directory.

Protean evaluation is added as simulation files and scripts in [`examples/Protean`](https://github.com/hamidralmasi/Protean/tree/master/examples/Protean).

In config.sh set NS3 to the path of the current directory. You can run a particular evaluation scenario by setting the desired parameters to `protean-evaluation.cc` with this command:

```shell
./waf --run "protean-evaluation \
--DT_alpha=$DT_ALPHA \
--Protean_beta=$PROTEAN_BETA \
--per_packet_Protean=$PER_PACKET_PROTEAN \
--load=$LOAD \
--StartTime=$START_TIME \
--EndTime=$END_TIME \
--FlowLaunchEndTime=$FLOW_END_TIME \
--serverCount=$SERVERS \
--spineCount=$SPINES \
--leafCount=$LEAVES \
--linkCount=$LINKS \
--spineLeafCapacity=$LEAF_SPINE_CAP \
--leafServerCapacity=$SERVER_LEAF_CAP \
--linkLatency=$LATENCY \
--TcpProt=$TCP \
--BufferSize=$BUFFER \
--statBuf=$STATIC_BUFFER \
--algorithm=$ALG \
--RedMinTh=$RED_MIN \
--RedMaxTh=$RED_MAX \
--request=$BURST_SIZE \
--queryRequestRate=$BURST_FREQ \
--nPrior=$N_PRIO \
--alphasFile=$ALPHAFILE \
--cdfFileName=$CDFFILE \
--alphaUpdateInterval=$ALPHA_UPDATE_INT \
--fctOutFile=$FLOWFILE \
--torOutFile=$TORFILE \
--algOutFile=$ALGFILE \
--useMaxDqDt=$USE_MAX_DQDT \
--useNormalizedDqDt=$USE_NORMALIZED_DQDT \
--useMultiPrioThresh=$USE_MULTI_PRIO \
--buildup_thresh=$BUILDUP_THRESH \
--proteanAlphasFile=$PROTEAN_ALPHAFILE"; echo "$FLOWFILE"
```

Some example evaluation scenarios can be found in [`examples/Protean/run-ALL.sh`](https://github.com/hamidralmasi/Protean/blob/master/examples/Protean/run-ALL.sh)


