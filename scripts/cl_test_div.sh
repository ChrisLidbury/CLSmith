#!/bin/bash
#
# Run generator on a range of seeds.
# Same as cl_test, but does not expect all threads to produce the same output.
# The result is the addition of the output of all the threads.

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
  eval "timeout 40 ${gen_exe} ${seed}"
  if [[ ! -f ${cl_filename} || $? != 0 ]] ; then
    echo "gen_error" >> ${output}
    continue
  fi
  res=$(timeout 180 ${cl_launcher} ${cl_filename} ${cl_platform_idx} ${cl_device_idx})
  if [[ $? != 0 ]] ; then
    echo "run_error" >> ${output}
    continue
  fi
  res=`echo "${res}" | sed 's/Compiler callback.*//g'`

  total=`echo "${res}" | cut -d, -f1 | sed 's/0x//g'`
  total="${total^^}"
  rest=`echo "${res}" | cut -d, -f2-`
  while [[ ${rest} != "" ]] ; do
    next=`echo "${rest}" | cut -d, -f1 | sed 's/0x//g'`
    next="${next^^}"
    rest=`echo "${rest}" | cut -d, -f2- -s`
    total=`echo "obase=16;ibase=16;${total}+${next}" | bc`
  done
  total="0x${total}"

  echo ${total} >> ${output}
done
