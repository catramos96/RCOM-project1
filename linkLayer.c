#include "linkLayer.h"

struct termios oldtio, newtio;
int retry = 0;

/**
* Inicializacao do layerLink
*/
void init_linkLayer(unsigned char* port)
{
    strcpy(data_link.port, port);
    data_link.baudRate = BAUDRATE;
    data_link.sequenceNumber = 0; //N(S) = 0
    data_link.timeout = TIMEOUT;
    data_link.numTransmissions = RETRANSMITIONS;
}

void handler()
{
    printf("TIMEOUT\n");
    retry = 1;
}

/*
* @param porta 
* @param flag TRANSMITTER / RECEIVER
* @return identificador da ligação de dados ; valor negativo em caso de erro
*/
int llopen(unsigned char* port, int isReceiver)
{
    int fd,c,res;
   
    //abre a porta para leitura e escrita
    fd = open(port, O_RDWR | O_NOCTTY );
    if (fd < 0) 
    {
        perror(port); 
        return -1;
    }

    // save current port settings
    if ( tcgetattr(fd,&oldtio) == -1) 
    { 
        perror("tcgetattr");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    // set input mode (non-canonical, no echo,...) 
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 5;   // inter-character timer unused : t=VTIME*0.1 
    newtio.c_cc[VMIN]     = 0;    // blocking read until 0 chars received
    
    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) 
    {
        perror("tcsetattr");
        return -1;
    }

    if(isReceiver)
    {
        if(llopen_receiver(fd) == -1)   return -1;
    }
    else
    {
        if(llopen_sender(fd) == -1) return -1;
    }
    
    return fd;
}

int llopen_receiver(int fd)
{
    int res;
    printf("RECEIVER\n");

    //Verificar e receber a trama SET 
    Message* msg = (Message*)malloc(sizeof(Message));

    ReturnType ret;
    do{
        ret = receive(fd,msg);
        
        if(ret == ERROR)
        {
            printf("Erro de leitura\n");
            return -1;
        }
        else if(ret == OK)
        {
            //verifica que e uma trama do tipo SET 
            if(msg->type == SET)
            {
                printf("Trama SET recebida!\n");
                break;
            }
        }
    }while(1);
        
    free(msg);  //liberta a mensagem

    //criacao da trama UA
    unsigned char *ua = build_frame_SU(UA,FRAME_A3);

    /*printf("llopen_receiver UA antes do stuffing\n");
    display(ua, FRAME_SIZE);*/

    int newsize = stuff(ua, FRAME_SIZE);

    /*printf("llopen_receiver UA depois do stuffing\n");
    display(ua, newsize);*/

    //envio da trama UA
    if((res = write(fd,ua,newsize)) == -1)
    {
        perror("write sender");
        return -1;
    }
    printf("trama UA enviada! \n");

    return 0;
}

int llopen_sender(int fd)
{
    signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res;
    int done = 0;
    printf("SENDER\n");

    //criacao da trama SET
    unsigned char *set = build_frame_SU(SET,FRAME_A3);
	
    /*printf("llopen_sender SET antes do sfuffing\n");
    display(set, FRAME_SIZE);*/
	
    //stuffing
    int newsize = stuff(set, FRAME_SIZE);
	
    /*printf("llopen_sender SET depois do sfuffing\n");
    display(set, newsize);*/
	
    Message* msg = (Message*)malloc(sizeof(Message));
    
    int tries = 0;
    while(!done)
    {
        if(tries == 0 || retry) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
        {
            if(tries >= data_link.numTransmissions)
            {
                printf("Numero de tentativas excedida!\n");
                return -1;
            }
            
            //envia a trama
            if((res = write(fd,set, newsize)) == -1)
            {
                perror("erro na escrita write()");
                return -1;
            }

            //ativa o alarme
            alarm(data_link.timeout);  
            retry = 0;
            tries++;
            printf("trama SET enviada!\n");
        }
        
        //Verificar e receber a trama UA
        ReturnType ret = receive(fd,msg);
        
        if(ret != EMPTY)
        {
            if(ret == ERROR)  
                return -1;
            else
            {
                //verifica o tipo da mensagem
                if(msg->type == UA)
                {
                    if(msg->controlAdress == 3)
                    {
                        done = 1;
                        free(set); // liberta a memoria da trama criada
                        printf("trama UA recebida!\n");
                    }
                    else
                    {
                        retry = 1;
                        printf("Erro no cabecalho da trama UA\n");
                    }
                }
            }
        }
    }
    
    alarm(0);
    free(msg);
    return 0;
}


