#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <utime.h>
#include "linkLayer.h"

/*PODE SER ALTERADO*/
struct applicationLayer {
	int fileDescriptor;  /*Descritor correspondente à porta série*/
	int status; 		 /*TRANSMITTER | RECEIVER*/
	int mode;			 /*NORMAL | SIMPLE_DEBUG | FULL DEBUG*/
	unsigned char file_path[128]; /*TRANSMITTER - path of the file to send
						   RECEIVER - path where the file will be saved*/
};

struct package{
	int type;
	//data
	unsigned char *data;
	unsigned char number;
	int size;
	//control
	unsigned char file_name[36];
	int total_size;
	int file_date;
	int file_perm;
};

int getFileSize(int file_descriptor);

int sendControlPackage(unsigned char control, unsigned char * filename, unsigned char * filesize, unsigned char * date, unsigned char * permissions);

int sendDataPackage(unsigned char *data, unsigned int size);

int receiveControlPackage(struct package *p,unsigned char * data);

int receiveDataPackage(struct package *p, unsigned char * data);

int receivePackage(struct package *p);

int sender();

int receiver();

int initApplicationLayer(unsigned char * port,int status, int mode,int max_size,unsigned char * file_path);
