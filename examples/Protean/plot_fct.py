import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys
linerate=10
rtt=8 * 10
bdp=linerate*1e9*rtt*1e-6/8

l20_protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-114-0.2-0.75-1-0.25-0-0-1-2.25.fct"
l20_dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-101-0.2-0.75-1-0.25-0-0-1-2.25.fct"
l20_cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-103-0.2-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]

l20_protean_flowDf=pd.read_csv(l20_protean_fctfile,delimiter=' ')
l20_dt_flowDf=pd.read_csv(l20_dt_fct_file,delimiter=' ')
l20_cs_flowDf=pd.read_csv(l20_cs_fct_file,delimiter=' ')


l20_protean_shortFlows=l20_protean_flowDf[(l20_protean_flowDf["flowsize"]<bdp)|(l20_protean_flowDf["incast"]==1)]
l20_protean_shortFlows=list(l20_protean_shortFlows["slowdown"])
l20_protean_shortFlows.sort()
l20_protean_short_999fct = l20_protean_shortFlows[int(len(l20_protean_shortFlows)*0.999)-1]
l20_protean_short_99fct  = l20_protean_shortFlows[int(len(l20_protean_shortFlows)*0.99)-1]
l20_protean_short_95fct  = l20_protean_shortFlows[int(len(l20_protean_shortFlows)*0.95)-1]

l20_dt_shortFlows=l20_dt_flowDf[(l20_dt_flowDf["flowsize"]<bdp)|(l20_dt_flowDf["incast"]==1)]
l20_dt_shortFlows=list(l20_dt_shortFlows["slowdown"])
l20_dt_shortFlows.sort()
l20_dt_short_999fct = l20_dt_shortFlows[int(len(l20_dt_shortFlows)*0.999)-1]
l20_dt_short_99fct  = l20_dt_shortFlows[int(len(l20_dt_shortFlows)*0.99)-1]
l20_dt_short_95fct  = l20_dt_shortFlows[int(len(l20_dt_shortFlows)*0.95)-1]

l20_cs_shortFlows=l20_cs_flowDf[(l20_cs_flowDf["flowsize"]<bdp)|(l20_cs_flowDf["incast"]==1)]
l20_cs_shortFlows=list(l20_cs_shortFlows["slowdown"])
l20_cs_shortFlows.sort()
l20_cs_short_999fct = l20_cs_shortFlows[int(len(l20_cs_shortFlows)*0.999)-1]
l20_cs_short_99fct  = l20_cs_shortFlows[int(len(l20_cs_shortFlows)*0.99)-1]
l20_cs_short_95fct  = l20_cs_shortFlows[int(len(l20_cs_shortFlows)*0.95)-1]

l50_protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-114-0.5-0.75-1-0.25-0-0-1-2.25.fct"
l50_dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-101-0.5-0.75-1-0.25-0-0-1-2.25.fct"
l50_cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-103-0.5-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]

l50_protean_flowDf=pd.read_csv(l50_protean_fctfile,delimiter=' ')
l50_dt_flowDf=pd.read_csv(l50_dt_fct_file,delimiter=' ')
l50_cs_flowDf=pd.read_csv(l50_cs_fct_file,delimiter=' ')


l50_protean_shortFlows=l50_protean_flowDf[(l50_protean_flowDf["flowsize"]<bdp)|(l50_protean_flowDf["incast"]==1)]
l50_protean_shortFlows=list(l50_protean_shortFlows["slowdown"])
l50_protean_shortFlows.sort()
l50_protean_short_999fct = l50_protean_shortFlows[int(len(l50_protean_shortFlows)*0.999)-1]
l50_protean_short_99fct  = l50_protean_shortFlows[int(len(l50_protean_shortFlows)*0.99)-1]
l50_protean_short_95fct  = l50_protean_shortFlows[int(len(l50_protean_shortFlows)*0.95)-1]

l50_dt_shortFlows=l50_dt_flowDf[(l50_dt_flowDf["flowsize"]<bdp)|(l50_dt_flowDf["incast"]==1)]
l50_dt_shortFlows=list(l50_dt_shortFlows["slowdown"])
l50_dt_shortFlows.sort()
l50_dt_short_999fct = l50_dt_shortFlows[int(len(l50_dt_shortFlows)*0.999)-1]
l50_dt_short_99fct  = l50_dt_shortFlows[int(len(l50_dt_shortFlows)*0.99)-1]
l50_dt_short_95fct  = l50_dt_shortFlows[int(len(l50_dt_shortFlows)*0.95)-1]

l50_cs_shortFlows=l50_cs_flowDf[(l50_cs_flowDf["flowsize"]<bdp)|(l50_cs_flowDf["incast"]==1)]
l50_cs_shortFlows=list(l50_cs_shortFlows["slowdown"])
l50_cs_shortFlows.sort()
l50_cs_short_999fct = l50_cs_shortFlows[int(len(l50_cs_shortFlows)*0.999)-1]
l50_cs_short_99fct  = l50_cs_shortFlows[int(len(l50_cs_shortFlows)*0.99)-1]
l50_cs_short_95fct  = l50_cs_shortFlows[int(len(l50_cs_shortFlows)*0.95)-1]

