def getPrediction(rf, a,b,c,d):
	arr = rf.predict([[a,b,c,d]])
	return arr[0]