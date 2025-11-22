#!/usr/bin/env bash
file=(A-n32-k5 A-n37-k6 A-n39-k5 A-n45-k6 A-n48-k7 A-n54-k7 A-n60-k9)

for f in "${file[@]}"; do
  wget --no-check-certificate "https://vrp.galgos.inf.puc-rio.br/media/com_vrp/instances/A/${f}.sol"
done
