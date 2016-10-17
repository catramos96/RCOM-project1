#include "linkLayerAux.c"

int alarmOff = 1;

/**
 * Inicializacao do layerLink
 */
void init_linkLayer(char* port){

    strcpy(data_link.port, port);
    data_link.baudRate = BAUDRATE;
    data_link.sequenceNumber = 0;
    data_link.timeout = 1;
    data_link.numTransmissions = 3;
}

void handler(){
    alarmOff = 1;
}

/*
void initAlarm(){
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    alarm(1);  
    alarmOff = 0;
}*/

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
   if((res = receive(fd,"SET")) == -1){
	perror("receive frame");
	exit(-1);
   }

   //criacao da trama UA
   char *ua = build_frame_SU("UA");

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
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    struct termios oldtio,newtio;
    int res;
    int done = 0;
    printf("SENDER\n");

    //criacao da trama SET
    char *set = build_frame_SU("SET");
    
    int tries = 0;
    while(!done)
    {
        
        if(tries == 0 || alarmOff) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
        {
            if(tries >= data_link.numTransmissions)
            {
                printf("Numero de tentativas excedida!\n");
                exit(-1);
            }
            
            //envia a trama
            if((res = write(fd,set, FRAME_SIZE)) == -1)
            {
                perror("write sender");
                exit(-1);
            }else
            {
                //ativa o alarme
                alarm(1);  
                alarmOff = 0;
                tries++;
                printf("trama enviada!\n");
            }

        }
        
        //Verificar e receber a trama UA ---> isto é para mudar (Eu devo receber a trama e no processamento verifica-la)
        if((res = receive(fd,"UA")) == -1)
        {
            return 1;
        }
        else
        {
            done = 1;
            free(set); // liberta a memoria
            printf("Comunicacao estabelecida!\n");
        }
        
    }

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
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res, tries = 0, done = 0;
    char* ua = build_frame_SU("UA");
    char* disc = build_frame_SU("DISC");

    while(!done)
    {
        
        if(tries == 0 || alarmOff) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
        {
            if(tries >= data_link.numTransmissions)
            {
                printf("Numero de tentativas excedida!\n");
                exit(-1);
            }
            
            //envia a trama
            if((res = write(fd,disc, FRAME_SIZE)) == -1){
                perror("write sender");
                exit(-1);
            }else
            {
                //ativa o alarme
                alarm(1);  
                alarmOff = 0;
                tries++;
                printf("trama disc enviada!\n");
            }

        }
        
        //Verificar e receber a trama NAO SEI
        if((res = receive(fd,"DISC")) == -1)
        {
            return 1;
        }
        else
        {
            done = 1;
            free(disc); // liberta a memoria
            printf("Comunicacao estabelecida!\n");
            
            //ENVIA UMA TRAMA DO TIPO UA
            if((res = write(fd,ua, FRAME_SIZE)) == -1){
                perror("write sender");
                exit(-1);
            }else{
                printf("trama ua enviada!\n");
            }
        }
        
    }
    
  /*  
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
	  */
    return 0;
}
