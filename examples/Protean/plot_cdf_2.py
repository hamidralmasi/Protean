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
palette = sns.color_palette("colorblind", 10)

# sns.set(font_scale=2)
# sns.set(rc={'figure.figsize':(8,8)})

for tcp in [cubic, dctcp]:
    for load in ["0.4","0.8"]:
        max_fct = 0
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
                                            flows=flowDF[(flowDF["incast"] == 1)]
                                            if (flows.max()["fct"] > max_fct):
                                                max_fct = flows.max()["fct"]

            if (alg == protean):
                c_label = "Protean"
                c_color = "black"
            elif (alg == dt):
                c_label = "DT"
                c_color = palette[3]
            elif (alg == cs):
                c_label = "CS"
                c_color = palette[0]
            elif (alg == fab):
                c_label = "FAB"
                c_color = palette[1]
            elif (alg == ib):
                c_label = "IB"
                c_color = palette[2]

            if (tcp == dctcp):
                c_linestyle = "-"
            elif (tcp == cubic):
                c_linestyle = "-"
            

            sns.ecdfplot(data=flows, x="fct",label=c_label, ls = c_linestyle, color=c_color)
            
        # plt.title("Incast FCT", fontsize=20)
        if (tcp == cubic):
            plt.xlabel("FCT (ms)", fontsize=20)
        elif (tcp == dctcp):
            plt.xlabel("FCT (ms)", fontsize=20)
        plt.ylabel("CDF", fontsize=20)
        plt.ylim(0.8, 1.0)
        plt.legend(loc='center right', fontsize=20, facecolor='white', edgecolor='white', framealpha=1)
        plt.tick_params(axis='both', which='major', labelsize=20)
        plt.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))
        plt.xticks(np.arange(0, max_fct+1, 5))
        plt.rc('axes', axisbelow=True)
        plt.tight_layout()
        plt.savefig(results_path + "/incast-fct-cdf-" + tcp + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
        plt.savefig(results_path + "/incast-fct-cdf-" + tcp + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".png")

        plt.figure()
