BOINC
=====

BOINC (Berkeley Open Infrastructure for Networked Computing) is a software system that makes it easy for researchers to create diverse applications, including those with large storage or communication resources and operate computationally intensive projects. BOINC projects are generally community built, and topics vary through all branches of science. Anybody can contribute to these projects by donating their computing power, when idle.

As part of the project, we try to implement a Computer Science problem in BOINC. Since graphs with a large number of nodes make good tasks, we try to implement a graph problem. Being an open-source initiative, the BOINC system has its own short-comings that affect the performance of projects. We plan to tackle one such issue regarding project deadlines. We propose a method to dynamically extend the deadline of the assigned tasks, depending on their performance. Finally, we develop a GUI for the Linux version of the client software to make it easier for volunteers to choose projects and allot resources.

The [BOINC GUI file](https://github.com/godlytalias/BOINC/blob/master/Boinc?raw=true) and the [BOINC core client - after running the script](http://boinc.berkeley.edu/dl/boinc_7.2.33_x86_64-pc-linux-gnu.sh) is to be placed in a single directory before executing.

BOINC_MAIN_DIRECTORY/ <br/>
&nbsp;&nbsp;&nbsp;/BOINC <br/>
&nbsp;&nbsp;&nbsp;/Boinc_GUI
