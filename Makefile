all: main.c 
	  gcc -o main linkLayer.c linkLayerAux.c applicationLayer.c main.c
