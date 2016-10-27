#include "linkLayerAux.c"

int alarmOff = 1;

/**
* Inicializacao do layerLink
*/
void init_linkLayer(char* port){

	strcpy(data_link.port, port);
	data_link.baudRate = BAUDRATE;
	data_link.sequenceNumber = 0; //N(S) = 0
	data_link.timeout = 1;
	data_link.numTransmissions = 3;
	data_link.frame_size = FRAME_SIZE;  //numero de Bytes da trama que esta a ser enviada
}

void handler(){
	alarmOff = 1;
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
	//char buf[BUF_SIZE];

	//abre a porta para leitura e escrita
	fd = open(port, O_RDWR | O_NOCTTY );
	if (fd <0) {
		perror(port); 
		return 1;
	}

	// save current port settings
	if ( tcgetattr(fd,&oldtio) == -1) { 
		perror("tcgetattr");
		return 1;
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
		return 1;
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
	if((res = receive(fd,SET)) == ERROR){
		perror("receive frame");
		return 1;
	}else
		printf("Trama SET recebida!\n");

	//criacao da trama UA
	char *ua = build_frame_SU(UA);

	/*printf("llopen_receiver UA antes do stuffing\n");
	display(ua, FRAME_SIZE);*/

	int newsize = stuff(&ua, FRAME_SIZE);

	/*printf("llopen_receiver UA depois do stuffing\n");
	display(ua, newsize);*/

	//envio da trama UA
	if((res = write(fd,ua,newsize)) == -1){
		perror("write sender");
		return 1;
	}else{
		printf("trama UA enviada! \n");
	}

	return 0;
}

int llopen_sender(int fd)
{
	signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
	struct termios oldtio,newtio;
	int res;
	int done = 0;
	printf("SENDER\n");

	//criacao da trama SET
	char *set = build_frame_SU(SET);
	
	/*printf("llopen_sender SET antes do sfuffing\n");
	display(set, FRAME_SIZE);*/
	
	//stuffing
	int newsize = stuff(&set, FRAME_SIZE);
	
	/*printf("llopen_sender SET depois do sfuffing\n");
	display(set, newsize);*/
	
	int tries = 0;
	while(!done)
	{
		
		if(tries == 0 || alarmOff) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
		{
			if(tries >= data_link.numTransmissions)
			{
				printf("Numero de tentativas excedida!\n");
				return 1;
			}
			
			//envia a trama
			if((res = write(fd,set, newsize)) == -1)
			{
				printf("erro na escrita write()");
			}
			
			//ativa o alarme
			alarm(1);  
			alarmOff = 0;
			tries++;
			printf("trama SET enviada!\n");

		}
		
		//Verificar e receber a trama UA ---> isto é para mudar (Eu devo receber a trama e no processamento verifica-la)
		if((res = receive(fd,UA)) == ERROR)
		{
			return 1;
		}
		else
		{
			done = 1;
			free(set); // liberta a memoria
			printf("trama UA recebida!\n");
			printf("Comunicacao estabelecida!\n");
		}
		
	}

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
	
	(void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
	int tries = 0;
	int n_written = 0;
	int done = 0;
	int res;

	//criacao da trama I
	char *frame_i = build_frame_I(buffer,length);  
	int size = length+FRAME_SIZE+1;
	
   /* printf("llwrite I antes do sfuffing\n");
	display(frame_i, size); */
	
	int newsize = stuff(&frame_i,size);
	
	/*printf("llwrite I depois do sfuffing\n");
	display(frame_i, newsize);*/

	//envio da trama I(0) (write)
	//rececao da trama RR / REJ (read)
	while(!done)
	{
		if(tries == 0 || alarmOff) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
		{
			if(tries >= data_link.numTransmissions)
			{
				printf("Numero de tentativas excedida!\n");
				return 1;
			}
			if((n_written = write(fd,frame_i,newsize)) == -1)    //Envio da Trama I0
			{
				printf("erro de escrita em write()");
			}
			//ativa o alarme
			alarm(1);  
			alarmOff = 0;
			tries++;
			printf("Trama I enviada!\n");
		}
		
		//Verificar e receber a trama RR OU REJ com Nr = 1
		ReturnType ret = receive(fd,RR_REJ);
                
		if(ret == REJ)
		{
			printf("Temos de enviar outra vez continuar\n");
			alarmOff = 1;
		}
		else if(ret == RR){
			 done = 1;
			free(frame_i);
			printf("Trama RR recebida!\n");
		}
		else{
			printf("outros erros");
			return 1;
		}	
	}

    return n_written;
}

/*
* @param fd identificador da ligação de dados
* @param buffer array de caracteres recebidos 
* @return comprimento do array (número de caracteres lidos) ; valor negativo em caso de erro
*/
int llread(int fd, char * buffer){
	
	int res;
	char *frame = NULL;
	
	//rececao da trama I (read) com verificacao de erros e desstuffing
	ReturnType ret = receive(fd,I);
	if(ret == ERROR)
	{
		return 1;
	}
	else if(ret == DATAERROR)
	{
		frame = build_frame_SU(REJ);   
	}
	else
	{
		printf("Trama I recebida!\n");
		memcpy(buffer, &data_link.frame,data_link.frame_size);//destination, source, num B
		
		//criacao da trama RR/REJ (ainda só vamos criar a RR)
		frame = build_frame_SU(RR);
	}
		
	/*printf("llread RR antes do sfuffing\n");
	display(rr,FRAME_SIZE);*/
	
	int newsize = stuff(&frame,FRAME_SIZE);
  
        /* printf("llread RR depois do sfuffing\n");
	display(rr,newsize);*/
	
	//envia a trama RR/REJ
	if((res = write(fd,frame,newsize)) == -1)    //Envio da Trama I0
	{
		perror("write llread");
		return 1;
	}
	else
		printf("Trama RR/REJ enviada!\n");
	
	return 0;
}

int llclose(int fd, int isReceiver)
{
	struct termios newtio, oldtio;
	int ret=0;
	switch(isReceiver)
	{
		case 0: 
			ret=llclose_sender(fd);
			break;
		case 1:
			ret=llclose_receiver(fd);
			break;
		default:
			printf("Segundo argumento errado, apenas use 1 se for o receiver e 0 para o sender");
			return 1;
	}
	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1)
	{
		perror("tcsetattr");
		return 1;
	}

	close(fd);
	return ret;//retorna 0 se correr sem problemas
}



int llclose_sender(int fd)
{
	(void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
	int res, tries = 0, done = 0;
	
	char* disc = build_frame_SU(DISC);
	
   /* printf("llclose_sender DISC antes do sfuffing\n");
	display(disc, FRAME_SIZE);*/
	
	int newsize = stuff(&disc, FRAME_SIZE);
	
	/*printf("llclose_sender DISC depois do sfuffing\n");
	display(disc, newsize);*/
		
	while(!done)
	{
		if(tries == 0 || alarmOff) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
		{
			if(tries >= data_link.numTransmissions)
			{
				printf("Numero de tentativas excedida!\n");
				return 1;
			}
			
			//envia a trama
			if((res = write(fd,disc, newsize)) == -1){
				printf("Erro na escrita write()");
			}
			//ativa o alarme
			alarm(1);  
			alarmOff = 0;
			tries++;
			printf("trama DISC enviada!\n");
		}
		
		//Verificar e receber a trama DISC
		if((res = receive(fd,DISC)) == ERROR)
		{
			return 1;
		}
		else
		{
			done = 1;
			free(disc); // liberta a memoria
			printf("Trama DISC recebida!\n");
		}
	}
	
	char* ua = build_frame_SU(UA);
	
	/*printf("llclose_sender UA antes do sfuffing\n");
	display(ua, FRAME_SIZE);*/
	
	int newsizeUA = stuff(&ua, FRAME_SIZE);
	
	/*printf("llclose_sender UA depois do sfuffing\n");
	display(ua, newsizeUA);*/
	
	//ENVIA UMA TRAMA DO TIPO UA
	if((res = write(fd,ua, newsizeUA)) == -1){
		printf("Erro na escrita write()");
		return 1;
	}
	else{
		printf("trama UA enviada!\n");
		return 0;
	}
}
	

int llclose_receiver(int fd)
{
	(void) signal(SIGALRM, handler);  // instala  rotina que atende interrupcao
	int res, tries = 0, done = 0;
	
	char* disc = build_frame_SU(DISC);
	
	/*printf("llclose_receiver DISC antes do sfuffing\n");
	display(disc, FRAME_SIZE);*/
	
	int newsize = stuff(&disc,FRAME_SIZE);
	
   /* printf("llclose_receiver DISC depois do sfuffing\n");
	display(disc, newsize);*/

	while(!done)
	{
		if(tries >= data_link.numTransmissions)
		{
			printf("Numero de tentativas excedida!\n");
			return 1;
		}
		
		if(tries==0)
		{
			//Verificar e receber a trama DISC
			if((res = receive(fd,DISC)) == ERROR)
			{
				return -1;
			}
			else
			{
				tries++;
				printf("Trama DISC recebida!\n");    
			}
		}
								
		if(tries == 1 || alarmOff) //só envia a trama se : for a primeira tentativa ou o alarme for desativado
		{
			if((res = write(fd,disc,newsize)) == -1)
			{
				printf("Erro na escrita write()");
				return -1;
			}
			//ativa o alarme
			alarm(1);  
			alarmOff = 0;
			printf("trama DISC enviada!\n");
													
			if((res = receive(fd,UA)) == OK){
				free(disc);
				done = 1;
				printf("trama UA recebida\n");
			}
		}
	}
	return 0;
}
