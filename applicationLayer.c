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

    char *pkg = malloc(buffer_size);
    memset(pkg,0,buffer_size);

    int i = 0;

    pkg[i] = control;

    printf("Type - %x\n",pkg[0]);               //DEBUG

    i++;

    pkg[i] = FILE_SIZE;
    pkg[i+1] = strlen(file_size);
    memcpy(pkg+i+2,file_size,strlen(file_size));

    printf("SizeT - %x\n",pkg[i]);                //DEBUG
    printf("SizeL - %x\n",pkg[i+1]);
    printf("SizeV - %s\n",pkg + i + 2);

    i += 2 + strlen(file_size);

    pkg[i] = FILE_NAME;
    pkg[i+1] = strlen(file_name);
    memcpy(pkg+i+2,file_name,strlen(file_name));

    printf("NameT - %x\n",pkg[i]);                //DEBUG
    printf("NameL - %x\n",pkg[i+1]);
    printf("NameV - %s\n",pkg + i + 2);

    i += 2 + strlen(file_name);

    pkg[i] = FILE_DATE;
    pkg[i+1] = strlen(file_date);
    memcpy(pkg+i+2,file_date,strlen(file_date));

    printf("DateT - %x\n",pkg[i]);                //DEBUG
    printf("DateL - %x\n",pkg[i+1]);
    printf("DateV - %s\n",pkg + i + 2);

    i += 2 + strlen(file_date);

    pkg[i] = FILE_PERM;
    pkg[i+1] = strlen(file_perm);
    memcpy(pkg + i + 2,file_perm,strlen(file_perm));

    printf("PermT - %x\n",pkg[i]);                //DEBUG
    printf("PermL - %x\n",pkg[i+1]);
    printf("PermV - %s\n",pkg + i + 2);

    /*printf("Pacote %s\n LENGTH %d\n\n",pkg,buffer_size);*/

      if(llwrite(infoLayer.fileDescriptor,pkg,buffer_size) == -1){
        perror("Could not send control PACKAGE");
        return -1;
      }
      free(pkg);
  return 0;
}

int sendDataPackage(char * data,unsigned int size)
{
  int buffer_size = 4 + size;
  char *pkg = malloc(buffer_size);

  pkg[0] = PKG_DATA;
  pkg[1] = sequenceNumber;
  pkg[2] = size/DATA_SIZE;                  //L2
  pkg[3] = size-DATA_SIZE*pkg[2];             //L1    SizeT = L2*DataSize + L1
  memcpy(pkg + 4,data,size);

  printf("Type - %x\n",pkg[0]);         //DEBUG
  printf("Number - %x\n",pkg[1]);
  printf("Total Size - %d\n",pkg[2]*DATA_SIZE + pkg[3]);
  printf("SIZE - %d\n",size);
  printf("DATA - \n%s\n\n",pkg+4);

  if(llwrite(infoLayer.fileDescriptor,pkg,buffer_size) == -1){
     perror("Could not send DATA PACKAGE");
     return -1;
  }
  free(pkg);
  return 0;
}

int receiveControlPackage(struct package *p,char * data){

   int n_bytes = 0, i = 1;
   char tmp[20];

      if(data[i++] != FILE_SIZE){
        perror("Package don't respect order");
        return -1;
      }
      n_bytes = data[i++];
      memcpy(tmp,data + i,n_bytes);
      p->total_size = atoi(tmp);
      i+= n_bytes;


      if(data[i++] != FILE_NAME){
        perror("Package don't respect order");
        return -1;
      }
      n_bytes = data[i++];
      memcpy(p->file_name,data + i,n_bytes);
      i+= n_bytes;


      if(data[i++] != FILE_DATE){
        perror("Package don't respect order");
        return -1;
      }
      memset(tmp,0,strlen(tmp));
      n_bytes = data[i++];
      memcpy(tmp,data + i,n_bytes);
      p->file_date = atoi(tmp);
      i+= n_bytes;


      if(data[i++] != FILE_PERM){
        perror("Package don't respect order");
        return -1;
      }
      memset(tmp,0,strlen(tmp));
      n_bytes = data[i++];
      memcpy(tmp,data + i,n_bytes);
      p->file_perm = atoi(tmp);

      
      printf("Type - %d\n",p->type);        //DEBUG
      printf("Size - %d\n",p->total_size);     
      printf("Name - %s\n",p->file_name);
      printf("Date - %d\n",p->file_date);
      printf("Perm - %d\n",p->file_perm);

    return 0;
}

int receiveDataPackage(struct package *p, char * data){
   int i = 1;
   int n2 = 0,n1 = 0;

      p->number = data[i++];

      n2 = data[i++]*DATA_SIZE;
      n1 = data[i++];

      p->size = n2+n1;
   
      memcpy(p->data,data + i,p->size);
    

      printf("Type - %d\n", p->type);             //DEBUG
      printf("SNumber - %d\n",p->number);
      printf("Size - %d\n",p->size);
      printf("Data:\n%s\n",p->data);

  return 0;
}

