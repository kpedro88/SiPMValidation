#!/bin/bash

for name in step1; do
  cmsRun ConfFile_standard_cfg.py inputName=${name} > log_analysis_${name}.log 2>&1 &
done
