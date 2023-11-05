import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys
linerate=10
rtt=8 * 10
bdp=linerate*1e9*rtt*1e-6/8
protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-114-0.8-0.75-1-0.25-0-0-1-2.25.fct"
dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-101-0.8-0.75-1-0.25-0-0-1-2.25.fct"
cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-103-0.8-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]

protean_flowDf=pd.read_csv(protean_fctfile,delimiter=' ')
dt_flowDf=pd.read_csv(dt_fct_file,delimiter=' ')
cs_flowDf=pd.read_csv(cs_fct_file,delimiter=' ')

protean_shortFlows=protean_flowDf[(protean_flowDf["flowsize"] < bdp)]
#protean_shortFlows=list(protean_shortFlows["slowdown"])

dt_shortFlows=dt_flowDf[(dt_flowDf["flowsize"] < bdp)]
#dt_shortFlows=list(dt_shortFlows["slowdown"])

cs_shortFlows=cs_flowDf[(cs_flowDf["flowsize"] < bdp)]
#cs_shortFlows=list(cs_shortFlows["slowdown"])

sns.set_theme(style="darkgrid")
sns.ecdfplot(data=protean_shortFlows, x="slowdown",label="Protean")
sns.ecdfplot(data=dt_shortFlows, x="slowdown", label="DT")
sns.ecdfplot(data=cs_shortFlows, x="slowdown", label="CS")

plt.title("Short FCT")
plt.xlabel("Slowdown with DCTCP and load=80%")

plt.legend()
plt.savefig("/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/cdf-short-fcts-single-large-2-102-103-114-0.8-0.75-1-0.25-0-0-1-2.25.png")