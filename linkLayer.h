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

/**
 * Estrutura de informação de uma trama
 */
typedef struct
{
    char port[20];        /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate;       /*Velocidade de transmissão*/
    unsigned int sequenceNumber;    /*Número de sequência da trama: 0, 1*/
    unsigned int timeout;  /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions;  /*Número de tentativas em caso de falha*/
    unsigned int mode;
}dataLink;

/**
 * Estrutura de dados que guarda os tipos de Control Field
 */
typedef enum {
    DISC, UA, SET, I, RR, REJ
} ControlFieldType;

/**
 * Estrutura de dados que guarda as informacoes recebidas da trama.
 * Guarda o tipo (Control Field);
 * Guarda o conteudo dos dados de uma mensagem e o respetivo tamanho (caso seja do tipo I);
 * Guarda o erro da analise (ou OK se não houver erros)
 */
typedef struct
{
    ControlFieldType type;
    //char isCommand; //ainda nao sei se devo colocar
    unsigned char message[BUF_SIZE];        
    unsigned int message_size;    
    unsigned int isRetransmission;  
    unsigned int controlAdress; //so pode ser 1 ou 3
} Message;

/**
 * Estrutura de informação que guarda os estados da leitura
 */
typedef enum {
    START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP
} State;

/**
 * Estrutura de dados que guarda o tipo de erros
 * 
 * ERROR - qualquer erro de escrita ou leitura ou que aborte o programa
 * DATA ERROR - erro na interpretacao dos dados da trama I
 * OK - sem erros
 */
typedef enum {
    ERROR, DATAERROR, OK, EMPTY
} ReturnType;



typedef struct
{
    unsigned int tramasIenviadas;
    unsigned int tramasIretransmitidas;
    unsigned int tramasIrecebidas;
    unsigned int timeouts;
    unsigned int REJenviados;
    unsigned int REJrecebidos;
}estatisticas;

estatisticas statistics;



//METODOS

void init_linkLayer(unsigned char *port, unsigned int mode);

int llopen(unsigned char *port, int isReceiver);

int llopen_sender(int fd);

int llopen_receiver(int fd);

int llwrite(int fd, unsigned char * buffer, int length);

int llread(int fd, unsigned char * buffer);

int llclose(int fd, int isReceiver);

int llclose_receiver(int fd);

int llclose_sender(int fd);

unsigned char* build_frame_SU(ControlFieldType flag, unsigned char flag_A);

unsigned char* build_frame_I(unsigned char* data, unsigned int data_length);

ReturnType receive(int fd, Message *msg);

unsigned char getControlField(ControlFieldType flag);

ControlFieldType setControlField(unsigned char c);

int stuff(unsigned char *frame, int frame_length);

int destuff(unsigned char *frame, int frame_length);

void display(unsigned char *frame, int n);

void displayStatistics(int status);

void initStatistics();

