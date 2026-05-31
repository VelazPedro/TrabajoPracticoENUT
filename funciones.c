#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"



int ArchivoAbrir(FILE **fp,const char *nombreArchivo,const char *modoApertura)//Propia version de fopen
{
    *fp = fopen(nombreArchivo,modoApertura);
    if (*fp == NULL)
    {
        fprintf(stderr, "Error abriendo \"%s\" en modo \"%s\" .", nombreArchivo, modoApertura);
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
    int i;
    int col = 0;
    char *token;

    rewind(fp);
    fgets(linea,MAX_LINEA,fp); //leemos el encabezado
    linea[strcspn(linea,"\n")] = '\0'; //eliminamos es salto de linea
    token = strtok(linea,SEPARADOR); //para identificar hasta DONDE debemos leer (en este caso espacios)

    while(token != NULL && col < MAX_COLUMNAS)
    {
        strncpy(indice[col].nombre, token, sizeof(indice[col].nombre)-1);
        indice[col].nombre[sizeof(indice[col].nombre)-1] = '\0'; //guardamos nombre de la columna
        indice[col].columna = col; //guardamos posicion de la columna
        col++;
        token = strtok(NULL,SEPARADOR);
    }
    for(i=0;i<col;i++)
    {
        sacarComillas(indice[i].nombre); //bueno, si lo explico quedo como un mogolico. Pero esto se hace en el encabezado
    }
    return col;
}

void guardarIndice(IndiceColumna indice[MAX_COLUMNAS],int col)
{
    FILE *fi;

    if(!ArchivoAbrir(&fi,"indice.bin","wb")) //en un nuevo archivo, guardamos el indice creado previamente en "crearIndice"
        exit(1);
    fwrite(&col,sizeof(int),1,fi); //guardamos la cantidad de columnas
    fwrite(indice,sizeof(IndiceColumna),col,fi); //guardamos todas las estructuras
    fclose(fi);
}

int cargarIndice(IndiceColumna indice[MAX_COLUMNAS])
{
    FILE *fi;
    int col=0;

    if(!ArchivoAbrir(&fi,"indice.bin","rb"))
        return -1;
    fread(&col, sizeof(int),1,fi); //leo la cantidad de columnas y la copio en col
    fread(indice,sizeof(IndiceColumna),col,fi); //leo "col" cantidad de veces el vector (recuperamos todas las estructuras creadas)
    fclose(fi);
    return col;
}

int buscarColumna(IndiceColumna indice[MAX_COLUMNAS],int col, const char *nombre)
{
    int i;

    for(i = 0;i<col;i++)
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
    int i,j;
    int fila;
    char **valores;
    int col_actual;
    char *token;

    //parte asignacion de memoria
    valores = malloc(nFilas * sizeof(char*));

    if(valores == NULL)
    {
        printf("Error de memoria\n");
        return NULL;
    }

    for(i=0;i<nFilas;i++)
     {
        valores[i] = malloc(nPosiciones * MAX_BYTES * sizeof(char));

        if(valores[i] == NULL)
        {
            printf("Error de memoria\n");

            for(j = 0; j < i; j++)
                free(valores[j]);

            free(valores);
            return NULL;
        }
        memset(valores[i], 0, nPosiciones * MAX_BYTES);
    }

    //parte leer
    rewind(fp);
    fgets(linea,MAX_LINEA,fp); //salteo nuevamente el encabezado
    fila=0;

    while(fgets(linea,MAX_LINEA,fp) != NULL) //leemos TODOS los registros
    {
        linea[strcspn(linea,"\n")] = '\0';
        token = strtok(linea,SEPARADOR); //dividimos las lineas segun su separador
        col_actual=0;

        while(token != NULL) //cuenta hasta que se termina el string (parando siempre en cada columna indicada por SEPARADOR)
        {
            for(i=0;i<nPosiciones;i++)
            {
                if(col_actual == posiciones[i]) // Compara columna actual con la buscada
                {
                    strncpy(valores[fila]+ i*MAX_BYTES,token,MAX_BYTES-1); //copiamos en valores[fila] el resultado
                    valores[fila][i * MAX_BYTES + MAX_BYTES - 1] = '\0';
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
    int i;
    int *posiciones = malloc(nNombres * sizeof(int)); //reservamos memoria segun la cantidad de elementos pedidos


    for(i=0;i < nNombres;i++)
        posiciones[i] = buscarColumna(indice,col,nombres[i]); //asignamos el indice de los nombres pedidos
    return posiciones;
}

int obtenerGrupoEdad(int edad)
{
    if(edad >= 14 && edad <= 29)
        return 0;

    if(edad >= 30 && edad <= 64)
        return 1;

    return 2;
}

const char *nombreGrupoEdad(int grupo)
{
    static const char *nombres[] =
    {
        "14 a 29 a�os",
        "30 a 64 a�os",
        "65 a�os y m�s"
    };

    return nombres[grupo];
}


void Punto1(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col,char *linea,int nFilas)
{
    const char *nombres[] = {"ID","WHOG","WPER","REGION","SEXO_SEL","EDAD_SEL"}; //columnas a trabajar
    int nNombres=sizeof(nombres) / sizeof(nombres[0]); //cantidad de elementos del vector nombres
    int i, r;
    int *posiciones;
    char **valores;

    posiciones = obtenerPosiciones(indice,col,nombres,nNombres);
    valores = leerColumna(fp,posiciones,nNombres,linea,nFilas);

    const int I_REGION = 3;
    const int I_WPER = 2;
    const int I_WHOG = 1;

    int conteoXRegion[6] = {0};
    long int sumWHOG[6] = {0};
    long int sumWPER[6] = {0};
    char regiones[6][30] = {"GBA","PAMPEANA","NOROESTE","NORESTE","CUYO","PATAGONIA"};
    for(i=0;i<nFilas;i++) //recorremos todos los registros obtenidos
    {
        char *region = valores[i] + I_REGION * MAX_BYTES; //obtenemos la region
        if(strcmp(region,"NA")!=0) //mientras que haya un dato no nulo...
        {
            r = atoi(region); //atoi = string a entero .. en criollo, convertimos "region"  en un numero
            if(r >= 1 && r<=6) //validamos que se encuentre en el rango
                conteoXRegion[r-1]++; //sumamos al contador region
            sumWHOG[r-1] += atoi(valores[i] + I_WHOG * MAX_BYTES);// sumamos hogares
            sumWPER[r-1] += atoi(valores[i] + I_WPER * MAX_BYTES);// sumamos personas
        }
    }
    printf(" ---------------------------------------------- PUNTO 1 ----------------------------------------------------- \n"); //mostramos resultados
    printf("REGI\xd3N\tcant_registros\tcant_hogares_est\tcant_personas_est\tnombre.region\n");
    for(i=0;i<6;i++)
    {
        printf("%d    \t%d    \t%ld  \t\t%ld  \t\t%s\n",i+1,conteoXRegion[i],sumWHOG[i],sumWPER[i],regiones[i]);
    }
    printf(" ------------------------------------------------------------------------------------------------------------ \n");
//xd3 es Ó

    //liberamos la memoria usada en el punto
    for(i = 0;i<nFilas;i++)
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

void Punto2(FILE *fp, IndiceColumna indice[MAX_COLUMNAS], int col, char *linea, int nFilas)
{
    const char *nombres[] =
    {
        "ID",
        "WHOG",
        "WPER",
        "REGION",
        "SEXO_SEL",
        "EDAD_SEL",
        "TP_GRANGRUPO_OCUPACIONYAUTOCONSUMO",
        "TP_GRANGRUPO_TRABAJOTOTAL",
        "TP_GRANGRUPO_TNR"
    };

    int nNombres = sizeof(nombres) / sizeof(nombres[0]);

    int *posiciones;
    char **valores;

    int i;
    int edad;
    int grupo;

    const int I_ID = 0;
    const int I_EDAD = 5;

    char GRUPO_EDAD_SEL[][30] = ///el 30 esta mal -- 5 cafeteras
    {
        "14 a 29 a\xf1os",
        "30 a 64 a\xf1os",
        "65 a\xf1os y m\xe1s"
    };

    posiciones = obtenerPosiciones(indice,col,nombres,nNombres);

    valores = leerColumna(fp,posiciones,nNombres,linea,nFilas);

    printf("\n---------------- PUNTO 2 ----------------\n");
    printf("ID\tEDAD\tGRUPO_EDAD_SEL\n");

    for(i = 0; i < 20 && i < nFilas; i++)
    {
        edad = atoi(valores[i] + I_EDAD * MAX_BYTES);

        grupo = obtenerGrupoEdad(edad);

        printf("%s\t%d\t%s\n", valores[i] + I_ID * MAX_BYTES, edad, GRUPO_EDAD_SEL[grupo]);
    }

    for(i = 0; i < nFilas; i++)
        free(valores[i]);

    free(valores);
    free(posiciones);
}

void Punto3(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col,char *linea,int nFilas)
{
    const char *nombres[] = {"REGION", "TIPO_HOGAR_DCPOREDAD", "WHOG"}; //columnas a trabajar
    int nNombres=sizeof(nombres) / sizeof(nombres[0]); //cantidad de elementos del vector nombres
    int i, r, t, j;
    int *posiciones;
    char **valores;
    int sumClasificacion_Hogares[4][REGIONES]= {{0}};
    char *region, *t_hogar;
    int whog;

     posiciones = obtenerPosiciones(indice,col,nombres,nNombres);
     valores = leerColumna(fp,posiciones,nNombres,linea,nFilas);

    const int I_REGION = 0;
    const int I_TIPO_HOGAR = 1;
    const int I_WHOG = 2;


    for(i = 0; i < nFilas; i++)
    {
        region = valores[i] + I_REGION * MAX_BYTES;
        t_hogar = valores[i] + I_TIPO_HOGAR * MAX_BYTES;
        whog = atoi(valores[i] + I_WHOG * MAX_BYTES);


        if(strcmp(region,"NA") != 0) //mientras que haya un dato no nulo...
        {
            r = atoi(region);
            if(strcmp(t_hogar,"NA") != 0)
            {
                t = atoi(t_hogar);
                sumClasificacion_Hogares[t-1][r-1] += whog;
            }
            else
                sumClasificacion_Hogares[3][r-1] += whog;
        }
    }
    
    char tipo_hogar_desc [][30] = {"Solo hasta 13 a\xf1os","Solo 14 y mas","Ambos tipos","Sin Demandantes"};
    
    printf(" ---------------------------------------------- PUNTO 3 ----------------------------------------------------- \n"); //mostramos resultados
    printf("TIPO_HOGAR_DCPOREDAD  GBA      PAMPEANA    NOROESTE     NORESTE       CUYO       PATAGONIA\n");
    for(i = 0; i < 4; i++)
    {
        printf("\n");
        printf("%.14s \t", tipo_hogar_desc[i]);
        for(j = 0; j < 6; j++)
        {
            printf(" %10d ", sumClasificacion_Hogares[i][j]);
        }
    }
    printf(" \n ------------------------------------------------------------------------------------------------------------ \n");

}

void Punto4(FILE *fp,IndiceColumna indice[MAX_COLUMNAS],int col, char*linea, int nFilas)
{
    const char *nombres[]={"REGION","WHOG","TIPO_HOGAR_DCTOTAL","TIPO_HOGAR_DCPOREDAD"};
    char tipo_hogar_desc [][30] = {"Solo hasta 13 a\xf1os","Solo 14 y mas","Ambos tipos","Sin Demandantes"};

    int nNombres=sizeof(nombres) / sizeof(nombres[0]);
    int *posiciones;
    char **valores;
    int i, j, region, tipo,maxTipo=0,edad,maxEdad=0,idx,hogaresTotales=0;

    const int I_REGION = 0;
    const int I_WHOG = 1;
    const int I_DCTOTAL = 2;
    const int I_DCPOREDAD = 3;

    int **conteo;
    long **hogares;
    int **conteoEdad;
    long **hogaresEdad;

    posiciones=obtenerPosiciones(indice,col,nombres,nNombres);
    valores=leerColumna(fp,posiciones,nNombres,linea,nFilas);

    for(i = 0; i < nFilas ; i++)
    {
        tipo = atoi(valores[i] + I_DCTOTAL * MAX_BYTES);
        edad = atoi(valores[i] + I_DCPOREDAD * MAX_BYTES);
        if(tipo > maxTipo)
            maxTipo = tipo;
        if(edad > maxEdad)
            maxEdad = edad;
    } //esto es para calcular la cantidad de tipos de dato distinto en DCTOTAL y por edad
    maxTipo++;
    maxEdad++;
    //printf("%d %d \n",maxTipo,maxEdad);

    conteo = malloc(6 * sizeof(int*));
    hogares = malloc(6 * sizeof(long*));
    conteoEdad = malloc(6 * maxTipo * sizeof(int*));
    hogaresEdad = malloc(6 * maxTipo * sizeof(long*));

    for(i = 0; i < 6; i++)
    {
        conteo[i] = calloc(maxTipo, sizeof(int));   // calloc inicializa en 0
        hogares[i] = calloc(maxTipo, sizeof(long));
    }
       for(i = 0; i < 6 * maxTipo; i++)
    {
        conteoEdad[i] = calloc(maxEdad, sizeof(int));   // calloc inicializa en 0
        hogaresEdad[i] = calloc(maxEdad, sizeof(long));
    }

   for(i = 0; i < nFilas; i++)
    {
        region = atoi(valores[i] + I_REGION * MAX_BYTES);
        tipo = atoi(valores[i] + I_DCTOTAL * MAX_BYTES);
        edad = atoi(valores[i] + I_DCPOREDAD * MAX_BYTES);
        if(region >= 1 && region <= 6)
        {
            conteo[region-1][tipo]++;
            hogares[region-1][tipo] += atoi(valores[i] + I_WHOG * MAX_BYTES);
        
            idx = (region-1) * maxTipo + tipo;
            if(tipo == 1 && edad >= 1 && edad < maxEdad)
            {
                conteoEdad[idx][edad-1]++;
                hogaresEdad[idx][edad-1] += atoi(valores[i] + I_WHOG * MAX_BYTES);
            }
        }
    }
    printf("\n--- PUNTO 4 (PARTE A) ---\n");
    for(i = 0; i < 6; i++)
    {
        printf("\nRegion %d\n", i+1);
        for(j = 0; j < maxTipo; j++)
            printf("DCTOTAL=%d -> Registros:%d Hogares:%ld\n", j, conteo[i][j], hogares[i][j]);
    }

    printf("\n--- PUNTO 4 (PARTE B) ---\n");
    for(i = 0; i < 6; i++)
    {
        printf("\nRegion %d\n", i+1);
        idx = i * maxTipo + 1; // esto hace que solo tome los que son tipo 
            for(j = 0; j < maxEdad-1; j++)
            {
                printf("DCPOREDAD=%d -> Registros:%d Hogares:%ld\n", j+1, conteoEdad[idx][j], hogaresEdad[idx][j]);
            }
    }





    for(i = 0; i < 6; i++)
    {
        free(conteo[i]);
        free(hogares[i]);
    }
    for(i = 0; i < 6 * maxTipo ; i++)
    {
        free(conteoEdad[i]);
        free(hogaresEdad[i]);
    }
    free(conteo);
    free(hogares);
    
    free(conteoEdad);
    free(hogaresEdad);


}
