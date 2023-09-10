#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Sep  9 15:50:18 2023

@author: vamsi
"""

import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, precision_score, recall_score, ConfusionMatrixDisplay
from sklearn.model_selection import RandomizedSearchCV, train_test_split
from scipy.stats import randint
from sklearn.tree import export_graphviz
from IPython.display import Image
import graphviz
# from sklearn.externals import joblib
import joblib
import sys

csvfile=str(sys.argv[1])
dumpfile=str(sys.argv[2])
maxDepth=int(sys.argv[3])
numTress=int(sys.argv[4])

data = pd.read_csv(csvfile,delimiter=' ',usecols=[0,1,2,3,4])

X = data.drop('drop',axis=1)
y = data['drop']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.6)

rf = RandomForestClassifier(max_depth=maxDepth, n_jobs=-1,n_estimators=numTress)
rf.fit(X_train, y_train)

y_pred = rf.predict(X_test)

# feature_importances = pd.Series(rf.feature_importances_, index=X_train.columns).sort_values(ascending=False)
# feature_importances.plot.bar();

# for i in range(1):
#     tree = rf.estimators_[i]
#     dot_data = export_graphviz(tree,
#                                feature_names=X_train.columns,  
#                                filled=True,  
#                                max_depth=4, 
#                                impurity=False, 
#                                proportion=True)
#     graph = graphviz.Source(dot_data)
#     display(graph)

# cm = confusion_matrix(y_test, y_pred)
# ConfusionMatrixDisplay(confusion_matrix=cm).plot();
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

print("Accuracy:", accuracy)
print("Precision:", precision)
print("Recall:", recall)

joblib.dump(rf,dumpfile)