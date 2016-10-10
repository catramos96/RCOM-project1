/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B9600 //38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define BUF_SIZE 255
#define TRAMA_SIZE 5
#define FLAG 0X7E

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, i;
    struct termios oldtio,newtio;
    char buf[BUF_SIZE];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
      
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    printf("New termios structure set \n");

//--------Guiao 2--------
  
   //rececao da trama SET
   char set[TRAMA_SIZE]; 
   char tmp;
   int n;

   while (STOP==FALSE) {   
      if((read(fd,&tmp,1)) == -1){
	perror("read receiver");
	exit(-1);
      }

      if ((n != 0) && tmp == FLAG) STOP=TRUE;
	
      set[n] = tmp;
      n++;

      printf("SET[%d] = %X\n",n,tmp);
    }
    printf("Trama recebida!\n");

   //desstuffing

   //verificacao da trama SET (erros no cabeçalho)

/*  //criacao da trama UA
    char ua[TRAMA_SIZE];
    trama[0] = (char)0x7E;	//flag
    trama[1] = (char)0x03;	//A
    trama[2] = (char)0x05;	//C
    trama[3] = (char)0x00;	//BCC -> ainda não sei o que isto é
    trama[4] = (char)0x7E;	//FLAG

   //stuffing

   //envio da trama UA
   if((write(fd,ua,TRAMA_SIZE)) == -1){
	perror("write sender");
	exit(-1);
   }else printf("trama enviada! \n");
*/

//--------Guiao 1--------
/*
    //rececao da mensagem
    i=0;
    char tmp;
    while (STOP==FALSE) {       
      res = read(fd,&tmp,1);
      buf[i] = tmp;
      i++;
      if (tmp =='\0') STOP=TRUE;
    }
    
    printf(":%s\n", buf);
	
    //envio da mensagem
    res = write(fd,buf,i);  
    printf("%d bytes written\n", i);
*/
	
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
