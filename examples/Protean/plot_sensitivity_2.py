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
cs="103"
protean="114"

results_path = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/Sensitivity/results"
sns.set(style='ticks')
sns.set_context("talk")
palette = sns.color_palette("colorblind", 5)
# sns.set(font_scale=2)
# sns.set(rc={'figure.figsize':(8,8)})
data = []
for buffer in ["3.44", "5.12", "9.6", "19.2"]:
        for tcp in [dctcp]:
                for load in ["0.5", "0.8"]:
                        for alg in [protean]:
                                for burst_freq in ["1"]:
                                        for burst_size in ["0.5", "0.75"]:
                                                for protean_beta in ["0.0625", "0.125", "0.25", "0.5"]:
                                                        for use_normalized_dqdt in ["0"]:
                                                                for use_max_dqdt in ["0"]:
                                                                        for use_multi_prio in ["1"]:
                                                                                for use_high_prio_shorts in ["1"]:
                                                                                        for buildup_thresh in ["0.625", "1.25", "2.5", "3.75", "5"]:
                                                                                                fct_file = results_path + "/fcts-single-large-" + tcp + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + "-" + buffer + ".fct"
                                                                                                flowDF=pd.read_csv(fct_file,delimiter=' ')
                                                                                                flowDF["fct"] = flowDF["fct"] / 1e6
                                                                                                flows=flowDF[(flowDF["incast"] == 1)]
                                                                                                flows=list(flows["fct"])
                                                                                                flows.sort()

                                                                                                fct999 = flows[int(len(flows)*0.999)-1]
                                                                                                fct99 = flows[int(len(flows)*0.99)-1]
                                                                                                fct95 = flows[int(len(flows)*0.95)-1]
                                                                                                fctavg = np.mean(flows)

                                                                                                c_load = "{0:.0f}".format(float(load)*100)
                                                                                                if (buildup_thresh == "0.625"):
                                                                                                        c_bt = "0.5"
                                                                                                elif (buildup_thresh == "1.25"):
                                                                                                        c_bt = "1"
                                                                                                elif (buildup_thresh == "2.5"):
                                                                                                        c_bt = "2"
                                                                                                elif (buildup_thresh == "3.75"):
                                                                                                        c_bt = "3"
                                                                                                elif (buildup_thresh == "5"):
                                                                                                        c_bt = "4"
                                                                                                data.append([c_load, fct999, fct99, fct95, fctavg, buffer, burst_size, protean_beta, c_bt])

df = pd.DataFrame(data, columns=["load", "fct999", "fct99", "fct95", "fctavg", "buffer", "burst_size", "protean_beta", "buildup_thresh"])

# TCP ######################################################################################################################

df_beta = df[(df["burst_size"] == "0.75") & (df["buffer"] == "9.6") & (df["load"] == "80")]
sns.lineplot(data = df_beta ,x = "protean_beta" ,y = "fct999", hue="buildup_thresh", style = "buildup_thresh", markers = ["8", "s", "D", "^", "X"], palette = palette, dashes = ['','','','',''])
# plt.title("Sensitivity to EWMA Beta", fontsize=20)
plt.xlabel("Beta", fontsize=20)
plt.ylabel("FCT (ms)", fontsize=18)
# plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=3)
plt.legend(title='Buildup Thresh.\n(Linerate)', loc="upper left", fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
# plt.yscale('log')
plt.tick_params(axis='both', which='major', labelsize=20)
plt.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))
plt.rc('axes', axisbelow=True)
plt.tight_layout()
plt.savefig(results_path + "/sensitivity-fct999-beta" + ".pdf")
plt.figure()

# # DCTCP ######################################################################################################################
# df_thresh = df[(df["burst_size"] == "0.75") & (df["protean_beta"] == "0.25") & (df["load"] == "50")]
# sns.lineplot(data = df_thresh ,x = "buffer" ,y = "fct999", hue="buildup_thresh")

# plt.title("Sensitivity to Buffer Buildup Threshold", fontsize=20)
# plt.xlabel("Buffer Size(KB per port per Gbps)", fontsize=20)
# plt.ylabel("99.9th %-ile Incast FCT", fontsize=18)
# # plt.legend(bbox_to_anchor=(0,0.75,1,0.2), loc="lower left", ncol=3)
# plt.legend(loc="best")
# # plt.yscale('log')
# plt.tick_params(axis='both', which='major', labelsize=20)
# plt.tight_layout()
# plt.savefig(results_path + "/sensitivity-fct999-threshold" + ".png")
# plt.figure()

