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

results_path = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Res-Diff_Alpha-L2090-BF123/results"
sns.set(style='ticks')
sns.set_context("talk")
palette = sns.color_palette("colorblind", 10)

# sns.set(font_scale=2)
# sns.set(rc={'figure.figsize':(8,8)})
data = []

for tcp in [cubic, dctcp]:
    for load in ["0.2","0.4","0.6","0.8"]:
        for alg in [dt, cs, protean]:
            for burst_freq in ["1"]:
                for burst_size in ["0.75"]:
                    for protean_beta in ["0.25"]:
                        for use_normalized_dqdt in ["0"]:
                            for use_max_dqdt in ["0"]:
                                for use_multi_prio in ["1"]:
                                    for use_high_prio_shorts in ["1"]:
                                        for buildup_thresh in ["2.25"]:
                                                fctfile = results_path + "/fcts-single-large-" + tcp + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".fct"
                                                fctDf=pd.read_csv(fctfile,delimiter=' ')
                                                longFlows=fctDf[(fctDf["flowsize"]>=2000000)]
                                                # find max finish time
                                                latestTime = (longFlows["time"].max() - 1e10) / 1e9
                                                # print("latestTime: " + str(latestTime))
                                                longGoodput=np.sum(list(longFlows["flowsize"])) * 8 / latestTime / 1e9
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

                                                data.append([c_tcp, c_load, c_alg, longGoodput])
                                                

df = pd.DataFrame(data, columns=["tcp", "load", "alg", "goodput"])
cubic_df = df[df["tcp"] == "CUBIC"]
dctcp_df = df[df["tcp"] == "DCTCP"]

# TCP ######################################################################################################################

bar = sns.barplot(data = cubic_df ,x = "load" ,y = "goodput", hue="alg", palette=[palette[3], palette[0], "black"])

hatches = ['o', '-', 'x']
# Loop over the bars
for i,thisbar in enumerate(bar.patches):
    # Set a different hatch for each bar
    thisbar.set_hatch(hatches[(i // 4) % 3])

# plt.title("Throughput", fontsize=20)
plt.xlabel("Load(%)", fontsize=20)
plt.ylabel("Goodput (Gbps)", fontsize=18)

plt.legend(loc='upper left', fontsize=20, facecolor='white', edgecolor='white', framealpha=1)
plt.tick_params(axis='both', which='major', labelsize=20)
plt.grid(color='gray', linestyle="-")
plt.rc('axes', axisbelow=True)
plt.gca().xaxis.grid(False)
plt.tight_layout()
plt.savefig(results_path + "/goodput-" + "1" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
plt.figure()

# DCTCP ######################################################################################################################
bar = sns.barplot(data = dctcp_df ,x = "load" ,y = "goodput", hue="alg", palette=[palette[3], palette[0], "black"])

hatches = ['o', '-', 'x']
# Loop over the bars
for i,thisbar in enumerate(bar.patches):
    # Set a different hatch for each bar
    thisbar.set_hatch(hatches[(i // 4) % 3])

# plt.title("Throughput", fontsize=20)
plt.xlabel("Load(%)", fontsize=20)
plt.ylabel("Goodput (Gbps)", fontsize=18)

plt.legend(loc='upper left', fontsize=20, facecolor='white', edgecolor='white', framealpha=1)
plt.tick_params(axis='both', which='major', labelsize=20)
plt.grid(color='gray', linestyle="-")
plt.rc('axes', axisbelow=True)
plt.gca().xaxis.grid(False)
plt.tight_layout()
plt.savefig(results_path + "/goodput-" + "2" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
plt.figure()