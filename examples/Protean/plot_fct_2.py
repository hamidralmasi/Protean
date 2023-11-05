from ctypes import c_long
from matplotlib import markers
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys
linerate=10
rtt=8 * 10
bdp=linerate*1e9*rtt*1e-6/8
cubic="1"
dctcp="2"
dt="101"
fab="102"
cs="103"
ib="104"
protean="114"

results_path = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/3Sec/results"
sns.set(style='ticks')
sns.set_context("talk")

# sns.set(font_scale=2)
# sns.set(rc={'figure.figsize':(8,8)})
data = []
palette = sns.color_palette("colorblind", 10)

for tcp in [cubic, dctcp]:
    for load in ["0.2","0.4","0.6","0.8"]:
        for alg in [dt, cs, fab, ib, protean]:
            for burst_freq in ["1"]:
                for burst_size in ["0.75"]:
                    for protean_beta in ["0.25"]:
                        for use_normalized_dqdt in ["0"]:
                            for use_max_dqdt in ["0"]:
                                for use_multi_prio in ["1"]:
                                    for use_high_prio_shorts in ["1"]:
                                        for buildup_thresh in ["2.25"]:
                                                fct_file = results_path + "/fcts-single-large-" + tcp + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".fct"
                                                flowDF=pd.read_csv(fct_file,delimiter=' ')
                                                flowDF["fct"] = flowDF["fct"] / 1e6

                                                short_flows=flowDF[(flowDF["flowsize"] < bdp)]
                                                incast_flows=flowDF[(flowDF["incast"] == 1)]
                                                short_flows=list(short_flows["fct"])
                                                incast_flows=list(incast_flows["fct"])
                                                short_flows.sort()
                                                incast_flows.sort()
                                                
                                                short_fct999 = short_flows[int(len(short_flows)*0.999)-1]
                                                short_fct99 = short_flows[int(len(short_flows)*0.99)-1]
                                                short_fct95 = short_flows[int(len(short_flows)*0.95)-1]
                                                short_fctavg = np.mean(short_flows)

                                                incast_fct999 = incast_flows[int(len(incast_flows)*0.999)-1]
                                                incast_fct99 = incast_flows[int(len(incast_flows)*0.99)-1]
                                                incast_fct95 = incast_flows[int(len(incast_flows)*0.95)-1]
                                                incast_fctavg = np.mean(incast_flows)

                                                if (tcp == dctcp):
                                                        c_tcp = "DCTCP"
                                                elif (tcp == cubic):
                                                        c_tcp = "CUBIC"
                                                if (alg == protean):
                                                        c_alg = "PROTEAN"
                                                elif (alg == dt):
                                                        c_alg = "DT"
                                                elif (alg == cs):
                                                        c_alg = "CS"
                                                elif (alg == fab):
                                                        c_alg = "FAB"
                                                elif (alg == ib):
                                                        c_alg = "IB"
                                                c_load = "{0:.0f}".format(float(load)*100)

                                                data.append([c_tcp, c_load, c_alg, short_fctavg, short_fct99, short_fct95, short_fct999, incast_fctavg, incast_fct99, incast_fct95, incast_fct999])

df = pd.DataFrame(data, columns=['tcp', 'load', 'alg', 'short_fctavg', 'short_fct99', 'short_fct95', 'short_fct999', 'incast_fctavg', 'incast_fct99', 'incast_fct95', 'incast_fct999'])
cubic_df = df[df["tcp"] == "CUBIC"]
dctcp_df = df[df["tcp"] == "DCTCP"]

for c_y in ['short_fctavg', 'short_fct99', 'short_fct95', 'short_fct999', 'incast_fctavg', 'incast_fct99', 'incast_fct95', 'incast_fct999']:

        # TCP ######################################################################################################################

        sns.lineplot(data = cubic_df ,x = "load" ,y = c_y, hue="alg", style = "alg", markers=['X', 'o', 'v', 'p','s'], palette=[palette[3], palette[0], palette[1], palette[2], "black"], dashes = ['','','','',''])

        # plt.title("Short FCT", fontsize=20)
        plt.xlabel("Load(%)", fontsize=20)
        plt.ylabel("FCT (ms)", fontsize=18)
        if (c_y == "short_fct999" or  c_y == "short_fctavg"):
                plt.legend(loc="upper center", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
        else:
                plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)

        if (c_y == "incast_fctavg" or c_y == "incast_fct999"):
                plt.legend(loc="upper center", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)

        plt.tick_params(axis='both', which='major', labelsize=20)
        plt.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))
        plt.rc('axes', axisbelow=True)
        plt.tight_layout()
        plt.savefig(results_path + "/" + c_y + "-1" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
        plt.savefig(results_path + "/" + c_y + "-1" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".png")

        plt.figure()

        # DCTCP ######################################################################################################################
        sns.lineplot(data = dctcp_df ,x = "load" ,y = c_y, hue="alg", style = "alg", markers=['X', 'o', 'v', 'p','s' ], palette=[palette[3], palette[0], palette[1], palette[2], "black"], dashes = ['','','','',''])
                
        # plt.title("Short FCT", fontsize=20)
        plt.xlabel("Load(%)", fontsize=20)
        plt.ylabel("FCT (ms)", fontsize=18)
        if (c_y == "short_fct999"):
                plt.legend(loc="center right", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
        else:
                plt.legend(bbox_to_anchor=(0,0.75,1,0.2), loc="lower left", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
        if (c_y == "short_fctavg"):
                plt.legend(bbox_to_anchor=(0.25,0.62,1,0.2), loc="lower left", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)

        if (c_y == "incast_fct999"):
                plt.legend(bbox_to_anchor=(0.5,0.2,1,0.2), loc="lower left", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
        if (c_y == "incast_fctavg"):
                plt.legend(bbox_to_anchor=(0,0.6,1,0.2), loc="lower left", ncol=1, fontsize=16, facecolor='white', edgecolor='white', framealpha=1)

        # plt.legend(loc="center", ncol=3)
        # plt.yscale('log')
        plt.tick_params(axis='both', which='major', labelsize=20)
        plt.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))
        plt.rc('axes', axisbelow=True)
        plt.tight_layout()
        plt.savefig(results_path + "/" + c_y + "-2" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
        plt.savefig(results_path + "/" + c_y + "-2" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".png")

        plt.figure()
