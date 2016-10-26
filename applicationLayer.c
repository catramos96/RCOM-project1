#include "applicationLayer.h"



/* IMPORTANTE ======================================================================================
Ver se o tamanho das STRINGS incluiem o +1 (para \0)
====================================================================================================*/


int getFileSize(int file_descriptor){
  int size = 0, current_pos = 0;

  //get current position
  if((current_pos = lseek(file_descriptor,0,SEEK_CUR)) == -1){
    perror("Error at gathering the file size");
    exit(-1);
  }

  //get n bytes to end of file
  if((size = lseek(file_descriptor,0,SEEK_END)) == -1){
    perror("Error at gathering the file size");
    exit(-1);
  }

  //put position to previous position
  lseek(file_descriptor,0,current_pos);

  return size;
}

int sendControlPackage(char control,char * file_name, char * file_size, char * file_date, char * file_perm)
{
    int buffer_size = 9 + strlen(file_size) + strlen(file_name) + strlen(file_date) + strlen(file_perm);

    char pkg[buffer_size];
    int i = 0,j = 0;

    pkg[i] = control;

    /*printf("Type - %x\n",pkg[0]);*/               //DEBUG

    i++;

    pkg[i] = FILE_SIZE;
    pkg[i+1] = strlen(file_size);
    memcpy(pkg+i+2,file_size,strlen(file_size));

    /*printf("SizeT - %x\n",pkg[i]);                //DEBUG
    printf("SizeL - %x\n",pkg[i+1]);
    printf("SizeV - %s\n",pkg + i + 2);*/

    i += 2 + strlen(file_size);

    pkg[i] = FILE_NAME;
    pkg[i+1] = strlen(file_name);
    memcpy(pkg+i+2,file_name,strlen(file_name));

    /*printf("NameT - %x\n",pkg[i]);                //DEBUG
    printf("NameL - %x\n",pkg[i+1]);
    printf("NameV - %s\n",pkg + i + 2);*/

    i += 2 + strlen(file_name);

    pkg[i] = FILE_DATE;
    pkg[i+1] = strlen(file_date);
    memcpy(pkg+i+2,file_date,strlen(file_date));

    /*printf("DateT - %x\n",pkg[i]);                //DEBUG
    printf("DateL - %x\n",pkg[i+1]);
    printf("DateV - %s\n",pkg + i + 2);*/

    i += 2 + strlen(file_date);

    pkg[i] = FILE_PERM;
    pkg[i+1] = strlen(file_perm);
    memcpy(pkg + i + 2,file_perm,strlen(file_perm));

    /*printf("PermT - %x\n",pkg[i]);                //DEBUG
    printf("PermL - %x\n",pkg[i+1]);
    printf("PermV - %s\n",pkg + i + 2);

    printf("Pacote %s\n LENGTH %d\n\n",pkg,buffer_size);*/

      /*if(llwrite(infoLayer.fileDescriptor,pkg,buffer_size) == -1){
        perror("Could not send control PACKAGE");
        exit(-1);
      }*/

  return 0;
}

int sendDataPackage(char * data, int sequenceN,unsigned int size)
{
	int buffer_size = 4 + size;
  char pkg[buffer_size];

  pkg[0] = PKG_DATA;
  pkg[1] = sequenceN;
  pkg[2] = size/DATA_SIZE;                  //L2
  pkg[3] = size-size/DATA_SIZE;             //L1    SizeT = L2*DataSize + L1
  memcpy(pkg + 4,data,size);

  /*printf("Type - %x\n",pkg[0]);         //DEBUG
  printf("Number - %x\n",pkg[1]);
  printf("L2 - %x\n",pkg[2]);
  printf("L1 - %x\n",pkg[3]);
  printf("Total Size - %x\n",size);
  printf("DATA - \n%x\n",pkg + 4);*/
 
  /*if(llwrite(infoLayer.fileDescriptor,pkg,buffer_size) == -1){
     perror("Could not send DATA PACKAGE");
     exit(-1);
  }*/
  
  return 0;
}

int receiveControlPackage(int type, char *  name, int size){

 // char * data = malloc(MAX_PKG_SIZE); //= malloc(sizeof(char)*MAX_PKG_SIZE);
	char *data = malloc(MAX_PKG_SIZE);
  	if(llread(infoLayer.fileDescriptor,data) == -1){
      perror("Could not read");
      exit(-1);
    }

    printf("%c\n",data[0]);

    //printf("%s\n",data);

    /*if(type == PKG_START || type == PKG_END){
      printf("OK\n");

    }
    else{
      perror("Type of package received unknown");
      exit(-1);
    }*/

    //free(data);
    return 0;
}

int receiveDataPackage(int type, int size, char * data){

  return 0;
}

