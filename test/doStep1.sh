#!/bin/bash

#commented entries for pre9, uncommented for pre8

CUSTOMS=(
"print 'no customs'" \
"process.mix.digitizers.hcal.he.photoelectronsToAnalog = cms.vdouble([57.5]*14)\nprocess.mix.digitizers.hcal.he.pixels = cms.int32(27370)\nprocess.es_hardcode.heUpgrade.pedestal = cms.double(17.3)\nprocess.es_hardcode.heUpgrade.pedestalWidth = cms.double(1.5)\nprocess.es_hardcode.heUpgrade.gain = cms.vdouble(1/3568.)" \
#"print 'no customs'" \
#"process.mix.digitizers.hcal.he.photoelectronsToAnalog = cms.vdouble([10]*14)\nprocess.mix.digitizers.hcal.he.pixels = cms.int32(4500*4*2)\nprocess.es_hardcode.heUpgrade.pedestal = cms.double(18)\nprocess.es_hardcode.heUpgrade.pedestalWidth = cms.double(5)\nprocess.es_hardcode.heUpgrade.gain = cms.vdouble(1/900.)"\
)

NAMES=(
"step1_old" \
"step1_new" \
#"step1_pre9_new" \
#"step1_pre9_old" \
)

for ((i=0; i < ${#CUSTOMS[@]}; i++)); do
  cmsDriver.py SinglePiE50HCAL_pythia8_cfi --python_filename run_${NAMES[$i]}.py --conditions auto:phase1_2017_design -n 10 --era Run2_2017 --geometry Configuration.Geometry.GeometryExtended2017dev_cff,Configuration.Geometry.GeometryExtended2017devReco_cff --eventcontent FEVTDEBUGHLT -s GEN,SIM,DIGI:pdigi_valid --datatier GEN-SIM-DIGI-RAW-HLTDEBUG --beamspot Realistic50ns13TeVCollision --fileout file:${NAMES[$i]}.root --customise SLHCUpgradeSimulations/Configuration/HCalCustoms.customise_Hcal2017Full --customise_commands "${CUSTOMS[$i]}" > log_${NAMES[$i]}.log 2>&1 &
done

