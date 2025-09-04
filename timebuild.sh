#!/bin/bash

maxmonthday=$(date -d "$(date --utc +%m)/1 + 1 month - 1 day" +%d) # 30 (for september)
currentmonthday=$(date --utc +%d)
perc=$((10#$currentmonthday * 100 / $maxmonthday))

vervalue1=0x$(date --utc +%Y%m%d)                                  # 0x20250903
vervalue2=0x1$(date --utc +%y%m)$perc                              # 0x1250930
vervalue3=0x01$(date --utc +%y%m)$perc                             # 0x01250930
vervalue4=$(date --utc +%y%m).$perc                                # 2509.30

echo Version Component 1: $vervalue1
echo Version Component 2: $vervalue2
echo Version Component 3: $vervalue3
echo Version Component 4: $vervalue4

filetopatch=./Platforms/AndromedaPkg/Andromeda.dsc.inc
filetopatch2=./Platforms/AndromedaPkg/Andromeda.new.dsc.inc

cat $filetopatch | sed -e "s/2412.74/$vervalue4/" >> $filetopatch2
rm $filetopatch
mv $filetopatch2 $filetopatch

cat $filetopatch | sed -e "s/0x01241274/$vervalue3/" >> $filetopatch2
rm $filetopatch
mv $filetopatch2 $filetopatch

cat $filetopatch | sed -e "s/0x1241274/$vervalue2/" >> $filetopatch2
rm $filetopatch
mv $filetopatch2 $filetopatch

cat $filetopatch | sed -e "s/0x20241223/$vervalue1/" >> $filetopatch2
rm $filetopatch
mv $filetopatch2 $filetopatch