int receivePackage(struct package *p){
  char* data = malloc(MAX_PKG_SIZE);
  memset(data,0,MAX_PKG_SIZE);

  if(llread(infoLayer.fileDescriptor,data) == -1){
      perror("Could not read");
      return -1;
    }

    p->type = data[0];
    printf("Type - %d\n",data[0]);

    if(p->type == PKG_START || p->type == PKG_END){
      if(receiveControlPackage(p,data) == -1){
        perror("Could not receive CONTROL package");
        return -1;
      }
    }
    else if(p->type == PKG_DATA){
        if(receiveDataPackage(p,data) == -1){
          perror("Could not receive DATA package");
          return -1;
        }
    }
    else{
      perror("Type of package received unknown");
      return -1;
    }

    free(data);
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

      int size = getFileSize(file);

      sprintf(file_size,"%d",size);
      printf("%s\n",file_size);
      sprintf(file_date,"%lu",st.st_mtime);
      sprintf(file_perm,"%u",st.st_mode);

      //PACKAGE START
     if(sendControlPackage(PKG_START,file_name,file_size,file_date,file_perm) == -1){
        perror("Could not send Start Package");
        exit(-1);
      }

      int STOP = 0, r = 0;
      int written = 0;
      
      char *data = (char*) malloc(DATA_SIZE);

      while(r = read(file,data,DATA_SIZE)){

        if(r == -1){
          perror("Could not read file");
          exit(-1);
        }

        if(sendDataPackage(data,r) == -1){  
          perror("Could not send DATA package");
          exit(-1);
        }
        sequenceNumber++;
        memset(data,0,DATA_SIZE);
        written+=r;
      }
      free(data); 

      //creates and sends package END
      if(sendControlPackage(PKG_END,file_name,file_size,file_date,file_perm) == -1){
        perror("Could not send Start Package");
        exit(-1);
      }

      if(written != size){
        perror("Data size and data read are diferent");
        exit(-1);
      }

      if(llclose(infoLayer.fileDescriptor,infoLayer.status) == -1){
        perror("Could not close port");
        exit(-1);
      }
      
    return 0;
}

int receiver(){
	
  int file;

  struct package pkg;

  if(receivePackage(&pkg)){
    perror("Error at receiving the START package");
    exit(-1);
  } 

  //create the file with the same permissions
  char path[128];
  sprintf(path,"%s/%s",infoLayer.file_path,pkg.file_name);
//pkg->file_perm
  if((file = open(path,O_APPEND | O_CREAT | O_WRONLY,S_IRWXU | S_IRWXG | S_IRWXO)) == -1){  //FALTAM FLAGS
    perror("Could not create the file in the receiver");
    exit(-1);
  }

  //set the last time modification
  struct utimbuf new_times;       
  struct stat st;

  stat(path, &st);

    new_times.actime = st.st_atime;  
    new_times.modtime = pkg.file_date;    
    utime(path, &new_times);

    //ver mais tarde
    /*printf("Permissions - %lu - %d\n",st.st_mode,pkg->file_perm);   //DEBUG
    printf("Date - %lu - %d\n",st.st_mtime,pkg->file_date);*/

    int type = 0;
    int written = 0;        //written per package
    int data_received = 0;    //written in the final

      //PACKAGES DATA
    struct package *p = malloc(sizeof(struct package));

     while(1){
        
        written = 0;

        if(receivePackage(p) == -1){
          perror("Could not receive DATA package");
          exit(-1);
        }

        if(p->type == PKG_END){
          break;
        }
        else if(p->type != PKG_DATA){
          break;
        }

        if(p->number != sequenceNumber){
          perror("Wrong order of the DATA packages received");
          exit(-1);
        }

        sequenceNumber++;

        //Write data on the created file
        do{      
          if((written += write(file , p->data+written , p->size)) == -1){
            perror("Could not write in the created file of receiver");
            exit(-1);
          }
        }while(written != p->size);

        data_received += written;     //confirmar com package end
        memset(p,0,sizeof(struct package));
      };
      free(p);

      //Checks if the data received has the sama size of the original file

        if(llclose(infoLayer.fileDescriptor,infoLayer.status) == -1){
        perror("Could not close port");
        exit(-1);
        }
        
      close(file);
	return 0;
}

int initApplicationLayer(char *port,int status, char * file_path){
  infoLayer.status = status;
  memcpy(infoLayer.file_path,file_path,strlen(file_path));

  //inicializa o dataLink
  init_linkLayer(port);

  if((infoLayer.fileDescriptor = llopen(port,status)) == -1){
    perror("Could not open port of the receiver");
    exit(-1);
  }

  if(status == TRANSMITTER){
    sender();
  }
  else if(status == RECEIVER){
    receiver();
  }
  else{
    perror("Wrong status (RECEIVER - 1 or TRANSMITTER - 0");
    exit(-1);
  }

  return 0;
}
