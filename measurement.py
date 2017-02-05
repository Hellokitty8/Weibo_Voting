
# coding: utf-8

# In[12]:

import numpy
gs=numpy.loadtxt("gs.txt")
gt=numpy.loadtxt("gt.txt")
uut=numpy.loadtxt("uut.txt")
uus=numpy.loadtxt("uus.txt")
uvt=numpy.loadtxt("uvt.txt")
uvs=numpy.loadtxt("uvs.txt")
vvt=numpy.loadtxt("vvt.txt")
vvs=numpy.loadtxt("vvs.txt")


# In[16]:

import matplotlib.pyplot as plt

X=sorted(uvt)
Y=[]
l=len(X)
Y.append(float(1)/l)
for i in range(2,l+1):
    Y.append(float(1)/l+Y[i-2])
#----------------------------

X1=sorted(uvs)
Y1=[]
l=len(X1)
Y1.append(float(1)/l)
for i in range(2,l+1):
    Y1.append(float(1)/l+Y1[i-2])

#====================================

uvs_X=X
uvs_Y=Y
uvt_X=X1
uvt_Y=Y1


# In[18]:

import matplotlib.pyplot as plt


import plotly.plotly as py


fig = plt.figure()

ax1 = fig.add_subplot(221)

ax1.plot(gt_X,gt_Y,color='r',label='User_Group_T')
ax1.plot(gs_X,gs_Y,color='c',label='User_Group_S')

ax2 = fig.add_subplot(222)
ax2.plot(uus_X,uus_Y,color='r',label='User_User_T')
ax2.plot(uut_X,uut_Y,color='c',label='User_User_S')

ax3 = fig.add_subplot(223)
ax3.plot(vvs_X,vvs_Y,color='r',label='Vote_Vote_T')
ax3.plot(vvt_X,vvt_Y,color='c',label='Vote_Vote_S')

ax4 = fig.add_subplot(224)
ax4.plot(uvs_X,uvs_Y,color='r',label='User_Vote_T')
ax4.plot(uvt_X,uvt_Y,color='c',label='User_Vote_S')


plt.tight_layout()
fig = plt.gcf()

ax1.set_xlabel('Similarity Score')
ax1.set_ylabel('CDF')

ax2.set_xlabel('Similarity Score')
ax2.set_ylabel('CDF')

ax3.set_xlabel('Similarity Score')
ax3.set_ylabel('CDF')

ax4.set_xlabel('Similarity Score')
ax4.set_ylabel('CDF')

plt.show()


# In[ ]:




# In[ ]:



