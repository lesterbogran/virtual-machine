#!/bin/bash

# be sure that ksp directory contains actual njc file
# put jour *.nj files into the TestSrc directory

cd TestSrc

ext=".asm"
counter=0
for file in *.nj
do
	file_name=${file//".nj"/""}
	out_file="$file_name$ext"
	./../njc $file > $out_file
	printf "$counter) compiled file $file \n"
	mv $out_file ../Test/
	counter=$((counter+1))
done

cd ..
