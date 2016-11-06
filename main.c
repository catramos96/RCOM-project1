#include "applicationLayer.h"

/**
 * Inicializacao da aplicacao. 
 * Recebe os dados do utilizador e inicializa e imprime as estatisticas.
 */
int main(int argc, char** argv)
{	
	RETRANSMITIONS=3;
	TIMEOUT=1;
   
    if(argc != 1)
    {
        perror("Invalid number of arguments");
        exit(1);
    }
    
    char port[20], status[2], mode[2], path[128], max_size[8];
	
    printf("\nPort:\n  /dev/ttyS0\n  /dev/ttyS1\n");
    scanf("%s", port);
	
    printf("\nStatus:\n  Transmitter (0)\n  Receiver (1)\n");
    scanf("%s", status);

    printf("\nMode:\n  Normal (0)\n  Simple Debug (1)\n  Full Debug(2)\n");
    scanf("%s", mode);
	
    printf("\nPath:\n  Receiver - Path to keep the fille\n  Transmitter - Path of the file to be sent\n");
    scanf("%s",path);

	if(atoi(status) == TRANSMITTER)
	{
	    printf("\nMax package size:\n");
   		scanf("%s",max_size);
		
		printf("\nMaximum retransmissions: ");
		scanf("%d",&RETRANSMITIONS);
		
		printf("Timeout for retransmissions: ");
		scanf("%d",&TIMEOUT);
	}
	else	//receiver
	{
		printf("\nTotal timeout: ");
		scanf("%d",&TIMEOUT);
		
		strcpy(max_size, "255");
	}
	
	initStatistics();

	//inicializacao da aplicacao
    initApplicationLayer(port,atoi(status),atoi(mode),atoi(max_size),path);
	 
	displayStatistics(atoi(status));
	
    return 0;
}

/**
 * inicializar struct estatisticas.
 */
void initStatistics()
{
    statistics.tramasIenviadas=0;
    statistics.tramasIretransmitidas=0;
    statistics.tramasIrecebidas=0;
    statistics.timeouts=0;
    statistics.REJenviados=0;
    statistics.REJrecebidos=0;
}

/**
 * Display das estatisticas recolhidas ao longo do programa.
 */
void displayStatistics(int status)
{
	if(status == TRANSMITTER)	//TRANSMITTER
	{
		printf("Number of I frames sent: %d\n",statistics.tramasIenviadas);
		printf("Number of timeouts: %d\n",statistics.timeouts);
		printf("Number of REJ frames received: %d\n",statistics.REJrecebidos);
	}
	else	//RECEIVER
	{
		printf("Number of I frames received: %d\n",statistics.tramasIrecebidas);
		printf("Number of I frames retransmissioned: %d\n",statistics.tramasIretransmitidas);
        printf("Number of REJ frames sent: %d\n",statistics.REJenviados);
	}
}

/**
 * application Layer
 */
/*
void test_application(char *a)
{
    //a-> porta, argv2 -> transmitter/receiver argv3->path
    sender(0,a);
}
*/

/**
 * Link layer
 */
 /*
void test_link(int total, char*a, char*b)
{
    if ( (total < 3) || 
        ((strcmp("/dev/ttyS0", a)!=0) && (strcmp("/dev/ttyS1", a)!=0)) ||
        ((strcmp("TRANSMITTER", b)!=0) && (strcmp("RECEIVER", b)!=0)) )
    {
      printf("Usage:\twserial SerialPort flag\n\tex: wserial /dev/ttyS1 TRANSMITTER\n");
      exit(1);
    }

    //inicializa o dataLink
    init_linkLayer(a,0); 

    int isReceiver = 1;
    if(strcmp("TRANSMITTER",b)==0) isReceiver=0;

    int fd = llopen(a,isReceiver);
    
    
    printf("-----------------------------------------------\n");
 
    char *data = "abcd";
    char *newData = malloc(4);
    if(isReceiver) llread(fd,newData);
    else llwrite(fd,data,4);
    
    printf("-----------------------------------------------\n");
    
    llclose(fd,isReceiver);
}
*/

/** 
 * Teste BYTE stuffing
 */
 
 /*
void test_byteStuffing()
{
    char *info = NULL;
    
    char *data = "a]}[b^c}}}~d";
    int data_length = 12;
    
    info = build_frame_I(data,data_length);     //criacao da trama I
    
    printf("\n--------------------------------------\n");
    
    int i;
    printf("Info antes do stuffing:\n");
    for(i = 0; i < data_length+6; i++){
        printf("%c\n",info[i]);
    }
    
    int size = stuff(info, data_length+6);
       
    printf("\n--------------------------------------\n");
    printf("\nInfo depois do stuffing:\n");
    for(i = 0; i < size; i++){
        printf("%c\n",info[i]);
    }
    
    int newsize = destuff(info, size);
    
    printf("\n--------------------------------------\n");
    printf("\nInfo depois do desstuffing:\n");
    for(i = 0; i < newsize; i++){
        printf("%c\n",info[i]);
    }
}
*/