#python -i mugun_HEback.py noiseScenario=0 algo=1 scaleByArea=false pileup=0
#cmsRun mugun_HEback.py noiseScenario=3000 algo=2 scaleByArea=true pileup=200 maxEvents=5

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import random
import os
import math

from Configuration.StandardSequences.Eras import eras

#---------------
# My definitions
#---------------
sourceTag = "EmptySource"
procName  = "GEN"
infile    = []
pulibrary = '/eos/cms/store/group/dpg_hgcal/comm_hgcal/deguio/pu_library/'
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

options.register('pileup',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "pileup")

options.register('momentum',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.float,
                 "momentum")


options.parseArguments()

print "InputFile=", options.inputFile, "noiseScenario=", options.noiseScenario, "/fb  algo=", options.algo, "scaleByArea=", options.scaleByArea, "pileup=", options.pileup, "momentum=", options.momentum, "maxEvents=", options.maxEvents

if options.inputFile != '':
    procName  = "DIGI3"
    sourceTag = "PoolSource"
    infile    = [options.inputFile]
    maxEvents = -1

if options.maxEvents:
    maxEvents = options.maxEvents


#-----------------------------------
# Standard CMSSW Imports/Definitions
#-----------------------------------
process = cms.Process(procName,eras.Phase2C8)

process.load("Configuration.StandardSequences.SimulationRandomNumberGeneratorSeeds_cff")
process.load("Configuration.StandardSequences.Simulation_cff")

if options.pileup==0:
    process.load("SimGeneral.MixingModule.mixNoPU_cfi")
else:
    process.load('SimGeneral.MixingModule.mix_POISSON_average_cfi')
    process.mix.input.nbPileupEvents.averageNumber = cms.double(options.pileup)
    process.mix.bunchspace = cms.int32(25)
    process.mix.minBunch = cms.int32(-2)
    process.mix.maxBunch = cms.int32(2)
#    process.mix.minBunch = cms.int32(0)
#    process.mix.maxBunch = cms.int32(0)

    if options.noiseScenario != 0:
        pulibrary += 'endOfLife'
    else:
        pulibrary += 'beginOfLife'
    mixFiles=[os.path.join(pulibrary.replace('/eos/cms',''),f) for f in os.listdir(pulibrary)]
    random.shuffle(mixFiles)
    process.mix.input.fileNames = cms.untracked.vstring(mixFiles)


process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration/StandardSequences/DigiToRaw_cff')
process.load('Configuration/StandardSequences/RawToDigi_cff')
process.load('Configuration.StandardSequences.RecoSim_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

process.load('RecoLocalCalo.HGCalRecProducers.HGCalUncalibRecHit_cfi')
process.load('RecoLocalCalo.HGCalRecProducers.HGCalRecHit_cfi')

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')

process.load("IOMC.EventVertexGenerators.VtxSmearedGauss_cfi")
#process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load('Configuration.Geometry.GeometryExtended2023D41Reco_cff')
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
def etaToR(eta, Z):
    return Z*math.tan(2*math.atan(pow(math.e, -1*eta)))

particle=13
eMin=options.momentum
eMax=options.momentum
etaMin=1.52
etaMax=2.82
zMin=410.1
zMax=410.1
rMin=etaToR(etaMin, zMin)
rMax=etaToR(etaMax, zMax)

process.generator = cms.EDProducer("CloseByParticleGunProducer",
    PGunParameters = cms.PSet(
        PartID = cms.vint32(particle),
        EnMin = cms.double(eMin),
        EnMax = cms.double(eMax),
        RMin = cms.double(rMin),
        RMax = cms.double(rMax),
        ZMin = cms.double(zMin),
        ZMax = cms.double(zMax),
        Delta = cms.double(2.5),
        Pointing = cms.bool(False),
        Overlapping = cms.bool(False),
        RandomShoot = cms.bool(False),
        NParticles = cms.int32(2000),
        MaxEta = cms.double(etaMin),
        MinEta = cms.double(etaMax),
        MaxPhi = cms.double(3.14159265359),
        MinPhi = cms.double(-3.14159265359),
    ),
    Verbosity = cms.untracked.int32(0),
    psethack = cms.string('single particle random energy'),
    AddAntiParticle = cms.bool(False),
    firstRun = cms.untracked.uint32(1)
)

##process.generator = cms.EDProducer("FlatRandomEGunProducer",
##    PGunParameters = cms.PSet(
##        # you can request more than 1 particle
##        PartID = cms.vint32(13),         # 211: pion, 11: electron, 13: muon
##        MinEta = cms.double(1.30),
##        MaxEta = cms.double(2.82),
##        MinPhi = cms.double(-3.14159265359),         # 3.14159265359
##        MaxPhi = cms.double(3.14159265359),
##        MinE   = cms.double(options.momentum),
##        MaxE   = cms.double(options.momentum)
##    ),
##    firstRun = cms.untracked.uint32(1),
##    AddAntiParticle = cms.bool(False)
##)

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


#-----------
# DIGI setup
#-----------
#--- CUSTOMIZATION scenario
from SLHCUpgradeSimulations.Configuration.aging import customise_aging_3000, agedHGCal
if options.noiseScenario == 3000:
    process = agedHGCal(process)


process.mix.digitizers.hgchebackDigitizer.digiCfg.algo = cms.uint32(options.algo)
process.mix.digitizers.hgchebackDigitizer.digiCfg.scaleByArea = cms.bool(options.scaleByArea)


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

outName="muGun_NTU_p"+str(options.momentum)+".root"
process.TFileService = cms.Service("TFileService",
                                   fileName = cms.string(outName)
)

#--------
# OUTPUT
#--------
process.FEVToutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('FEVT'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string(
        'file:muGun_FEVT.root'
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
    process.hgcalTupleHGCRecHits*
    process.hgcalTupleTree
)

process.ntu_path = cms.Path(
    process.ntu
)

process.HGCalUncalibRecHit.HGCEEdigiCollection = cms.InputTag('simHGCalUnsuppressedDigis','EE')
process.HGCalUncalibRecHit.HGCHEFdigiCollection = cms.InputTag('simHGCalUnsuppressedDigis','HEfront')
process.HGCalUncalibRecHit.HGCHEBdigiCollection = cms.InputTag('simHGCalUnsuppressedDigis','HEback')
process.reco_path = cms.Path(
    process.HGCalUncalibRecHit*
    process.HGCalRecHit
)

process.gen_path = cms.Path(
 process.mix *
 process.addPileupInfo *
 process.rawDataCollector
)
if procName == 'GEN':
    process.gen_path.insert(0,process.generator * process.VtxSmeared * process.generatorSmeared * process.genParticles * process.g4SimHits)

process.outpath = cms.EndPath(process.FEVToutput)


#schedule
process.schedule = cms.Schedule(
    process.gen_path,
    process.reco_path,
    #process.outpath,
    process.ntu_path
)



process.options = cms.untracked.PSet(numberOfThreads = cms.untracked.uint32(1),
                                     numberOfStreams = cms.untracked.uint32(0))


#random seed generation
from IOMC.RandomEngine.RandomServiceHelper import RandomNumberServiceHelper
randSvc = RandomNumberServiceHelper(process.RandomNumberGeneratorService)
randSvc.populate()
