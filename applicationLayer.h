#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <utime.h>
#include "linkLayer.h"

/*@brief struct that contains the essencial information to be used on the applicationLayer
 @attr	fileDescriptor - File descritor to write and read information from
 @attr	status - To know if the user is the receiver or the sender
 @attr	mode - For debug details
 @attr 	file_path - Path of the file to be sent or to be saved
*/
struct applicationLayer {
	int fileDescriptor;  			
	int status; 		 				/*TRANSMITTER | RECEIVER*/
	int mode;			 				/*NORMAL | SIMPLE_DEBUG | FULL DEBUG*/
	unsigned char file_path[128]; 		
};

/*@brief struct that contains the separated information about a package
 @attr type - Type of package
 @attr data - Part of information of a file
 @attr number - Sequence number of the package
 @attr size - Size in bytes of the data
 @attr file_name - Name of the file
 @attr total_size - Total size of the file to be sent/receiveControlPackage
 @attr file_date - Last modified date of the file to be sent/received
 @attr file_perm - Permissions of acess of the file to be sent/received
*/
struct package{
	int type;							/*PKG_START | PKG_END | PKG_DATA*/
	//Type = PKG_DATA
	unsigned char *data;			
	unsigned char number;			
	int size;						
	//Type = PKG_START
	unsigned char file_name[36];	
	int total_size;					
	int file_date;		
	int file_perm;					
};

/*@brief Function that codifies the information of a file into a control start package structure and sends
 to fileDescriptor of infoLayer (global applicationLayer struct) using llwrite(...)
 @control package structure - |TYPE|FILE_SIZE|LENGTH SIZE|SIZE|FILE_NAME|LENGTH NAME|NAME|FILE_DATE|LENGTH DATE|DATE|FILE_PERM|LENGTH PERM|PERM|
 @param	file_name - Name of the file to be sent
 @param file_size - Total size of the file to be sent
 @param file_date - Last modified date of the file to be sent
 @param file_perm - Permissions of acess of the file to be sent
 @return 0 on sucess or -1 otherwise
*/
int sendStartPackage(unsigned char * file_name, unsigned char * file_size, unsigned char * file_date, unsigned char * file_perm);

/*@brief Function that codifies the information of a file into a control end package structure and sends it
 to fileDescriptor of infoLayer (global applicationLayer struct) using llwrite(...)
 @control package structure - |TYPE|
 @return 0 on sucess or -1 otherwise
*/
int sendEndPackage();

/*@brief Function that codifies the information of a file into a data package structure and sends it
 to fileDescriptor of infoLayer (global applicationLayer struct) using llwrite(...)
 @data package structure - |TYPE|SEQ NUMBER|SIZE/256|SIZE - SIZE/256|DATA|
 @param	data - Information to be sent
 @param size - Size of the information to be sent
 @return 0 on sucess or -1 otherwise
*/
int sendDataPackage(unsigned char *data, unsigned int size);

/*@brief Function that decodifies the information in data (in the form of a control package structure) and saves it in a struct package
 @control package structure - |TYPE|FILE_SIZE|LENGTH SIZE|SIZE|FILE_NAME|LENGTH NAME|NAME|FILE_DATE|LENGTH DATE|DATE|FILE_PERM|LENGTH PERM|PERM|
 @param p - Struct package where the information contained in the data will be saved
 @param	data - Information to be decodified
 @return 0 on sucess or -1 otherwise
*/
int receiveControlPackage(struct package *p,unsigned char * data);

/*@brief Function that decodifies the information in data (in the form of a data package structure) and saves it in a struct package
 @control package structure - |TYPE|FILE_SIZE|LENGTH SIZE|SIZE|FILE_NAME|LENGTH NAME|NAME|FILE_DATE|LENGTH DATE|DATE|FILE_PERM|LENGTH PERM|PERM|
 @param p - Struct package where the information contained in the data will be saved
 @param	data - Information to be decodified
 @return 0 on sucess or -1 otherwise
*/
int receiveDataPackage(struct package *p, unsigned char * data);

/*@brief Function that receives a package using llread from the fileDescriptor of infoLayer (global applicationLayer struct)
 @control package structure - |TYPE|FILE_SIZE|LENGTH SIZE|SIZE|FILE_NAME|LENGTH NAME|NAME|FILE_DATE|LENGTH DATE|DATE|FILE_PERM|LENGTH PERM|PERM|
 @param p - Struct package where the information contained in the data will be saved
 @param	data - Information to be decodified
 @return 0 on sucess or -1 otherwise
*/
int receivePackage(struct package *p);

/*@brief Function that sends a file to the fileDescriptor of infoLayer (global applicationLayer struct)
 the file path is in the file_path of infoLayer (global applicationLayer struct)
 In the end it will close the fileDescriptor with llclose(...)
 @return 0 on sucess or -1 otherwise
*/
int sender();

/*@brief Function that receives a file from the fileDescriptor of infoLayer (global applicationLayer struct)
 the file path where the file will be created and edited is in the file_path of infoLayer (global applicationLayer struct)
 In the end it will close the fileDescriptor with llclose(...)
 @return 0 on sucess or -1 otherwise
*/
int receiver();

/*@brief Function initializes the global struct applicationLayer infoLayer with the parameters received
 opens the fileDescritor with llopen(...) and calls sender() or receiver() depending on the status
 @param port - Port to be used in the conection
 @param status - if it's the trasmitter or the receiver
 @param mode - debugging details
 @param max_size - Max size of the information to be sent about the file in each data package
 @param file_path - Path of the file to be sent/received
 @return 0 on sucess or -1 otherwise
*/
int initApplicationLayer(unsigned char * port,int status, int mode,int max_size,unsigned char * file_path);
