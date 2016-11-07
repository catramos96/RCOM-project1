#include "linkLayer.h"

/**
 * Cria uma trama do tipo S ou U de acordo com o endereco de controlo.
 * @param flag Tipo de trama
 * @param flag_A endereco de controlo
 * @return trama criada
 */
unsigned char* build_frame_SU(ControlFieldType flag, unsigned char flag_A)      
{
    unsigned char *frame = NULL;
    frame = (unsigned char *) malloc(FRAME_SIZE);
 
    frame[0] = FLAG;    
    frame[1] = flag_A;
    frame[2] = getControlField(flag);
    frame[3] = (frame[1] ^ frame[2]);
    frame[4] = FLAG;
    
    return frame;
}

/**
 * Metodo que cria uma trama do tipo I
 * @param campo de dados.
 * @param tamanho do campo de dados.
 * @return FrameI : |F|A|C|BCC1|DATA|BCC2|F|
 */
unsigned char* build_frame_I(unsigned char* data, unsigned int data_length){

    unsigned char *frame = NULL;
    int total = FRAME_SIZE + data_length + 1 ; //frame size, with data length and an extra byte for BCC2
    frame = (unsigned char *) malloc(total);
    
    frame[0] = FLAG;    
    frame[1] = FRAME_A3;    //o camando I e sempre 0x03
    frame[2] = (data_link.sequenceNumber << 6);	//campo de endereco depende do numero da sequencia
    frame[3] = (frame[1] ^ frame[2]);	//campo de controlo 1
    
    //data
    int i = 0;
    unsigned char bcc2 = (unsigned char)0x0;
    for(i = 0; i < data_length; i++)
    {
        frame[4+i] = data[i];
        bcc2 ^= frame[4 + i];  
    }
    
    frame[4+data_length] = bcc2;	//campo de controlo 2
    frame[5+data_length] = FLAG;
    
    return frame;
}

/**
 * Recebe uma trama de qualquer tipo byte a byte interpretada por uma maquina de estados.
 * Tramas com cabecalho errado sao ignoradas.
 * Se nao estiver a ler nada retorna EMPTY
 * @param fd File descriptor para ler da porta
 * @param msg Struct que sera preenchida neste metodo com todas as informacoes necessarias pelos metodos que a invocam.
 * @return ReturnType
 */
