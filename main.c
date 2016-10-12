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

    llopen(argv[1],isReceiver);

    return 0;
}
