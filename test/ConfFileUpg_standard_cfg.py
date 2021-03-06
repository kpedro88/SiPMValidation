import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing ('analysis')
options.register ('inputName',
				  'step1',
				  VarParsing.multiplicity.singleton,
				  VarParsing.varType.string,
				  "name of input file")
options.parseArguments()

process = cms.Process("Demo")

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.Geometry.GeometrySimDB_cff')
process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.MessageLogger.cerr.FwkReport.reportEvery = 1

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring("file:"+options.inputName+".root")
)

process.demo = cms.EDAnalyzer("HcalUpgradeValidation",
    rootOutputFile = cms.string('HcalUpgradedigis_'+options.inputName+'.root'),
    HcalUpgradetag = cms.InputTag("simHcalUnsuppressedDigis","HBHEUpgradeDigiCollection")
)

process.p = cms.Path(process.demo)

# Automatic addition of the customisation function from SLHCUpgradeSimulations.Configuration.HCalCustoms
from SLHCUpgradeSimulations.Configuration.HCalCustoms import customise_Hcal2017Full

#call to customisation function customise_Hcal2017Full imported from SLHCUpgradeSimulations.Configuration.HCalCustoms
process = customise_Hcal2017Full(process)

