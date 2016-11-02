#include "applicationLayer.h"

static unsigned char sequenceNumber = 1;
static struct applicationLayer infoLayer;

int getFileSize(int file_descriptor){
  int size = 0, current_pos = 0;

  //get current position
  if((current_pos = lseek(file_descriptor,0,SEEK_CUR)) == -1){
    perror("Error at gathering the file size");
    return -1;
  }

  //get n bytes to end of file
  if((size = lseek(file_descriptor,0,SEEK_END)) == -1){
    perror("Error at gathering the file size");
    return -1;
  }

  //put position to previous position
  lseek(file_descriptor,0,current_pos);

  return size;
}

void loadingBar(float file_size_processed, float file_total_size) {
  float percentage = 100.0 * file_size_processed / file_total_size;

  printf("\rStatus: %6.2f%% [", percentage);

  int i, len = 50;
  int pos = percentage * len / 100.0;

  for (i = 0; i < len; i++){
  	if(i <= pos)
  		printf("=");
  	else
  		printf(" ");
  }
    

  printf("]");

  fflush(stdout);
}

int sendStartPackage(unsigned char * file_name, unsigned char * file_size, unsigned char * file_date, unsigned char * file_perm)
{
  int buffer_size = 9 + strlen(file_size) + strlen(file_name) + strlen(file_date) + strlen(file_perm);

  unsigned char *pkg = malloc(buffer_size);
  memset(pkg,0,buffer_size);

  int i = 0;

  pkg[i] = PKG_START;

  i++;

  pkg[i] = FILE_SIZE;
  pkg[i+1] = strlen(file_size);
  memcpy(pkg+i+2,file_size,strlen(file_size));

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("Type - %x\n",pkg[0]);
      printf("SizeT - %x\n",pkg[i]);             
      printf("SizeL - %x\n",pkg[i+1]);
      printf("SizeV - %s\n",pkg + i + 2);
  }
  
  i += 2 + strlen(file_size);

  pkg[i] = FILE_NAME;
  pkg[i+1] = strlen(file_name);
  memcpy(pkg+i+2,file_name,strlen(file_name));

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("NameT - %x\n",pkg[i]);             
      printf("NameL - %u\n",pkg[i+1]);
      printf("NameV - %s\n",pkg + i + 2);
  }

  i += 2 + strlen(file_name);

  pkg[i] = FILE_DATE;
  pkg[i+1] = strlen(file_date);
  memcpy(pkg+i+2,file_date,strlen(file_date));

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("DateT - %x\n",pkg[i]);             
      printf("DateL - %x\n",pkg[i+1]);
      printf("DateV - %s\n",pkg + i + 2);
  }

  i += 2 + strlen(file_date);

  pkg[i] = FILE_PERM;
  pkg[i+1] = strlen(file_perm);
  memcpy(pkg + i + 2,file_perm,strlen(file_perm));

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("PermT - %x\n",pkg[i]);               
      printf("PermL - %x\n",pkg[i+1]);
      printf("PermV - %s\n",pkg + i + 2);
  }
  
  if(infoLayer.mode == FULL_DEBUG){
      printf("Pacote %s\n LENGTH %d\n\n",pkg,buffer_size);
  }
  
  if(llwrite(infoLayer.fileDescriptor,pkg,buffer_size) == -1){
    return -1;
  }
  
  free(pkg);    
  return 0;
}

int sendEndPackage()
{
  unsigned char pkg[1];
  pkg[0] = PKG_END;

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("Type - %x\n",pkg[0]);
  }
  
  if(llwrite(infoLayer.fileDescriptor, pkg, 1) == -1){
    return -1;
  }
  
  return 0;
}

int sendDataPackage(unsigned char * data,unsigned int size)
{
  int buffer_size = 5 + size;
  unsigned char *pkg = malloc(buffer_size);
  memset(pkg,0,buffer_size);

  pkg[0] = PKG_DATA;
  pkg[1] = sequenceNumber;
  pkg[2] = size/DATA_SIZE;                  //L2
  pkg[3] = size-DATA_SIZE*pkg[2];             //L1    SizeT = L2*DataSize + L1
  memcpy(pkg + 4,data,size);

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("Type - %x\n",pkg[0]);        
      printf("Number - %x\n",pkg[1]);
      printf("Total Size - %d\n",pkg[2]*DATA_SIZE + pkg[3]);
      printf("SIZE - %d\n",size);
  }

  if(infoLayer.mode == FULL_DEBUG){
      printf("DATA - \n%s\n\n",pkg+4);
  }

  if(llwrite(infoLayer.fileDescriptor,pkg,buffer_size) == -1){
     return -1;
  }
  free(pkg);
  return 0;
}

