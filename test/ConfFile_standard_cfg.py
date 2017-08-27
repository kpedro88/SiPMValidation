import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing ('analysis')
options.register ('inputName',
				  'step1',
				  VarParsing.multiplicity.singleton,
				  VarParsing.varType.string,
				  "name of input file")
options.register ('tagName',
				  'simHcalUnsuppressedDigis',
				  VarParsing.multiplicity.singleton,
				  VarParsing.varType.string,
				  "name of input tag")
options.parseArguments()

from Configuration.StandardSequences.Eras import eras

process = cms.Process('Demo',eras.Phase2C2)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.Geometry.GeometryExtended2023D17Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2023D17_cff')
process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring("file:"+options.inputName+".root")
)

process.demo = cms.EDAnalyzer("QIE11Validation",
    rootOutputFile = cms.string('QIE11digis_'+options.inputName+'.root'),
    QIE11tag = cms.InputTag(options.tagName,"HBHEQIE11DigiCollection")
)

process.p = cms.Path(process.demo)
