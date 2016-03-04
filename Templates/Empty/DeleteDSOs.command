#!/bin/sh

cd "`dirname "$0"`"

for i in $(find . -type f \( -iname "*.cs" \))
do
   file=${i}.dso
   if [ -e $file ]
   then
   	echo "Removing ${file}"
   	rm $file
   fi
   file=${i}.edso
   if [ -e $file ]
   then
   	echo "Removing ${file}"
      rm $file
   fi
done
