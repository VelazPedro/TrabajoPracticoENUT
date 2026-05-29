#ifndef FUNCIONES_H_INCLUDED
#define FUNCIONES_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LINEA 65536
#define MAX_COLUMNAS 512
#define MAX_NOMBRES 64
#define SEPARADOR " "
#define MAX_BYTES 16

typedef struct{
    char nombre[MAX_NOMBRES];
    int columna;
} IndiceColumna;

int ArchivoAbrir(FILE **fp, const char *nombreArchivo, const char *modoApertura, int mostrarError);
int contarFilas(FILE *fp,  char *linea);
void sacarComillas(char *s);
int crearIndice(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],char* linea);
void guardarIndice(IndiceColumna indice[MAX_COLUMNAS],int col);
int cargarIndice(IndiceColumna indice[MAX_COLUMNAS]);
int buscarColumna(IndiceColumna indice[MAX_COLUMNAS],int col, const char *nombre);
int buscarPosicion(IndiceColumna indice[MAX_COLUMNAS],int col, const char *nombre); // Puede parecer lo mismo que buscarColumna pero sirve para que las funciones lo aprovechen de forma generica
char **leerColumna(FILE *fp, int posiciones[],int nPosiciones, char *linea, int nFilas);
int *obtenerPosiciones(IndiceColumna indice[MAX_COLUMNAS],int col,const char *nombres[],int nNombres); //este devuelve el vector posiciones que le podemos mandar a leerColumna
void Punto1(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col,char *linea,int nFilas);
void Punto2(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col, char*linea, int nFilas);
#endif
