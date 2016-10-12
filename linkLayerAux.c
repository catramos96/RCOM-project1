#include "linkLayer.h"

volatile int STOP=FALSE;

char* build_frame_SU(int isUA){

    char *frame = NULL;
    frame = (char *) malloc(FRAME_SIZE);
 
    frame[0] = FLAG;    
    frame[1] = FRAME_A;
    if(isUA) 
	frame[2] = FRAME_C_UA;
    else
	frame[2] = FRAME_C_SET;
    frame[3] = (frame[1] ^ frame[2]);
    frame[4] = FLAG;
    
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
	    {
		if (tmp!=FLAG)
		    correct = 0;
		break;
	    }
	    case 1:
	    {
		if (tmp==FRAME_A)
		   break;
		else if(tmp==FLAG)
		{
		    correct = 0;
		    n=1;
		}
		else
		{
		    n=0;
		    correct = 0;
		}
		break;
	   }
	   case 2:
	   {
		if(isUA && tmp == FRAME_C_UA)
		   break;
		else if(!isUA && tmp == FRAME_C_SET)
		   break;
		else if(tmp==FLAG)
		{
		   correct=0;
		   n=1;
		}
		else
		{
		   correct = 0;
		   n=0;
		}
		break;
	   }
	   case 3:
	   {
		if (tmp==(frame[1]^frame[2]))
		   break;
		else if(tmp==FLAG)
		{
		   correct = 0;
		   n=1;
		}
		else
		{
		   n=0;
		   correct = 0;
		}
		break;
	   }
	   case 4:
	   {
		if (tmp!=FLAG)
		{
		   n=0;
		   correct = 0;
		}
		break;
	   }
	   default:
	   {
		n=0;
		break;
	   }
	}
	if(correct)
	{
	   frame[n] = tmp;
	   n++;
	}
	else		//fazer mudanças de forma a aceitar a maquina de estados pretendida
	{
	   tries++;
	   if(tries >=3)
	   {
	   	printf("Too many attempts to send trama set \n");
	   	return -1;			
	   }
	}
     }
   }
   printf("Trama recebida!\n");
   return 0;
}
