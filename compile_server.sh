
gcc -c -Wall server.c
gcc -c -Wall audio.c

gcc -o server server.o audio.o -lportaudio


