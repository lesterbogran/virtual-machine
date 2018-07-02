#!/bin/bash

# this script executes all tests
# be sure u have nja assembler in ksp
# all tests must be allocated in directory ./Test
# put just *.asm files

rm -rf TestBins

mkdir TestBins
ext=".bin"
return_path="../TestBins/"

cd Test

for file in *.asm
do
	file_name=${file//".asm"/""}
	out_file="$file_name$ext"
	./../nja $file $out_file
	mv $out_file $return_path
done

# now created all test files
# starting njvm
cd ..

for file in TestBins/*.bin
do
	echo "executing file: $file"
	./njvm $file
done

