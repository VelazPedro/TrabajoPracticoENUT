#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"



int ArchivoAbrir(FILE **fp,const char *nombreArchivo,const char *modoApertura, int mostrarError)//Propia version de fopen
{
    *fp = fopen(nombreArchivo,modoApertura);
    if (*fp == NULL)
    {
        if (mostrarError == 1)
        {
             fprintf(stderr, "Error abriendo \"%s\" en modo \"%s\" .", nombreArchivo, modoApertura);
        }
        return 0;

    }
    return 1;
}
// la funcion analiza caracter a caracter y saca las comillas, si hay dos o mas caracteres consecutivos entre comillas las va sacando (s es el string despues de haberlo separado en otra funcion)
int contarFilas(FILE* fp, char *linea)
{
    int nFilas = 0;
    rewind(fp);
    while(fgets(linea,MAX_LINEA,fp)!=NULL)
    {
        nFilas++;
    }
    return nFilas-1;
}
void sacarComillas(char *s)
{
    size_t len = strlen(s);
    if(len >= 2 && s[0] == '"' && s[len-1] == '"')
    {
        memmove(s,s+1,len-2);
        s[len-2] = '\0'; //ese len-2 sirve para que movamos las comillas al final y no las retornemos
    }
}

// Hacer archivo index que tenga NombreColumna -- Posicion
int crearIndice(FILE* fp,IndiceColumna indice[MAX_COLUMNAS],char* linea)
{
    int col = 0;
    rewind(fp);
    fgets(linea,MAX_LINEA,fp);
    linea[strcspn(linea,"\n")] = '\0';
    char *token = strtok(linea,SEPARADOR);

    while(token != NULL && col < MAX_COLUMNAS)
    {
        strncpy(indice[col].nombre, token, sizeof(indice[col].nombre)-1);
        indice[col].nombre[sizeof(indice[col].nombre)-1] = '\0';
        indice[col].columna = col;
        col++;
        token = strtok(NULL,SEPARADOR);
    }
    for(int i=0;i<col;i++)
    {
        sacarComillas(indice[i].nombre);
    }
    return col;
}
void guardarIndice(IndiceColumna indice[MAX_COLUMNAS],int col)
{
    FILE *fi;
    if(!ArchivoAbrir(&fi,"indice.bin","wb",1))
        return;
    fwrite(&col,sizeof(int),1,fi);
    fwrite(indice,sizeof(IndiceColumna),col,fi);
    fclose(fi);
}
int cargarIndice(IndiceColumna indice[MAX_COLUMNAS])
{
    FILE *fi;
    int col=0;
    if(!ArchivoAbrir(&fi,"indice.bin","rb",1))
        return 0;
    fread(&col, sizeof(int),1,fi); //leo la cantidad de columnas y la copio en col
    fread(indice,sizeof(IndiceColumna),col,fi); //leo "col" cantidad de veces el vector
    fclose(fi);
    return col;
}
int buscarColumna(IndiceColumna indice[MAX_COLUMNAS],int col, const char *nombre)
{
    for(int i = 0;i<col;i++)
    {
        if(strcmpi(indice[i].nombre,nombre) == 0)
            return i;
    }
    return -1;
}
int buscarPosicion(IndiceColumna indice[MAX_COLUMNAS],int col, const char *nombre)
{
    int pos = buscarColumna(indice,col,nombre);
        if(pos == -1)
            printf("campo no encontrado \n");
        else
            printf("Columna: %d \n",pos);
    return pos;
}
char **leerColumna(FILE *fp, int posiciones[],int nPosiciones, char *linea, int nFilas)
{

    char **valores = malloc(nFilas * sizeof(char*));
    for(int i=0;i<nFilas;i++)
     {
       valores[i]=malloc(nPosiciones * MAX_BYTES *sizeof(char));
       memset(valores[i],0,nPosiciones * MAX_BYTES);
     }

    //parte leer
    rewind(fp);
    fgets(linea,MAX_LINEA,fp); //salteo nuevamente el encabezado
    int fila=0;

    while(fgets(linea,MAX_LINEA,fp) != NULL)
    {
        linea[strcspn(linea,"\n")] = '\0';
        char *token = strtok(linea,SEPARADOR);
        int col_actual=0;

        while(token != NULL) //cuenta hasta que se termina el string
        {
            for(int i=0;i<nPosiciones;i++)
            {
                if(col_actual == posiciones[i])
            {
                strncpy(valores[fila]+ i*MAX_BYTES,token,MAX_BYTES-1);
                break; //este break rompe el while de token una vez ya detecta la posicion
            }
            }

            col_actual++;
            token = strtok(NULL,SEPARADOR);
        }
        fila++;
    }
    return valores;
}
int *obtenerPosiciones(IndiceColumna indice[MAX_COLUMNAS],int col,const char *nombres[],int nNombres)
{
    int *posiciones = malloc(nNombres * sizeof(int));
    for(int i=0;i < nNombres;i++)
        posiciones[i] = buscarColumna(indice,col,nombres[i]);
    return posiciones;
}
void Punto1(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col,char *linea,int nFilas)
{
    const char *nombres[] = {"ID","WHOG","WPER","REGION","SEXO_SEL","EDAD_SEL"};
    int nNombres=sizeof(nombres) / sizeof(nombres[0]);
    int *posiciones = obtenerPosiciones(indice,col,nombres,nNombres);
    char **valores = leerColumna(fp,posiciones,nNombres,linea,nFilas);

    const int I_REGION = 3,I_WHOG = 1, I_WPER = 2;
    int conteoXRegion[6] = {0};
    long int sumWHOG[6] = {0};
    long int sumWPER[6] = {0};
    char regiones[6][30] = {"GBA","PAMPEANA","NOROESTE","NORESTE","CUYO","PATAGONIA"};
    for(int i=0;i<nFilas;i++)
    {
        char *region = valores[i] + I_REGION * MAX_BYTES;
        if(strcmp(region,"NA")!=0)
        {
            int r = atoi(region); //atoi = string a entero
            if(r >= 1 && r<=6)
            conteoXRegion[r-1]++;
            sumWHOG[r-1] += atoi(valores[i] + I_WHOG * MAX_BYTES);
            sumWPER[r-1] += atoi(valores[i] + I_WPER * MAX_BYTES);
        }
    }
    printf(" ---------------------------------------------- PUNTO 1 ----------------------------------------------------- \n");
    printf("REGI\xd3N\tcant_registros\tcant_hogares_est\tcant_personas_est\tnombre.region\n");
    for(int i=0;i<6;i++)
    {
        printf("%d    \t%d    \t%ld  \t\t%ld  \t\t%s\n",i+1,conteoXRegion[i],sumWHOG[i],sumWPER[i],regiones[i]);
    }
    printf(" ------------------------------------------------------------------------------------------------------------ \n");
//xd3 es Ó

    for(int i = 0;i<nFilas;i++)
        free(valores[i]);

    free(valores);
    free(posiciones);

    /*
    int total = 0;
    for(int i=0;i<6;i++)
    {
        total+= conteoXRegion[i];
    }
    printf("\n total %d",total); //esto da la misma cantidad que nfilas asi que funciona bien
    */

}

void Punto2(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col, char*linea, int nFilas)
{
    const char *nombres[] = {"ID","WHOG","WPER","REGION","SEXO_SEL","EDAD_SEL","TP_GRANGRUPO_OCUPACIONYAUTOCONSUMO","TP_GRANGRUPO_TRABAJOTOTAL","TP_GRANGRUPO_TNR"};
    int nNombres=sizeof(nombres) / sizeof(nombres[0]);
    int *posiciones = obtenerPosiciones(indice,col,nombres,nNombres);
    char **valores = leerColumna(fp,posiciones,nNombres,linea,nFilas);
    char GRUPO_EDAD_SEL[][30] = {"14 a 29 a\xf1os","30 a 64 a\xf1os","65 a\xf1os y m\xe1s"}; //\xfl es para la ñ y \xel para á

}
