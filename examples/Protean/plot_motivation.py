#from turtle import title
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys

def calculate_ticks(ax, ticks, round_to=0.1, center=False):
    upperbound = np.ceil(ax.get_ybound()[1]/round_to)
    lowerbound = np.floor(ax.get_ybound()[0]/round_to)
    dy = upperbound - lowerbound
    fit = np.floor(dy/(ticks - 1)) + 1
    dy_new = (ticks - 1)*fit
    if center:
        offset = np.floor((dy_new - dy)/2)
        lowerbound = lowerbound - offset
    values = np.linspace(lowerbound, lowerbound + dy_new, ticks)
    return values*round_to

sns.set(style='ticks')
sns.set_context("talk")
palette = sns.color_palette("colorblind", 10)

# algfilename = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/results/alg-single-1-114-0.6-0.3-1-0.25-0-0-1-2.25.stat"
algfilename = sys.argv[1]
alg_name = sys.argv[2]
algdata = np.genfromtxt(algfilename, delimiter=" ", skip_header=1, usemask = True)

# df= pd.read_csv(filename, header=[0], delimiter=' ')
# df = df.iloc[: , :-1]
# df = df[["time", "P1_instDqDt", "P1_ewmaDqDt"]]

# sns.lineplot(data=df)
# 10000120000 0 2 0.0028 99.9972 0 0 0 0 0 9.99246e+07 0 0 0 0 0 0 9.99246e+07 0 0
# 10000252000 0 2 52.2852 47.7148 0 0 1.21715 27.9231 962380 5.79878e+07 0 0 -1.23891 -0.22265 3.71672 83268 4.76422e+07 0 0

#Original: 10000706000-10006655000
# 10002140000
# 10002870000

slice = algdata[(algdata[:,0]>=10001950000) & (algdata[:,0]<=10003284000)]

# print(data)
# sns.set_theme(style="darkgrid")
# x = data[:,0]
# y_p1_inst = data[:,6]
# y_p1_ewma = data[:,7]

#Slice
x = (slice[:,0] - 1e10)/1e3
#y_p1_inst = slice[:,6]
y_p1_ewma = slice[:,7] * 8

q1_len = slice[:,9] #/ (slice[:,2]*1000000) * 100
q2_len = slice[:,16] #/ (slice[:,2]*1000000) * 100

q1_thresh = slice[:,10] / (slice[:,2]*1000000) * 100
q2_thresh = slice[:,17] / (slice[:,2]*1000000) * 100

q1_len_perc = q1_len / (q1_len+q2_len) * 100
q2_len_perc = q2_len / (q1_len+q2_len) * 100

q1_thresh_perc = q1_thresh / (q1_thresh+q2_thresh) * 100
q2_thresh_perc = q2_thresh / (q1_thresh+q2_thresh) * 100


# y_p2_inst = slice[:,13]
y_p2_ewma = slice[:,14] * 8

# p1_thresh = slice[:,10] / (slice[:,2]*1000000)
# p2_thresh = slice[:,17] / (slice[:,2]*1000000)

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


# sns.lineplot(x, y_p1_ewma , ci= None, label="Protean-DqDt-EWMA_P1", estimator=None)
# # sns.lineplot(x, y_p1_inst, ci= None, label="Protean-DqDt-Inst_P1", estimator=None)
# sns.lineplot(x , q1_len, ci= None, label="Protean-Q1_len", estimator=None)
# sns.lineplot(x, y_p2_ewma , ci= None, label="Protean-DqDt-EWMA_P2", estimator=None)
# # sns.lineplot(x, y_p2_inst, ci= None, label="Protean-DqDt-Inst_P2", estimator=None)
# sns.lineplot(x , q2_len, ci= None, label="Protean-Q2_len", estimator=None)

# sns.lineplot(x, p1_thresh, ci= None, label="Protean-P1_thresh", estimator=None)
# sns.lineplot(x, p2_thresh, ci= None, label="Protean-P2_thresh", estimator=None)
# plt.savefig(filename+"-slice-with-Qlen"+".png")


fig, ax1 = plt.subplots()

ax2 = ax1.twinx()

