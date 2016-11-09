import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing ('analysis')
options.register ('inputName',
				  'step1',
				  VarParsing.multiplicity.singleton,
				  VarParsing.varType.string,
				  "name of input file")
options.register ('dR',
				  0.3,
				  VarParsing.multiplicity.singleton,
				  VarParsing.varType.float,
				  "dR cut")
options.parseArguments()

process = cms.Process("Demo")

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.GeometrySimDB_cff')
process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2017_design', '')

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring("file:"+options.inputName+".root")
)

process.demo = cms.EDAnalyzer("FullSimPionAnalyzer",
    fileName = cms.string('tree_pion_'+options.inputName+'.root'),
    dRcut = cms.double(options.dR)
)

process.p = cms.Path(process.demo)
