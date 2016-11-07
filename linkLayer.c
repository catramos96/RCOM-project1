#include "linkLayer.h"

struct termios oldtio, newtio;
int retry = 0;

/**
 * Inicializacao da struct data_link
 */
void init_linkLayer(unsigned char* port, unsigned int mode)
{
    strcpy(data_link.port, port);
    data_link.baudRate = BAUDRATE;
    data_link.sequenceNumber = 0; //N(S) = 0
    data_link.timeout = TIMEOUT;
    data_link.numTransmissions = RETRANSMITIONS;
    data_link.mode = mode;
}

/**
 * Handler para o alarme.
 */
void handler()
{
    if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
        printf("TIMEOUT\n");
    statistics.timeouts++;
    retry = 1;
}

/**
 * Inicializa a struct newtio, e abre o file descriptor correspondente a porta.
 * @param port
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

	//invoca o método adequado
    if(isReceiver)	
		return llopen_receiver(fd);
	
    return llopen_sender(fd);
    
}

/**
 * Metodo invocado pelo llopen para a maquina recetora.
 * Recebe tramas do tipo SET e envia do tipo UA.
 */
int llopen_receiver(int fd)
{
    int res;
    if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
        printf("RECEIVER\n");

    //Verificar e receber a trama SET 
    Message* msg = (Message*)malloc(sizeof(Message));

    ReturnType ret;
    do{
        ret = receive(fd,msg);
        
        if(ret == ERROR)
            return -1;
        else if(ret == OK)
        {
            //verifica que e uma trama do tipo SET 
            if(msg->type == SET)
            {
                if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                    printf("Trama SET recebida!\n");
                break;
            }
        }
    }while(1);
        
    free(msg);  //liberta a mensagem

    //criacao da trama UA
    unsigned char *ua = build_frame_SU(UA,FRAME_A3);

    if(data_link.mode == FULL_DEBUG)
    {
        printf("llopen_receiver UA antes do stuffing\n");
        display(ua, FRAME_SIZE);
    }

	//faz stuff da trama 
    int newsize = stuff(ua, FRAME_SIZE);

    if(data_link.mode == FULL_DEBUG)
    {
        printf("llopen_receiver UA depois do stuffing\n");
        display(ua, newsize);
    }

    //envio da trama UA
    if((res = write(fd,ua,newsize)) == -1)
    {
        perror("write sender");
        return -1;
    }
    if(data_link.mode == SIMPLE_DEBUG  || data_link.mode == FULL_DEBUG)
        printf("trama UA enviada! \n");

    return fd;
}

/** 
 * Metodo invocado pelo metodo llopen pela maquina emissora.
 * Cria e envia tramas do tipo SET e recebe do tipo UA.
 * Se recebida com sucesso, a comunicacao e estabelecida.
 */
int llopen_sender(int fd)
{
    signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res;
    int done = 0;
    if(data_link.mode == SIMPLE_DEBUG  || data_link.mode == FULL_DEBUG)
        printf("SENDER\n");

    //criacao da trama SET
    unsigned char *set = build_frame_SU(SET,FRAME_A3);
	
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llopen_sender SET antes do sfuffing\n");
        display(set, FRAME_SIZE);
    }
	
    //stuffing
    int newsize = stuff(set, FRAME_SIZE);
	
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llopen_sender SET depois do sfuffing\n");
        display(set, newsize);
    }
	
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
            
            if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
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
                //verifica o tipo da mensagem (UA com campo de endereco 3)
                if(msg->type == UA)
                {
                    if(msg->controlAdress == FRAME_A3)
                    {
                        done = 1;	//termina o ciclo
                        free(set); // liberta a memoria da trama criada
                        
                        if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                            printf("trama UA recebida!\n");
                    }
                    else	//erro no cabecalho
                    {
                        retry = 1;	//para o alarme
                        
                        if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                            printf("Erro no cabecalho da trama UA\n");
                    }
                }
            }
        }
    }
    
    alarm(0);
    free(msg);
	
    return fd;
}


