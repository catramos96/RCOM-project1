#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 	0
#define TRUE 	1

#define BUF_SIZE 	255
#define FRAME_SIZE 	5

//flags 
#define RECEIVER		1
#define TRANSMITTER		0

//modos DEBUG
#define NORMAL			0
#define SIMPLE_DEBUG	1
#define FULL_DEBUG		2

//main
static int BAUDRATE =  B9600;  
unsigned int RETRANSMITIONS;
unsigned int TIMEOUT;
unsigned int DATA_SIZE;
unsigned int MAX_PKG_SIZE;

//data link
#define FLAG 	(unsigned char)0x7E		//flag de inicio e fim de tramas

#define ESCAPE 	(unsigned char)0x7D		//escape flag
#define AUX 	(unsigned char)0x20  	//flag auxiliar para o byte stuffing

#define FRAME_A1 	(unsigned char)0x01	//campo de endereco
#define FRAME_A3 	(unsigned char)0x03	//campo de endereco 

#define FRAME_C_UA 	(unsigned char)0x07 //Campo de controlo para trama UA
#define FRAME_C_SET (unsigned char)0x03 //Campo de controlo para a trama SET
#define FRAME_C_I 	(unsigned char)0x40 //Campo de controlo para a trama I quando s=1
#define FRAME_C_DISC 	(unsigned char)0x0B	//campo de controlo para trama DISC
#define FRAME_C_RR 	(unsigned char)0x05	//campo de controlo para trama RR quando N(r) = 0
#define FRAME_C_REJ (unsigned char)0x01	//campo de controlo para trama REJ quando N(r) = 0

//application
#define	FILE_SIZE	1
#define FILE_NAME	2
#define	FILE_DATE	3
#define	FILE_PERM	4

#define	PKG_DATA	1
#define	PKG_START	2
#define PKG_END		3
