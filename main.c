#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

int main(void)
{
    system("chcp 1252 > nul"); // con esto me permite poner acentos y la ñ con codigos
    FILE *fp;
    IndiceColumna indice[MAX_COLUMNAS];
    char *linea = malloc(MAX_LINEA);
    if(!ArchivoAbrir(&fp,"enut2021_base.csv","rt",1))
        {
            exit(1);
        }

    int col = crearIndice(fp,indice,linea);
    guardarIndice(indice,col);

    col = cargarIndice(indice);
    for(int i = 0;i<col;i++)
    {
        printf(" %s -> %d\n",indice[i].nombre,indice[i].columna);
    }
    printf("\n");

    int nFilas = contarFilas(fp,linea);
    Punto1(fp,indice,col,linea,nFilas);
    Punto2(fp,indice,col,linea,nFilas);
    free(linea);
    fclose(fp);
    return 0;
}
