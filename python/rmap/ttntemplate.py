import collections

ttntemplate=[]
ttntemplate.append(collections.OrderedDict())  # null template 0
ttntemplate.append(collections.OrderedDict())  # template 1: temperature and humidity
ttntemplate[1]["B12101"]={"nbit":16,"offset":22315,"scale":100,"timerange":"254,0,0","level":"103,2000,-,-"}
ttntemplate[1]["B13003"]={"nbit":7,"offset":0,"scale":1,"timerange":"254,0,0","level":"103,2000,-,-"}

ttntemplate.append(collections.OrderedDict())  # template 2: temperature and humidity
ttntemplate[2]["B12101"]={"nbit":16,"offset":22315,"scale":100,"timerange":"254,0,0","level":"103,2000,-,-"}
ttntemplate[2]["B13003"]={"nbit":7,"offset":0,"scale":1,"timerange":"254,0,0","level":"103,2000,-,-"}
ttntemplate[2]["B15198"]={"nbit":20,"offset":0,"scale":10000000000,"timerange":"254,0,0","level":"103,2000,-,-"}

