# audio_chat
An audio and text chat between two computers using tcp_stream, select syscall and lportaudio library

audio recording implemented using lportaudio library

* to install portaudio lib in ubuntu:-
sudo apt-get install libasound-dev portaudio19-dev libportaudio2 libportaudiocpp0

If your audio driver is snd-hda-intel then their may be some problem in configurations

try to run the demo.sh, it will automatically start recording for 20 seconds and play it back.

If it work as expected then all good 

else if it's not recording then their is problem in the configuration of ALSA

else if it's only recording but you are not able to listen sound, make sure alsa mixer is not muted.


In case or error “Assertion 'ret == self->nfds' failed”  try to re-run

## TO RUN
on 1st pc

./compile_server.sh

./server

on 2nd pc

./compile_client.sh

./client <server_ip> 3490

here 3490 is port number

Ofcourse both can run on same pc, u need to use loopback address in place of server_ip

on 1st terminal

./compile_server.sh

./server

on 2nd terminal

./compile_client.sh

./client 127.0.0.1 3490

NOTE:- server need to run first in all cases.

Ignore ALSA warnings
