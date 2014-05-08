#!/bin/bash
#
# Run generator on a range of seeds.

if [[ $# -ne 1 ]] ; then
  echo "Expected one arg (directory)."
  exit 1
fi

if [[ ! -f "../src/CLSmith/CLSmith" ]] ; then
  echo "Build generator first."
  exit 1
fi
if [[ ! -f "../src/CLSmith/cl_launcher" ]] ; then
  echo "Build launcher first."
  exit 1
fi
if [[ ! -f "../runtime/CLSmith.h" ]] ; then
  echo "Cannot find CLSmith.h in ../runtime/"
  exit 1
fi
if [[ ! -f "../runtime/safe_math_macros.h" ]] ; then
  echo "Cannot find CLSmith.h in ../runtime/"
  exit 1
fi
if [[ ! -f "cl_test.sh" ]] ; then
  echo "Cannot find cl_test.sh"
  exit 1
fi

dir="${HOME}/$1"

if [[ -d "${dir}" ]] ; then
  echo "Test dir already exists."
  exit 1
fi

mkdir $dir
cp "../src/CLSmith/CLSmith" $dir
cp "../src/CLSmith/cl_launcher" $dir
cp "../runtime/CLSmith.h" $dir
cp "../runtime/safe_math_macros.h" $dir
cp "cl_test.sh" $dir
chmod +x "${dir}/cl_test.sh"
