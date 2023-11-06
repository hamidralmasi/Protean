
Protean
================================
This repository contains the implementation of the [Protean](https://ieeexplore.ieee.org/document/10229046) on The Network Simulator, Version 3.

## Table of Contents:


1) [Building ns-3](#building-ns-3)
2) [Running Protean](#running-protean)
3) [Input Evaluation Parameters](#input-evaluation-parameters)
4) [Parsing the Results](#parsing-the-results)



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
./waf --run "protean-evaluation --DT_alpha=$DT_ALPHA --Protean_beta=$PROTEAN_BETA \
--per_packet_Protean=$PER_PACKET_PROTEAN --load=$LOAD --StartTime=$START_TIME --EndTime=$END_TIME \
--FlowLaunchEndTime=$FLOW_END_TIME --serverCount=$SERVERS --spineCount=$SPINES \
--leafCount=$LEAVES --linkCount=$LINKS --spineLeafCapacity=$LEAF_SPINE_CAP --leafServerCapacity=$SERVER_LEAF_CAP \
--linkLatency=$LATENCY --TcpProt=$TCP --BufferSize=$BUFFER --statBuf=$STATIC_BUFFER --algorithm=$ALG \
--RedMinTh=$RED_MIN --RedMaxTh=$RED_MAX --request=$BURST_SIZE --queryRequestRate=$BURST_FREQ \
--nPrior=$N_PRIO --alphasFile=$ALPHAFILE --cdfFileName=$CDFFILE --alphaUpdateInterval=$ALPHA_UPDATE_INT \
--fctOutFile=$FLOWFILE --torOutFile=$TORFILE --algOutFile=$ALGFILE \
--useMaxDqDt=$USE_MAX_DQDT --useNormalizedDqDt=$USE_NORMALIZED_DQDT \
--useMultiPrioThresh=$USE_MULTI_PRIO --buildup_thresh=$BUILDUP_THRESH --proteanAlphasFile=$PROTEAN_ALPHAFILE"; echo "$FLOWFILE"
```

Some example evaluation scenarios can be found in [`examples/Protean/run-ALL.sh`](https://github.com/hamidralmasi/Protean/blob/master/examples/Protean/run-ALL.sh)

## Input Evaluation Parameters

| Parameter | Description |
|-----------|-------------|
| `StartTime` | Start time of the simulation |
| `EndTime` | End time of the simulation |
| `FlowLaunchEndTime` | End time of the flow launch period |
| `randomSeed` | Random seed, 0 for random generated |
| `load` | Load of the network, 0.0 - 1.0 |
| `serverCount` | The Server count |
| `spineCount` | The Spine count |
| `leafCount` | The Leaf count |
| `linkCount` | The Link count |
| `spineLeafCapacity` | Spine <-> Leaf capacity in Gbps |
| `leafServerCapacity` | Leaf <-> Server capacity in Gbps |
| `linkLatency` | linkLatency in microseconds |
| `TcpProt` | Tcp protocol |
| `BufferSize` | BufferSize in Bytes |
| `statBuf` | staticBuffer in fraction of Total buffersize |
| `algorithm` | Buffer Management algorithm |
| `RedMinTh` | Min Threshold for RED in packets |
| `RedMaxTh` | Max Threshold for RED in packets |
| `UseEcn` | Ecn Enabled |
| `request` | Query Size in Bytes |
| `queryRequestRate` | Query request rate (poisson arrivals) |
| `nPrior` | number of priorities |
| `alphasFile` | alpha values file (should be exactly nPrior lines) |
| `proteanAlphasFile` | Protean alpha values file (should be exactly nPrior lines) |
| `cdfFileName` | File name for flow distribution |
| `cdfName` | Name for flow distribution |
| `printDelay` | printDelay in NanoSeconds |
| `alphaUpdateInterval` | (Number of Rtts) update interval for alpha values |
| `fctOutFile` | File path for FCTs |
| `torOutFile` | File path for ToR statistic |
| `rto` | min Retransmission timeout value in MicroSeconds |
| `torPrintall` | torPrintall |
| `DT_alpha` | alpha parameter in Dynamic Threshold |
| `Protean_beta` | beta parameter in Protean EWMA |
| `buildup_thresh` | Threshold for queue buildup |
| `per_packet_Protean` | whether Protean calculations should be based on packets and unit time (as opposed to bytes and absolute time) |
| `useMaxDqDt` | use Max(Dq/Dt s) instead of EWMA |
| `useNormalizedDqDt` | use DqDt/Sum(DqDt) for positive ones instead of raw DqDt |
| `useMultiPrioThresh` | use smaller buildup threshold with multiple priorities |
| `useHighPrioShorts` | prioritize short flows |

## Parsing the Results

To read and parse the output files such as `fctOutFile` you can pass corresponding parameters to `results.sh` script which in turn calculated the desired evaluation metrics.

## Citation
```bib
@INPROCEEDINGS{10229046,
  author={Almasi, Hamidreza and Vardekar, Rohan and Vamanan, Balajee},
  booktitle={IEEE INFOCOM 2023 - IEEE Conference on Computer Communications}, 
  title={Protean: Adaptive Management of Shared-Memory in Datacenter Switches}, 
  year={2023},
  pages={1-10},
  doi={10.1109/INFOCOM53939.2023.10229046}}
```

Correspondence to: Hamidreza Almasi <halmas3@uic.edu>.