sns.lineplot(x, y_p1_ewma, ax = ax1, label="\u0394"+r'$Q_{1}$'+"/\u0394t", estimator=None, color=palette[3])
sns.lineplot(x, y_p2_ewma, ax = ax1, label="\u0394"+r'$Q_{2}$'+"/\u0394t", estimator=None, color=palette[0])

sns.lineplot(x, q1_len_perc, ax= ax2, label=r'$Q_{1}$'+" Len.", estimator=None, color=palette[3], linestyle="--")
sns.lineplot(x, q2_len_perc, ax= ax2, label=r'$Q_{2}$'+" Len.", estimator=None, color=palette[0], linestyle="--")

# sns.lineplot(x, q1_thresh_perc, ax= ax2, label="Incast Threshold", estimator=None, color="yellow")
# sns.lineplot(x, q2_thresh_perc, ax= ax2, label="Long Flow Threshold", estimator=None, color="black")


# if (alg_name == "101"):
#     plt.title("DT")

# else:
#     plt.title("PROTEAN")

plt.grid()


if (alg_name=="101"):
    ax1.legend(bbox_to_anchor=(0.12,0.10),loc='lower left', fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
    ax2.legend(bbox_to_anchor=(0.45,0.42),loc='lower left', fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
else:
    ax1.legend(bbox_to_anchor=(0.3,0.08),loc='lower left', fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
    ax2.legend(bbox_to_anchor=(0.3,0.75),loc='lower left', fontsize=16, facecolor='white', edgecolor='white', framealpha=1)

# ax1.legend(bbox_to_anchor=(0.11,0.1),loc='lower left', fontsize=10, ncol=2, frameon=False)
# ax2.legend(bbox_to_anchor=(-0.08,1.02),loc='lower left', fontsize = 14, ncol=2, frameon=False)

#bbox_to_anchor=(-0.05,-0.5), 
#bbox_to_anchor=(-0.15,1.02), 
# ax1.set_xlabel('Time (\u03BCs)')
ax1.set_xlabel('Time(s)')
ax1.set_ylabel('Gradient (Gbps)')
ax2.set_ylabel('% Occupancy')

ax1.set_axisbelow(True)
ax1.yaxis.grid(color='gray', linestyle=(0, (3, 5, 1, 5)))

ax1.set_yticks(np.linspace(0, 150, 6))
ax2.set_yticks(np.linspace(0, 100, 5))
ax2.grid(None)


ax1.get_legend().remove()
ax2.get_legend().remove()
h1, l1 = ax1.get_legend_handles_labels()
h2, l2 = ax2.get_legend_handles_labels()
if (alg_name=="101"):
    ax1.legend(h1+h2, l1+l2, loc='center right',fontsize=16, facecolor='white', edgecolor='white', framealpha=1)
else:
    ax1.legend(h1+h2, l1+l2, loc='lower left',fontsize=16, facecolor='white', edgecolor='white', framealpha=1, bbox_to_anchor=(0.34,0.56))


# x = np.linspace(0, 100, 100)
# y = 0.95 - ((50 - x) / 200) ** 2
# err = (1 - y) / 2
# y += np.random.normal(0, err / 10, y.size)

# upper = gaussian_filter1d(y + err, sigma=3)
# lower = gaussian_filter1d(y - err, sigma=3)

# fig, ax = plt.subplots(ncols=2)

# ax[0].errorbar(x, y, err, color='dodgerblue')

# ax[1].plot(x, y, color='dodgerblue')
# ax[1].fill_between(x, upper, lower, color='crimson', alpha=0.2)
plt.tick_params(axis='both', which='major', labelsize=20)
if (alg_name=="101"):
    plt.xticks([1950, 2050, 2150, 2290, 2980], [r'$t_{0}$', r'$t_{1}$',r'$t_{2}$', r'$t_{3}$', r'$t_{4}$'])
else:
     plt.xticks([1950, 2050, 2175, 2368, 3280], [r'$t_{0}$', r'$t_{1}$',r'$t_{2}$', r'$t_{3}$', r'$t_{4}$'])
# plt.xticks(rotation=30)
# plt.yticks([4, 8, 12], ['Low', 'Medium', 'High'])
plt.tight_layout()
plt.savefig(algfilename+"-burst-absorbtion"+".pdf")