ReturnType receive(int fd, Message *msg)
{
   unsigned char* buf = (unsigned char *) malloc(BUF_SIZE);
   
   State state = START;
   int size = 0;
   int done = 0;
   int hasData = 1; 	//flag que indica se é uma trama do tipo I
   unsigned char bcc = (unsigned char)0x00;
   int res = 0;

   while (!done) 
   {   
       unsigned char c;
       
       //le ate chegar ao estado de STOP
       if (state != STOP) 
       {
           res = read(fd,&c,1);	//le um byte
           if(res == -1)
           {
               perror("read receiver");
               return ERROR;
           }
           else if(res == 0){	//quando nao le nada, retorna empty
               return EMPTY;
           }
       }
       
	   //maquina de estados
       switch (state) {
           case START:  //So muda de estado quando aparecer uma flag
               if (c == FLAG) 
               {
                   buf[size] = c;
                   size++;
                   state = FLAG_RCV;
               }
               break;
           case FLAG_RCV:   //muda de estado se receber um campo de endereco, ou algo diferente da flag
               if(c == FRAME_A1 || c == FRAME_A3) 
               {
                   buf[size] = c;
                   size++;
                   state = A_RCV;
               }
               else if(c != FLAG)
               {
                    size = 1;
                    state = START;
               }
               break;
           case A_RCV:      //muda sempre de estado, se flag => start, se não guarda o controlField para comparar no futuro
               if(c == FLAG)
               {
                   size = 1;
                   state = FLAG_RCV;
               }
               else
               {
                   buf[size] = c;
                   size++;
                   state = C_RCV;
               }
               break;
           case C_RCV:  //primeiro campo de segurança
               if(c == FLAG)
               {
                   size = 1;
                   state = FLAG_RCV;
               }
               else
               {
                   bcc = buf[1] ^ buf[2]; //bcc2 calculado
                        
                   if(c == bcc)
                   {
                        buf[size] = c;
                        size++;
                        state = BCC_OK;
                   }
                   else
                   {
                        size = 0;
                        state = START;
                   }
               }
               break;
           case BCC_OK: // Se receber uma flag acaba, se não, recebemos todos os caracteres para serem depois analisados.
               if(c == FLAG)    
               {
				   if(size == FRAME_SIZE-1) //trama do tipo S ou UA
                   {
                       hasData = 0;
                   }
                   buf[size] = c;
				   size++;
                   state = STOP;    //ultimo byte
               }
               else //trama do tipo I
               {
                   buf[size] = c;
                   size++;
               }
               break;
           case STOP:	//fim da maquina de estados
               done = 1;
               break;
           default:
               break;
       }
       
   }

   buf = (unsigned char*)realloc(buf,size);  //realloc para o tamanho certo da trama

   if(data_link.mode == FULL_DEBUG)
   {
	   printf("Receive antedo destuffing\n");
	   display(buf,size);
   }
  
   //destuff
   int newsize = destuff(buf,size);   //recebe com stuff, fazemos destuffing
   
   if(data_link.mode == FULL_DEBUG)
   {
	   printf("Receive depois do destuffing\n");
       display(buf,newsize); 
   }
  
   //---- Comeca a parte de analise da trama ----//
   
   msg->controlAdress = buf[1];
   
   //trama do tipo I
   if(hasData)
   {  
       if(msg->controlAdress != FRAME_A3)    
		   return OK;  //erro de cabecalho antes de preencher o resto da mensagem
       
        msg->isRetransmission = 0;	//inicializa sem ser retransmissao
        
       //analisar o sequence number => se recebe diferente ao que está guardado e porque ocorreu uma retransmissao
       unsigned int ns = (buf[2]) >> 6;
       if(ns != data_link.sequenceNumber)
       {
           if(data_link.mode == SIMPLE_DEBUG)
                printf("WARNING: ocorreu uma retransmissao\n");
        
			msg->isRetransmission = 1;	// e uma retransmissao 
			
			data_link.sequenceNumber ^= 1;	//atualiza o sequenceNumber
			
        	//recebe o controlField (depois de atualizar o sequenceNumber)
			//como e uma retransmissao, para nao haver erros, temos de atualizar o sequenceNumber da struct para ser equivalente ao recebido
            msg->type = setControlField(buf[2]);

			statistics.tramasIretransmitidas++;
       }
       else
       {
            msg->type = setControlField(buf[2]);    //recebe o controlField (antes de mudar o sequenceNumber porque ainda sao iguais)
            
            data_link.sequenceNumber ^= 1;  //atualiza o sequenceNumber
        
            msg->message_size = newsize - FRAME_SIZE - 1; //-1 por causa da proteção dupla	(tamanho so dos dados)

            int i;
            unsigned char bcc2 = (unsigned char)0x0;
            for(i = 0; i < msg->message_size; i++)
                bcc2 ^= buf[4+i];
            
            if(bcc2 != buf[4+msg->message_size])	//os bcc's nao correspondem
                return DATAERROR;
            
            //coloca a mensagem recebida na struct
            memcpy(msg->message, &buf[4], msg->message_size); //destination, source, num 
       }
   }
   else //trama do tipo U ou S
   {
         msg->type = setControlField(buf[2]);	
         
         //caso tenha recebido RR ou REJ, é necessário mudar o sequenceNumber para o N(r)
         if(msg->type == RR || msg->type == REJ)
         {
            if(msg->controlAdress != FRAME_A3)  
				return OK; // erro de cabecalho
            
            data_link.sequenceNumber = buf[2] >> 7;      //atualizo no momento que recebo (sender)
         }
         
         //tipo SET so pode ter control Adress 3
         if(msg->type == SET && msg->controlAdress != FRAME_A3) 
			 return OK;  //erro de cabecalho
   }
   
   free(buf);   //liberta o espaco em memoria
 
   return OK;   
}

