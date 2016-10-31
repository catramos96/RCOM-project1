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
#include "linkLayerAux.h"

/*PODE SER ALTERADO*/
struct applicationLayer {
	int fileDescriptor;  /*Descritor correspondente à porta série*/
	int status; 		 /*TRANSMITTER | RECEIVER*/
	char file_path[128]; /*TRANSMITTER - path of the file to send
						   RECEIVER - path where the file will be saved*/
};

struct package{
	int type;
	//data
	char data[DATA_SIZE];
	char number;
	int size;
	//control
	char file_name[36];
	int total_size;
	int file_date;
	int file_perm;
};

static struct applicationLayer infoLayer;
static unsigned int sequenceNumber = 1;

int getFileSize(int file_descriptor);

int sendControlPackage(char control, char * filename, char * filesize, char * date, char * permissions);

int sendDataPackage(char *data, unsigned int size);

int receiveControlPackage(struct package *p,char * data);

int receiveDataPackage(struct package *p, char * data);

int receivePackage(struct package *p);

int sender();

int receiver();

int initApplicationLayer(char * port,int status, char * file_path);
