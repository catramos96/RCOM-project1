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

static unsigned int sequenceNumber;

applicationPackage CreateStartPackage(char * filename, char * filesize, char * date, char * permissions);

applicationPackage CreateDataPackage(char * data, unsigned int size);

applicationPackage CreateEndPackage();


