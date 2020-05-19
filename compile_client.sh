
gcc -c -Wall client.c
gcc -c -Wall audio.c

gcc -o client client.o audio.o -lportaudio


