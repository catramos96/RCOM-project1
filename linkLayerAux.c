#include "linkLayer.h"

//volatile int STOP=FALSE;

/**
 * Cria uma trama do tipo S ou U
 */
char* build_frame_SU(char *flag)      
{
    char *frame = NULL;
    frame = (char *) malloc(FRAME_SIZE);
 
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
char* build_frame_I(char* data, unsigned int data_length){

    char *frame = NULL;
    int total = FRAME_SIZE + data_length+1 ; //frame size, with data length and an extra byte for BCC2
    frame = (char *) malloc(total);
    
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    frame[2] = (FRAME_C_I << 6) & data_link.sequenceNumber; //s=0
    frame[3] = (frame[1] ^ frame[2]);
    
    //data
    int i = 0;
    for(i = 0; i < data_length; i++){
        frame[4+i] = data[i];
        frame[4 + data_length] ^= frame[4 + i];  //BCC2
    }
    
    frame[5+data_length] = FLAG;
    
    return frame;
}

/**
 * Recebe uma trama
 */
ReturnType receive(int fd, char* flag){
    
   int maxSixe = 30; // este valor ainda não sei se esta correto, mas quando estiver vai faz parte das constantes

   //ReturnType rt = OK;

   char* buf = NULL;
   buf = (char *) malloc(maxSixe);
   
   State state = START;
   int size = 0;
   int done = 0;
   int hasData = 1; //flag que indica se é uma trama do tipo I
   char bcc;
   char tmp;

   while (!done) 
   {   
       unsigned char c;
       
       if (state != STOP) {
           if((read(fd,&c,1)) == -1){
               perror("read receiver");
			   return ERROR;
           }
       }
       
       switch (state) {
           case START:
               if (c == FLAG) 
               {
                   buf[size] = c;
                   size++;
                   state = FLAG_RCV;
               }
               break;
           case FLAG_RCV:
               if(c == FRAME_A)       //nao esta bem!!!!!!!!!!!!!!!!!!!!!! temos de ter em conta se é 0x03 ou 0x01 (ainda so funciona para 0x03)
               {
                   buf[size] = c;
                   size++;
                   state = A_RCV;
               }
               else if(c != FLAG)
               {
                    size =1;
                    state = START;
               }
               break;
           case A_RCV:
		tmp = getControlField(flag);
               if(c == tmp)
               {
                   buf[size] = c;
                   size++;
                   state = C_RCV;
               }
               else if(c == FLAG)
               {
                   size = 1;
                   state = FLAG_RCV;
               }
               else
               {
                   size = 0;
                   state = START;
               }
               break;
           case C_RCV:
               
               bcc = buf[1] ^ buf[2];
               
               if(c == bcc)
               {
                   buf[size] = c;
                   size++;
                   state = BCC_OK;
               }
               else if(c == FLAG)
               {
                   size = 1;
                   state = FLAG_RCV;
               }
               else
               {
                   size = 0;
                   state = START;
               }
               break;
           case BCC_OK:
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
   
   buf = (char*)realloc(buf,size);  //realloc para o tamanho certo da trama
    
   /*printf("Receive antes do desstuffing\n");
   display(buf,size);*/
  
   int newsize = desstuff(&buf,size);   //recebe com stuff, fazemos desstuffing
   
  /* printf("Receive depois do desstuffing\n");
   display(buf,newsize); */
   
   //se e uma trama do tipo I, e necessario analisar depois do stuffing
   if(hasData)
   {
       data_link.frame_size = newsize - FRAME_SIZE - 1; //-1 por causa da proteção dupla	

       int i;
       char bcc2 = (char)0x0;
       for(i = 0; i < data_link.frame_size; i++){
           bcc2 ^= buf[4+i];
       }
       
       if(bcc2 != buf[4+data_link.frame_size]){
           return DATAERROR;
       }
        
	   //colocar a mensagem recebida na struct
	   memcpy(data_link.frame, &buf[4], data_link.frame_size); //destination, source, num 
   }
   
   //se recebemos uma trama do tipo RR significa que o N(s) a proxima trama do tipo I vai depender deste N(r), por isso podemos ja tratar do assunto
   //PS: nao sei se isto esta correto estar aqui, mas funciona
   if(strcmp(flag,"RR") == 0)
   {
       data_link.sequenceNumber = (tmp >> 7) & 0;
   }
   
   free(buf);   //liberta o espaco em memoria
 
   return OK;
    
}

/**
 * 
 */
char getControlField(char* flag)
{
    if(strcmp("SET",flag) == 0){
        return FRAME_C_SET;
    }
    if(strcmp("UA",flag) == 0){
        return FRAME_C_UA;
    }
    if(strcmp("DISC",flag) == 0){
        return FRAME_C_DISC;
    }
    if(strcmp("RR",flag) == 0){
        // o nr é sempre o oposto do que recebe (TRATAR DISTO DEPOIS)
        int nr = 1;
        if(data_link.sequenceNumber == 0) nr = 0;   
        return (FRAME_C_RR << 7) & nr;
        //return FRAME_C_RR;
    }
    if(strcmp("REJ",flag) == 0){
        // o nr é sempre o oposto do que recebe (TRATAR DISTO DEPOIS)
        int nr = 1;
        if(data_link.sequenceNumber == 0) nr = 0;   
        return (FRAME_C_RR << 7) & nr;
        //return FRAME_C_REJ;
    }
    if(strcmp("I",flag) == 0){
        return (FRAME_C_I << 6) & data_link.sequenceNumber; //o controlo depende do sequenceNumber
		//return FRAME_C_I;
    }
        
    printf("ainda nao esta definida");
    
}

/**
 * 
 */
int stuff(char **frame, int frame_length){
     
    int newframe_length = 2; //as flags de incio e fim
    
    //por cada flag ou escape vou adicionar +1 byte
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        char c = (*frame)[i];
        if ((c == FLAG) || (c == ESCAPE))
        {
            newframe_length++;
        }
        newframe_length++;
    }
    
    //reservar espaço na memoria    
    *frame = (char*)realloc(*frame,newframe_length);
    
    for(i = 1; i < (frame_length-1); i++)
    {
        char c = (*frame)[i];
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
int desstuff(char **frame, int frame_length){
    
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        char c = (*frame)[i];
        if (c == ESCAPE)
        {
            memmove(*frame + i, *frame + i + 1, frame_length - i - 1);//destino, source, num de bytes -> avançar 1 casa todos os bytes para a tras
            frame_length--;
            
            (*frame)[i] ^= AUX;
        }
    }
    
    //realocar o espaco em memoria (o tamanho foi modificado)
    *frame = (char *)realloc(*frame, frame_length);
    
    return frame_length;
}

/**
 * So para debugging
 */
void display(char*frame, int n){
    
    printf(">>>>>>>>>>\n");
    
    int i;
    for(i = 0; i < n; i++){
        printf("frame[%d] = 0x%x\n",i,frame[i]);
    }
    
    printf(">>>>>>>>>>\n");
}
