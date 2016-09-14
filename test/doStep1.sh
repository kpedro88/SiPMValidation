#!/bin/bash

CUSTOMS=(
"print 'no customs'" \
)

NAMES=(
"step1" \
)

for ((i=0; i < ${#CUSTOMS[@]}; i++)); do
  cmsDriver.py SinglePiE50HCAL_pythia8_cfi --runUnscheduled --python_filename run_${NAMES[$i]}.py --conditions auto:phase1_2017_design -n 100 --era Run2_2017 --geometry Configuration.Geometry.GeometryExtended2017dev_cff,Configuration.Geometry.GeometryExtended2017devReco_cff --eventcontent FEVTDEBUG -s GEN,SIM,DIGI:pdigi_valid,L1,DIGI2RAW,HLT:@relval25ns,RAW2DIGI,L1Reco,RECO --datatier GEN-SIM-RECO --beamspot Realistic50ns13TeVCollision --fileout file:${NAMES[$i]}.root --customise SLHCUpgradeSimulations/Configuration/HCalCustoms.customise_Hcal2017Full --customise_commands "${CUSTOMS[$i]}" --nThread 4 > log_${NAMES[$i]}.log 2>&1 &
done

