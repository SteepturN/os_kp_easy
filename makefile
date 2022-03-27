##
# Project Title
#
# @file
# @version 0.1

compile: A B C
A: A.cpp
	g++ -g -Wall -pedantic -o A A.cpp
B: B.cpp
	g++ -g -Wall -pedantic -o B B.cpp
C: C.cpp
	g++ -g -Wall -pedantic -o C C.cpp

# end
