## Installation

To install this code in CMSSW:
```
cmsrel CMSSW_8_1_0_pre11
cd CMSSW_8_1_0_pre11/src/
git cms-init
mkdir test
cd test
git clone git@github.com:kpedro88/SiPMValidation -b DigiReco
scram b -j 8
```

## Instructions for CMSSW_8_1_X

* Run GEN-SIM-DIGI with script [doStep1.sh](./test/doStep1.sh)
* Run analyzer with script [doAnalysis.sh](./test/doAnalysis.sh)
* Run macro to produce plots, example:
```
root -b -l -q 'plotPulses.C+({"QIE11digis_vs_RecHits_step1.root"},{"81Xpre11 digis (revertSiPM)"},{"81Xpre11_revert_digi"},{125},{kBlack},1,1,0)'
root -b -l -q 'plotPulses.C+({"QIE11digis_vs_RecHits_step1.root"},{"81Xpre11 rechits (revertSiPM)"},{"81Xpre11_revert_rechit"},{125},{kBlack},1,1,1)'
```
