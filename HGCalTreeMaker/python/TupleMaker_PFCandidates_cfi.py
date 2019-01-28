import FWCore.ParameterSet.Config as cms

tuplePFCandidates = cms.EDProducer("TupleMaker_PFCandidates",
  source    = cms.untracked.InputTag('particleFlow', ''),
  PackedCandidate = cms.untracked.bool(False),
  Prefix    = cms.untracked.string  ("PFPar"),
  Suffix    = cms.untracked.string  ("")
)

tuplePackedPFCandidates = tuplePFCandidates.clone(
  source    = cms.untracked.InputTag('packedPFCandidates', ''),
  PackedCandidate = cms.untracked.bool(True),
  Prefix    = cms.untracked.string  ("PFPar"),
  Suffix    = cms.untracked.string  ("")
)
