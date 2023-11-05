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

results_path = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/Pressure/results"
sns.set(style='ticks')
sns.set_context("talk")
data = []
palette = sns.color_palette("colorblind", 10)
# sns.set(font_scale=2)
# sns.set(rc={'figure.figsize':(8,8)})
for buffer in ["19.2","9.6","5.12","3.44"]:
    for tcp in [dctcp]:
        for load in ["0.4","0.5","0.6","0.7","0.8"]:
            for alg in [dt, cs, protean]:
                for burst_freq in ["1"]:
                    for burst_size in ["0.125","0.25","0.375", "0.5", "0.625", "0.75"]:
                        for protean_beta in ["0.125"]:
                            for use_normalized_dqdt in ["0"]:
                                for use_max_dqdt in ["0"]:
                                    for use_multi_prio in ["1"]:
                                        for use_high_prio_shorts in ["1"]:
                                            for buildup_thresh in ["2.25"]:

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
                                                if (alg == protean):
                                                        c_alg = "PROTEAN"
                                                elif (alg == dt):
                                                        c_alg = "DT"
                                                elif (alg == cs):
                                                        c_alg = "CS"
                                                c_load = "{0:.0f}".format(float(load)*100)
                                                c_burst_size = "{0:.1f}".format(float(burst_size)*100).rstrip('0').rstrip('.')
                                                data.append([c_alg,c_load,fct999,fct99,fct95,fctavg, buffer, c_burst_size])

df = pd.DataFrame(data, columns=['alg','load','fct999','fct99','fct95','fctavg','buffer','burst_size'])

for load in ["0.4","0.8"]:
    for buffer in ["9.6"]:
        loadstr = "{0:.0f}".format(float(load)*100)
        pressure_df = df[(df["load"] == loadstr) & (df["buffer"] == buffer)]

        sns.lineplot(data = pressure_df ,x = "burst_size" ,y = "fct999", hue="alg", style="alg", markers=['X', 'o', 's'], palette=[palette[3], palette[0], "black"], dashes = ['','',''])
        # plt.title("Buffer Pressure with Load= " + loadstr + "%", fontsize=20)
        plt.xlabel("Incast Size (% of buffer)", fontsize=20)
        plt.ylabel("FCT (ms)", fontsize=18)
        # plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=3)
        plt.legend(loc="best", fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
        plt.tick_params(axis='both', which='major', labelsize=20)
        plt.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))
        plt.rc('axes', axisbelow=True)
        plt.tight_layout()
        plt.savefig(results_path + "/pressure-fct999-" + "2" + "-" + alg + "-" + load + "-" + burst_size + "-" + buffer + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
        plt.figure()




# pressure_df = df[df["load"] == "50" & df["burst_size"] == "0.5"]

# sns.lineplot(data = pressure_df ,x = "buffer" ,y = "fct999", hue="alg", marker="8")

# plt.title("Short FCT", fontsize=20)
# plt.xlabel("Buffer Size(KB per port per Gbps)", fontsize=20)
# plt.ylabel("99.9th %-ile Incast FCT", fontsize=18)
# # plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=3)
# plt.legend(loc="best", fontsize=16)
# plt.tick_params(axis='both', which='major', labelsize=20)
# plt.tight_layout()
# plt.savefig(results_path + "/pressure-fct999-" + "1" + "-" + alg + "-" + load + "-" + burst_size + "-" + burst_freq + "-" + protean_beta + "-" + use_normalized_dqdt + "-" + use_max_dqdt + "-" + use_multi_prio + "-" + use_high_prio_shorts + "-" + buildup_thresh + ".pdf")
# plt.figure()

