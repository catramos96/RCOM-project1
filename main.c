#include "linkLayer.c"

/**
 * Recebe as informações iniciais.
 * porta
 * flag: TRANSMITTER | RECEIVER
 * file				//por agora ainda nao
 */
int main(int argc, char** argv)
{
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

    //llopen(argv[1],isReceiver);
 
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
