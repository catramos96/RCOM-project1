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

typedef struct
{	
    char port[20]; 	/*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate; 	/*Velocidade de transmissão*/
    unsigned int sequenceNumber;	/*Número de sequência da trama: 0, 1*/
    unsigned int timeout;  	/*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; 	/*Número de tentativas em caso de falha*/
    char frame[BUF_SIZE];	/*Trama*/
    unsigned int frame_size;
}dataLink;

typedef enum {
    START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP
} State;

//ERROR - qualquer erro de escrita ou leitura ou que aborte o programa
//DATA ERROR - erro na interpretacao dos dados da trama I
//OK - sem erros
typedef enum {
    ERROR, DATAERROR, OK, MISTAKENTYPE, IS_RR, IS_REJ
} ReturnType;

typedef enum {
    DISC, UA, SET, I, RR, REJ, RR_REJ
} ReceiveType;

static dataLink data_link;

void init_linkLayer(char *port);

int llopen(char *port, int isReceiver);

int llopen_sender(int fd);

int llopen_receiver(int fd);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

char* build_frame_SU(ReceiveType flag); //tramas do tipo S ou UA

char* build_frame_I(char* data, unsigned int data_length);

ReturnType receive(int fd, ReceiveType flag);

char getControlField(ReceiveType flag);

int stuff(char **frame, int frame_length);

int desstuff(char **frame, int frame_length);

int llclose(int fd, int isReceiver);

int llclose_receiver(int fd);

int llclose_sender(int fd);
