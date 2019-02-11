#------------------------------------------------------------------------------------
# Imports
#------------------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
import FWCore.ParameterSet.VarParsing as VarParsing

#------------------------------------------------------------------------------------
# Declare the process and input variables
#------------------------------------------------------------------------------------
options = VarParsing.VarParsing ('analysis')
process = cms.Process("Trees",eras.Phase2)

options.register ('skipEvents', 0, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.int, "no of skipped events")

options.inputFiles = 'file:/afs/cern.ch/work/d/deguio/HGCAL/DigiStudies/CMSSW_10_5_0_pre1_firstLook/src/24088.0_SinglePiPt25Eta1p7_2p7+SinglePiPt25Eta1p7_2p7_2023D28_GenSimHLBeamSpotFull+DigiFullTrigger_2023D28+RecoFullGlobal_2023D28+HARVESTFullGlobal_2023D28/step2.root'
options.outputFile = 'digiTuple.root'

options.maxEvents = -1 # -1 means all events
#options.skipEvents = 0 # default is 0.

#------------------------------------------------------------------------------------
# Get and parse the command line arguments
#------------------------------------------------------------------------------------
options.parseArguments()
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
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.Geometry.GeometryExtended2023D28Reco_cff') #==>> use D28 for now
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.L1Reco_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('PhysicsTools.PatAlgos.slimming.metFilterPaths_cff')
process.load('Configuration.StandardSequences.PATMC_cff')
process.load('Configuration.StandardSequences.Validation_cff')
process.load('DQMOffline.Configuration.DQMOfflineMC_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#KH
process.MessageLogger.cerr.FwkReport.reportEvery = 1

#------------------------------------------------------------------------------------
# Set up our analyzer
#------------------------------------------------------------------------------------
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_Tree_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_Event_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_GenParticles_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCDigis_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HBHERecHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCRecHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCSimHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_SimTracks_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_RecoTracks_cfi")

process.load("Validation.HGCalValidation.hgcalHitValidation_cfi")

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
    #process.hgcalTupleHBHERecHits*
    #process.hgcalTupleHGCRecHits*
    process.hgcalTupleGenParticles*
    process.hgcalTupleHGCSimHits*
    #process.hgcalTupleSimTracks*
    #process.hgcalTupleGeneralTracks*
    process.hgcalTupleHGCDigis*
    process.hgcalTupleTree
)


#-----------------------------------------------------------------------------------
# Path and EndPath definitions
#-----------------------------------------------------------------------------------
process.ntuple = cms.Path(
    #process.hgcalHitValidation*
    process.tuple_step
)

process.schedule = cms.Schedule(process.ntuple)
