# exemplo de makefile com variaveis
# variaveis
INCLUDES = applicationLayer.h linkLayer.h constants.h
SOURCES = main.c linkLayer.c applicationLayer.c linkLayerAux.c
OBJFILES = main.o linkLayer.o applicationLayer.o linkLayerAux.o
EXEC = main

# regras de sufixo
.SUFFIXES : .c .o
.SUFFIXES : .s .o

# como transformar um .c num .o ; $< -- nome do ficheiro
.c.o:
	gcc -Wall -g -c $<
.s.o:
	gcc -Wall -g -c $<
	
${OBJFILES}: ${SOURCES} ${INCLUDES}

${EXEC}: ${OBJFILES}
	gcc -Wall -g -o ${EXEC} ${OBJFILES}
	
run: ${EXEC}
	./${EXEC}
	
clean:
	rm -f ${OBJFILES} ${EXEC}
	
all: ${EXEC}