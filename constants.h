#define BAUDRATE B9600 //38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define BUF_SIZE 255
#define FRAME_SIZE 5
#define FLAG (char)0x7E		//flag de inicio e fim de tramas
#define FRAME_C_UA (char)0x05 	//Campo de controlo para trama UA
#define FRAME_C_SET (char)0x03 	//Campo de controlo para a trama SET
#define FRAME_A (char)0x03	//campo de endereco para tramas tipo U e S
