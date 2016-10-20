#include "applicationLayer.h"
/* IMPORTANTE ======================================================================================
Ver se o tamanho das STRINGS incluiem o +1 (para \0)
====================================================================================================*/


/*argumentos
– filename: nome de ficheiro
– filesize: tamanho do ficheiro
- date: data do ficheiro
- permissions: permissoes do ficheiro
retorno
– pacote de controlo inicial com as informacoes do ficheiro  */
int sendControlPackage(char control,int fd,char * file_name, char * file_size, char * file_date, char * file_perm)
{
  char *pkg = (char*)malloc(MAX_PKG_SIZE*sizeof(char)+1);

    sprintf(pkg,"%c%c%c%s%c%c%s%c%c%s%c%c%s",control,
      FILE_SIZE,(char)strlen(file_size),file_size,
      FILE_NAME,(char)strlen(file_name),file_name,
      FILE_DATE,(char)strlen(file_date),file_date,
      FILE_PERM,(char)strlen(file_perm),file_perm);

  int written = 0;
  int STOP = 0;

   /* while(!STOP){
      if((written += llwrite(fd,pkg,strlen(pkg))) == -1){
        perror("Could not send START PACKAGE");
        exit(-1);
      }
      if(written == strlen(pkg))
        STOP = 1;
    }
*/
  free(pkg);

	return 0;
}

/*argumentos
– data: parcela de dados do ficheiro
- size: tamanho do pacote de dados
retorno
– pacote de dados do ficheiro a enviar */
int sendDataPackage(int fd, char * data, int sequenceN,unsigned int size)
{
	char *pkg = (char*)malloc(MAX_PKG_SIZE*sizeof(char)+1);

  
  sprintf(pkg,"%c%c%c%c%s",PKG_DATA,
                          (char)sequenceN,
                          (char)size-(size/DATA_SIZE),
                          (char)size-(size/DATA_SIZE),
                          data);
	

  int written = 0;
  int STOP = 0;
  
   /* while(!STOP){
      if((written += llwrite(fd,pkg,strlen(pkg))) == -1){
        perror("Could not send DATA PACKAGE");
        exit(-1);
      }
      if(written == strlen(pkg))
        STOP = 1;
    }
*/
  
  free(pkg);
	return 0;
}

int sender(char* port, char* filepath){

	//inicializa o dataLink
    //init_linkLayer(port);

    int fd = 0;// = llopen(port,0);		//0 - is not the receiver

    int file;

    if((file = open(filepath, O_RDONLY)) == -1){			//Meter mais flags
    	perror("Could not open file with filepath");
    	exit(-1);
    }

    //get status struct from the file
   	struct stat st;
   	if(stat(filepath,&st) == -1){
   		perror("Could not acess file status");
   		exit(-1);
   	}

   	//get file_name position
   	int pos = 0, i;
   	for(i = 0; i < strlen(filepath);i++){
   		if(filepath[i] == '/'){
   			pos = i;
   		}
   	}
   	
   	//information about the file
   	
   	char *file_name = filepath+pos+1;
   	char file_size[16], file_date[16], file_perm[16];

   	sprintf(file_size,"%lu",st.st_size);
   	sprintf(file_date,"%lu",st.st_mtime);
   	sprintf(file_perm,"%u",st.st_mode);

   if(sendControlPackage(PKG_START,fd,file_name,file_size,file_date,file_perm) == -1){
      perror("Could not send Start Package");
      exit(-1);
    }

   
   	int STOP = 0, r = 0;
    sequenceNumber = 0;

    while(!STOP){

      char *data = (char*) malloc(sizeof(char)*DATA_SIZE + 1);
      memset(data,'\0',sizeof(char)*DATA_SIZE + 1);             //clear alocated memmory

      if((r = read(file,data,DATA_SIZE)) == -1){
    		perror("Could not read file");
    		exit(-1);
    	}
    	else if(r < DATA_SIZE){
    		STOP = 1;
      }

      sendDataPackage(fd,data,sequenceNumber,r);

      free(data);      
  	 	sequenceNumber++;
    }

   	//creates and sends package END
   	//applicationPackage apk_end = createEndPackage();
   	/*if(llwrite(PKG_END,fd,apk_start,strlen(apk_end)) == -1){
   		perror("Could not send END PACKAGE");
   		exit(-1);
   	}*/

   /* close(fd);
    close(file);
    free(data);*/

	return 0;
}

int reciever(char* port){

	//inicializa o dataLink
    /*init_linkLayer(port);
    int fd = llopen(port,1);		//1 - is the receiver*/

    //espera com llread o package START string[0] = 2 
    //analiza a informação e cria o ficheiro com os parametros indicados (permissões/data/nome)
    //abre ficheiro com TRUNCATE e WRITE only
    //espera por outro packate
    //	se string[1] -> data
    //		armazena informação no fim do ficheiro
    //	se string[0] -> end
    //lclode
    //close file
    // free alocated memory



	return 0;
}