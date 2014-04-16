#!/bin/bash
for i in 21 23 24 26 27
do 
ssh guest@192.168.41.$i -t "rm BOINC -r" &
 scp boinc_7.2.42_i686-pc-linux-gnu.sh guest@192.168.41.$i:/home/guest
 ssh guest@192.168.41.$i -t "sh boinc_7.2.42_i686-pc-linux-gnu.sh"
 ssh guest@192.168.41.$i -t "cd BOINC ; ./boinc" &
done
