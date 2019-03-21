#POSSIBLE CONFIGURATIONS
NOISESCENARIO = [0, 3000]
ALGO          = [0, 1, 2]
SCALEBYAREA   = [True, False]
SCALEBYDOSE   = [True, False]


#JOB PARAMS
NJOBS       = 20
UNITSPERJOB = 100
EOSLOCATION = '/store/group/dpg_hgcal/comm_hgcal/deguio'
FOLDERNAME  = 'gen_mu150'

from WMCore.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = False
config.General.workArea = FOLDERNAME
config.General.requestName = '' #see below

config.section_('JobType')
config.JobType.psetName = '../mugun_HEback.py'
config.JobType.pyCfgParams = [] #see below
config.JobType.pluginName = 'PrivateMC'
#config.JobType.outputFiles = [''] #use autocollection
#config.JobType.maxJobRuntimeMin = 2750 #45 h
#config.JobType.maxMemoryMB = 2500 #2.5 GB
config.JobType.allowUndistributedCMSSW = True

config.section_('Data')
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = UNITSPERJOB
config.Data.totalUnits = UNITSPERJOB * NJOBS

config.Data.publication = False
#config.Data.outputDatasetTag = config.General.requestName
config.Data.outLFNDirBase = '' #see below
config.Data.ignoreLocality = True

config.section_('User')

config.section_('Site')
config.Site.storageSite = 'T2_CH_CERN'
config.Site.whitelist = ['T2_CH_CERN']


if __name__ == '__main__':
    from CRABAPI.RawCommand import crabCommand
    from multiprocessing import Process

    def submit(config):
        res = crabCommand('submit', config = config)

    #########From now on that's what users should modify: this is the a-la-CRAB2 configuration part.

    requestNameList = []
    pyCfgParamsList = []
    outLFNDirBaseList = []
    for noise in NOISESCENARIO:
        for al in ALGO:
            for scA in SCALEBYAREA:
                for scD in SCALEBYDOSE:
                    requestName = FOLDERNAME+"_noiseScenario_"+str(noise)+"_algo_"+str(al)+"_scaleArea_"+str(scA)+"_scaleDose_"+str(scD)
                    requestNameList.append(requestName)
                    outLFNDirBaseList.append(EOSLOCATION+'/'+requestName)
                    cfgParams = ['noiseScenario='+str(noise), 'algo='+str(al), 'scaleByArea='+str(scA), 'scaleByDose='+str(scD)]
                    pyCfgParamsList.append(cfgParams)

                    #print requestName, cfgParams


    for req,pars,out in zip(requestNameList,pyCfgParamsList,outLFNDirBaseList):
        config.General.requestName = req
        config.JobType.pyCfgParams = pars
        config.Data.outLFNDirBase = out

        print 'REQUEST:', req
        print 'PARAMETERS:', pars
        print 'OUTPUT:', out
        p = Process(target=submit, args=(config,))
        p.start()
        p.join()