int receiveStartPackage(struct package *p, unsigned char * data)
{
  int n_bytes = 0, i = 1;
  char tmp[20];

  if(data[i++] != FILE_SIZE){
    printf("Package doesn't respect order");
    return -1;
  }
  
  n_bytes = data[i++];
  memcpy(tmp,data + i,n_bytes);
  p->total_size = atoi(tmp);
  i+= n_bytes;

  if(data[i++] != FILE_NAME){
    printf("Package doesn't respect order");
    return -1;
  }

  n_bytes = data[i++];
  memcpy(p->file_name,data + i,n_bytes);
  i+= n_bytes;

  if(data[i++] != FILE_DATE){
    printf("Package doesn't respect order");
    return -1;
  }

  memset(tmp,0,strlen(tmp));
  n_bytes = data[i++];
  memcpy(tmp,data + i,n_bytes);
  p->file_date = atoi(tmp);
  i+= n_bytes;

  if(data[i++] != FILE_PERM){
    printf("Package doesn't respect order");
    return -1;
  }

  memset(tmp,0,strlen(tmp));
  n_bytes = data[i++];
  memcpy(tmp,data + i,n_bytes);
  p->file_perm = atoi(tmp);
  
  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("Type - %d\n",p->type);     
      printf("Size - %d\n",p->total_size);     
      printf("Name - %s\n",p->file_name);
      printf("Date - %d\n",p->file_date);
      printf("Perm - 0x%X\n",p->file_perm);
  }

  return 0;
}

int receiveDataPackage(struct package *p, unsigned char * data)
{
  int i = 1;
  int n2 = 0,n1 = 0;

  p->number = data[i++];

  n2 = data[i++]*DATA_SIZE;
  n1 = data[i++];

  p->size = n2+n1;
   
  memcpy(p->data,data + i,p->size);

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("Type - %d\n", p->type);      
      printf("SNumber - %d\n",p->number);
      printf("Size - %d\n",p->size);
      
  }
  if(infoLayer.mode == FULL_DEBUG){
      printf("DATA - %s\n",p->data);
      for( i = 0 ; i < p->size ; i++){
        printf("%c",p->data[i]);
      }
      printf("\n");
  }

  return 0;
}

int receivePackage(struct package *p)
{
  unsigned char* data = malloc(MAX_PKG_SIZE);
  memset(data,0,MAX_PKG_SIZE);

  if(llread(infoLayer.fileDescriptor,data) == -1){
    //printf("Could not read\n");
    return -1;
  }

  p->type = data[0];

  if(p->type == PKG_START){
    if(receiveStartPackage(p,data) == -1){
      printf("Could not receive START package");
      return -1;
    }
  }

  else if(p->type == PKG_DATA){
    if(receiveDataPackage(p,data) == -1){
      printf("Could not receive DATA package");
      return -1;
    }
  }
  else if(p->type != PKG_END){
    printf("Type of package received unknown");
    return -1;
  }

  free(data);
  return 0;
}

int sender()
{
  int file;

  //Open the file that it's going to be sent
  if((file = open(infoLayer.file_path, O_RDONLY)) == -1){      //Meter mais flags
    perror("Could not open file with filepath\n");
    return -1;
  }

  //get status struct from the file
  struct stat st;
  if(stat(infoLayer.file_path, &st) == -1){
    perror("Could not access file status\n");
    return -1;
  }

  unsigned char *file_name = basename(infoLayer.file_path);
  unsigned char file_size[16], file_date[16], file_perm[16];
  int size = getFileSize(file);

  sprintf(file_size,"%d",size);
  sprintf(file_date,"%lu",st.st_mtime);
  sprintf(file_perm,"%u",st.st_mode);

  //PACKAGE START
  if(sendStartPackage(file_name,file_size,file_date,file_perm) == -1){
  	printf("Could not send START Package\n");
  	return -1;
  }

  int STOP = 0, r = 0;
  int written = 0;
  unsigned char *data = (unsigned char*) malloc(DATA_SIZE);

  while(r = read(file,data,DATA_SIZE)){
    if(r == -1){
      perror("Could not read file\n");
      return -1;
    }

    if(sendDataPackage(data,r) == -1){  
      printf("Could not send DATA package\n");
      return -1;
    }
    sequenceNumber++;
    memset(data,0,DATA_SIZE);
    written+=r;

    if(infoLayer.mode == NORMAL){
      loadingBar(written,size);
    } 
  }

  free(data);

  //creates and sends package END
  if(sendEndPackage() == -1){
    printf("Could not send END Package\n");
    return -1;
  }

  if(written != size){
    printf("Data size and data read are diferent\n");
    return -1;
  }

  if(llclose(infoLayer.fileDescriptor,infoLayer.status) == -1){
    printf("Could not close port\n");
    return -1;
  }

  if(infoLayer.mode == NORMAL){
      printf("\nFile sent!\n");
    } 

  return 0;
}

