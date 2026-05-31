#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

int main()
{
    system("chcp 1252 > nul"); // con esto me permite poner acentos y la � con codigos
    FILE *fp; // el puntero de nuestro archivo comun
    IndiceColumna indice[MAX_COLUMNAS]; // guarda el nombre de columnas y la posicion en si (Ej: "ID" / O)
    char *linea = malloc(MAX_LINEA); //reserva de espacio de memoria de una de las lineas
    if(!ArchivoAbrir(&fp,"enut2021_base.csv","r"))
        exit(1);

    int col = crearIndice(fp,indice,linea); // Analizamos la primer linea del archivo
    guardarIndice(indice,col); //guardamos en un archivo el indice

    col = cargarIndice(indice);
    for(int i = 0;i<col;i++) //mostramos el indice y su codigo
    {
        printf(" %s -> %d\n",indice[i].nombre,indice[i].columna);
    }
    printf("\n");

    int nFilas = contarFilas(fp,linea); //contamos las filas sin contar encabezado

    Punto1(fp,indice,col,linea,nFilas);

    //Punto2(fp,indice,col,linea,nFilas);

    //Punto3(fp,indice,col,linea,nFilas);

    Punto4(fp,indice,col,linea,nFilas);


    //cierre de archivos y liberacion de memoria
    free(linea);
    fclose(fp);
    return 0;
}
