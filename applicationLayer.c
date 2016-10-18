#include "applicationLayer.h"

/*argumentos
– filename: nome de ficheiro
– filesize: tamanho do ficheiro
- date: data do ficheiro
- permissions: permissoes do ficheiro
retorno
– pacote de controlo inicial com as informacoes do ficheiro  */
applicationPackage createStartPackage(char * filename, char * filesize, char * date, char * permissions)
{
	char content[255]; //C + T1 + L1 + V1*4 + T2 + L2 + V2*(strlen) + T3 + L3 + V3*4 + T4 + L4 + V4*2 = 19 + strlen => max = 274 
	content[0] = 2; 							//C  - 2 (START)
	content[1] = 0;								//T1 - 0 (tamanho)
	content[2] = 4;								//L1 - tamanho de V1
	memcpy(content + 3, filesize, 4);			//V1 - tamanho do ficheiro
	
	int L2 = strlen(filename);
	content[7] = 1;								//T2 - 1 (nome)
	content[8] = L2;							//L2 - tamanho de V2
	memcpy(content + 9, filename, L2);			//V2 - nome do ficheiro
	
	content[9 + L2] = 2;						//T3 - 2 (data)
	content[10 + L2] = 4;						//L3 - tamanho de V3
	memcpy(content + 11 + L2, date, 4);			//V3 - data do ficheiro
	
	content[15 + L2] = 3;						//T4 - 3 (permissoes)
	content[16 + L2] = 2;						//L4 - tamanho de V3
	memcpy(content + 17 + L2, permissions, 2);	//V4 - permissoes do ficheiro
	
	applicationPackage package;
	package.content = content;
	package.size = 19 + L2;
	
	return package;
}

/*argumentos
– data: parcela de dados do ficheiro
- size: tamanho do pacote de dados
retorno
– pacote de dados do ficheiro a enviar */
applicationPackage createDataPackage(char * data, unsigned int size)
{
	char content[255];
	content[0] = 1;							//C
	content[1] = sequenceNumber++;			//N
	memcpy(content + 2, (char*)&size, 2);	//L2 L1
	memcpy(content + 4, data, size);
	
	applicationPackage package;
	package.content = content;
	package.size = size;
	
	return package;
}

/*retorno
– pacote de controlo para sinalizar fim do envio do ficheiro */
applicationPackage createEndPackage()
{
	char content[1] = { 3 };   // 3 (END)
	applicationPackage package;
	package.content = content;
	package.size = 1;
	
	return package;
}

int sender(char* port, char* filepath){

	//inicializa o dataLink
    init_linkLayer(port);

    int fd = llopen(port,0);		//0 - is not the receiver

    int file;

    if((file = open(filepath, O_RDONLY)) == -1){			//Meter mais flags
    	perror("Could not open file with filepath");
    	exit(-1);
    }

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
   	char file_data[16];
   		sprintf(file_data,"%ld",st.st_mtime);
   	char file_size[16];
   		sprintf(file_size,"%ld",st.st_size);
   	char file_premissions[16];
   		sprintf(file_premissions,"%d",st.st_mode);


   	//creates and sends package START
   	applicationPackage apk_start = createStartPackage(file_name,file_size,file_data,file_premissions);
   	if(llwrite(fd,apk_start,strlen(apk_start)) == -1){
   		perror("Could not send START PACKAGE");
   		exit(-1)
   	}

   	/*
	While(!end of file)
	{
		if(read() < 245)
			end of file = 1
		llwrite pck dados
	}
	llwrite pkg end
	llclose

   	*/

   	//creates and sends package END
   	applicationPackage apk_end = createEndPackage();
   	if(llwrite(fd,apk_start,strlen(apk_end)) == -1){
   		perror("Could not send END PACKAGE");
   		exit(-1)
   	}

    close(fd);

	return 0;
}

int reciever(char* port){

	return 0;
}