int receiver(unsigned char *path){

  int file;

  struct package * pkg = malloc(sizeof(struct package));

  if(receivePackage(pkg)){
    printf("Error at receiving the START package\n");
    return -1;
  } 
  
  sprintf(path,"%s/%s",infoLayer.file_path,pkg->file_name);


  printf("Path: %s\n", path);

  //pkg->file_perm
  if((file = open(path, O_TRUNC | O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO)) == -1){  //FALTAM FLAGS
    perror("Could not create the file in the receiver\n");
    return -1;
  }
  
  //create the file with the same permissions
  if (fchmod(file, pkg->file_perm))
  {
    perror("chmod");
    return -1;
  }

  //set the last time modification
  struct utimbuf new_times;       
  struct stat st;

  stat(path, &st);

  new_times.actime = st.st_atime;  
  new_times.modtime = pkg->file_date;    
  utime(path, &new_times);

  if(infoLayer.mode == SIMPLE_DEBUG || infoLayer.mode == FULL_DEBUG){
      printf("Permissions - %lu - %d\n",st.st_mode,pkg->file_perm); 
      printf("Date - %lu - %d\n",st.st_mtime,pkg->file_date);
  }

  int written = 0;        //written per package
  int data_received = 0;    //written in the final
  
  //PACKAGES DATA

  while(1){
        
    written = 0;
    memset(pkg->data,0,DATA_SIZE);
    if(receivePackage(pkg) == -1){
      printf("Could not receive DATA package\n");
      return -1;
    }

    if(pkg->type != PKG_DATA){
      if(pkg->type == PKG_END)
        break;
      else{
        printf("Last package not an END package\n");
        return -1;
      }
    }

    if(pkg->number != sequenceNumber){
      printf("Wrong order of the DATA packages received (%u != %u)\n", pkg->number, sequenceNumber);
      return -1;
    }

    sequenceNumber++;

    //Write data on the created file
    do{
		if((written += write(file , pkg->data+written , pkg->size)) == -1){
			perror("Could not write in the created file of receiver\n");
			return -1;
		}
    }
    while(written < pkg->size);

    data_received += written;     //confirmar com package end
    
    if(infoLayer.mode == NORMAL){
      loadingBar(data_received,pkg->total_size);
    } 
  };    

  //Checks if the data received has the sama size of the original file
  int filesize = getFileSize(file);
  if (filesize == -1){
  	return -1;
  }
  
  if(filesize != pkg->total_size){
    printf("File size doesn't match info in start package: %d/%d\n", filesize, pkg->total_size);  
  }  
  
  if(llclose(infoLayer.fileDescriptor,infoLayer.status) == -1){
    printf("Could not close port\n");
    return -1;
  }
  free(pkg);
  close(file);

  if(infoLayer.mode == NORMAL){
      printf("\nFile received!\n");
  } 
  
  return 0;
}

int initApplicationLayer(unsigned char *port, int status, int mode,unsigned char * file_path)
{
  infoLayer.status = status;
  infoLayer.mode = mode;
  memcpy(infoLayer.file_path, file_path, strlen(file_path));

  //inicializa o dataLink
  init_linkLayer(port,infoLayer.mode);

  if((infoLayer.fileDescriptor = llopen(port,status)) == -1){
    printf("Could not open connection\n");
    return -1;
  }

  if(status == TRANSMITTER){
    return sender();
  }
  
  else if(status == RECEIVER){
  	int res;
  	unsigned char newFilePath[256];
  	res = receiver(newFilePath);
  	if (res == -1 && strlen(newFilePath) > 0 ){
  		remove(newFilePath);
  	}
  	return res;
  }
  else{
    printf("Wrong status (RECEIVER - 1 or TRANSMITTER - 0\n");
    return -1;
  }
}