l80_protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-114-0.8-0.75-1-0.25-0-0-1-2.25.fct"
l80_dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-101-0.8-0.75-1-0.25-0-0-1-2.25.fct"
l80_cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-1-103-0.8-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]
l80_protean_flowDf=pd.read_csv(l80_protean_fctfile,delimiter=' ')
l80_dt_flowDf=pd.read_csv(l80_dt_fct_file,delimiter=' ')
l80_cs_flowDf=pd.read_csv(l80_cs_fct_file,delimiter=' ')


l80_protean_shortFlows=l80_protean_flowDf[(l80_protean_flowDf["flowsize"]<bdp)|(l80_protean_flowDf["incast"]==1)]
l80_protean_shortFlows=list(l80_protean_shortFlows["slowdown"])
l80_protean_shortFlows.sort()
l80_protean_short_999fct = l80_protean_shortFlows[int(len(l80_protean_shortFlows)*0.999)-1]
l80_protean_short_99fct  = l80_protean_shortFlows[int(len(l80_protean_shortFlows)*0.99)-1]
l80_protean_short_95fct  = l80_protean_shortFlows[int(len(l80_protean_shortFlows)*0.95)-1]


l80_dt_shortFlows=l80_dt_flowDf[(l80_dt_flowDf["flowsize"]<bdp)|(l80_dt_flowDf["incast"]==1)]
l80_dt_shortFlows=list(l80_dt_shortFlows["slowdown"])
l80_dt_shortFlows.sort()
l80_dt_short_999fct = l80_dt_shortFlows[int(len(l80_dt_shortFlows)*0.999)-1]
l80_dt_short_99fct  = l80_dt_shortFlows[int(len(l80_dt_shortFlows)*0.99)-1]
l80_dt_short_95fct  = l80_dt_shortFlows[int(len(l80_dt_shortFlows)*0.95)-1]

l80_cs_shortFlows=l80_cs_flowDf[(l80_cs_flowDf["flowsize"]<bdp)|(l80_cs_flowDf["incast"]==1)]
l80_cs_shortFlows=list(l80_cs_shortFlows["slowdown"])
l80_cs_shortFlows.sort()
l80_cs_short_999fct = l80_cs_shortFlows[int(len(l80_cs_shortFlows)*0.999)-1]
l80_cs_short_99fct  = l80_cs_shortFlows[int(len(l80_cs_shortFlows)*0.99)-1]
l80_cs_short_95fct  = l80_cs_shortFlows[int(len(l80_cs_shortFlows)*0.95)-1]

# dt_data = [[20, l20_dt_short_999fct, l20_dt_short_99fct, l20_dt_short_95fct], [80, l80_dt_short_999fct, l80_dt_short_99fct, l80_dt_short_95fct]]
# protean_data = [[20, l20_protean_short_999fct, l20_protean_short_99fct, l20_protean_short_95fct], [80, l80_protean_short_999fct, l80_protean_short_99fct, l80_protean_short_95fct]]
data = [['DT', 20, l20_dt_short_999fct, l20_dt_short_99fct, l20_dt_short_95fct],
        ['DT', 50, l50_dt_short_999fct, l50_dt_short_99fct, l50_dt_short_95fct],
        ['DT', 80, l80_dt_short_999fct, l80_dt_short_99fct, l80_dt_short_95fct],
        ['PROTEAN', 20, l20_protean_short_999fct, l20_protean_short_99fct, l20_protean_short_95fct],
        ['PROTEAN', 50, l50_protean_short_999fct, l50_protean_short_99fct, l50_protean_short_95fct],
        ['PROTEAN', 80, l80_protean_short_999fct, l80_protean_short_99fct, l80_protean_short_95fct],
        ['CS', 20, l20_cs_short_999fct, l20_cs_short_99fct, l20_cs_short_95fct],
        ['CS', 50, l50_cs_short_999fct, l50_cs_short_99fct, l50_cs_short_95fct],
        ['CS', 80, l80_cs_short_999fct, l80_cs_short_99fct, l80_cs_short_95fct]]

df = pd.DataFrame(data, columns=['Method', 'Load(%)', '99.9th percentile', '99th percentile', '95th percentile'])

sns.set_theme(style="darkgrid")

sns.barplot(data = df ,x = 'Load(%)' ,y = '99.9th percentile', hue="Method")

# plt.title("Incast FCT")
plt.xlabel("Load(%)")
plt.ylabel("99.9th FCT Slowdown - Incast with TCP")


# plt.legend()
plt.savefig("/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fct-999-short-incast-mix-single-large-1-101-103-114-0.2-0.75-1-0.25-0-0-1-2.25.png")