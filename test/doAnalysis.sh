#!/bin/bash

for name in step1_old step1_new; do
  cmsRun ConfFile_standard_cfg.py inputName=${name} > log_analysis_${name}.log 2>&1 &
  #for HcalUpgradeDataFrames in 81X
  #cmsRun ConfFileUpg_standard_cfg.py inputName=${name} > log_analysis_${name}.log 2>&1 &
  #for 62XSLHC
  #cmsRun ConfFile62XSLHC_standard_cfg.py inputName=${name} > log_analysis_${name}.log 2>&1 &
done
