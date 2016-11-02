#include "applicationLayer.h"

/**
 * Recebe as informações iniciais.
 * porta
 * flag: TRANSMITTER | RECEIVER
 * file				//por agora ainda nao
 */
int main(int argc, char** argv)
{
   ///testar application applicationLayer
    if(argc != 1)
    {
        perror("Invalid number of arguments");
        exit(1);
    }
    
    char port[20], status[2], mode[2], path[128];
	
    printf("\nPORT:\n  /dev/ttyS0\n  /dev/ttyS1\n");
    scanf("%s", port);
	
    printf("\nSTATUS:\n  Transmitter (0)\n  Receiver (1)\n");
    scanf("%s", status);

    printf("\nMODE:\n  Normal (0)\n  Simple Debug (1)\n  Full Debug(3)\n");
    scanf("%s", mode);
	
    printf("\nPATH:\n  Receiver - Path to keep the fille\n  Transmitter - Path of the file to be sent\n");
    scanf("%s",path);
    printf("\n\n");
	
/*
        printf("Baud rate (Write 0 for default value): ");
	scanf("%d", BAUDRATE);
	if(BAUDRATE==0)
		BAUDRATE=B9600;
	
	printf("Maximum frame size (Write 0 for default value): ");
	scanf("%d",BUF_SIZE);
	if(BUF_SIZE==0)
		BUF_SIZE=126;
	
	printf("Maximum retransmissions: ");
	scanf("%d",RETRANSMITIONS);
	
	printf("Timeout: ");
	scanf("%d",TIMEOUT);	
*/	
    initApplicationLayer(port,atoi(status),atoi(mode),path);
    
    /**
     * application Layer
     */
    
    //argv[1]-> porta, argv2 -> transmitter/receiver argv3->path
    //sender(0,argv[1]);
    
    /*
    * Link layer
    */
/*
    if ( (argc < 3) || 
        ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0)) ||
        ((strcmp("TRANSMITTER", argv[2])!=0) && (strcmp("RECEIVER", argv[2])!=0)) )
    {
      printf("Usage:\twserial SerialPort flag\n\tex: wserial /dev/ttyS1 TRANSMITTER\n");
      exit(1);
    }

    //inicializa o dataLink
    init_linkLayer(argv[1]); 

    int isReceiver = 1;
    if(strcmp("TRANSMITTER", argv[2])==0) isReceiver=0;

    int fd = llopen(argv[1],isReceiver);
    
    
    printf("-----------------------------------------------\n");
 
    char *data = "abcd";
    char *newData = malloc(4);
    if(isReceiver) llread(fd,newData);
    else llwrite(fd,data,4);
    
    printf("-----------------------------------------------\n");
    
    llclose(fd,isReceiver);
*/
    /*
     * Teste BYTE stuffing
     */
    
    /*
    char *info = NULL;
    
    char *data = "a]\n}[b^c\n}}}~\nd";
    int data_length = 15;
    
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
    
    int newsize = desstuff(info, size);
    
    printf("\n--------------------------------------\n");
    printf("\nInfo depois do desstuffing:\n");
    for(i = 0; i < newsize; i++){
        printf("%c\n",info[i]);
    }*/
    return 0;
}
