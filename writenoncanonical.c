/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B9600 //38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define BUF_SIZE 255
#define TRAMA_SIZE 5

volatile int STOP=FALSE;

int flag=1, conta=1;

void atende()                   // atende alarme
{
	//printf("alarme # %d\n", conta);
	flag=1;
	conta=0;
}

int main(int argc, char** argv)
{
    int fd,c,res;
    struct termios oldtio,newtio;
    char buf[BUF_SIZE];

    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 30;   /* inter-character timer unused t=TIME*0.1 */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */


    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

//--------GUIÃO 2--------

    //constroi a trama SET
    char trama[TRAMA_SIZE];
    trama[0] = (char)0x7E;	//flag
    trama[1] = (char)0x03;	//A
    trama[2] = (char)0x03;	//C
    trama[3] = (char)0x00;	//BCC -> ainda não sei o que isto é
    trama[4] = (char)0x7E;	//FLAG

    //fazer stuffing
    
    //envia a trama
    if((write(fd,trama, TRAMA_SIZE)) == -1){
	perror("write sender");
	exit(-1);
    }else printf("trama enviada!\n");
/*
    //alarme
    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

    while(conta){
       if(flag){
          alarm(3);                 // activa alarme de 3s
          flag=0;
       }if((read(fd,&tmp,1)) > 0) flag = 0;
    }
    

  //rececao da trama UA
    char tmp;
    char ua[TRAMA_SIZE];
    while (STOP==FALSE) {   
      if((read(fd,&tmp,1)) == -1){
	perror("read sender");
	exit(-1);
       }

      if ((n != 0) && tmp == FLAG) STOP=TRUE;
	
      ua[n] = tmp;
      n++;

      printf("UA[%d] = %x\n",n,tmp);
    }
    printf("Trama recebida!\n");

    //destuffing

    //verificação da trama
 */   
  


//--------GUIÃO 1--------
/*
    //le do teclado
    gets(buf);
    buf[strlen(buf)] = '\0';
    
    //envia a informacao
    res = write(fd,buf, BUF_SIZE); 

    printf("%d bytes written\n", res);
	
    //rececao da mensagem
    char msg[BUF_SIZE];
    int n;
    char tmp;

    n = 0;
    while (STOP==FALSE) { 
      res = read(fd,&tmp,1);     
      msg[n] = tmp;
      n++;
      if (tmp =='\0') STOP=TRUE;
    }
   
    printf(":%s\n", msg);
*/
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
}
