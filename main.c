//#include "linkLayer.c"
#include "applicationLayer.c"

/**
 * Recebe as informações iniciais.
 * porta
 * flag: TRANSMITTER | RECEIVER
 * file
 */
int main(int argc, char** argv)
{
   ///testar application layer
 /*   if(argc != 4)
    {
      exit(1);
    }
    initApplicationLayer(argv[1],atoi(argv[2]),argv[3]);
    //sender(0,argv[1]);*/

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
    char *data1 = "efgh";
    char *newData1 = malloc(4);
    char *newData2 = malloc(4);
    
    if(isReceiver){
        llread(fd,newData1);
        printf("NEW DATA : %s\n",newData1);
        llread(fd,newData2);
        printf("NEW DATA : %s\n",newData2);
    }
    else{
        llwrite(fd,data,4);
        llwrite(fd,data1,4);
    }
    
    printf("-----------------------------------------------\n");
    
    llclose(fd,isReceiver);
    
    //free(newData);
    
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
