#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Sep  9 15:50:18 2023

@author: vamsi
"""

import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, precision_score, recall_score, f1_score, ConfusionMatrixDisplay
from sklearn.model_selection import RandomizedSearchCV, train_test_split
from scipy.stats import randint
from sklearn.tree import export_graphviz
from IPython.display import Image
# import graphviz
# from sklearn.externals import joblib
import joblib
import sys
#%%
csvfile=str(sys.argv[1])
# csvfile="/home/vamsi/lakewood/src/phd/codebase/ns3-datacenter-Old/simulator/ns-3.35/examples/Credence/lqd-logs/lqdtrace-2-0.8-0.75-2-WS-0.csv"
dumpfile=str(sys.argv[2])
# dumpfile="/home/vamsi/lakewood/src/phd/codebase/ns3-datacenter-Old/simulator/ns-3.35/examples/Credence/rf_models/model-2-0.8-0.75-2-WS-0.joblib"
maxDepth=int(sys.argv[3])
switchId=str(sys.argv[4])
# maxDepth=4
# switchId=0
numTrees=int(sys.argv[4])
# numTrees=1
#%%
data = pd.read_csv(csvfile,delimiter=' ',usecols=[0,1,2,3,4])
#%%
X = data.drop('drop',axis=1).values
y = data['drop'].values

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.6)
#%%
trees=[1,2,4,8,16,32,64,128]
for numTrees in trees:
    rf = RandomForestClassifier(max_depth=maxDepth, n_jobs=-1,n_estimators=numTrees)
    rf.fit(X_train, y_train)

    y_pred = rf.predict(X_test)
    
    cm = confusion_matrix(y_test, y_pred)

    try:
        accuracy = accuracy_score(y_test, y_pred)
    except:
        accuracy = 1
    if (accuracy==0):
        accuracy = 1
    try:
        precision = precision_score(y_test, y_pred)
    except:
        precision = 1
    if (precision==0):
        precision = 1
    try:
        recall = recall_score(y_test, y_pred)
    except:
        recall = 1
    if (recall==0):
        recall = 1
    try:
        f1score = f1_score(y_test,y_pred)
    except:
        f1score = 1
    if (f1score==0):
        f1score = 1
        
    try:
        tn = cm[0][0]
        fp = cm[0][1]
        fn = cm[1][0]
        tp = cm[1][1]
        myScore = 1/((tn+fp)/(tn - min([(19)*fn,tn])))
    except:
        myScore=1
    if myScore==0:
        myScore=1
    print(accuracy,precision,recall,f1score,numTrees,maxDepth,myScore)

    # joblib.dump(rf,dumpfile+'-'+str(numTrees)+'-'+switchId+'.joblib')



#%%
# feature_importances = pd.Series(rf.feature_importances_, index=["queueLength","sharedOccupancy", "averageQueueLength","averageOccupancy"]).sort_values(ascending=False)
# feature_importances.plot.bar();
#%%
# for i in range(1):
#     tree = rf.estimators_[i]
#     dot_data = export_graphviz(tree,
#                                 feature_names=["queueLength","sharedOccupancy", "averageQueueLength","averageOccupancy"],  
#                                 filled=True,  
#                                 max_depth=4, 
#                                 impurity=False, 
#                                 proportion=True)
#     graph = graphviz.Source(dot_data)
#     display(graph)

# cm = confusion_matrix(y_test, y_pred)
# ConfusionMatrixDisplay(confusion_matrix=cm).plot();
#%%


#%%
# dumpfile="/home/vamsi/lakewood/src/phd/codebase/ns3-datacenter-Old/simulator/ns-3.35/examples/Credence/rf_models/model-2-0.8-0.875-2-WS-0.joblib"
# rf = joblib.load(dumpfile)

#%%

# df=pd.DataFrame()
# df={"queueLength":[1],"sharedOccupancy":[2],"averageQueueLength":[3],"averageOccupancy":[3]}
# rf.feature_names_in_
# i=0
# while i<1000:
#     i=i+1
#     df=[[1,2,3,4]]
#     a = rf.predict(df)[0]
#     print(a)

# rf.predict([[1046792,1046792,617276,1040028]])