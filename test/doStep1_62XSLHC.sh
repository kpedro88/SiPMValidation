#!/bin/bash

CUSTOMS=(
"process.mix.digitizers.hcal.doIonFeedback = cms.bool(False)"
)

NAMES=(
#"step1_new" \
"step1_old" \
)

for ((i=0; i < ${#CUSTOMS[@]}; i++)); do
  cmsDriver.py SinglePiE50HCAL_cfi --python_filename run_${NAMES[$i]}.py --conditions auto:upgrade2019 -n 10 --geometry Extended2019 --eventcontent FEVTDEBUGHLT -s GEN,SIM,DIGI:pdigi_valid --datatier GEN-SIM-DIGI-RAW-HLTDEBUG --beamspot Gauss --fileout file:${NAMES[$i]}.root --customise SLHCUpgradeSimulations/Configuration/combinedCustoms.cust_2019 --customise_commands "${CUSTOMS[$i]}" > log_${NAMES[$i]}.log 2>&1 &
done

