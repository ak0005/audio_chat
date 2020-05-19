#include "audio.h"
#include <stdio.h>

int main() {
    printf("Demo, recording!\n");
    int sec=20;

    if(initiallise()==-1){
        //error;
        return -1;
    }
    
    if(record(sec)==-1){
        printf("FAILED\n");
    }


    SAMPLE *a=(SAMPLE *) malloc( numBytes );  
    for(long i=0; i<numSamples; i++ ) 
        a[i]=data.recordedSamples[i];       //copy record.

    terminate();                            //delete original

    
    if(initiallise()==-1){                  // reintiallisation
        //error;
        return -1;
    }

    adjustSecond(sec);
    for(long i=0;i<numSamples; i++ ) data.recordedSamples[i] = a[i];  //copy the original one
    play();      //play the recorded then copied sound
    terminate();


    free(a);      //free dynamically allocated mem
    return 0;
}
