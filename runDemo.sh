gcc -Wall -c audio.c
gcc -Wall -c demo.c
gcc -Wall -o main audio.o demo.o -lportaudio
./main
