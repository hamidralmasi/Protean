import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys
linerate=10
rtt=8 * 10
bdp=linerate*1e9*rtt*1e-6/8

l20_protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-114-0.2-0.75-1-0.25-0-0-1-2.25.fct"
l20_dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-101-0.2-0.75-1-0.25-0-0-1-2.25.fct"
l20_cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-103-0.2-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]

l20_protean_flowDf=pd.read_csv(l20_protean_fctfile,delimiter=' ')
l20_dt_flowDf=pd.read_csv(l20_dt_fct_file,delimiter=' ')
l20_cs_flowDf=pd.read_csv(l20_cs_fct_file,delimiter=' ')


l20_protean_longFlows=l20_protean_flowDf[(l20_protean_flowDf["flowsize"]>=2000000)]
l20_protean_longBytes=list(l20_protean_longFlows["flowsize"])
l20_protean_longFCTs=list(l20_protean_longFlows["fct"])
l20_protean_longGoodput=np.sum(l20_protean_longBytes)*8/1000000000/2

l20_dt_longFlows=l20_dt_flowDf[(l20_dt_flowDf["flowsize"]>=2000000)]
l20_dt_longBytes=list(l20_dt_longFlows["flowsize"])
l20_dt_longFCTs=list(l20_dt_longFlows["fct"])
l20_dt_longGoodput=np.sum(l20_dt_longBytes)*8/1000000000/2

l20_cs_longFlows=l20_cs_flowDf[(l20_cs_flowDf["flowsize"]>=2000000)]
l20_cs_longBytes=list(l20_cs_longFlows["flowsize"])
l20_cs_longFCTs=list(l20_cs_longFlows["fct"])
l20_cs_longGoodput=np.sum(l20_cs_longBytes)*8/1000000000/2

l50_protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-114-0.5-0.75-1-0.25-0-0-1-2.25.fct"
l50_dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-101-0.5-0.75-1-0.25-0-0-1-2.25.fct"
l50_cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-103-0.5-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]

l50_protean_flowDf=pd.read_csv(l50_protean_fctfile,delimiter=' ')
l50_dt_flowDf=pd.read_csv(l50_dt_fct_file,delimiter=' ')
l50_cs_flowDf=pd.read_csv(l50_cs_fct_file,delimiter=' ')


l50_protean_longFlows=l50_protean_flowDf[(l50_protean_flowDf["flowsize"]>=2000000)]
l50_protean_longBytes=list(l50_protean_longFlows["flowsize"])
l50_protean_longFCTs=list(l50_protean_longFlows["fct"])
l50_protean_longGoodput=np.sum(l50_protean_longBytes)*8/1000000000/2

l50_dt_longFlows=l50_dt_flowDf[(l50_dt_flowDf["flowsize"]>=2000000)]
l50_dt_longBytes=list(l50_dt_longFlows["flowsize"])
l50_dt_longFCTs=list(l50_dt_longFlows["fct"])
l50_dt_longGoodput=np.sum(l50_dt_longBytes)*8/1000000000/2

l50_cs_longFlows=l50_cs_flowDf[(l50_cs_flowDf["flowsize"]>=2000000)]
l50_cs_longBytes=list(l50_cs_longFlows["flowsize"])
l50_cs_longFCTs=list(l50_cs_longFlows["fct"])
l50_cs_longGoodput=np.sum(l50_cs_longBytes)*8/1000000000/2

l80_protean_fctfile = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-114-0.8-0.75-1-0.25-0-0-1-2.25.fct"
l80_dt_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-101-0.8-0.75-1-0.25-0-0-1-2.25.fct"
l80_cs_fct_file = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/fcts-single-large-2-103-0.8-0.75-1-0.25-0-0-1-2.25.fct"

#protean_fctfile = sys.argv[1]
#dt_fct_file = sys.argv[2]
l80_protean_flowDf=pd.read_csv(l80_protean_fctfile,delimiter=' ')
l80_dt_flowDf=pd.read_csv(l80_dt_fct_file,delimiter=' ')
l80_cs_flowDf=pd.read_csv(l80_cs_fct_file,delimiter=' ')


l80_protean_longFlows=l80_protean_flowDf[(l80_protean_flowDf["flowsize"]>=2000000)]
l80_protean_longBytes=list(l80_protean_longFlows["flowsize"])
l80_protean_longFCTs=list(l80_protean_longFlows["fct"])
l80_protean_longGoodput=np.sum(l80_protean_longBytes)*8/1000000000/2

l80_dt_longFlows=l80_dt_flowDf[(l80_dt_flowDf["flowsize"]>=2000000)]
l80_dt_longBytes=list(l80_dt_longFlows["flowsize"])
l80_dt_longFCTs=list(l80_dt_longFlows["fct"])
l80_dt_longGoodput=np.sum(l80_dt_longBytes)*8/1000000000/2

l80_cs_longFlows=l80_cs_flowDf[(l80_cs_flowDf["flowsize"]>=2000000)]
l80_cs_longBytes=list(l80_cs_longFlows["flowsize"])
l80_cs_longFCTs=list(l80_cs_longFlows["fct"])
l80_cs_longGoodput=np.sum(l80_cs_longBytes)*8/1000000000/2

# dt_data = [[20, l20_dt_long_999fct, l20_dt_long_99fct, l20_dt_long_95fct], [80, l80_dt_long_999fct, l80_dt_long_99fct, l80_dt_long_95fct]]
# protean_data = [[20, l20_protean_long_999fct, l20_protean_long_99fct, l20_protean_long_95fct], [80, l80_protean_long_999fct, l80_protean_long_99fct, l80_protean_long_95fct]]
data = [['DT', 20, l20_dt_longGoodput],
        ['DT', 50, l50_dt_longGoodput],
        ['DT', 80, l80_dt_longGoodput],
        ['PROTEAN', 20, l20_protean_longGoodput],
        ['PROTEAN', 50, l50_protean_longGoodput],
        ['PROTEAN', 80, l80_protean_longGoodput],
        ['CS', 20, l20_cs_longGoodput],
        ['CS', 50, l50_cs_longGoodput],
        ['CS', 80, l80_cs_longGoodput]]

df = pd.DataFrame(data, columns=['Method', 'Load(%)', 'Goodput(%)'])

sns.set_theme(style="darkgrid")

sns.barplot(data = df ,x = 'Load(%)' ,y = 'Goodput(%)', hue="Method")

# plt.title("Incast FCT")
plt.xlabel("Load(%)")
plt.ylabel("Goodput(Gbps)")


# plt.legend()
plt.savefig("/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/ALL-Results-CUDC-L20to90-BF123-plus_motivation/results/goodput-take2-long-single-large-2-101-103-114-0.2-0.75-1-0.25-0-0-1-2.25.png")