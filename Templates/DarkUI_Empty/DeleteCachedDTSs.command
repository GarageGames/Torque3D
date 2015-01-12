#!/bin/sh

cd "`dirname "$0"`"

for i in $(find . -type f \( -iname "*.dae" \))
do
	len=$((${#i} - 4))
   file=${i:0:$len}.cached.dts
   if [ -e $file ]
   then
   	echo "Removing ${file}"
   	rm $file
   fi
done