/**
 * Usado pela maquina emissora. Recebe um pacote e cria uma trama do tipo I. De seguida envia-a e espera pela resposta RR(sucesso) ou REJ(erro no bcc2).
 * Ciclo protegido por timeouts e numero maximo de retransmisssoes.
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
    int size = length + FRAME_SIZE + 1;	//frame_size e o tamanho de uma trama normal (5) + 1 para o campo de controlo duplo da trama I
    
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llwrite I antes do sfuffing\n");
        display(frame_i, size);
    }
    
	//stuff
    int newsize = stuff(frame_i,size);
    
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llwrite I depois do sfuffing\n");
        display(frame_i, newsize);
    }

    Message* msg = (Message*)malloc(sizeof(Message));
        
    //envio da trama I (write)
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
            if((n_written = write(fd,frame_i,newsize)) == -1)    //Envio da Trama I
            {
                perror("Erro de escrita em write()");
                return -1;
            }
            //ativa o alarme
            alarm(data_link.timeout);  
            retry = 0;	//liga o alarme
            tries++;	//mais uma tentativa
            
            if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                printf("Trama I enviada !\n");
            
            statistics.tramasIenviadas++;
        }
        
        //Verificar e recebe a trama RR OU REJ
        ReturnType ret = receive(fd,msg);
        
        if(ret == ERROR)   
            return -1;
        else if(ret != EMPTY)	//significa que esta a ser lido qualquer coisa
        {
            //verifica o tipo da mensagem
            if(msg->type == RR)	//sem erros
            {
                done = 1;	//termina o ciclo
                free(frame_i);
                
                if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                    printf("Trama RR recebida!\n");
            }
            else if(msg->type == REJ)	//erro no bcc2
            {
                retry = 1;	//para o alarme
                
                if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                    printf("Trama REJ recebida\n");
                statistics.REJrecebidos++;
            }
        }
    }
    
    alarm(0);
    free(msg);
	
    return n_written;
}


/**
 * Metodo invocado pela maquina recetora. Recebe tramas do tipo I e cria/envia tramas do tipo RR ou REJ.
 * Ciclo protegido por um temporizador de tempo maximo disponivel para realizar o metodo.
 * @param fd identificador da ligação de dados
 * @param buffer array de caracteres recebidos 
 * @return comprimento do array (número de caracteres lidos) ; valor negativo em caso de erro
 */
int llread(int fd, unsigned char * buffer)
{
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao

    int res;
    unsigned char *frame = NULL;
    int done = 0;
    int send = 0;
    
    Message* msg;

    alarm(data_link.timeout);
    retry = 0;
    
    while(!done)
    {
		//relacionado com o temporizador global
    	if(retry)
            return -1;
    	
		//rececao da trama I (read)
    	msg = (Message*)malloc(sizeof(Message));
        ReturnType ret = receive(fd,msg);

        if(ret == ERROR)
            return -1;
        else if(ret == DATAERROR)	//erro no bcc2
        {
            frame = build_frame_SU(REJ,FRAME_A3);   // constroi a trama REJ se erros nos dados
            send = 1;	//envia a trama
            statistics.REJenviados++;
            
            if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                printf("Data error receiving frame I (bcc2) !\n");
        }
        else if(ret == OK)	//esta a receber alguma coisa
        { 
            if(msg->type == I)	//trama do tipo I
            {
                if(!msg->isRetransmission)  //se nao for uma retransmissao, copia a mensagem para o buffer 
                {
                    memcpy(buffer, &msg->message,msg->message_size);//destination, source, num B
                    done = 1;   //termina o ciclo
                }
                
                frame = build_frame_SU(RR,FRAME_A3);  //criacao da trama RR
                send = 1;	//envia a trama
                
                if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                    printf("Trama I recebida!\n");  //caso de sucesso
                    
                statistics.tramasIrecebidas++;
            }
        }

        if(send)	//permissao para enviar a trama
        {
            if(data_link.mode == FULL_DEBUG)
            {
                printf("llread REJ antes do sfuffing\n");
                display(frame,FRAME_SIZE);
            }
        
			//stuff
            int newsize = stuff(frame,FRAME_SIZE);
          
            if(data_link.mode == FULL_DEBUG)
            {
                printf("llread RR/REJ depois do sfuffing\n");
                display(frame,newsize);
            }
        	
            //envia a trama RR/REJ
            if((res = write(fd,frame,newsize)) == -1)   
            {
                perror("write llread");
                return -1;
            }
            send = 0;	//termina a permissao para enviar tramas
            
            if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                printf("Trama RR/REJ enviada!\n");
        }
    
    }
    
    free(msg);
    alarm(0);
	
    return 0;
}

/**
 * Metodo que repoe os valores da estrutura.
 * @param fd File descriptor
 * @param isReceiver Flag que indica qual e a maquina (emissora ou recetora)
 * @return -1 se erro ou 0 se sucesso.
 */
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
            return -1;
    }
    
    sleep(1);	//assegurar que nao ha problemas
    
	//repor definicoes
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

	//fechar a porta
    close(fd);
    return ret; //retorna 0 se correr sem problemas
}

/**
 * Metodo invocado pelo llclose para a maquina emissora. 
 * Envia tramas do tipo DISC, recebe outra do tipo DISC e de seguida recebe do tipo UA.
 * @param fd 
 * @return 0 se sucesso ou -1 se temerros.
 */
