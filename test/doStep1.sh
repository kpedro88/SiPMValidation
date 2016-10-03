#!/bin/bash

#commented entries for pre9, uncommented for pre8

CUSTOMS=(
"print 'no customs'" \
"process.mix.digitizers.hcal.he.doSiPMSmearing = cms.bool(True)" \
)

NAMES=(
"step1" \
"step1_smear" \
)

for ((i=0; i < ${#CUSTOMS[@]}; i++)); do
  cmsDriver.py SinglePiE50HCAL_pythia8_cfi --python_filename run_${NAMES[$i]}.py --conditions auto:phase1_2017_design -n 100 --era Run2_2017 --geometry Configuration.Geometry.GeometryExtended2017dev_cff,Configuration.Geometry.GeometryExtended2017devReco_cff --eventcontent FEVTDEBUGHLT -s GEN,SIM,DIGI:pdigi_valid --datatier GEN-SIM-DIGI-RAW-HLTDEBUG --beamspot Realistic50ns13TeVCollision --fileout file:${NAMES[$i]}.root --customise SLHCUpgradeSimulations/Configuration/HCalCustoms.customise_Hcal2017Full --customise_commands "${CUSTOMS[$i]}" --nThread 4 > log_${NAMES[$i]}.log 2>&1 &
done

