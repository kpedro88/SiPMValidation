## Installation

To install this code in CMSSW (e.g. CMSSW_8_1_0_pre8):
```
cmsrel CMSSW_8_1_0_pre8
cd CMSSW_8_1_0_pre8/src/
git cms-init
mkdir test
cd test
git clone git@github.com:kpedro88/SiPMValidation
scram b -j 8
```

## Instructions for CMSSW_8_1_X

* Run GEN-SIM-DIGI with script [doStep1.sh](./test/doStep1.sh)
* Run analyzer with script [doAnalysis.sh](./test/doAnalysis.sh)
* Run macro to produce plots, example:
```
root -b -q -l 'plotPulses.C+(\
{"QIE11digis_step1_old.root","QIE11digis_step1_new.root"},\
{"81Xpre8 (old)","81Xpre8 (new)"},\
{"81Xpre8_old","81Xpre8_new"},\
{125,125},\
{kBlack,kBlue},\
1,1)'
```

## Special instructions for CMSSW_6_2_X_SLHC

* `git cms-merge-topic kpedro88:81X_SiPM_params_620SLHC28` to get new HE segmentation and new params (revert last commit to return to old params)
* QIE11 analyzer will not compile
* use special script [doStep1_62XSLHC.sh](./test/doStep1_62XSLHC.sh) for GEN-SIM-DIGI
