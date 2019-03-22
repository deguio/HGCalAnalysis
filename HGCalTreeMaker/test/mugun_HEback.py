#python -i mugun_HEback.py noiseScenario=0 algo=1 scaleByArea=false scaleByDose=false

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import os

from Configuration.StandardSequences.Eras import eras

#---------------
# My definitions
#---------------
sourceTag = "EmptySource"
procName  = "GEN"
infile    = []
maxEvents = 5


options = VarParsing.VarParsing('analysis')
options.register('inputFile',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "input file")

options.register('noiseScenario',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.float,
                 "noise")

options.register('algo',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "algo") #0 empty digitizer, 1 calice digitizer, 2 realistic digitizer

options.register('scaleByArea',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "scaleByArea")

options.register('scaleByDose',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "scaleByDose")



options.parseArguments()

print "InputFile=", options.inputFile, "noiseScenario=", options.noiseScenario, "/fb  algo=", options.algo, "scaleByArea=", options.scaleByArea, "scaleByDose=", options.scaleByDose

if options.inputFile != '':
    procName  = "DIGI"
    sourceTag = "PoolSource"
    infile    = [options.inputFile]
    maxEvents = -1


#-----------------------------------
# Standard CMSSW Imports/Definitions
#-----------------------------------
process = cms.Process(procName,eras.Phase2C4)

process.load("Configuration.StandardSequences.SimulationRandomNumberGeneratorSeeds_cff")
process.load("Configuration.StandardSequences.Simulation_cff")

process.load('RecoLocalCalo.Configuration.RecoLocalCalo_Cosmics_cff')

process.load("SimGeneral.MixingModule.mixNoPU_cfi")
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration/StandardSequences/DigiToRaw_cff')
process.load('Configuration/StandardSequences/RawToDigi_cff')
process.load('Configuration.StandardSequences.RecoSim_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = '103X_upgrade2023_realistic_v2'

process.load("IOMC.EventVertexGenerators.VtxSmearedGauss_cfi")
#process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load('Configuration.Geometry.GeometryExtended2023D28Reco_cff')
process.load("Configuration.StandardSequences.MagneticField_cff")
process.g4SimHits.UseMagneticField = False #no mag field

process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(maxEvents)
)

process.source = cms.Source(sourceTag)

if options.inputFile != '':
    process.source.fileNames = cms.untracked.vstring(infile)

#-----------
# GEN setup
#-----------
process.generator = cms.EDProducer("FlatRandomEGunProducer",
    PGunParameters = cms.PSet(
        # you can request more than 1 particle
        PartID = cms.vint32(13),         # 211: pion, 11: electron
        MinEta = cms.double(1.30),
        MaxEta = cms.double(2.82),
        MinPhi = cms.double(-3.14159265359),         # 3.14159265359
        MaxPhi = cms.double(3.14159265359),
        MinE   = cms.double(150.0),
        MaxE   = cms.double(150.0)
    ),
    firstRun = cms.untracked.uint32(1),
    AddAntiParticle = cms.bool(False)
)

###--- NB: vertex spacial displacement in cm
###--- specifically for the center of ieta=33, iphi=3
###
#process.VtxSmeared.MeanX = 61.0       # R=57.6-68.6cm ieta ring 33, *cos (0.261799)
#process.VtxSmeared.MeanY = 16.3       #                             *sin (0.261799)
#process.VtxSmeared.MeanZ = 1100.0     # 1115 cm - HF surface

process.VtxSmeared.SigmaX = 0.00001   # further reduce vertex smearing
process.VtxSmeared.SigmaY = 0.00001
process.VtxSmeared.SigmaZ = 0.00001

#--- CUSTOMIZATION: releasing g4Sim constraints (PR #25251)
process.g4SimHits.Generator.ApplyPCuts = cms.bool(False)
process.g4SimHits.Generator.ApplyEtaCuts = cms.bool(False)

#--- CUSTOMIZATION mixing module
# (-37 ns wrt regular pp interaction point)
#process.mix.digitizers.hcal.hf1.timePhase = cms.double(-24.0)
#process.mix.digitizers.hcal.hf2.timePhase = cms.double(-23.0)

#from SimCalorimetry.HGCalSimProducers.hgcalDigitizer_cfi import *
#process.mix.theDigitizers = cms.PSet( hgcee      = cms.PSet( hgceeDigitizer),
#                                      hgchefront = cms.PSet( hgchefrontDigitizer),
#                                      hgcheback  = cms.PSet( hgchebackDigitizer)
#                                      )

#--- CUSTOMIZATION of vertex smearing
process.VtxSmeared.src = cms.InputTag("generator", "unsmeared")
process.generatorSmeared = cms.EDProducer("GeneratorSmearedProducer")
process.g4SimHits.Generator.HepMCProductLabel = cms.InputTag('VtxSmeared')


#--- CUSTOMIZATION scenario
from SLHCUpgradeSimulations.Configuration.aging import customise_aging_3000
if options.noiseScenario == 3000:
    process = customise_aging_3000(process)

process.mix.digitizers.hgchebackDigitizer.digiCfg.algo = cms.uint32(options.algo)
process.mix.digitizers.hgchebackDigitizer.digiCfg.scaleByArea = cms.bool(options.scaleByArea)
process.mix.digitizers.hgchebackDigitizer.digiCfg.scaleByDose = cms.bool(options.scaleByDose)


#process.HGCAL_noise_MIP = cms.PSet(
#    value = cms.double(1./options.SoN)
#    )


#-------------------
# ntuplizer imports
#-------------------
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_Tree_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_Event_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_GenParticles_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCDigis_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HBHERecHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCRecHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_HGCSimHits_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_SimTracks_cfi")
process.load("HGCalAnalysis.HGCalTreeMaker.HGCalTupleMaker_RecoTracks_cfi")

process.TFileService = cms.Service("TFileService",
                                   fileName = cms.string("muGun_NTU.root")
)

#--------
# OUTPUT
#--------
process.FEVToutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM-DIGI-RAW'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string(
        #'file:/eos/cms/store/group/dpg_hcal/comm_hcal/deguio/HGCal/DigiStudies/ana_h2_muGun_processing_GEN-SIM-DIGI-RAW.root'
        'file:muGun_GEN-SIM-DIGI-RAW.root'
        ),
    outputCommands = process.FEVTDEBUGHLTEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

#---------------------
# sequence definition
#---------------------
process.ntu = cms.Sequence(
    process.hgcalTupleEvent*
    process.hgcalTupleGenParticles*
    process.hgcalTupleHGCSimHits*
    process.hgcalTupleHGCDigis*
    process.hgcalTupleTree
)

process.ntu_path = cms.Path(
    process.ntu
)


process.gen_path = cms.Path(
 process.mix *
 process.addPileupInfo *
 process.rawDataCollector
)
if procName == 'GEN':
    process.gen_path.insert(0,process.generator * process.VtxSmeared * process.generatorSmeared * process.genParticles * process.g4SimHits)


#schedule
process.schedule = cms.Schedule(
    process.gen_path,
    process.ntu_path
)


#process.outpath = cms.EndPath(process.FEVToutput)

#process.options = cms.untracked.PSet(numberOfThreads = cms.untracked.uint32(4),
#                                     numberOfStreams = cms.untracked.uint32(0))
