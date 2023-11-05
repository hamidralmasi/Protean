#from turtle import title
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

import sys
sns.set(style='ticks')
sns.set_context("talk")
sns.set_palette("colorblind")
# algfilename = "/home/evl/halmas3/data/buffer_sizing_v2/simulator/ns-3.35/examples/Protean/results/alg-single-1-114-0.6-0.3-1-0.25-0-0-1-2.25.stat"
algfilename_dt = sys.argv[1]
algfilename_protean = sys.argv[2]

algdata_dt = np.genfromtxt(algfilename_dt, delimiter=" ", skip_header=1, usemask = True)
algdata_protean = np.genfromtxt(algfilename_protean, delimiter=" ", skip_header=1, usemask = True)

slice_dt = algdata_dt[(algdata_dt[:,0]>=10002050000) & (algdata_dt[:,0]<=10003284000)]
slice_protean = algdata_protean[(algdata_protean[:,0]>=10002050000) & (algdata_protean[:,0]<=10003284000)]

#Slice
time_dt = slice_dt[:,0]
q1_len_dt = slice_dt[:,9] / 1e6 
q2_len_dt = slice_dt[:,16] / 1e6
q1_thresh_dt = slice_dt[:,10] / 1e6
q2_thresh_dt = slice_dt[:,17] / 1e6

time_protean = slice_protean[:,0]
q1_len_protean = slice_protean[:,9] / 1e6 
q2_len_protean = slice_protean[:,16] / 1e6 
q1_thresh_protean = slice_protean[:,10] / 1e6
q2_thresh_protean = slice_protean[:,17] / 1e6

#Plot for Q1
sns.lineplot(x = time_dt ,y = q1_len_dt, label="DT Q1 Length", estimator=None, ls= (0, (3, 5, 1, 5, 1, 5)))
sns.lineplot(x = time_dt ,y = q1_thresh_dt, label="DT Q1 Thresh.", estimator=None, ls=(0, (1, 1)))
sns.lineplot(x = time_dt ,y = q1_len_protean, label="Protean Q1 Length", estimator=None, ls= (0, (3, 10, 1, 10)))
sns.lineplot(x = time_dt ,y = q1_thresh_protean, label="Protean Q1 Thresh.", estimator=None, ls=':')

plt.xlabel("Time (\u03BCs)", fontsize=20)
plt.ylabel("Incast Queue (MB)", fontsize=20)
# plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=3)
plt.legend(loc="upper right", fontsize=14)
plt.tick_params(axis='both', which='major', labelsize=20)
plt.tight_layout()

plt.savefig(algfilename_dt+"-MotivIncastQlen"+".png")
plt.figure()


#Plot for Q2
sns.lineplot(x = time_dt ,y = q2_len_dt, label="DT Q2 Length", estimator=None)
sns.lineplot(x = time_dt ,y = q2_thresh_dt, label="DT Q2 Thresh.", estimator=None)
sns.lineplot(x = time_dt ,y = q2_len_protean, label="Protean Q2 Length", estimator=None)
sns.lineplot(x = time_dt ,y = q2_thresh_protean, label="Protean Q2 Thresh.", estimator=None)


plt.xlabel("Time (\u03BCs)", fontsize=20)
plt.ylabel("Non-Incast Queue (MB)", fontsize=20)
# plt.legend(bbox_to_anchor=(0,0.7,1,0.2), loc="lower left", ncol=3)
plt.legend(loc="upper right", fontsize=14)
plt.tick_params(axis='both', which='major', labelsize=20)
plt.tight_layout()

plt.savefig(algfilename_dt+"-MotivNonIncast"+".png")
plt.figure()
