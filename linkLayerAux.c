#include "linkLayer.h"

//volatile int STOP=FALSE;

/**
 * Cria uma trama do tipo S ou U
 */
unsigned char* build_frame_SU(ControlFieldType flag)      
{
    unsigned char *frame = NULL;
    frame = (unsigned char *) malloc(FRAME_SIZE);
 
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    frame[2] = getControlField(flag);
    frame[3] = (frame[1] ^ frame[2]);
    frame[4] = FLAG;
    
    return frame;
}

/*
 * @brief Function that creates a frame I
 * @param data to transport
 * @param data_length
 * @param s NS of Frame I
 * @return FrameI : |F|A|C|BCC1|DATA|BCC2|F|
 */
unsigned char* build_frame_I(unsigned char* data, unsigned int data_length){

    unsigned char *frame = NULL;
    int total = FRAME_SIZE + data_length+1 ; //frame size, with data length and an extra byte for BCC2
    frame = (unsigned char *) malloc(total);
    
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    frame[2] = (data_link.sequenceNumber << 6);
    frame[3] = (frame[1] ^ frame[2]);
    
    //data
    int i = 0;
    unsigned char bcc2 = (unsigned char)0x0;
    for(i = 0; i < data_length; i++){
        frame[4+i] = data[i];
        bcc2 ^= frame[4 + i];  //BCC2
    }
    
    frame[4+data_length] = bcc2;
    frame[5+data_length] = FLAG;
    
    return frame;
}

/**
 * Recebe uma trama
 */
ReturnType receive(int fd, Message *msg){

   unsigned char* buf = (unsigned char *) malloc(BUF_SIZE);
   
   State state = START;
   int size = 0;
   int done = 0;
   int hasData = 1; //flag que indica se é uma trama do tipo I
   unsigned char bcc = (unsigned char)0x00;

   while (!done) 
   {   
       unsigned char c;
       
       //le ate chegar ao estado de STOP
       if (state != STOP) 
       {
           if((read(fd,&c,1)) == -1)
           {
               perror("read receiver");
               return ERROR;
           }
       }
       
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
               if(c == FRAME_A)       //nao esta bem!!!!!!!! temos de ter em conta se é 0x03 ou 0x01 (ainda so funciona para 0x03)
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
                   bcc = buf[1] ^ buf[2]; //fica com os 2 utimos bits
                        
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
           case STOP:
               done = 1;
               break;
           default:
               break;
       }
       
   }

   buf = (unsigned char*)realloc(buf,size);  //realloc para o tamanho certo da trama

   //printf("Receive antedo desstuffing\n");
   //display(buf,size);
  
   int newsize = desstuff(&buf,size);   //recebe com stuff, fazemos desstuffing
   
   //printf("Receive depois do desstuffing\n");
   //display(buf,newsize); 
  
  //---- Comeca a parte de analise da trama ----//
   

   //trama do tipo I
   if(hasData)
   {
        
       //analisar o sequence number => se recebe igual ao que está guardado e porque ocorreu uma retransmissao
       unsigned int ns = (buf[2]) >> 6;
       if(ns == data_link.sequenceNumber){
           printf("WARNING: ocorreu uma retransmissao\n");
       }
       
       data_link.sequenceNumber = ns;

       msg->type = setControlField(buf[2]);    //recebe o controlField antes de mudar o sequenceNumber
        
       msg->message_size = newsize - FRAME_SIZE - 1; //-1 por causa da proteção dupla	

       int i;
       unsigned char bcc2 = (unsigned char)0x0;
       for(i = 0; i < msg->message_size; i++){
           bcc2 ^= buf[4+i];
       }
       
       if(bcc2 != buf[4+msg->message_size]){
           return DATAERROR;
       }
        
        //colocar a mensagem recebida na struct
        memcpy(msg->message, &buf[4], msg->message_size); //destination, source, num 
   }
   else{
         //caso tenha recebido RR ou REJ, é necessário mudar o sequenceNumber para o N(r)
         msg->type = setControlField(buf[2]);
         if(msg->type == RR || msg->type == REJ)
         {
            int nr = buf[2] >> 7;
            data_link.sequenceNumber = nr;      //atualizo
         }
   }
   
   free(buf);   //liberta o espaco em memoria
 
   return OK;   
}

/**
 * 
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
    else if(RR == flag){  // o nr é sempre o oposto do que recebe 
        unsigned char nr = 1;
        if(data_link.sequenceNumber == 1) nr = 0;   
        return (FRAME_C_RR | (nr << 7));
    }
    else if(REJ == flag){   //igual para o REJ
        unsigned char nr = 1;
        if(data_link.sequenceNumber == 1) nr = 0;   
        return FRAME_C_REJ | (nr << 7);
    }
    else if(I == flag){
        return (data_link.sequenceNumber << 6); //o controlo depende do sequenceNumber
    }
    else
        printf("ainda nao esta definida\n");
    
}

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
        printf("%x ainda nao esta definida\n",c);
}

/**
 * 
 */
int stuff(unsigned char **frame, int frame_length){
     
    int newframe_length = 2; //as flags de incio e fim
    
    //por cada flag ou escape vou adicionar +1 byte
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        unsigned char c = (*frame)[i];
        if ((c == FLAG) || (c == ESCAPE))
        {
            newframe_length++;
        }
        newframe_length++;
    }
    
    //reservar espaço na memoria    
    *frame = (unsigned char*)realloc(*frame,newframe_length);
    
    for(i = 1; i < (frame_length-1); i++)
    {
        unsigned char c = (*frame)[i];
        if(c == FLAG || c == ESCAPE)
        {
            memmove(*frame + i + 1, *frame + i, frame_length - i);//destino, source, num de bytes -> avançar 1 casa todos os bytes para a frente
            (*frame)[i] = ESCAPE;
            (*frame)[i++] = c ^ AUX;
            frame_length++;
        }
    }
    
    return newframe_length;
}

/**
 * Processo contrario ao stuff
 */
int desstuff(unsigned char **frame, int frame_length){
    
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        unsigned char c = (*frame)[i];
        if (c == ESCAPE)
        {
            memmove(*frame + i, *frame + i + 1, frame_length - i - 1);//destino, source, num de bytes -> avançar 1 casa todos os bytes para a tras
            frame_length--;
            
            (*frame)[i] ^= AUX;
        }
    }
    
    //realocar o espaco em memoria (o tamanho foi modificado)
    *frame = (unsigned char *)realloc(*frame, frame_length);
    
    return frame_length;
}

/**
 * So para debugging
 */
void display(unsigned char*frame, int n){
    
    printf(">>>>>>>>>>\n");
    
    int i;
    for(i = 0; i < n; i++){
        printf("frame[%d] = 0x%x\n",i,frame[i]);
    }
    
    printf(">>>>>>>>>>\n");
}
