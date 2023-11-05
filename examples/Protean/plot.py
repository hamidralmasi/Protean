import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys
 
filename = "/home/halmas3/ns3-datacenter/simulator/ns-3.35/examples/Protean/results/alg-single-2-114-0.002-0.3-1-0.2.stat"
filename = sys.argv[1]
data = np.genfromtxt(filename, delimiter=" ", skip_header=1, usemask = True)


# df= pd.read_csv(filename, header=[0], delimiter=' ')
# df = df.iloc[: , :-1]
# df = df[["time", "P1_instDqDt", "P1_ewmaDqDt"]]

# sns.lineplot(data=df)
# 10000120000 0 2 0.0028 99.9972 0 0 0 0 0 9.99246e+07 0 0 0 0 0 0 9.99246e+07 0 0
# 10000252000 0 2 52.2852 47.7148 0 0 1.21715 27.9231 962380 5.79878e+07 0 0 -1.23891 -0.22265 3.71672 83268 4.76422e+07 0 0
slice = data[(data[:,0]>=10000120000) & (data[:,0]<=10000252000)]

# print(data)
sns.set(style='ticks')
# sns.set_context("talk")
# sns.set_palette("colorblind")

palette = sns.color_palette("colorblind", 10)

# x = data[:,0]
# y_p1_inst = data[:,6]
# y_p1_ewma = data[:,7]

#Slice
x = slice[:,0]/1e3 - 1e7
y_p1_inst = slice[:,6] * 8
y_p1_ewma = slice[:,7] * 8

q1_len = slice[:,9] / (slice[:,2]*1000000) * 100
q2_len = slice[:,16] / (slice[:,2]*1000000) * 100

y_p2_inst = slice[:,13] * 8
y_p2_ewma = slice[:,14] * 8

p1_thresh = slice[:,10] / (slice[:,2]*1000000)
p2_thresh = slice[:,17] / (slice[:,2]*1000000)

#Whole
# x = data[:,0]
# y_p1_inst = data[:,6]
# y_p1_ewma = data[:,7]

# q1_len = data[:,9] / (data[:,2]*1000000) * 100
# q2_len = data[:,16] / (data[:,2]*1000000) * 100

# y_p2_inst = data[:,13]
# y_p2_ewma = data[:,14]

# p1_thresh = data[:,10] / (data[:,2]*1000000) * 100
# p2_thresh = data[:,17] / (data[:,2]*1000000) * 100

# print(y_p1_ewma)
# print(len(x))
# print(len(y_p1_inst))
# print(len(y_p1_ewma))
# print(max(y_p1_inst))
# print(max(y_p1_ewma))

# bigdqdt = data[data[:,6] > 10]
# print(len(bigdqdt))
# print(bigdqdt)

sns.lineplot(x, y_p1_ewma , ci= None, label="P1 EWMA(\u0394q/\u0394t)", estimator=None, color=palette[3])
sns.lineplot(x, y_p1_inst, ci= None, label="P1 Inst. \u0394q/\u0394t", estimator=None, ls=(0, (3, 1, 1, 1)), color=palette[3])
# sns.lineplot(x , q1_len, ci= None, label="Protean-Q1_len", estimator=None)
sns.lineplot(x, y_p2_ewma , ci= None, label="P2 EWMA(\u0394q/\u0394t)", estimator=None, color=palette[0])
sns.lineplot(x, y_p2_inst, ci= None, label="P2 Inst. \u0394q/\u0394t", estimator=None, ls=(0, (3, 1, 1, 1)), color=palette[0])
# sns.lineplot(x , q2_len, ci= None, label="Protean-Q2_len", estimator=None)

# sns.lineplot(x, p1_thresh, ci= None, label="Protean-P1_thresh", estimator=None)
# sns.lineplot(x, p2_thresh, ci= None, label="Protean-P2_thresh", estimator=None)

# plt.title("Short FCT", fontsize=20)
plt.xlabel("Time (\u03BCs)", fontsize=20)
plt.ylabel("Gradient (Gbps)", fontsize=20)
# plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=3)
plt.legend(loc="upper right", fontsize=14, facecolor='white', edgecolor='white', framealpha=1)
plt.tick_params(axis='both', which='major', labelsize=20)
plt.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))
plt.rc('axes', axisbelow=True)
plt.tight_layout()

plt.savefig(filename+"-smoothness"+".pdf")