int sender(){
      int file;

      //Open the file that it's going to be sent
      if((file = open(infoLayer.file_path, O_RDONLY)) == -1){      //Meter mais flags
        perror("Could not open file with filepath");
        exit(-1);
      }

      //get status struct from the file
      struct stat st;
      if(stat(infoLayer.file_path,&st) == -1){
        perror("Could not acess file status");
        exit(-1);
      }

      //get file_name position
      int pos = 0, i;
      for(i = 0; i < strlen(infoLayer.file_path);i++){
        if(infoLayer.file_path[i] == '/'){
          pos = i;
        }
      }
      
      //information about the file
      char *file_name = infoLayer.file_path+pos+1;
      char file_size[16], file_date[16], file_perm[16];
      
      sprintf(file_size,"%d",getFileSize(file));
      sprintf(file_date,"%lu",st.st_mtime);
      sprintf(file_perm,"%u",st.st_mode);

      //PACKAGE START
     if(sendControlPackage(PKG_START,file_name,file_size,file_date,file_perm) == -1){
        perror("Could not send Start Package");
        exit(-1);
      }

     
      /*int STOP = 0, r = 0;
      sequenceNumber = 1;

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
      }*/

     /* printf("A preparar END\n");

      //creates and sends package END
      if(sendControlPackage(PKG_END,fd,file_name,file_size,file_date,file_perm) == -1){
        perror("Could not send End Package");
        exit(-1);
      }

      printf("END enviado!\n");

      if(llclose(fd,0) == -1){
          perror("Could not close port on sender");
          exit(-1);
      }
      */
    return 0;
}

int receiver(){
	
  int file, size, type;
  char *data = malloc(DATA_SIZE);

  printf("A analizePackage\n");

  if(receiveControlPackage(type,data,size)){
    perror("Error at receiving the START package");
    exit(-1);
  }

  //printf("%d\n",type);
  

  //Checks if the package is the START
  /*if(pkg->type != PKG_START){
    perror("First package received was not the START package");
    exit(-1);
  }

  //create the file with the same permissions
  char path[128];
  sprintf(path,"%s/%s",filepath,pkg->name);

  if((file = open(path,O_APPEND | O_CREAT | O_WRONLY,pkg->perm)) == -1){
    perror("Could not create the file in the receiver");
    exit(-1);
  }

  //set the last time modification
  struct utimbuf new_times;
  struct stat st;

  stat(path, &st);
  */
    /*new_times.actime = st.st_atime;  
    new_times.modtime = pkg->date;    
    utime(pkg->name, &new_times);

    int type = PKG_DATA;
    int written = 0;        //written per package
    int data_sent = 0;    //written in the final
    int last_seq_number = -1;*/

    //DEBUG
    /*char d2[MAX_PKG_SIZE+1];
    sprintf(d2,"%c%c%c%c%s",(char)PKG_DATA,
                  (char) 0, //Sequence Number
                  (char) 0, //N2 = SIZE_DATA * N2
                  (char) 19, //N1 SIZE = N2*SIZE_DATA+N1
                  "ola isto e um teste");*/

      //PACKAGES DATA
      /*while(type == PKG_DATA){

        written = 0;

        if(llread(fd,data) == -1){
          perror("Could not process START package");
          exit(-1);
        }

        //analizePackage(d2,pkg); //DEBUG

        analizePackage(data,pkg);

        if(last_seq_number+1 != pkg->number){
          perror("Packages data with sequence number not ordenated");
          exit(-1);
        }

        last_seq_number++;  //to keep the order
        type = pkg->type;

        //Write data on the created file
        do{       //PRECISO CICLO ?
          if((written += write(file , pkg->data+written , pkg->size)) == -1){
            perror("Could not write in the created file of receiver");
            exit(-1);
          }
        }while(written != pkg->size);

        data_sent += written;     //confirmar com package end
      };*/

     
      //PACKAGE END
      /*if(llread(fd,data) == -1){
          perror("Could not process START package");
          exit(-1);
        }

      analizePackage(data,pkg);

      //Checks if the package is the END
      if(pkg->type != PKG_END){
          perror("Last package recieved was not an END package");
          exit(-1);
      }

      //Checks if the data received has the sama size of the original file
      if(pkg->size != data_sent){
        perror("The size of the file and the data sent dont mach");
        exit(-1);
      }

      if(llclose(fd,1) == -1){
          perror("Could not close port on sender");
          exit(-1);
      }
  */
      //close(file);
	return 0;
}

int initApplicationLayer(char *port,int status, char * file_path){
  infoLayer.status = (int)status;
  memcpy(infoLayer.file_path,file_path,strlen(file_path));

  //inicializa o dataLink
  init_linkLayer(port);

  if((infoLayer.fileDescriptor = llopen(port,status)) == -1){
    perror("Could not open port of the receiver");
    exit(-1);
  }

  printf("Status - %d\nFilePath - %s\n",infoLayer.status, infoLayer.file_path);

  if(status == TRANSMITTER){
    printf("TRANSMITTER\n");
    sender();
  }
  else if(status == RECEIVER){
    printf("RECEIVER\n");
    receiver();
  }
  else{
    perror("Wrong status (RECEIVER - 1 or TRANSMITTER - 0");
    exit(-1);
  }

  close(infoLayer.fileDescriptor);

  return 0;
}