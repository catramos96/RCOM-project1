#include "applicationLayer.h"

/**
 * Recebe as informações iniciais.
 * porta
 * flag: TRANSMITTER | RECEIVER
 * file				//por agora ainda nao
 */
int main(int argc, char** argv)
{
   ///testar application layer
    if(argc != 1)
    {
		perror("Invalid number of arguments");
      exit(1);
    }
	
	char port[20], status[5], path[128];
	
	printf("port (Write d for default value): ");
	scanf("%s", port);
	if(port=="d")				//TMP
		strcpy(port,"/dev/ttyS1");
	
	printf("Transmitter (0) / receiver (1): ");
	scanf("%s", status);
	
	printf("File path: ");
	scanf("%s",path);
	
	/*printf("Baud rate (Write 0 for default value): ");
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
	
    initApplicationLayer(port,atoi(status),path);
	//argv[1]-> porta, argv2 -> transmitter/receiver argv3->path
   //sender(0,argv[1]);
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
    char *info_stuffed = NULL;
    char *info_desstuffed = NULL;
    char *data = "a}b~";
    
    info = build_frame_I(data,strlen(data),0);
    
    printf("MAIN : info size %d\n\n",strlen(info));
    
    int i;
    printf("Info antes do stuffing:\n");
    for(i = 0; i < 10; i++){
        printf("info[%d] = 0x%x\n",i,info[i]);
    }
    
    info_stuffed = stuff(info, 10);
       
    printf("\nInfo depois do stuffing:\n");
    for(i = 0; i < 12; i++){
        printf("info_stuffed[%d] = 0x%x\n",i,info_stuffed[i]);
    }
    
    info_desstuffed = desstuff(info_stuffed, 12);
    
    printf("\nInfo depois do desstuffing:\n");
    for(i = 0; i < 10; i++){
        printf("info_desstuffed[%d] = 0x%x\n",i,info_desstuffed[i]);
    }
    */
    return 0;
}
