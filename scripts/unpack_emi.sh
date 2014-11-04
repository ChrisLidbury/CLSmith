#!/usr/bin/bash

cd emi

for file in /cygdrive/c/prog/clStressTest/experiments/basic_viar_emi/*.zip
do
  mkdir $(basename $file .zip)
  cp $file $(basename $file .zip)
  cd $(basename $file .zip)
  unzip -q $(basename $file)
  rm $(basename $file)
  cd ..
done

cd ..
