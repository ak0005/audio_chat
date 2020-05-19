#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> // for close
#include <string.h>
#include <netdb.h>
#include "audio.h"

int recievedByte;
int expectedByte;

int audio_processing_flag = 0;
int src_playing_flag = 0;

SAMPLE *a;

int getSec(char msg[100])
{
    int sec = 0;
    for (int i = 2; i < 100; ++i)
    {
        if (msg[i] == '\0' || msg[i] - '0' > 10 || msg[i] - '0' < 0)
            break;
        sec += msg[i] - '0';
        sec *= 10;
    }
    sec /= 10;
    fflush(stdout);
    return sec;
}

int recieveText(int newfd, char msg[])
{
    int temp = (recv(newfd, msg, 100, 0));
    if (temp == -1)
    {
        perror("recv");
        return -1;
    }
    if (temp == 0)
    {
        printf("They exited");
        return -1;
    }
    return 0;
}

int recieveAudio(int newfd)
{

    int temp = (recv(newfd, a, numBytes, 0));

    if (temp == -1)
    {
        free(a);
        perror("recv");
        return -1;
    }
    if (temp == 0)
    {
        free(a);
        printf("They exited");
        fflush(stdout);
        return -1;
    }

    long recievedIndex = recievedByte / sizeof(SAMPLE), gotIndex = temp / sizeof(SAMPLE);

    for (long i = recievedIndex; i < recievedIndex + gotIndex && i < numSamples; i++)
        data.recordedSamples[i] = a[i];

    recievedByte += temp;
    if (recievedByte >= expectedByte)
    {
        recievedByte = expectedByte = 0;
        free(a);
        int k = play();

        if(send(newfd,&k,sizeof(int),0)==-1){
        audio_processing_flag = 0;
            return -1;
        }
        audio_processing_flag = 0;

        return (k);
    }

    return 0;
}

int handleStreamClient(int newfd, char msg[])
{
    if (src_playing_flag)
    {
        if (recieveText(newfd, msg) == -1)
            return -1;
		  fflush(stdin);
        printf("\n Now You can send another msg\n");
        src_playing_flag = 0;
        return 0;
    }

    if (recievedByte != expectedByte)
    {
        if (recieveAudio(newfd) == -1)
        {
            audio_processing_flag = 0;
            perror("RecieveAudio");
            return 0;
        }
        return 0;
    }

    if (recieveText(newfd, msg) == -1)
        return -1;

    if (msg[0] == '0')
    {
        printf("Recieved text\n");
        fflush(stdout);

        printf("\n==msg==\n");
        printf("%s\n", msg+2);
        fflush(stdout);
        if (strncmp("bye", msg, 3) == 0)
            return -1;
    }
    else
    {
        audio_processing_flag = 1;
        printf("Recieving Audio\n");
        fflush(stdout);
        if (adjustSecond(getSec(msg)) == -1)
        {
            audio_processing_flag = 0;
            return -1;
        }
        a = (SAMPLE *)malloc(numBytes);
        expectedByte = numBytes;
        recievedByte = 0;
        fflush(stdout);
        return (0);
    }

    return 0;
}

int clientStreamSocket(char ip[], char port[])
{
    int sockfd;
    struct addrinfo hints, *results, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port, &hints, &results) == -1)
    {
        perror("STREAM: getaddrinfo");
        return (-1);
    }

    for (p = results; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("STREAM: sockfd");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("STREAM: connect");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        perror("STREAM: Unable to create socket\n");
        return (-1);
    }

    return sockfd;
}

int sendNormalMsg(int newfd, char msg[100])
{
    if ((send(newfd, msg, strlen(msg) + 1, 0)) == -1)
    {
        perror("send 1.1");
        return -1;
    }

    if (strncmp("0 bye", msg, 5) == 0)
        return -1;

    return 0;
}

int sendAudioMsg(int newfd, char msg[100])
{
    audio_processing_flag = 1;

    if ((send(newfd, msg, strlen(msg) + 1, 0)) == -1)
    {
        audio_processing_flag = 0;
        perror("send 1.1");
        return -1;
    }

    int sec = getSec(msg);

    if (record(sec) == -1)
    {
        audio_processing_flag = 0;
        perror("RECORD");
        return -1;
    }
    printf("num of bytes %d\n", numBytes);

    SAMPLE *a = (SAMPLE *)malloc(numBytes);
    for (long i = 0; i < numSamples; i++)
        a[i] = data.recordedSamples[i];

    if ((send(newfd, a, numBytes, 0)) == -1)
    {
        audio_processing_flag = 0;
        free(a);
        perror("send 1.2");
        return -1;
    }
    free(a);
    
    src_playing_flag = 1;
    audio_processing_flag = 0;
    return 0;
}

int main(int v, char *argv[])
{
    if (v < 3)
    {
        printf("need server IP(127.0.0.1) as argument 1st and server port(3490) as argument second\n");
        exit(1);
    }

    int i;
    fd_set readfds, readfdsm;
    int streamSockfd, maxfd;
    char msg[100];

    if ((streamSockfd = clientStreamSocket(argv[1], argv[2])) == -1)
    {
        exit(1);
    }

    maxfd = streamSockfd;

    printf("Please wait initiallising mic, it will take a minute (ignore ALSA warnings (if any))\n");
    fflush(stdout);
    if (initiallise() == -1)
    {
        perror("Mic initiallisation");
        exit(1);
    }

    FD_ZERO(&readfdsm);
    FD_SET(streamSockfd, &readfdsm);
    FD_SET(0, &readfdsm);

    printf("\n\nstarting chat, type \"0 bye\" (Case sensitive) to exit\n to send audio press '1 {sec}'\nexample:- to record audio of 10 sec enter \"1 10\" \nfor text enter your text with '0 ' as the superscript\n example:- to send \"my text\" enter \"0 my text\"\n\n");
    fflush(stdout);

    while (1)
    {
        readfds = readfdsm;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            break;
        }

        for (i = 0; i <= maxfd; ++i)
        {
            if (FD_ISSET(i, &readfds))
            {
                if (i == 0)
                {
                    fgets(msg, sizeof(msg), stdin);

                    if (strlen(msg) < 3 || msg[1] != ' ' || (msg[0] != '1' && msg[0] != '0'))
                    {
                        printf("Msg not recognized, see the description above\n\n");
                        fflush(stdout);
                        continue;
                    }
                    if (audio_processing_flag)
                    {
                        printf("\naudio under processing please wait\n");
                        continue;
                    }
                    if (src_playing_flag)
                    {
                        printf("\npls wait let the other user listen your audio\n");
                        continue;
                    }
                    if (msg[0] == '0')
                    {
                        if (sendNormalMsg(streamSockfd, msg) == -1)
                            break;
                    }
                    else
                    {
                        if (sendAudioMsg(streamSockfd, msg) == -1)
                        {
                            perror("SendAudioMsg");
                            continue;
                        }
                    }
                }
                else if (i == streamSockfd && handleStreamClient(i, msg) == -1)
                {
                    break;
                }
            }
        }

        if (i <= maxfd)
            break;
    }

    terminate();
    close(streamSockfd);

    return (0);
}
