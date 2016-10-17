#include "linkLayer.h"

//volatile int STOP=FALSE;

char* build_frame_SU(char *flag)      
{

    char *frame = NULL;
    frame = (char *) malloc(FRAME_SIZE);
 
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    
    frame[2] = getControlField(flag);
    /*
    if(isUA==1) 
	frame[2] = FRAME_C_UA;
    else if (isUA==2)
	frame[2] == FRAME_C_DISC;
    else
	frame[2] = FRAME_C_SET;*/
    
    frame[3] = (frame[1] ^ frame[2]);
    frame[4] = FLAG;
    
    return frame;
}

/*
 * @briefFunction that creates a frame I
 * @param data to transport
 * @param data_length
 * @param s NS of Frame I
 * @FrameI : |F|A|C|BCC1|DATA|BCC2|F|
 */
char* build_frame_I(char* data, unsigned int data_length){

    char *frame = NULL;
    data_link.frame_size = FRAME_SIZE + data_length+1 ; //frame size, with data length and an extra byte for BCC2
    frame = (char *) malloc(data_link.frame_size);
    
    //printf("BUILD : frame size (suposto) %d\n",data_link.frame_size);
 
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
    
    //printf("BUILD : frame size (real) %d\n",strlen(frame));
    
    return frame;
}

int receive(int fd, char* flag){

   char* frame = NULL;
   frame = (char *) malloc(data_link.frame_size);     //comecamos por reservar espaço para uma frame do tipo S ou U, se for do tipo I fazemos realloc
   
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
               return -1;
           }
       }
       
       switch (state) {
           case START:
               if (c == FLAG) 
               {
                   frame[size] = c;
                   size++;
                   state = FLAG_RCV;
               }
               break;
           case FLAG_RCV:
               if(c == FRAME_A)       //nao esta bem!!!!!!!!!!!!!!!!!!!!!! temos de ter em conta se é 0x03 ou 0x01 (ainda so funciona para 0x03)
               {
                   frame[size] = c;
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
                   frame[size] = c;
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
               
               bcc = frame[1] ^ frame[2];
               
               if(c == bcc)
               {
                   frame[size] = c;
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
                   frame[size] = c;
                   state = STOP;    //ultimo byte
               }
               else //trama do tipo I
               {
                   frame[size] = c;
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
   
   frame = desstuff(frame,data_link.frame_size);
   
   if(hasData)
   {
       int data_length = data_link.frame_size - FRAME_SIZE - 1; //-1 por causa da proteção dupla
       
       int i, bcc2;
       for(i = 0; i < data_length; i++){
           bcc2 ^= frame[4+i];
       }
       
       if(bcc2 != frame[4+data_length]){
           printf("erro no bcc2!\n");
           return -1;
       }
        
        //nao sei se faltam coisas aqui... tipo colocar a mensagem em algum lado, ou assim.
       
   }
   
   //se recebemos uma trama do tipo RR significa que o N(s) a proxima trama do tipo I vai depender deste N(r), por isso podemos ja tratar do assunto
   //PS: nao sei se isto esta correto estar aqui, mas funciona
   if(strcmp(flag,"RR") == 0)
   {
       data_link.sequenceNumber = (tmp >> 7) & 0;
   }
 
   return 0;
    
}

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
        // o nr é sempre o oposto do que recebe
        int nr = 1;
        if(data_link.sequenceNumber == 0) nr = 0;
        
        return (FRAME_C_RR << 7) & nr;
    }
    if(strcmp("I",flag) == 0){
        return (FRAME_C_I << 6) & data_link.sequenceNumber; //o controlo depende do sequenceNumber
    }
        
    printf("ainda nao esta definida");
    
}

/**
 * 
 */
char* stuff(char *frame, int frame_length){
     
    int newframe_length = 2; //as flags de incio e fim
    
    //por cada flag ou escape vou adicionar +1 byte
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        char c = frame[i];
        if ((c == FLAG) || (c == ESCAPE))
        {
            newframe_length++;
        }
        newframe_length++;
    }
    
    data_link.frame_size = newframe_length;
    
    //reservar espaço na memoria
    char* newframe = (char *) malloc(newframe_length);
    
    int j = 0; //percorre newframe
    newframe[j] = FLAG;
    j++;
    
    //ignora o primeiro caracter e ultimos caracteres que correpespondesm a flag
    for(i =1; i < (frame_length-1); i++){
        if(frame[i] == FLAG){
            newframe[j]= ESCAPE;
            j++;
            newframe[j]= FLAG ^ AUX;
        }else if(frame[i] == ESCAPE){
            newframe[j] = ESCAPE;
            j++;
            newframe[j]= ESCAPE ^ AUX;
        }else{
            newframe[j] = frame[i];
        }
        j++;
    }
    newframe[j] = FLAG;
    
    return newframe;
}

char* desstuff(char *frame, int frame_length){
    
    int newframe_length = frame_length; //começa com o original e tira quando encontra um escape
    
    int i;
    for (i = 1; i < (frame_length - 1); i++)
    {
        char c = frame[i];
        if (c == ESCAPE)
        {
            newframe_length--;
        }
    }
    
    data_link.frame_size = newframe_length;
    
     //reservar espaço na memoria
    char* newframe = (char *) malloc(newframe_length);
    
    int j = 0; //percorre newframe
    newframe[j] = FLAG;
    j++;
    
    i = 1;
    //ignora o primeiro caracter e ultimos caracteres que correpespondesm a flag
    while(i != frame_length-1)
    {
        if(frame[i] == ESCAPE)
        {
            i++;
            if(frame[i] == (FLAG ^ AUX)){
                newframe[j] = FLAG;
            }else if(frame[i] == (ESCAPE ^ AUX)){
                newframe[j] = ESCAPE;
            }
        }else
        {
            newframe[j] = frame[i];
        }
        
        i++;
        j++;
    }
    newframe[j] = FLAG;
    
    return newframe;
}
