#! /usr/bin/env python

import os
import sys
import optparse
import datetime
import time

#POSSIBLE CONFIGURATIONS
NOISESCENARIO = [0, 3000]
ALGO          = [2]
SCALEBYAREA   = [True]

#JOB PARAMS
NJOBS       = 10  #number of jobs per configuration
UNITSPERJOB = 50
EXEC        = '/afs/cern.ch/work/d/deguio/HGCAL/DigiStudies/CMSSW_10_6_X_2019-04-07-0000_preparePR/src/HGCalAnalysis/HGCalTreeMaker/test/condor/runProduction.sh'
FOLDERNAME  = 'gen_mu150'






pwd = os.environ['PWD']
current_time = datetime.datetime.now()
timeMarker = "submit_%04d%02d%02d_%02d%02d%02d" % (current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute,current_time.second)
workingDir = pwd+"/"+timeMarker
os.system("mkdir -p "+workingDir)

requestNameList = []
pyCfgParamsList = []
for noise in NOISESCENARIO:
    for al in ALGO:
        for scA in SCALEBYAREA:
            requestName = FOLDERNAME+"_$(ProcId)_noiseScenario_"+str(noise)+"_algo_"+str(al)+"_scaleArea_"+str(scA)
            requestNameList.append(requestName)
            cfgParams = '$(ProcId) noiseScenario='+str(noise)+' algo='+str(al)+' scaleByArea='+str(scA)+' maxEvents='+str(UNITSPERJOB)
            pyCfgParamsList.append(cfgParams)


#prepare condor sub fileName
with open(workingDir+"/condor.sub", "w") as fo:
    fo.write("+JobFlavour = \"tomorrow\"\n\n")
    fo.write("transfer_output_files = \"\"\n")
    fo.write("executable = "+EXEC+"\n")
    fo.write("log        = "+workingDir+"/output.log\n")
    fo.write("requirements = (OpSysAndVer =?= \"CentOS7\")\n")
    fo.write("max_retries = 3\n")
    fo.write("\n")

    for req,pars in zip(requestNameList,pyCfgParamsList):
        fo.write("output = "+workingDir+"/"+req+".out\n")
        fo.write("error = "+workingDir+"/"+req+".err\n")
        fo.write("arguments = "+pars+"\n")
        fo.write("queue "+str(NJOBS)+"\n")
        fo.write("\n")
