#!/bin/bash
#
# Run generator on a range of seeds.

seed_min=1
seed_max=1000
seed_step=1

cl_launcher='./cl_launcher'
cl_filename='CLProg.c'
cl_platform_idx=0
cl_device_idx=0

output='Result.csv'
gen_exe='./CLSmith'

while [[ $# > 1 ]] ; do
  arg=$1; shift;
  case $arg in
    -seed_min) seed_min=$1;;
    -seed_max) seed_max=$1;;
    -seed_step) seed_step=$1;;
    -cl_launcher) cl_launcher=$1;;
    -cl_filename) cl_filename=$1;;
    -cl_platform_idx) cl_platform_idx=$1;;
    -cl_device_idx) cl_device_idx=$1;;
    -output) output=$1;;
    -gen_exe) gen_exe=$1;;
    *) echo "Unknown arg $arg";;
  esac
  shift
done

if [[ -f ${output} ]] ; then
  echo "Overwriting file ${output}"
fi

> ${output}
for seed in `seq $seed_min $seed_step $seed_max` ; do
  if [[ -f ${cl_filename} ]] ; then
    rm ${cl_filename}
  fi
  eval "${gen_exe} ${seed}"
  if [[ ! -f ${cl_filename} || $? != 0 ]] ; then
    echo "gen_error" >> ${output}
    continue
  fi
  res=$(${cl_launcher} ${cl_filename} ${cl_platform_idx} ${cl_device_idx})
  if [[ $? != 0 ]] ; then
    echo "run_error" >> ${output}
    continue
  fi
  res=`echo "${res}" | sed 's/Compiler callback.*//g'`

  item=`echo "${res}" | cut -d, -f1`
  rest=`echo "${res}" | cut -d, -f2-`
  while [[ ${rest} != "" ]] ; do
    comp=`echo "${rest}" | cut -d, -f1`
    rest=`echo "${rest}" | cut -d, -f2- -s`
    if [[ ${item} -ne ${comp} ]] ; then
      item=different
      break
    fi
  done

  echo ${item} >> ${output}
done
