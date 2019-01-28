#------------------------------------------------------------------------------------
# Imports
#------------------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
import FWCore.ParameterSet.VarParsing as VarParsing

#------------------------------------------------------------------------------------
# Declare the process and input variables
#------------------------------------------------------------------------------------
#process = cms.Process('NOISE',eras.Run2_50ns)#for 50ns 13 TeV data
#process = cms.Process('NOISE',eras.Run2_25ns)#for 25ns 13 TeV data
options = VarParsing.VarParsing ('analysis')
process = cms.Process("Trees",eras.Phase2) 



##
## Setup command line options
##
options.register ('skipEvents', 0, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.int, "no of skipped events")
options.register ('isMINIAOD', False, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.bool, "MINIAODSIM input file(s)?")

##
## Default
##
options.maxEvents = -1 # -1 means all events
#options.skipEvents = 0 # default is 0.

##
## get and parse the command line arguments
##
options.parseArguments()
print("isMINIAOD: ", options.isMINIAOD)
print("maxEvents: ", options.maxEvents)

#
# Dataset e.g.
# dasgoclient --query 'dataset dataset=/RelValTTbar_13/CMSSW_10_2_0_pre3-*realistic*/GEN-SIM-RECO'                 
# dasgoclient --query 'file dataset=/RelValTTbar_13/CMSSW_10_2_0_pre3-101X_upgrade2018_realistic_v7-v1/GEN-SIM-RECO'
#
# TTbar sample
#
# MINIAODSIM
if options.isMINIAOD: 
    #options.inputFiles = '/store/relval/CMSSW_10_3_0_pre4/RelValTTbar_13/MINIAODSIM/PUpmx25ns_103X_upgrade2018_realistic_v4-v1/20000/17E57223-3406-7340-B867-2DDC36E7C371.root'
    #options.outputFile = 'relval_ttbar_2018_pmx25ns_miniaodsim.root'
    #options.inputFiles = '/store/user/bcaraway/crab_outputs/TTbar_14TeV/CMSSW_10_4_0_pre2_D30_Step3_v3/181207_200604/0000/step3_inMINIAODSIM_9.root'
    #options.outputFile = 'relval_ttbar_2023_D30_miniaodsim.root'
    options.inputFiles = '/store/relval/CMSSW_10_4_0_mtd4/RelValNuGun/MINIAODSIM/PU25ns_103X_upgrade2023_realistic_v2_2023D35PU200-v1/20000/892E4505-33D5-CB42-9409-2410B4C3FF3B.root'
    options.outputFile = 'relval_nugun_pu25ns_2023_D35_miniaodsim.root'
# GEN-SIM-RECO
else:
    options.inputFiles = 'file:/home/bcaraway/HGcal/CMSSW_10_4_X_2018-10-26-2300/src/HGCalAnalysis/HGCalTreeMaker/test/matrixTestSubmit/step3.root'
    options.outputFile = 'ttbar_10_4_D30_pt25.root'
#
#
#
print("maxEvents: ", options.maxEvents)
print("inputFiles: ", options.inputFiles)
print("outputFile: ", options.outputFile)

#------------------------------------------------------------------------------------
# Get and parse the command line arguments
#------------------------------------------------------------------------------------
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
    secondaryFileNames = cms.untracked.vstring(options.secondaryInputFiles),
    skipEvents = cms.untracked.uint32(options.skipEvents) # default is 0.
)

process.TFileService = cms.Service("TFileService", 
                                   fileName = cms.string(options.outputFile)
)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
    Rethrow = cms.untracked.vstring("ProductNotFound"), # make this exception fatal
    fileMode  =  cms.untracked.string('NOMERGE') # no ordering needed, but calls endRun/beginRun etc. at file boundaries
)

#------------------------------------------------------------------------------------
# import of standard configurations
#------------------------------------------------------------------------------------
# import of standard configurations
#process.load('Configuration.StandardSequences.Services_cff')
#process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
#process.load('Configuration.Geometry.GeometryExtended2023D17Reco_cff')  # <=== to be checked
#process.load('Configuration.Geometry.GeometryExtended2023D28Reco_cff')
#process.load('Configuration.Geometry.GeometryExtended2023D30Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2023D35Reco_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.L1Reco_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
#process.load('PhysicsTools.PatAlgos.slimming.metFilterPaths_cff')
#process.load('Configuration.StandardSequences.PATMC_cff')
process.load('Configuration.StandardSequences.Validation_cff')
process.load('DQMOffline.Configuration.DQMOfflineMC_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#KH
process.MessageLogger.cerr.FwkReport.reportEvery = 100

#------------------------------------------------------------------------------------
# Set up our analyzer
#------------------------------------------------------------------------------------

process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_Tree_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_Event_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_GenParticles_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HBHERecHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCRecHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCSimHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_SimTracks_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_RecoTracks_cfi")

process.load("Validation.HGCalValidation.hgcalHitValidation_cfi")

process.load("HGCalAnalysis.HGCalTreeMaker.TupleMaker_PFCandidates_cfi")

#------------------------------------------------------------------------------------
# Specify Global Tag
#------------------------------------------------------------------------------------
from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')

#------------------------------------------------------------------------------------
# HGCalTupleMaker sequence definition
#------------------------------------------------------------------------------------
process.tuple_step = cms.Sequence(
    # Make HCAL tuples: Event, run, ls number
    process.hgcalTupleEvent*
    # Make HCAL tuples: digi info
    #
    process.tuplePFCandidates*
    #
    process.hgcalTupleHBHERecHits*
    process.hgcalTupleHGCRecHits*
    process.hgcalTupleGenParticles*
    process.hgcalTupleHGCSimHits*
    process.hgcalTupleSimTracks*
    process.hgcalTupleGeneralTracks*
    process.hgcalTupleTree


)

#
# in case we are using MINIAOD files
#
if options.isMINIAOD: 
    process.tuple_step = cms.Sequence(
        # Make HCAL tuples: Event, run, ls number
        process.hgcalTupleEvent*
        # Make HCAL tuples: gen info
        #process.hcalTupleGenParticles*
        #
        process.tuplePackedPFCandidates*
        #
        process.hgcalTupleTree
    )

#-----------------------------------------------------------------------------------
# Path and EndPath definitions
#-----------------------------------------------------------------------------------
process.preparation = cms.Path(
    process.tuple_step
)
