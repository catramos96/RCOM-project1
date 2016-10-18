#include "applicationLayer.h"

/*argumentos
– filename: nome de ficheiro
– filesize: tamanho do ficheiro
- date: data do ficheiro
- permissions: permissoes do ficheiro
retorno
– pacote de controlo inicial com as informacoes do ficheiro  */
applicationPackage CreateStartPackage(char * filename, char * filesize, char * date, char * permissions)
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
	
	applicationPackage package { content, 19 + L2};
	
	return package;
}

/*argumentos
– data: parcela de dados do ficheiro
- size: tamanho do pacote de dados
retorno
– pacote de dados do ficheiro a enviar */
applicationPackage CreateDataPackage(char * data, unsigned int size);
{
	char content[255];
	content[0] = 1;							//C
	content[1] = sequenceNumber++;			//N
	memcpy(content + 2, (char*)&size, 2);	//L2 L1
	memcpy(content + 4, data, size);
	
	applicationPackage package { content, size};
	
	return package;
}

/*retorno
– pacote de controlo para sinalizar fim do envio do ficheiro */
applicationPackage CreateEndPackage();
{
	char content[1] = { 3 };   // 3 (START)
	applicationPackage package { content, 1};
	
	return package;
}