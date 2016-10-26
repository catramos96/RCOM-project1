#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <utime.h>
#include "constants.h"
#include "linkLayer.c"

/*PODE SER ALTERADO*/
struct applicationLayer {
	int fileDescriptor;  /*Descritor correspondente à porta série*/
	int status; 		 /*TRANSMITTER | RECEIVER*/
	char file_path[128]; /*TRANSMITTER - path of the file to send
						   RECEIVER - path where the file will be saved*/
};

static struct applicationLayer infoLayer;
static unsigned int sequenceNumber = 1;

int getFileSize(int file_descriptor);

int sendControlPackage(char control, char * filename, char * filesize, char * date, char * permissions);

int sendDataPackage(char *data, int sequenceN, unsigned int size);

int receiveControlPackage(int type, char * name, int size);

int receiveDataPackage(int type, int size, char * data);

int sender();

int receiver();

int initApplicationLayer(char * port,int status, char * file_path);