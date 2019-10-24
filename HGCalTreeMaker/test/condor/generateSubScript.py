#! /usr/bin/env python

import os
import sys
import optparse
import datetime
import time

#POSSIBLE CONFIGURATIONS
NOISESCENARIO = [0]
PILEUP        = [0]
ALGO          = [2]
SCALEBYAREA   = [True]
MOMENTUM      = [0.12, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18, 0.19, 0.2, 0.23, 0.26, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
#1.2, 1.4, 1.6, 1.8, 2.0, 4.0, 6.0, 8.0, 10.0, 20.0, 40.0, 60.0, 80.0, 100.0, 150.0, 200.0]

#JOB PARAMS
NJOBS       = 100  #number of jobs per configuration
UNITSPERJOB = 100
EXEC        = 'runProduction.sh'
FOLDERNAME  = 'gen_mu_pscan'


pwd = os.environ['PWD']
EXEC = pwd+"/"+EXEC
current_time = datetime.datetime.now()
timeMarker = "submit_%04d%02d%02d_%02d%02d%02d" % (current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute,current_time.second)
workingDir = pwd+"/"+timeMarker
os.system("mkdir -p "+workingDir)

requestNameList = []
pyCfgParamsList = []
for noise in NOISESCENARIO:
    for pu in PILEUP:
        for al in ALGO:
            for scA in SCALEBYAREA:
                for pp in MOMENTUM:
                    requestName = FOLDERNAME+"_$(ProcId)_noiseScenario_"+str(noise)+"_pileup_"+str(pu)+"_algo_"+str(al)+"_scaleArea_"+str(scA)+"_p_"+str(pp)
                    requestNameList.append(requestName)
                    cfgParams = '$(ProcId) noiseScenario='+str(noise)+' algo='+str(al)+' pileup='+str(pu)+' scaleByArea='+str(scA)+' maxEvents='+str(UNITSPERJOB)+' momentum='+str(pp)
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
