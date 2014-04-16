#!/bin/bash
for i in 21 23 24 26 27 #IPs of clients in which project is to be runned
do
 ssh guest@192.168.41.$i -t "cd BOINC ; ./boinccmd --project_attach http://192.168.41.99/graphiso d15212155cbc0632aab97d9f9ec08f40" &
done