/*
* Usado pelo TRANSMITTER
* @param fd identificador da ligação de dados
* @param buffer array de caracteres a transmitir 
* @param length  comprimento do array de caracteres 
* @return número de caracteres escritos ; valor negativo em caso de erro
*/
int llwrite(int fd, unsigned char * buffer, int length)
{
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int tries = 0;
    int n_written = 0;
    int done = 0;
    int res;

    //criacao da trama I
    unsigned char *frame_i = build_frame_I(buffer,length);  
    int size = length+FRAME_SIZE+1;
    
    /* printf("llwrite I antes do sfuffing\n");
    display(frame_i, size); */
    
    int newsize = stuff(frame_i,size);
    
    /*printf("llwrite I depois do sfuffing\n");
    display(frame_i, newsize);*/

    Message* msg = (Message*)malloc(sizeof(Message));
        
    //envio da trama I(0) (write)
    //rececao da trama RR / REJ (read)
    while(!done)
    {
        if(tries == 0 || retry) //só envia a trama se : for a primeira tentativa ou o alarme disparar
        {
            if(tries >= data_link.numTransmissions)
            {
                printf("Numero de tentativas excedida!\n");
                return -1;
            }
            if((n_written = write(fd,frame_i,newsize)) == -1)    //Envio da Trama I0
            {
                perror("Erro de escrita em write()");
                return -1;
            }
            //ativa o alarme
            alarm(data_link.timeout);  
            retry = 0;
            tries++;
            
            printf("Trama I enviada !\n");
        }
        
        //Verificar e receber a trama RR OU REJ com Nr = 1
        ReturnType ret = receive(fd,msg);
        
        if(ret == ERROR)   
            return -1;
        else if(ret != EMPTY)
        {
            //verifica o tipo da mensagem
            if(msg->type == RR)
            {
                done = 1;
                free(frame_i);
                
                printf("Trama RR recebida!\n");
            }
            else if(msg->type == REJ)
            {
                retry = 1;
                
                printf("Trama REJ recebida\n");
            }
        }
    }
    
    alarm(0);
    free(msg);
    return n_written;
}


/*
* @param fd identificador da ligação de dados
* @param buffer array de caracteres recebidos 
* @return comprimento do array (número de caracteres lidos) ; valor negativo em caso de erro
*/
int llread(int fd, unsigned char * buffer)
{
    int res;
    unsigned char *frame = NULL;
    int done = 0;
    int send = 0;
    
    //rececao da trama I (read) com verificacao de erros e desstuffing
    Message* msg = (Message*)malloc(sizeof(Message));
    
    while(!done)
    {
        ReturnType ret = receive(fd,msg);

        if(ret == ERROR)
            return -1;
        else if(ret == DATAERROR)
        {
            frame = build_frame_SU(REJ,FRAME_A3);   // constroi a trama REJ se erros nos dados
            send = 1;
            
            printf("Data error receiving frame I (bcc2) !\n");
        }
        else if(ret == OK)
        { 
            if(msg->type == I)
            {
                if(!msg->isRetransmission)  //se nao for uma retransmissao, copia a mensagem para o buffer 
                {
                    memcpy(buffer, &msg->message,msg->message_size);//destination, source, num B
                    done = 1;   //duvida => nao devia estar do lado de fora?
                }
                
                frame = build_frame_SU(RR,FRAME_A3);  //criacao da trama RR
                send = 1;
                
                printf("Trama I recebida!\n");  //caso de sucesso
            }
        }
	
        /*printf("llread RR antes do sfuffing\n");
        display(rr,FRAME_SIZE);*/

        if(send)
        {
            int newsize = stuff(frame,FRAME_SIZE);
          
            /* printf("llread RR depois do sfuffing\n");
            display(rr,newsize);*/
        	
            //envia a trama RR/REJ
            if((res = write(fd,frame,newsize)) == -1)    //Envio da Trama I0
            {
                perror("write llread");
                return -1;
            }
            send = 0;
            
            printf("Trama RR/REJ enviada!\n");
        }
    
    }
    
    free(msg);
    alarm(0);
    return 0;
}

int llclose(int fd, int isReceiver)
{
    int ret=0;

    switch(isReceiver)
    {
        case 0: 
            ret = llclose_sender(fd);
            break;
        case 1:
            ret = llclose_receiver(fd);
            break;
        default:
            //printf("Segundo argumento errado, apenas use 1 se for o receiver e 0 para o sender");
            return -1;
    }
    
    sleep(1);
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

    close(fd);
    return ret; //retorna 0 se correr sem problemas
}

