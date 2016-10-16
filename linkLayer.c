#include "linkLayerAux.c"

int flag=1, conta=1;

/**
 * Inicializacao do layerLink
 */
void init_linkLayer(char* port){

    strcpy(data_link.port, port);
    data_link.baudRate = BAUDRATE;
    data_link.sequenceNumber = 0;
    data_link.timeout = 3;
    data_link.numTransmissions = 3;
}

void atende()                   // atende alarme
{
   //printf("alarme # %d\n", conta);
    flag=1;
    conta=0;
}

/*
 * @param porta 
 * @param flag TRANSMITTER / RECEIVER
 * @return identificador da ligação de dados ; valor negativo em caso de erro
 */
int llopen(char* port, int isReceiver)
{
    int fd,c,res;
    struct termios newtio, oldtio;
    char buf[BUF_SIZE];
 
    //abre a porta para leitura e escrita
    fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {
	perror(port); 
	exit(-1);
    }

    // save current port settings
    if ( tcgetattr(fd,&oldtio) == -1) { 
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    // set input mode (non-canonical, no echo,...) 
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 30;   // inter-character timer unused : t=TIME*0.1 
    newtio.c_cc[VMIN]     = 5;    // blocking read until 5 chars received

    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if(isReceiver) llopen_receiver(fd);
    else llopen_sender(fd);
    
    return fd;

}

int llopen_receiver(int fd)
{
   struct termios oldtio,newtio;
   int res;
   printf("RECEIVER\n");

   //Verificar e receber a trama SET 
   if((res = receive_verify_SU(fd,0)) == -1){
	perror("receive frame");
	exit(-1);
   }

   //criacao da trama UA
   int isUA = 1;
   char *ua = build_frame_SU(isUA);

   //envio da trama UA
   if((res = write(fd,ua,FRAME_SIZE)) == -1){
	perror("write sender");
	exit(-1);
   }else 
	printf("trama enviada! \n");
	
   //finalizacao
   tcsetattr(fd,TCSANOW,&oldtio);
   close(fd);
   
   return 0;
}

int llopen_sender(int fd)
{
    struct termios oldtio,newtio;
    int res;
    printf("SENDER\n");

    //criacao da trama SET
    int isUA = 0;
    char *set = build_frame_SU(isUA);
   
    //envia a trama
    if((res = write(fd,set, FRAME_SIZE)) == -1){
        perror("write sender");
        exit(-1);
    }else printf("trama enviada!\n");

    free(set); // liberta a memoria

   //Verificar e receber a trama UA (sem alarme)
   if((res = receive_verify_SU(fd,1)) == -1){
        perror("receive frame");
        exit(-1);
   }

/*    //alarme
    char tmp;	
    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

    while(conta){
       if(flag){
          alarm(3);                 // activa alarme de 3s
          flag=0;
       }if((read(fd,&tmp,1)) > 0){
	  printf("alarme desativado\n");
	  flag = 0;
	  conta--;
      }
    }
    
    //rececao da trama UA ---->>>> ISTO E PARA MUDAR
    char ua[FRAME_SIZE];
    int n = 0;
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

*/

    //terminacao
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
}


/*
 * Usado pelo TRANSMITTER
 * @param fd identificador da ligação de dados
 * @param buffer array de caracteres a transmitir 
 * @param length  comprimento do array de caracteres 
 * @return número de caracteres escritos ; valor negativo em caso de erro
 */
int llwrite(int fd, char * buffer, int length){
    
    //criacao da trama I
    char *frame_i = build_frame_I(buffer,length,0); //ainda e para mudar este 0
    
    //byte stuffing
    stuff(frame_i,strlen(frame_i));
    
    //envio da trama I (write)
    
    
    //rececao da trama RR / REJ (read)
}

/*
 * @param fd identificador da ligação de dados
 * @param buffer array de caracteres recebidos 
 * @return comprimento do array (número de caracteres lidos) ; valor negativo em caso de erro
 */
int llread(int fd, char * buffer){
    
    //rececao da trama I (read)
    
    //desstuffing
    
    //verificacao de erros
    
    //criacao da trama RR/REJ
    
    //envio da trama RR / REJ (write)
    
}

int llclose_sender(int fd)
{
	int res, tries = 0, disconnected = 0;
	char* ua = build_frame_SU(1);
	char* disc = build_frame_SU(2);
	
	while(!disconnected)
	{
		
			if (tries > 3)
			{
				//PARAR ALARME!!!!
				printf("Numero maximo de tentativas.\n");
				return 1;
			}
				tries++;
				
				if((res = write(fd,disc, FRAME_SIZE)) == -1){
					perror("write sender");
					exit(-1);
				}else printf("trama enviada!\n");

				if (tries == 1)
				{
					//COMECAR ALARME
				}
		
				if((res = receive_verify_SU(fd,2)) == -1){	
					perror("receive frame");
					exit(-1);
   
   
				if((res = write(fd,ua, FRAME_SIZE)) == -1){
					perror("write sender");
					exit(-1);
				}else 
				{
					printf("trama enviada!\n");
					//PARAR ALARME!!!!
					disconnected = 1;
				}
	  }
	return 0;
}