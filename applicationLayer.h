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
#include "linkLayer.h"

/*PODE SER ALTERADO*/
struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status; 		/*TRANSMITTER | RECEIVER*/
};

typedef struct {
	char * content;		//conteudo do package
	unsigned int size;	//num de bytes do package
}applicationPackage;

static unsigned int sequenceNumber = 0;

int sender(char* port, char* filepath);

int reciever(char* port);

int sendControlPackage(char control, int fd,char * filename, char * filesize, char * date, char * permissions);

int sendDataPackage(int fd,char *data, int sequenceN, unsigned int size);


