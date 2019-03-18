
#JOB PARAMS
NJOBS = 200 
UNITSPERJOB = 100
EOSLOCATION = '/store/group/dpg_hgcal/comm_hgcal/deguio/'
FOLDERNAME = 'gen_mu150'
TAG = 'newDigi_v2'
SoN = '5'


from WMCore.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = False
config.General.workArea = FOLDERNAME
config.General.requestName = FOLDERNAME+'_SoN_'+SoN+'_'+TAG

config.section_('JobType')
config.JobType.psetName = '../mugun_HEback.py'
config.JobType.pyCfgParams = ['SoN='+SoN]
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
config.Data.outLFNDirBase = EOSLOCATION+'/'+config.General.requestName
config.Data.ignoreLocality = True

config.section_('User')

config.section_('Site')
config.Site.storageSite = 'T2_CH_CERN'
config.Site.whitelist = ['T2_CH_CERN']
