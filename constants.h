#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

#define FRAME_SIZE 5

#define FLAG (unsigned char)0x7E		//flag de inicio e fim de tramas

#define ESCAPE (unsigned char)0x7D	//escape flag
#define AUX (unsigned char)0x20  	//escape flag

#define FRAME_A (unsigned char)0x03	//campo de endereco 
#define FRAME_A2 (unsigned char)0x01	//campo de endereco 

#define FRAME_C_UA (unsigned char)0x07 	//Campo de controlo para trama UA
#define FRAME_C_SET (unsigned char)0x03 	//Campo de controlo para a trama SET
#define FRAME_C_I (unsigned char)0x40    //Campo de controlo para a trama I quando s=1
#define FRAME_C_DISC (unsigned char)0x0B	//campo de controlo para trama DISC
#define FRAME_C_RR (unsigned char)0x05	//campo de controlo para trama RR
#define FRAME_C_REJ (unsigned char)0x01	//campo de controlo para trama REJ

#define	FILE_SIZE	1
#define FILE_NAME	2
#define	FILE_DATE	3
#define	FILE_PERM	4

#define	PKG_DATA	1
#define	PKG_START	2
#define PKG_END		3

#define DATA_SIZE 		114
#define MAX_PKG_SIZE	124

#define RECEIVER		1
#define TRANSMITTER		0

static int BAUDRATE =  B9600; 
static unsigned int RETRANSMITIONS = 3;
static unsigned int TIMEOUT = 1;
#define BUF_SIZE 255
//unsigned int BUF_SIZE = 255;	//126
