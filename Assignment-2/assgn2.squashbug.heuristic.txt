### Report for the Malware detection in Custom Shell

We have used two heuristics for the "squashbug" command.

• HEURISTIC 1---->
    Using the number of children spawned and the number of CPU clock cycles between 
system boot and process execution start time collectively as a heuristic.
    We are taking the weighted sum of both, the number of children spawned by the 
malware will be obviously higher than its child processes. However, only using it as 
a heuristic can give false positives as a system process can have a significantly 
higher number of children spawned than malware, in turn affecting the weighted 
ranking algorithm. We incorporated two other factors. First is that the number of 
CPU clock cycles between the system boot and process start time for system processes 
will be significantly less than other processes. However, we will have to give it a 
lower weight because the child process can have significantly more clock cycles than 
malware, as it is spawned later. The second one is that we do not consider processes 
with PID less than 4500 as malware because they are system processes. We can change 
the threshold value considering the system and other factors.

• HEURISTIC 2---->
    Going up in the ancestry tree until we find a parent-child duo where the names 
of both processes differ.
    In this heuristic, we have assumed that the malware will only spawn a child 
using its own executable. Hence, this heuristic will always be proper and safe as it 
will never suggest a system process. 