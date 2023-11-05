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

results_path = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/20Sec/results"
sns.set(style='ticks')
sns.set_context("talk")
palette = sns.color_palette("colorblind", 10)

# sns.set(font_scale=2)
# sns.set(rc={'figure.figsize':(8,8)})
data = []

for tcp in [cubic, dctcp]:
    for load in ["0.2", "0.4", "0.6", "0.8"]:
        for alg in [dt, cs, protean]:
            for burst_freq in ["1"]:
                for burst_size in ["0.75"]:
                    for protean_beta in ["0.25"]:
                        for use_normalized_dqdt in ["0"]:
                            for use_max_dqdt in ["0"]:
                                for use_multi_prio in ["1"]:
                                    for use_high_prio_shorts in ["1"]:
                                        for buildup_thresh in ["2.25"]:
                                                torfile = results_path + "/tor-single-large-" + tcp + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".stat"
                                                torDf=pd.read_csv(torfile,delimiter=' ',header=[0])
                                                torDf = torDf.iloc[: , :-1]
                                                torDf=torDf[(torDf["time"]>10000000000)&(torDf["time"]<30000000000)&(torDf["priority1"] > 0)&(torDf["priority2"] > 0)]
                                                p_ack = list(torDf["priority0"])
                                                p_incast = list(torDf["priority1"])
                                                p_nonincast = list(torDf["priority2"])
                                                perc_incast = np.divide(np.array(p_incast) * 100 ,np.array(p_ack)+np.array(p_incast)+np.array(p_nonincast))
                                                perc_incast.sort()
                                                perc_inc999 = perc_incast[int(len(perc_incast)*0.999)-1]
                                                perc_inc99 = perc_incast[int(len(perc_incast)*0.99)-1]
                                                perc_inc95 = perc_incast[int(len(perc_incast)*0.95)-1]
                                                perc_incavg = np.mean(perc_incast)
                                                median_inc = np.median(perc_incast)

                                                perc_nonincavg = 100 - perc_incavg
                                                perc_noninc999 = 100 - perc_inc999
                                                perc_noninc99 = 100 - perc_inc99
                                                perc_noninc95 = 100 - perc_inc95
                                                median_noninc = 100 - median_inc
                                                print(perc_incavg, perc_nonincavg)
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
                                                c_load = "{0:.0f}".format(float(load)*100)

                                                data.append([c_tcp, c_load, c_alg, perc_incavg, perc_nonincavg, median_inc, median_noninc, perc_inc999, perc_noninc999, perc_inc99, perc_noninc99, perc_inc95, perc_noninc95])
                                                

df = pd.DataFrame(data, columns=["tcp", "load", "alg", "incast_avg", "nonincast_avg", "median_inc", "median_noninc", "incast_999", "nonincast_999", "incast_99", "nonincast_99", "incast_95", "nonincast_95"])
cubic_df = df[df["tcp"] == "CUBIC"]
dctcp_df = df[df["tcp"] == "DCTCP"]

for c_y in ["incast_avg", "median_inc"]:

        # TCP ######################################################################################################################
        bar = sns.barplot(data = cubic_df ,x = "load" ,y = c_y, hue="alg", palette=[palette[3], palette[0], "black"])
        
        hatches = ['o', '-', 'x']
        # Loop over the bars
        for i,thisbar in enumerate(bar.patches):
                # Set a different hatch for each bar
                thisbar.set_hatch(hatches[(i // 4) % 3])

        # plt.title("Throughput", fontsize=20)
        plt.xlabel("Load(%)", fontsize=20)
        plt.ylabel("% Incast Occupancy", fontsize=18)

        plt.legend(loc='upper left', fontsize=17, facecolor='white', edgecolor='white', ncol = 3,  framealpha=1, bbox_to_anchor=(-0.02, 1.2))
        plt.tick_params(axis='both', which='major', labelsize=20)
        plt.grid(color='gray', linestyle="-")
        plt.rc('axes', axisbelow=True)
        plt.gca().xaxis.grid(False)
        plt.tight_layout()
        plt.savefig(results_path + "/stack-" + "1-" + c_y + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".png")
        plt.savefig(results_path + "/stack-" + "1-" + c_y + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")

        plt.figure()

        # DCTCP ######################################################################################################################
        bar = sns.barplot(data = dctcp_df ,x = "load" ,y = c_y, hue="alg", palette=[palette[3], palette[0], "black"])

        hatches = ['o', '-', 'x']
        # Loop over the bars
        for i,thisbar in enumerate(bar.patches):
                # Set a different hatch for each bar
                thisbar.set_hatch(hatches[(i // 4) % 3])

        # plt.title("Throughput", fontsize=20)
        plt.xlabel("Load(%)", fontsize=20)
        plt.ylabel("% Incast Occupancy", fontsize=18)

        plt.legend(loc='upper left', fontsize=17, facecolor='white', edgecolor='white', ncol = 3, framealpha=1, bbox_to_anchor=(-0.02, 1.2))
        plt.tick_params(axis='both', which='major', labelsize=20)
        plt.grid(color='gray', linestyle="-")
        plt.rc('axes', axisbelow=True)
        plt.gca().xaxis.grid(False)
        plt.tight_layout()
        plt.savefig(results_path + "/stack-" + "2-" + c_y + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".png")
        plt.savefig(results_path + "/stack-" + "2-" + c_y + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")

        plt.figure()