/**
 * Metodo que retorna a flag correspondente ao tipo pedido.
 */
unsigned char getControlField(ControlFieldType flag)
{    
    if(SET == flag){
        return FRAME_C_SET;
    }
    else if(UA == flag){
        return FRAME_C_UA;
    }
    else if(DISC == flag){
        return FRAME_C_DISC;
    }
    else if(RR == flag){ 
        return (FRAME_C_RR | (data_link.sequenceNumber << 7));
    }
    else if(REJ == flag)    //contrucao das tramas rej com a o num sequencia recebido pela trama I. O num sequencia passa para ao oposto para a rececao da nova trama I
    {
        unsigned char c = FRAME_C_REJ | (data_link.sequenceNumber << 7);
        data_link.sequenceNumber ^= 1;
        return c;
    }
    else if(I == flag){
        return (data_link.sequenceNumber << 6); //o controlo depende do sequenceNumber
    }
    else
        if(data_link.mode == SIMPLE_DEBUG)
            printf("Erro no cabecalho a receber a trama\n");
    
}

/**
 * Metodo que retorna o tipo de acordo com a flag recebida.
 */
ControlFieldType setControlField(unsigned char c){
  
    if(FRAME_C_SET == c){
        return SET;
    }
    else if(FRAME_C_UA == c){
        return UA;
    }
    else if(FRAME_C_DISC == c){
        return DISC;
    }
    else if(FRAME_C_RR == c || ((unsigned char)0x85) == c){
        return RR;
    }
    else if(FRAME_C_REJ == c || ((unsigned char)0x81) == c){
        return REJ;
    }
    else if((data_link.sequenceNumber << 6) == c ){
        return I;
    }
    else
        if(data_link.mode == SIMPLE_DEBUG)
            printf("Erro no cabecalho a receber a trama\n");
}

/**
 * 
 */
int stuff(unsigned char *frame, int frame_length){
   
    int newframe_length = 2; //as flags de incio e fim
    
    //por cada flag ou escape vou adicionar +1 byte
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        unsigned char c = frame[i];
        if ((c == FLAG) || (c == ESCAPE))
        {
            newframe_length++;
        }
        newframe_length++;
    }
    
    //reservar espaço na memoria    
    frame = (unsigned char*)realloc(frame,newframe_length);
    
    //percorre a trama 
    for(i = 1; i < (frame_length-1); i++)
    {
        unsigned char c = frame[i];
        if(c == FLAG || c == ESCAPE)
        {
            memmove(frame + i + 1, frame + i, frame_length - i);//destino, source, num de bytes -> avançar 1 casa todos os bytes para a frente
            frame[i] = ESCAPE;
            frame[i+1] ^= AUX;
            
            frame_length++;
        }
    }
    
    return newframe_length;
}

/**
 * Processo contrario ao stuff.
 */
int destuff(unsigned char *frame, int frame_length){
    
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        unsigned char c = frame[i];
        if (c == ESCAPE)
        {
            memmove(frame + i, frame + i + 1, frame_length - i - 1);//destino, source, num de bytes -> avançar 1 casa todos os bytes para a tras
            frame[i] ^= AUX;
            
            frame_length--; //atualiza o tamanho
        }
    }
    
    //realocar o espaco em memoria (o tamanho foi modificado)
    frame = (unsigned char *)realloc(frame, frame_length);
    
    return frame_length;
}
    
    
    
/**
 * So para debugging.
 * Faz o display completo da trama.
 */
void display(unsigned char*frame, int n){
    
    printf(">>>>>>>>>>\n");
    
    int i;
    for(i = 0; i < n; i++){
        printf("frame[%d] = 0x%x\n",i,frame[i]);
    }
    
    printf(">>>>>>>>>>\n");
}
