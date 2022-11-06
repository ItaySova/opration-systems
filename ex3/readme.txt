readme
the file for this assignment is a cpp file.
The compilation command is:

g++ ex3.cpp -o ex3.out -std=c++11 -pthread

The run command the file need to recieve is:

./ex3.out <configuration file name>

example for run commands:

./ex3.out conf.txt
./ex3.out ./dir/conf.txt

both commands were tested on the planet server.
the configuration file the program is built to is from the following format:
<producer number>
<number of products>
<queue size>
.
.
.
<Co-Editor queue size>

for example: 
**************************
1
30
5

2
25
3

3
16
30

17
******************************
the format was approved during practice number 10 by Haim Shafir.
It's important to note that some of the debugging prints has been changed to comments
so feel free to change them back if needed (for convenience).
