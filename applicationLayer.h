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

struct package{
	char type;
	//data information
	int number;
	int size;
	char data[DATA_SIZE];
	//control information
	char name[32];
	int date;
	int perm;
};

static unsigned int sequenceNumber = 0;

int sendControlPackage(char control, int fd,char * filename, char * filesize, char * date, char * permissions);

int sendDataPackage(int fd,char *data, int sequenceN, unsigned int size);

int analizePackage(char* data, struct package pkg);

int sender(char* port, char* filepath);

//int receiver(char* port);

int receiver(char* msg);