int llclose_sender(int fd)
{
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res, tries = 0, done = 0;
    
    Message* msg = (Message*)malloc(sizeof(Message));
	
	//construcao da trama DISC
    unsigned char* disc = build_frame_SU(DISC,FRAME_A3);
	
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llclose_sender DISC antes do sfuffing\n");
        display(disc, FRAME_SIZE);
    }
	
	//stuff
    int newsize = stuff(disc, FRAME_SIZE);
	
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llclose_sender DISC depois do sfuffing\n");
        display(disc, newsize);
    }
		
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
            
            if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                printf("trama DISC enviada!\n");
        }
        
        //Verificar e receber a trama DISC
        ReturnType ret = receive(fd,msg);
        
        if(ret == ERROR)    
            return -1;
        else if(ret != EMPTY)	//esta a receber alguma coisa
        {
            if(msg->type == DISC)	//trama do tipo disc com o campo de endereco a 1
            {
                if(msg->controlAdress == FRAME_A1)
                {
                    done = 1;	//termina o ciclo
                    free(disc); // liberta a memoria
                    
                    if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                        printf("Trama DISC recebida!\n");
                }
                else	//erro de cabecalho
                {
                    if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                        printf("Erro no cabecalho da trama DISC\n");
                }
            }
        }
    }
    
    free(msg);
        
	//controi trama do tipo UA
    unsigned char* ua = build_frame_SU(UA,FRAME_A1);
	
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llclose_sender UA antes do sfuffing\n");
        display(ua, FRAME_SIZE);
    }
	
	//stuff
    int newsizeUA = stuff(ua, FRAME_SIZE);
	
    if(data_link.mode == FULL_DEBUG)
    {
        printf("llclose_sender UA depois do sfuffing\n");
        display(ua, newsizeUA);
    }
	
    //envia trama do tipo UA
    if((res = write(fd,ua, newsizeUA)) == -1)
    {
        perror("Erro na escrita write()");
        return -1;
    }
    if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
        printf("trama UA enviada!\n");
    
    alarm(0);
    return 0;
}

/**
 * Metodo invocado pelo llclose para a maquina recetora. 
 * Recebe uma trama do tipo DISC e confirma com o envio de outra do tipo DISC. No fim recebe uma trama do tipo UA.
 * @param fd 
 * @return 0 se sucesso ou -1 se tem Serros.
 */
int llclose_receiver(int fd)
{
    (void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
    int res, tries = 0, done = 0;
    ReturnType ret;
    
    Message* msg = (Message*)malloc(sizeof(Message));
	
	//constroi trama do tipo DISC
    unsigned char* disc = build_frame_SU(DISC,FRAME_A1);

    if(data_link.mode == FULL_DEBUG)
    {
        printf("llclose_receiver DISC antes do sfuffing\n");
        display(disc, FRAME_SIZE);
    }
    
	//stuff
    int newsize = stuff(disc,FRAME_SIZE);

    if(data_link.mode == FULL_DEBUG)
    {
        printf("llclose_receiver DISC depois do sfuffing\n");
        display(disc, newsize);
    }

    while(!done)
    {
        if(tries >= data_link.numTransmissions)
        {
            printf("Numero de tentativas excedida!\n");
            return -1;
        }
        if(tries == 0)	//na primeira tentativa deve receber uma trama do tipo DISC
        {
            ret = receive(fd,msg);
            if(ret == ERROR)    
                return -1;
            else if(ret == OK)	//se recebe alguma coisa
            {
                if(msg->type == DISC)	//e do tipo DISC com control adress 3
                {
                    if(msg->controlAdress == FRAME_A3)
                    {
                        tries++;	//passa para a proxima 'tentativa'
                        if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                            printf("Trama DISC recebida!\n");  
                    }
                    else	//cabecalho errado
                    {
                        if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                            printf("Erro no cabecalho da trama DISC\n");
                    }
                }
            }
        }
        
        if(tries == 1 || retry) //só envia a trama se : for a segunda tentativa (ja recebeu um disc antes) ou o alarme disparar
        {
			//escreve o DISC construido
            if((res = write(fd,disc,newsize)) == -1)
            {
                perror("Erro na escrita write()");
                return -1;
            }
            //ativa o alarme
            alarm(data_link.timeout);  
            retry = 0;
            
            if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                printf("trama DISC enviada!\n");
            
			//espera para receber a trama UA
            ret = receive(fd,msg);
            
            if(ret == OK)	//recebeu alguma coisa	
            {
                if(msg->type == UA)	//e do tipo UA com control adress 1
                {
                    if(msg->controlAdress == FRAME_A1)
                    {
                        free(disc);
                        done = 1;	//termina o ciclo
                        
                        if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
                            printf("trama UA recebida\n");
                    }
                    else	//erro no cabecalho
                    {
                        if(data_link.mode == SIMPLE_DEBUG || data_link.mode == FULL_DEBUG)
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
