#include "linkLayer.h"

unsigned char* build_frame_SU(ControlFieldType flag);

unsigned char* build_frame_I(unsigned char* data, unsigned int data_length);

ReturnType receive(int fd, Message *msg);

unsigned char getControlField(ControlFieldType flag);

ControlFieldType setControlField(unsigned char c);

int stuff(unsigned char *frame, int frame_length);

int desstuff(unsigned char *frame, int frame_length);

void display(unsigned char *frame, int n);