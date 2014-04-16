#!/bin/bash
for i in 21 23 24 26 27 #IPs of systems to be controlled
do
 ssh guest@192.168.41.$i -t "cd BOINC ; ./boinccmd --project  http://192.168.41.99/graphiso detach"
ssh guest@192.168.41.$i -t "cd BOINC ; ./boinccmd --quit" &
done
