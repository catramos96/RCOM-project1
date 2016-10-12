#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"

typedef struct{	
    char port[20]; 	/*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate; 	/*Velocidade de transmissão*/
    unsigned int sequenceNumber;	/*Número de sequência da trama: 0, 1*/
    unsigned int timeout;  	/*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; 	/*Número de tentativas em caso de falha*/
    char frame[BUF_SIZE];	/*Trama*/
}dataLink;

static dataLink data_link;

void init_linkLayer(char *port);

int llopen(char *port, int isReceiver);

int llopen_sender(int fd);

int llopen_receiver(int fd);

char* build_frame_SU(int is); //tramas do tipo S ou UA
