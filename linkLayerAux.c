#include "linkLayer.h"

volatile int STOP=FALSE;

char* build_frame_SU(int isUA){

    char *frame = NULL;
    frame = (char *) malloc(FRAME_SIZE);
 
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    if(isUA==1) 
	frame[2] = FRAME_C_UA;
	else if (isUA==2)
	frame[2] == FRAME_C_DISC;
    else
	frame[2] = FRAME_C_SET;
    frame[3] = (frame[1] ^ frame[2]);
    frame[4] = FLAG;
    
    return frame;
}

char* build_frame_I(char* data, unsigned int data_length,char s){

    char *frame = NULL;
    int total = FRAME_SIZE + data_length+1 ; //frame size, with data length and an extra byte for BCC2
    frame = (char *) malloc(total);
    
    printf("BUILD : frame size (suposto) %d\n",total);
 
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    if(s) 
	frame[2] = FRAME_C_I;
    else
	frame[2] = s; //s=0
    
    frame[3] = (frame[1] ^ frame[2]);
    
    //data
    int i = 0;
    for(i = 0; i < data_length; i++){
        frame[4+i] = data[i];
        frame[4 + data_length] ^= frame[4 + i];  //BCC2
    }
    
    frame[5+data_length] = FLAG;
    
    printf("BUILD : frame size (real) %d\n",strlen(frame));
    
    return frame;
}

int receive_verify_SU(int fd, int isUA){

   char tmp = "";
   int n = 0;
   int flagRead = 0;
   char* frame = NULL;
   frame = (char *) malloc(FRAME_SIZE);

   while (STOP==FALSE) 
   {   
       if((read(fd,&tmp,1)) == -1){
           perror("read receiver");
           exit(-1);
        }

        if(tmp == FLAG && flagRead == 0){
            flagRead = 1;
        } 
      
      if(flagRead)
      {
          if ((n != 0) && tmp == FLAG) STOP=TRUE;
          
          int correct = 1;
          int tries = 0;

          //Verificação do trama
          switch(n)
          {
              case 0:
                  if (tmp!=FLAG)
                      correct = 0;
                  break;
              case 1:
                  if (tmp==FRAME_A)
                      break;
                  else if(tmp==FLAG)
                  {
                      correct = 0;
                      n=1;
        
                  }else{
		    n=0;
		    correct = 0;
		 }
		 break;
	   case 2:
               if(isUA && tmp == FRAME_C_UA)
                   break;
               else if(!isUA && tmp == FRAME_C_SET)
		   break;
               else if(tmp==FLAG){
                   correct=0;
		   n=1;
                   
               }
               else{
		   correct = 0;
		   n=0;
               }
               break;
	   case 3:
               if (tmp==(frame[1]^frame[2]))
		   break;
               else if(tmp==FLAG){
		   correct = 0;
		   n=1;
               }
               else{
		   n=0;
		   correct = 0;
               }
               break;
	   case 4:
               if (tmp!=FLAG){
                   n=0;
		   correct = 0;
               }
               break;
	   default:
               n=0;
               break;
	}
	if(correct)
	{
            frame[n] = tmp;
            n++;
	}
	else		//fazer mudanças de forma a aceitar a maquina de estados pretendida
	{
            tries++;
            if(tries >=3){
                printf("Too many attempts to send trama set \n");
                return -1;			
	    }
        }
      }
   }
   printf("Trama recebida!\n");
   return 0;
    
}

/**
 * CÓDIGO DO PROFESSOR
 */
int receiveFrame(int fd){
    
    char buf[BUF_SIZE]; //valor recebido

    int stop = 0;
    int hasFirst = 0;
    int nfr = 0;

    while (!stop)
    {
        int n;
        n = (int)read(fd, buf + nfr, BUF_SIZE - nfr);
        if (n <= 0)
        {
            stop = 1;
            break;
        }
        nfr += n;
    }


    if (!hasFirst)
    {
        int k;
        for (k = 0; k < nfr; k++)
        {
            if (FLAG == buf[k])
            {
                break;
            }
        }

        if (k < nfr)
        {
            if (k)
            {
                memmove(buf, buf + k, nfr - k);
                nfr -= k;
            }
            hasFirst = 1;
        } else
        {
            nfr = 0;
        }
    }

    if (hasFirst && (nfr > 1))
    {
        int k;
        for (k = 1; k < nfr; k++)
        {
            if (buf[k] == FLAG)
            {
                break;
            }
        }

        if (k < nfr)
        {
            //process(buf, k+1); //se for uma trama S ou U e uma maquina de estados?
            if (nfr > k + 1)
            {
                memmove(buf, buf + k + 1, nfr - k - 1);
            }
            nfr -= k + 1;
            hasFirst = 0;
        }
    }

    if (BUF_SIZE == nfr)
    {
        nfr = 0;
    }

return 0;
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