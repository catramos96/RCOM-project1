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

    sprintf(pkg,"%c%c%c%s%c%c%s%c%c%s%c%c%s",(char)control,
      (char)FILE_SIZE,(char)strlen(file_size),file_size,
      (char)FILE_NAME,(char)strlen(file_name),file_name,
      (char)FILE_DATE,(char)strlen(file_date),file_date,
      (char)FILE_PERM,(char)strlen(file_perm),file_perm);

  int written = 0;
  int STOP = 0;
  

    while(!STOP){
      if((written += llwrite(fd,pkg,strlen(pkg))) == -1){
        perror("Could not send START PACKAGE");
        exit(-1);
      }
      if(written == strlen(pkg))
        STOP = 1;
    }

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

  
  sprintf(pkg,"%c%c%c%c%s",(char)PKG_DATA,
                          (char)sequenceN,
                          (char)size-(size/DATA_SIZE),
                          (char)size-(size/DATA_SIZE),
                          data);


  int written = 0;
  int STOP = 0;

    while(!STOP){
      if((written += llwrite(fd,pkg,strlen(pkg))) == -1){
        perror("Could not send DATA PACKAGE");
        exit(-1);
      }
      if(written == strlen(pkg))
        STOP = 1;
    }
  
  free(pkg);
	return 0;
}

int analizePackage(char* data, struct package pkg){

int n_bytes = 0;
char tmp[36];
int i = 0;

//DEBUG
//Test control package
/*char data[MAX_PKG_SIZE+1];
  sprintf(data,"%c%c%c%s%c%c%s%c%c%s%c%c%s",(char)PKG_START,
                (char) 0, (char) 3, "999",
                (char) 1, (char) 11, "constants.h",
                (char) 2, (char) 10, "1477342734",
                (char) 3, (char) 5, "33152");*/

//Test data package
/*char data[MAX_PKG_SIZE+1];
  sprintf(data,"%c%c%c%c%s",(char)PKG_DATA,
                (char) 0, //Sequence Number
                (char) 0, //N2 = SIZE_DATA * N2
                (char) 19, //N1 SIZE = N2*SIZE_DATA+N1
                "ola isto e um teste");*/


if(data[0] == (char)PKG_START || data[0] == (char)PKG_END){

  pkg.type = data[i];
  i++;

  if(data[i] != (char)FILE_SIZE){                 // SIZE
    perror("Wrong order of START pck received");
    exit(-1);
  }

  n_bytes = data[i+1];
  memcpy(tmp,data+2,n_bytes);
  pkg.size = atoi(tmp);
  i += n_bytes+2;

  if(data[i] != (char)FILE_NAME){                 
    perror("Wrong order of START pck received");  // NAME
    exit(-1);
  }

  n_bytes = data[i+1];
  memcpy(pkg.name,data+i+2,n_bytes);
  i+= n_bytes+2;

  if(data[i] != (char)FILE_DATE){                 // DATE
    perror("Wrong order of START pck received");
    exit(-1);
  }

  n_bytes = data[i+1];
  printf("NB Date - %d\n",n_bytes);
  memcpy(tmp,data+i+2,n_bytes);
  pkg.date = atoi(tmp);
  i+= n_bytes+2;

  if(data[i] != (char)FILE_PERM){                 // DATE
    perror("Wrong order of START pck received");
    exit(-1);
  }

  n_bytes = data[i+1];
  memcpy(tmp,data+i+2,n_bytes+1);   //because of /0
  pkg.perm = atoi(tmp);

  /* DEBUG
  printf("Type - %d\n",pkg.type);
  printf("NB Size - %d\n",n_bytes);
  printf("Size - %d\n", pkg.size);
  printf("NB Name - %d\n",n_bytes);
  printf("Name - %s\n", pkg.name);
  printf("Date - %d\n", pkg.date);
  printf("NB PERM - %d\n",n_bytes);
  printf("Perm - %d\n", pkg.perm);
  */
}
else if(data[0] == PKG_DATA){

  pkg.type = data[0];

  pkg.number = data[1];
  pkg.size = data[2]*DATA_SIZE+data[3];
  memcpy(pkg.data,data + 4,pkg.size);

  /* DEBUG */
  /*printf("Type - %d\n",pkg.type);
  printf("Number - %d\n",pkg.number);
  printf("N2 - %d\n", data[2]);
  printf("N1 - %d\n",data[3]);
  printf("Size - %d\n", pkg.size);
  printf("Data - %s\n", pkg.data);*/

}
else{
  perror("Package of unknown type");
  exit(-1);
}

return 0;
}

int sender(char* port, char* filepath){

	//inicializa o dataLink
    //init_linkLayer(port);

    int fd = 0;//llopen(port,0);		//0 - is not the receiver    DESCOMENTAR

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
   	if(sendControlPackage(PKG_END,fd,file_name,file_size,file_date,file_perm) == -1){
      perror("Could not send End Package");
      exit(-1);
    }

    if(llclose(fd,0) == -1){
        perror("Could not close port on sender");
        exit(-1);
    }
    close(file);

	return 0;
}

//int receiver(char* port){
int receiver(char* msg){

struct package pkg;
analizePackage("",pkg);
return 0;
}

	//inicializa o dataLink
  //init_linkLayer(port);
  /*int fd = 0; //llopen(port,1);		//1 - is the receiver       DESCOMENTAR!!!

  char *data = malloc(MAX_PKG_SIZE*sizeof(char));

  if(llread(fd,data) == -1){
    perror("Could not process START package");
    exit(-1);
  }

  if(data[0] != PKG_START){
    perror("First package recieved was not a START package");
    exit(-1);
  }

  //cria o ficheiro com as permissões de acesso

  char type = PKG_DATA;

    //Recieve DATA
    while(type == PKG_DATA){

      if(llread(fd,data) == -1){
        perror("Could not process START package");
        exit(-1);
      }

      type = data[0];

      //processar informação
      //escrever no ficheiro

    }

    if(type != PKG_END){
        perror("Last package recieved was not an END package");
        exit(-1);
    }

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

    if(llclose(fd,1) == -1){
        perror("Could not close port on sender");
        exit(-1);
    }

	return 0;
}*/