int llclose_sender(int fd)
{
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res, tries = 0, done = 0;
    
    Message* msg = (Message*)malloc(sizeof(Message));
	
    unsigned char* disc = build_frame_SU(DISC,FRAME_A3);
	
   /* printf("llclose_sender DISC antes do sfuffing\n");
    display(disc, FRAME_SIZE);*/
	
    int newsize = stuff(disc, FRAME_SIZE);
	
    /*printf("llclose_sender DISC depois do sfuffing\n");
    display(disc, newsize);*/
		
    while(!done)
    {
        if(tries == 0 || retry) //só envia a trama se : for a primeira tentativa ou o alarme disparar
        {
            if(tries >= data_link.numTransmissions)
            {
                printf("Numero de tentativas excedida!\n");
                return -1;
            }
            //envia a trama
            if((res = write(fd,disc, newsize)) == -1)
            {
                perror("Erro na escrita write()");
                return -1;
            }
            //ativa o alarme
            
            alarm(data_link.timeout);  
            retry = 0;
            tries++;
            
            printf("trama DISC enviada!\n");
        }
        
        //Verificar e receber a trama DISC
        ReturnType ret = receive(fd,msg);
        
        if(ret == ERROR)    
            return -1;
	else if(ret != EMPTY)
        {
            if(msg->type == DISC)
            {
                if(msg->controlAdress == 1)
                {
                    done = 1;
                    free(disc); // liberta a memoria
                    
                    printf("Trama DISC recebida!\n");
                }
                else
                {
                    printf("Erro no cabecalho da trama DISC\n");
                }
            }
        }
    }
    
    free(msg);
        
    unsigned char* ua = build_frame_SU(UA,FRAME_A1);
	
    /*printf("llclose_sender UA antes do sfuffing\n");
    display(ua, FRAME_SIZE);*/
	
    int newsizeUA = stuff(ua, FRAME_SIZE);
	
    /*printf("llclose_sender UA depois do sfuffing\n");
    display(ua, newsizeUA);*/
	
    //ENVIA UMA TRAMA DO TIPO UA
    if((res = write(fd,ua, newsizeUA)) == -1)
    {
        perror("Erro na escrita write()");
        return -1;
    }
    printf("trama UA enviada!\n");
    
    alarm(0);
    return 0;
}

int llclose_receiver(int fd)
{
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res, tries = 0, done = 0;
    ReturnType ret;
    
    Message* msg = (Message*)malloc(sizeof(Message));
	
    unsigned char* disc = build_frame_SU(DISC,FRAME_A1);
	
    /*printf("llclose_receiver DISC antes do sfuffing\n");
    display(disc, FRAME_SIZE);*/
    
    int newsize = stuff(disc,FRAME_SIZE);
	
   /* printf("llclose_receiver DISC depois do sfuffing\n");
    display(disc, newsize);*/

    while(!done)
    {
        if(tries >= data_link.numTransmissions)
        {
            printf("Numero de tentativas excedida!\n");
            return -1;
        }
        if(tries == 0)
        {
            //Verificar e receber a trama DISC
            ret = receive(fd,msg);
            if(ret == ERROR)    
                return -1;
            else if(ret == OK)
            {
                if(msg->type == DISC)
                {
                    if(msg->controlAdress == 3)
                    {
                        tries++;
                        printf("Trama DISC recebida!\n");  
                    }
                    else
                    {
                        printf("Erro no cabecalho da trama DISC\n");
                    }
                }
            }
        }
        
        if(tries == 1 || retry) //só envia a trama se : for a primeira tentativa ou o alarme disparar
        {
            if((res = write(fd,disc,newsize)) == -1)
            {
                perror("Erro na escrita write()");
                return -1;
            }
            //ativa o alarme
            alarm(data_link.timeout);  
            retry = 0;
            printf("trama DISC enviada!\n");
            
            ret = receive(fd,msg);
            
            if(ret == OK)
            {
                if(msg->type == UA)
                {
                    if(msg->controlAdress == 1)
                    {
                        free(disc);
                        done = 1;
                        printf("trama UA recebida\n");
                    }
                    else
                    {
                        printf("Erro no cabecalho da trama UA\n");
                    }
                }
            }
        }
    }
    
    alarm(0);
    free(msg);
    return 0;
}
