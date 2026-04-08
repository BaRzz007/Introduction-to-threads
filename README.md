# Introduction-to-threads
This repo contains code examples to illustrate threading in a computer operating system

Copied from [CS170 Lecture notes - UCSB](https://sites.cs.ucsb.edu/~rich/class/cs170/notes/IntroThreads/)

## Man pages
[pthread_create](https://man7.org/linux/man-pages/man3/pthread_create.3.html)
[pthread_join](https://man7.org/linux/man-pages/man3/pthread_join.3.html)

## Compile
```
gcc -g avg-1thread.c -o avg-1thread -lpthread
```
```
gcc -g avg-manythread.c -o avg-manythread -lpthread
```
Makefile will be added later