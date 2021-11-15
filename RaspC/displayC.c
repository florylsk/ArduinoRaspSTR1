#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wformat-overflow="
/**********************************************************
 *  INCLUDES
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>


/**********************************************************
 *  CONSTANTES
 *********************************************************/
#define NS_PER_S  1000000000
#define PERIOD_NS  500000000
#define TIME_TO_RELOAD   30
#define INIT_HIGH   0

// DISPLAY DELAY
#define TIME_DISPLAY_SEC 0
#define TIME_DISPLAY_NSEC 500000000


// FORCING ERRORS
#define TIME_BW_ERRORS   30
#define ERROR_DELAY_TIME 1


//CTES PANTALLA
#define LINEA_SIZE 80
#define FILAS_SIZE 25
#define PANTALLA_SIZE LINEA_SIZE*FILAS_SIZE

// CTES MAIN SCREEN
#define SC_LINEA_SIZE 60
#define SC_FILAS_SIZE 15
#define SC_FIRST_COL  10
#define SC_FIRST_ROW  3

// SIMUL
#define SIMUL_DISTANCE 20000.0
#define SIMUL_STOP_TIME 50
#define SIMUL_SLOPE_TIME 10  // max 100
#define SIMUL_ISDARK_TIME 50  // max 100
#define SIMUL_INIT_SPEED 55.0 



#define PATH_SIZE 500

/**********************************************************
 *  TIPOS
 *********************************************************/

struct periodic_data {
    struct timespec per;
    int sig_num;
};


struct control_data_backup_t {
    double speed;
    int gas;
    int brake;
    int mixer;
    int light;
    int isDark;
    struct timespec timeMix;
    int distance;
    int plain_up_down;
    
    
    // draw data
    int loadStep;
    int gHighInit;
    int step;
    int stepInc;
    int run;
    struct timespec initTime;
    int tramosPantalla[SC_LINEA_SIZE][2];
    int tramoInicial;
};

struct comand_data_t {
    double speed;
    int gas;
    int brake;
    int mixer;
    struct timespec timeMix;
    int light;
    int isDark;
    int distance;
    int plain_up_down;
    int stop;
    pthread_mutex_t cerrojo;
};

/**********************************************************
 *  VARIABLES GLOBALES
 *********************************************************/

struct comand_data_t gCmdData;

/* unsigned char gCarreta[4][4] = {
 {195,179,192, 32},
 {196, 32,196,'O'},
 {180,179,217, 32},
 { 32,'<', 32, 32}};
 
 unsigned char gDepositoLleno[4][3] = {
 {195,179,192},
 {196, 32,196},
 {196, 32,203},
 {180,179,217}};
 
 unsigned char gDepositoVacio[4][3] = {
 {179,195,192},
 { 32,196,196},
 { 32,196,203},
 {179,180,217}};
 
 unsigned char gFilas[22][15] ={
 { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,219,219,219,219,219},
 { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,219,219,219,219,219},
 { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 
 {219,219,219,219,176,176,176,176,176,176,219,219,219,219,219},
 {219,219,219,223,176,176,176,176,176,176,219,219,219,219,219},
 {219,219,219,219,176,176,176,176,176,176,220,219,219,219,219},
 
 { 32,195,179,192, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 { 32,196, 32,196, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 { 32,196, 32,203, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 { 32,180,179,217, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 
 { 32,179,195,192, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 { 32, 32,196,196, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 { 32, 32,196,203, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 { 32,179,180,217, 32, 32, 32, 32, 32, 32,220,219,219,219,219},
 
 {219,195,179,192,176,176,176,176,176,176,220,219,219,219,219},
 {219,196, 32,196,176,176,176,176,176,176,220,219,219,219,219},
 {219,196, 32,203,176,176,176,176,176,176,220,219,219,219,219},
 {219,180,179,217,176,176,176,176,176,176,220,219,219,219,219},
 
 {219,179,195,192,176,176,176,176,176,176,220,219,219,219,219},
 {219, 32,196,196,176,176,176,176,176,176,220,219,219,219,219},
 {219, 32,196,203,176,176,176,176,176,176,220,219,219,219,219},
 {219,179,180,217,176,176,176,176,176,176,220,219,219,219,219}};
 */
unsigned char gCarreta[4][4] = {
    {'|','|','|',' '},
    {'~',' ','_','O'},
    {'|','|','|',' '},
    {' ','<',' ',' '}};

unsigned char gDepositoLleno[5][4] = {
    {'|','|','|',' '},
    {'-',' ','_',' '},
    {'-',' ','_','H'},
    {'-',' ','_',' '},
    {'|','|','|',' '}};

unsigned char gDepositoVacio[5][4] = {
    {'|','|','|',' '},
    {' ','-','_',' '},
    {' ','-','_','H'},
    {' ','-','_',' '},
    {'|','|','|',' '}};

unsigned char gFilas[22][15] ={
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','#','#','#'},
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','#','#','#','#','#'},
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    
    {'#','#','#','#',':',':',':',':',':',':','#','#','#','#','#'},
    {'#','#','#','/',':',':',':',':',':',':','#','#','#','#','#'},
    {'#','#','#','#',':',':',':',':',':',':','o','#','#','#','#'},
    
    {' ','|','|','|',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ','-',' ','_',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ','-',' ','_',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ','|','|','|',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ','|','|','|',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ',' ','-','_',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ',' ','-','_',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    {' ','|','|','|',' ',' ',' ',' ',' ',' ','o','#','#','#','#'},
    
    {'#','|','|','|',':',':',':',':',':',':','o','#','#','#','#'},
    {'#','-',' ','_',':',':',':',':',':',':','o','#','#','#','#'},
    {'#','-',' ','_',':',':',':',':',':',':','o','#','#','#','#'},
    {'#','|','|','|',':',':',':',':',':',':','o','#','#','#','#'},
    
    {'#','|','|','|',':',':',':',':',':',':','o','#','#','#','#'},
    {'#',' ','-','_',':',':',':',':',':',':','o','#','#','#','#'},
    {'#',' ','-','_',':',':',':',':',':',':','o','#','#','#','#'},
    {'#','|','|','|',':',':',':',':',':',':','o','#','#','#','#'}};

char gCabecera[PANTALLA_SIZE]="";


//---------------------------------------------------------------------------
//                           FUNCIONES AUXILIARES                           
//---------------------------------------------------------------------------
/**********************************************************
 *  Function: getClock
 *********************************************************/
double getClock()
{
    struct timespec tp;
    double reloj;
    
    clock_gettime (CLOCK_REALTIME, &tp);
    reloj = ((double)tp.tv_sec) +
    ((double)tp.tv_nsec) / ((double)NS_PER_S);
    //printf ("%d:%d",tp.tv_sec,tp.tv_nsec);
    
    return (reloj);
}

/**********************************************************
 *  Function: diffTime
 *********************************************************/
void diffTime(struct timespec end, 
              struct timespec start,
              struct timespec *diff)
{
    if (end.tv_nsec < start.tv_nsec) {
        diff->tv_nsec = NS_PER_S - start.tv_nsec + end.tv_nsec;
        diff->tv_sec = end.tv_sec - (start.tv_sec+1);
    } else {
        diff->tv_nsec = end.tv_nsec - start.tv_nsec;
        diff->tv_sec = end.tv_sec - start.tv_sec;
    }
}

/**********************************************************
 *  Function: addTime
 *********************************************************/
void addTime(struct timespec end, 
             struct timespec start,
             struct timespec *add)
{
    unsigned long aux;
    aux = start.tv_nsec + end.tv_nsec;
    add->tv_sec = start.tv_sec + end.tv_sec +
    (aux / NS_PER_S);
    add->tv_nsec = aux % NS_PER_S;
}

/**********************************************************
 *  Function: compTime
 *********************************************************/
int compTime(struct timespec t1, 
             struct timespec t2)
{
    if (t1.tv_sec == t2.tv_sec) {
        if (t1.tv_nsec == t2.tv_nsec) {
            return (0);
        } else if (t1.tv_nsec > t2.tv_nsec) {
            return (1);
        } else if (t1.tv_nsec < t2.tv_nsec) {
            return (-1);
        }
    } else if (t1.tv_sec > t2.tv_sec) {
        return (1);
    } else if (t1.tv_sec < t2.tv_sec) {
        return (-1);
    }
    return (0);
}

/**********************************************************
 *  Function: retraso
 *********************************************************/
double retraso ()
{	
    static int init = 0;
    static int lastCount = 0;
    static struct timespec initClock;
    struct timespec actualClock, diffClock;
    int actualCount;
    int lapso;
    int ret = 0;
    
    if (0 == init) {
        clock_gettime(CLOCK_REALTIME, &initClock);
    }
    clock_gettime(CLOCK_REALTIME, &actualClock);
    diffTime(actualClock,initClock, &diffClock);
    lapso = diffClock.tv_sec;
    actualCount = lapso / TIME_BW_ERRORS;
    if (lastCount < actualCount) {
        ret = ERROR_DELAY_TIME * actualCount;
    }
    return (ret);
}


/**********************************************************
 *  Function: iniciarTramos
 *********************************************************/
int iniciarTramos (struct control_data_backup_t *data)
{
    int i;
    
    for (i=0;i<SC_LINEA_SIZE;i++) {
        data->tramosPantalla [i][0] = 0;
        data->tramosPantalla [i][1] = 0;
    }
    data->tramoInicial = data->step % SC_LINEA_SIZE;
    return (0);
}

/**********************************************************
 *  Function: crearTramos
 *********************************************************/
int cambiarTramos (struct control_data_backup_t *data)
{
    int inc, nuevoTramoInicial;
    int i, pos;
    
    // calcuoalr cuantos pasos ha avanzado desde la œltima vez
    nuevoTramoInicial = data->step % SC_LINEA_SIZE;
    if (nuevoTramoInicial >= data->tramoInicial) {
        inc = nuevoTramoInicial - data->tramoInicial;
    } else {
        inc = nuevoTramoInicial + SC_LINEA_SIZE - data->tramoInicial;
    }
    data->tramoInicial = nuevoTramoInicial;
    //printf ("TramoInicial: %d\n",data->tramoInicial);
    //printf ("plain_up_down: %d\n",data->plain_up_down);
    //printf ("gHighInit: %d\n",data->gHighInit);
    
    
    
    for (i=0;i<SC_LINEA_SIZE/2;i++) {
        pos = (data->tramoInicial + SC_LINEA_SIZE/2 + i) % SC_LINEA_SIZE;
        // down light
        if ( (4 > data->plain_up_down) && (0 == data->isDark) ) {
            if (0 == (data->step+i) % 2) {
                data->tramosPantalla [pos][0] = 0;
                data->tramosPantalla [pos][1] = 1;
            } else {
                data->tramosPantalla [pos][0] = -1;
                data->tramosPantalla [pos][1] = 2;
            }
            // down dark
        } else if ( (4 > data->plain_up_down) && (0 != data->isDark) ) {
            if (0 == (data->step+i) % 2) {
                data->tramosPantalla [pos][0] = 0;
                data->tramosPantalla [pos][1] = 4;
            } else {
                data->tramosPantalla [pos][0] = -1;
                data->tramosPantalla [pos][1] = 5;
            }
            // plain ligh
        } else if ( (7 > data->plain_up_down) && (0 == data->isDark) ) {
            data->tramosPantalla [pos][0] = 0;
            data->tramosPantalla [pos][1] = 0;
            // plain dark
        } else if ( (7 > data->plain_up_down) && (0 != data->isDark) ) {
            data->tramosPantalla [pos][0] = 0;
            data->tramosPantalla [pos][1] = 3;
            // up light
        } else if (0 == data->isDark) {
            if (0 == (data->step+i) % 2) {
                data->tramosPantalla [pos][0] = 0;
                data->tramosPantalla [pos][1] = 2;
            } else {
                data->tramosPantalla [pos][0] = 1;
                data->tramosPantalla [pos][1] = 1;
            }
            // up dark
        } else if (0 != data->isDark) {
            if (0 == (data->step+i) % 2) {
                data->tramosPantalla [pos][0] = 0;
                data->tramosPantalla [pos][1] = 5;
            } else {
                data->tramosPantalla [pos][0] = 1;
                data->tramosPantalla [pos][1] = 4;
            }
        }
    }
    return (0);
}



//---------------------------------------------------------------------------
//                           FUNCIONES DE DIBUJO                           
//---------------------------------------------------------------------------

/**********************************************************
 *  Function: crearCabecera
 *********************************************************/
int crearCabecera(char cabecera[PANTALLA_SIZE])
{
    char cabeceraAux[PANTALLA_SIZE];
    int i;
    
    // CREAR CABECERA
    cabeceraAux[0] = '\0';
    for (i=0;i<(SC_FIRST_ROW/2)-1+(SC_FIRST_ROW%2);i++) {
        sprintf(cabecera,"%s\n",cabeceraAux);
        sprintf(cabeceraAux,"%s",cabecera);
    }
    sprintf(cabecera,"%s\t\t\tPRACTICA DE TIEMPO REAL\n",cabeceraAux); // 3 lineas
    sprintf(cabeceraAux,"%s",cabecera);
    for (i=0;i<(SC_FIRST_ROW/2);i++) {
        sprintf(cabecera,"%s\n",cabeceraAux);
        sprintf(cabeceraAux,"%s",cabecera);
    }
    
    return (0);
}

/**********************************************************
 *  Function: crearEscenario
 *********************************************************/
int crearEscenario(struct control_data_backup_t data, char pantalla[PANTALLA_SIZE])
{
    char pantallaAux[PANTALLA_SIZE];
    char linea[LINEA_SIZE];
    char inicioLinea[LINEA_SIZE];
    int subindice,high,pos;
    int i,j;
    
    
    pantallaAux[0] = '\0';
    
    // inicio Linea
    for (i=0;i<(SC_FIRST_COL-1);i++) {
        inicioLinea[i]=' ';
    }
    inicioLinea[SC_FIRST_COL-1]='\0';
    
    // INICIO CAJA
    //sprintf(linea,"%s%c",inicioLinea,201);
    sprintf(linea,"%s%c",inicioLinea,'*');
    for (i=0;i<SC_LINEA_SIZE;i++) {
        //linea[SC_FIRST_COL+i]=205;
        linea[SC_FIRST_COL+i]='*';
    }
    //linea[SC_LINEA_SIZE+SC_FIRST_COL]=187;
    linea[SC_LINEA_SIZE+SC_FIRST_COL]='*';
    linea[SC_LINEA_SIZE+SC_FIRST_COL+1]='\n';
    linea[SC_LINEA_SIZE+SC_FIRST_COL+2]='\0';
    sprintf(pantalla,"%s%s",pantallaAux,linea);
    sprintf(pantallaAux,"%s",pantalla);
    
    // PANTALLA
    for (i=0;i<15;i++) {
        //sprintf(linea,"%s%c",inicioLinea,186);
        sprintf(linea,"%s%c",inicioLinea,'*');
        high=data.gHighInit;
        for (j=0;j<60;j++) {
            pos = (data.tramoInicial+j) % SC_LINEA_SIZE;
            high=high+data.tramosPantalla[pos][0];
            subindice=i-high;
            if (subindice < 0) subindice = 0;
            if (subindice > 14) subindice = 14;
            linea[SC_FIRST_COL+j]=gFilas[data.tramosPantalla[pos][1]][subindice];
        }
        //linea[70]=186;
        linea[70]='*';
        linea[71]='\n';
        linea[72]='\0';
        sprintf(pantalla,"%s%s",pantallaAux,linea);
        sprintf(pantallaAux,"%s",pantalla);
    }
    
    // FIN CAJA
    //sprintf(linea,"         %c",200);
    sprintf(linea,"         %c",'*');
    for (j=0;j<60;j++) {
        //linea[10+j]=205;
        linea[10+j]='*';
    }
    //linea[70]=188;
    linea[70]='*';
    linea[71]='\n';
    linea[72]='\0';
    sprintf(pantalla,"%s%s",pantallaAux,linea);
    sprintf(pantallaAux,"%s",pantalla);
    
    return (0);
}

/**********************************************************
 *  Function: crearInferior
 *********************************************************/
int crearInferior(struct control_data_backup_t data, char inferior[PANTALLA_SIZE])
{
    char rute[10];
    char isDark[10];
    char light[10];
    char load[10];
    char strTime[10];
    char strSpeed[10];
    char strMixer[10];
    int auxTime = 1;
    int auxMixer = 1;
    int auxSpeed = 1;
    struct timespec timeAux;
    struct timespec timeTemp;
    int auxDistance;
    
    clock_gettime(CLOCK_REALTIME, &timeAux);
    
    switch (data.plain_up_down) {
        case 1:
        case 2:
        case 3: sprintf(rute," UP   "); break;
        case 4:
        case 5:
        case 6: sprintf(rute," FLAT "); break;
        case 7:
        case 8:
        case 9: sprintf(rute," DOWN "); break;
        default: sprintf(rute,"ERROR"); break;
    }
    switch (data.isDark) {
        case 0: sprintf(isDark,"no");break;
        case 1: sprintf(isDark,"yes");   break;
        default: sprintf(isDark,"ERROR"); break;
    }
    switch (data.light) {
        case 0: sprintf(light,"off");break;
        case 1: sprintf(light,"on");   break;
        default: sprintf(light,"ERROR"); break;
    }
    switch (data.loadStep) {
        case 0: sprintf(load,"off");break;
        default: sprintf(load,"on "); break;
    }
    
    auxDistance= data.distance;
    diffTime(timeAux, data.initTime, &timeTemp);
    auxTime = (int) (timeTemp.tv_sec);
    auxSpeed = (int) (data.speed);
    diffTime(timeAux, data.timeMix, &timeTemp);
    auxMixer = (int) (timeTemp.tv_sec);
    sprintf(strTime,"%3d",(auxTime));
    sprintf(strSpeed,"%5d",(auxSpeed));
    sprintf(strMixer,"%5d",(auxMixer));
    
    sprintf(inferior,
            "\n"
            "\t\t\t\tTime:%s sec\n"
            "  Speed:%s\tFloor:%s\tTime Mix:%s\tIsDark:%s\tDistance:%3d \n"
            "  Gas:%d\t\tBrake:%d\t\tMixer:%d  \tLights:%s\tLoading:%s  \n",
            strTime,
            strSpeed,rute,strMixer,isDark,auxDistance,
            data.gas,data.brake,data.mixer,light,load);
    return (0);
}

/**********************************************************
 *  Function: crearCarretilla
 *********************************************************/
int crearCarretilla(struct control_data_backup_t data, char pantalla[PANTALLA_SIZE])
{
    int i,j,pos,high,subindice;
    int auxHigh;
    static int status=0;
    static int status2=0;
    int distance;
    int col;
    int pos_x=0;
    int pos_y=0;
    int aux_x = 0;
    int avgPointHigh = 0;
    
    
    // modificar rueda carretilla
    if ( (0==(status%2)) && (data.run != 0)) {
        gCarreta[1][3]='O';
    } else {
        gCarreta[1][3]='0';
    }
    // modificar mezclador carretilla
    if (data.mixer != 0) {
        switch (status) {
            case 0:  gCarreta[1][1]='-';break;
            case 1:  gCarreta[1][1]='\\';break;
            case 2:  gCarreta[1][1]='|';break;
            case 3:  gCarreta[1][1]='/';break;
        }
    } else {
        gCarreta[1][1]='-';
    }
    // modificar faros y luz
    if (data.light != 0) {
        // dibujar faro
        gCarreta[3][1] = '<';
        // iluminar PANTALLA
        auxHigh = 0;
        pos = (data.tramoInicial+29) % SC_LINEA_SIZE;
        auxHigh = auxHigh-data.tramosPantalla[pos][0];
        pos = (data.tramoInicial+28) % SC_LINEA_SIZE;
        auxHigh = auxHigh-data.tramosPantalla[pos][0];
        pos = (data.tramoInicial+27) % SC_LINEA_SIZE;
        auxHigh = auxHigh-data.tramosPantalla[pos][0];
        for (i=5;i<11;i++) {
            high=auxHigh;
            for (j=27;j<33;j++) {
                pos = (data.tramoInicial+j) % SC_LINEA_SIZE;
                high=high+data.tramosPantalla[pos][0];
                subindice=i+high;
                pantalla[(subindice*72)+10+j]=' ';
            }
        }
    } else {
        // borrar faro
        gCarreta[3][1] = ' ';
    }
    
    // Poner carretilla
    if ( (data.isDark == 0) || (data.light != 0) ){
        for (i=7;i<11;i++) {
            for (j=28;j<32;j++) {
                if (gCarreta[j-28][i-7] != ' ') {
                    pantalla[(i*72)+10+j]=gCarreta[j-28][i-7];
                }
            }
        }
    }
    
    
    // Arreglar lo de poner el deposito a la altura adecuada
    
    // Calcular la posicion x del deposito segun su distancia
    pos_x = (SC_LINEA_SIZE-33) + (data.distance/10);
    
    // calcular la altura del punto medio
    avgPointHigh=0;
    for (i=0; i<SC_LINEA_SIZE-32; i++) {
        pos = (data.tramoInicial+i) % SC_LINEA_SIZE;
        avgPointHigh = avgPointHigh+data.tramosPantalla[pos][0];
    }
    
    // calcular la altura adecuada para pos_x
    if (SC_LINEA_SIZE > pos_x+2) {
        aux_x = pos_x;
    } else {
        aux_x = SC_LINEA_SIZE;
    }
    auxHigh=0;
    for (i=0; i<aux_x; i++) {
        pos = (data.tramoInicial+i) % SC_LINEA_SIZE;
        auxHigh = auxHigh+data.tramosPantalla[pos][0];
    }
    pos_y =  0-(avgPointHigh-auxHigh);
    
    //printf ("pos_x:%d, pos_y:%d\n",pos_x,pos_y);
    //pos_x=SC_LINEA_SIZE-33;
    //pos_y=0;
    
    // dibujar el deposito.
    for (i=pos_y; i<(pos_y+4); i++) {
        for (j=pos_x; j<(pos_x+5); j++) {
            if ( (i<0) || (i>=SC_FILAS_SIZE) ||
                (j<0) || (j>=SC_LINEA_SIZE) ) {
                continue;
            }
            if (pos_x >= (SC_LINEA_SIZE/2)-3) {
                pantalla[((i+1)*72)+SC_FIRST_COL+j]=gDepositoVacio[j-pos_x][i-pos_y];
            } else {
                pantalla[((i+1)*72)+SC_FIRST_COL+j]=gDepositoLleno[j-pos_x][i-pos_y];
            }
        }
    }
    
    // si el deposito esta encima de la carretilla y la la carretilla esta parada
    if ( (pos_x == SC_LINEA_SIZE-33) && (data.loadStep == 1) ){
        // poner frenos
        pantalla[(11*72)+10+28]='|'; pantalla[(10*72)+10+28]='(';
        pantalla[(11*72)+10+30]='|'; pantalla[(10*72)+10+30]=')';
        // poner manguera
        pantalla[(5*72)+10+29] = 'H';
        pantalla[(6*72)+10+29] = 'H';
        //pantalla[(7*72)+10+29] = 'H';
    }
    
    status = (status +1)%4;
    return (0);
}

/**********************************************************
 *  Function: dibujar
 *********************************************************/
int dibujar(struct control_data_backup_t data)
{
    char pantalla[PANTALLA_SIZE];
    char screen[PANTALLA_SIZE];
    char inferior[PANTALLA_SIZE];
    //text_console_ioctl_data_t con_data;
    
    //con_data.pos=0;
    
    
    //RECOLOCAR CURSOR
    //ioctl(1,TEXT_CONSOLE_SET_CURSOR_POS,&con_data);
    // CREAR CUERPO
    crearEscenario(data,screen);
    // CREAR CARRETILLA
    crearCarretilla(data,screen);
    // CREAR INFERIOR
    crearInferior(data,inferior);
    // UNIR Y IMPRIMIR
    sprintf(pantalla,"%s%s%s",gCabecera,screen,inferior);
    write(1,pantalla,strlen(pantalla));
    
    return (0);
}

/**********************************************************
 *  Function: incrementar_paso
 *********************************************************/
int incrementar_paso(struct control_data_backup_t *data)
{
    int i,pos;
    
    if (data->stepInc > (SC_LINEA_SIZE/2)) {
        printf ("ERROR SIMULADOR: Demasiados pasos de una vez\n");
        exit (-1);
    }
    for (i=0;i<data->stepInc;i++) {
        //RECALCULAR ALTURAS
        pos = (data->tramoInicial +i) % SC_LINEA_SIZE;
        //printf ("(%d,%d),",pos,data->tramosPantalla[pos][0]);
        data->gHighInit=data->gHighInit+data->tramosPantalla[pos][0];
        pos = (data->tramoInicial+(SC_LINEA_SIZE/2)+i) % SC_LINEA_SIZE;
        //printf ("(%d,%d),",pos,data->tramosPantalla[pos][0]);
        data->gHighInit=data->gHighInit-data->tramosPantalla[pos][0];
        data->step=data->step+1;
    }
    //printf("\n");
    
    return (0);
}  

//---------------------------------------------------------------------------
//                      FUNCIONES DEL THREAD DEL DISPLAY                     
//---------------------------------------------------------------------------

/**********************************************************
 *  Function: getControl
 *********************************************************/
int getControl(struct control_data_backup_t *data, struct comand_data_t *cmd)
{
    //static int status = 0;
    static int OldDistance = -1;
    
    // bloquear cerrojo
    pthread_mutex_lock (&(cmd->cerrojo));
    // Copiar ordenes
    data->speed = cmd->speed;
    data->gas = cmd->gas;
    data->brake = cmd->brake;
    data->mixer = cmd->mixer;
    data->timeMix = cmd->timeMix;
    data->light = cmd->light;
    data->isDark = cmd->isDark;
    data->plain_up_down = cmd->plain_up_down;
    data->loadStep = cmd->stop;
    data->distance = cmd->distance;
    
    // desbloquear cerrojo
    pthread_mutex_unlock (&(cmd->cerrojo));
    
    
    // segun estemos parados o no
    if (data->loadStep == 1) {
        // si esta en parada, no se mueve y la distancia efectiva es siempre cero
        data->stepInc = 0;
        data->run = 0;
    } else {
        
        // por defecto esta corriendo.
        data->run = 1;
        // obtener el incremento del step
        data->stepInc = data->speed / 10;
        if ((data->speed > 0.0) && (data->speed <= 10.0)) data->stepInc = 1;
        else if (data->speed <= 0.0) {data->stepInc = 0;data->run = 0;}
    }
    
    //incrementar paso;
    incrementar_paso(data);
    
    return (0);
}

/**********************************************************
 *  Function: drawScreen
 *********************************************************/
int drawScreen(struct control_data_backup_t *data, struct comand_data_t *cmd) 
{
    //incrementar paso;
    getControl(data,cmd);
    
    // choose the coprrect map
    cambiarTramos(data);
    
    // draw all togheter
    dibujar(*data);
    
    // DEBUG
    // int i,pos;
    // for (i=0;i<SC_LINEA_SIZE;i++) {
    // 	 pos = (data->tramoInicial +i)  % SC_LINEA_SIZE;
    //	 printf ("(%d,%d),",data->tramosPantalla[pos][0],data->tramosPantalla[pos][1]);
    // }
    //printf ("\n");
    //printf ("TramoInicial: %d\n",data->tramoInicial);
    //printf ("setp: %d\n",data->step);
    
    return 0;
} 

/**********************************************************
 *  Function: initDisplay
 *********************************************************/
int initDisplay(struct control_data_backup_t *data)
{
    int pos;
    
    data->speed = 0;
    data->gas = 0;
    data->brake = 0;
    data->mixer = 0;
    data->light = 0;
    data->isDark = 0;
    data->loadStep = 0;
    clock_gettime(CLOCK_REALTIME, &(data->timeMix));
    data->distance = -1000;
    
    // iniciar resto var.
    data->gHighInit = 0;
    data->step = 0;
    data->stepInc = 0;
    data->run = 0;
    data->plain_up_down = 5;   //plain
    clock_gettime(CLOCK_REALTIME, &(data->initTime));
    
    // inciar tramos
    iniciarTramos(data);
    
    //CALCULAR LAS ALTURAS
    data->gHighInit = INIT_HIGH;
    for (pos=(SC_LINEA_SIZE-1);pos>=0;pos--) {
        data->gHighInit=data->gHighInit-(int)(data->tramosPantalla[pos][0]);
    }
    
    // CREAR CABECERA
    crearCabecera (gCabecera);
    data->stepInc = 1;
    return (0);
}

/**********************************************************
 *  Function: displayThread
 *********************************************************/
void * displayThread(void *arg)
{
    struct control_data_backup_t data;
    struct sigevent event;
    timer_t timer_id;
    struct timespec periodTime;
    struct itimerspec timerdata;
    int sig_num, received_sig;
    sigset_t set;
    int error;
    
    // get signal to use
    sig_num = (int) arg;
    if ((sig_num < SIGRTMIN) || (sig_num < SIGRTMAX)) {
        sig_num = SIGRTMAX;
    }
    
    // set periodTime time
    periodTime.tv_sec=0;
    periodTime.tv_nsec=PERIOD_NS;
    
    /* Create timer */
    event.sigev_notify = SIGEV_SIGNAL;
    event.sigev_signo = sig_num;
    timer_create (CLOCK_REALTIME, &event, &timer_id);
    
    /* Arm periodic timer */
    timerdata.it_interval = periodTime;
    timerdata.it_value = periodTime;
    timer_settime (timer_id, 0, &timerdata, NULL);
    
    // init signal set to choose the signal to wait for
    sigemptyset (&set);
    sigaddset (&set, sig_num);
    
    
    // init data for display
    initDisplay (&data);
    
    while (1) {
        
        // draw screen
        drawScreen(&data, &gCmdData);
        
        // wait until next period
        sigwait(&set, &received_sig);
        
        //if there have been missed periods
        if (0 != timer_getoverrun(timer_id)) {
            error = sig_num;
            //printf("Error: timer overflow\n");
        }
    }
    //return (NULL);
}
int initShared (struct comand_data_t *cmd)
{
    
    cmd->speed=0; //0;
    cmd->gas=0;
    cmd->brake=0;
    cmd->mixer=0;
    clock_gettime(CLOCK_REALTIME, &(cmd->timeMix));
    cmd->light=0;
    cmd->isDark=0;
    cmd->distance=-1000;
    cmd->plain_up_down=5;
    cmd->stop = 0;
    pthread_mutex_init (&(cmd->cerrojo),NULL);
    
    return(0);
}
//---------------------------------------------------------------------------
//                           FUNCIONES A EXPORTAR                          
//---------------------------------------------------------------------------
/**********************************************************
 *  Function: displayInit
 *  Parameters: signalRT = numero de se–al
 entre SIGRTMIN y SIGRTMIN
 *********************************************************/
int displayInit(int signalRT)
{
    pthread_t thread_simul;
    
    // INICIAR var. compartida
    initShared(&(gCmdData));
    
    /* Create first thread */
    pthread_create (&thread_simul, NULL, displayThread, (void *)signalRT);
    
    return (0);
}

/**********************************************************
 *  Function: displaySlope
 *	Parameters: slope = -1 => down ground
 *				slope =  0 => flat ground
 *				slope =  1 =>   up ground
 *********************************************************/

int displaySlope(int slope) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer la pendiente
    if (slope == -1) { //down
        gCmdData.plain_up_down = 9;
    } else if (slope == 0) { //flat
        gCmdData.plain_up_down = 5;
    } else if (slope == 1) { //up
        gCmdData.plain_up_down = 1;
    }
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displaySpeed
 *	Parameters: speed = value of speed to display
 *********************************************************/
int displaySpeed(double speed) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer la velocidad
    gCmdData.speed = speed;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displayGas
 *	Parameters: gas = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayGas(int gas) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer el acelerador
    gCmdData.gas = gas;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displayBrake
 *	Parameters: brake = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayBrake(int brake) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer el freno
    gCmdData.brake = brake;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displayMix
 *	Parameters: mixer = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayMix(int mixer) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // si cambia el mixer
    if (gCmdData.mixer != mixer) {
        // actualizar el reloj
        clock_gettime(CLOCK_REALTIME, &(gCmdData.timeMix));
    }
    // establecer el mixer
    gCmdData.mixer = mixer;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displayLightSensor
 *	Parameters: isDark = 1 -> Dark; 0 -> Bright
 *********************************************************/
int displayLightSensor(int isDark) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer si esta oscuro
    gCmdData.isDark = isDark;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displayLamps
 *	Parameters: light = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayLamps(int light) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer si los focos estan encendidos
    gCmdData.light = light;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}


/**********************************************************
 *  Function: displayDistance
 *	Parameters: distance >= 0 Set distance to next deposit
 *						  < 0 There is no new deposit
 *  NOTE: if distance is zero, the wagon will stop
 *********************************************************/
int displayDistance(int distance) 
{
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer una nueva distancia
    gCmdData.distance = distance;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/**********************************************************
 *  Function: displayStop
 *	Parameters: stop = 1 -> Wagon ios stopped under a wagon;
 *					   0 -> otherwise
 *********************************************************/
int displayStop(int stop)
{
    
    struct timespec timeInit, timeEnd, timeDiff, timeWait;
    
    /* calculate wait time including errors */
    timeWait.tv_sec = TIME_DISPLAY_SEC;
    timeWait.tv_nsec = TIME_DISPLAY_NSEC;
    
    // obtener tiempo inicial
    clock_gettime(CLOCK_REALTIME, &timeInit);
    
    // bloquear cerrojo
    pthread_mutex_lock (&(gCmdData.cerrojo));
    // establecer si esta en una parada
    gCmdData.stop = stop;
    // desbloquear cerrojo
    pthread_mutex_unlock (&(gCmdData.cerrojo));
    
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    diffTime(timeEnd, timeInit, &timeDiff);
    diffTime(timeWait, timeDiff, &timeDiff);
    
    nanosleep (&timeDiff,NULL);
    
    return (0);
}

/***********************************************************
 * SIMULADOR
 ************************************************************/

struct simulation_t {
    // activadores
    int gas;
    int brake;
    int mix;
    int lamps;
    int errorMode;
    // sensores
    float speed;
    char slope[10];
    int isDark;
    int distance;
    char isStop[10];
};

/**********************************************************
 *  Function: simul_init
 *********************************************************/

int simul_init (struct simulation_t *simulData)
{
    simulData->gas = 0;
    simulData->brake = 0;
    simulData->mix = 0;
    simulData->lamps = 0;
    simulData->errorMode = 0;
    simulData->speed = SIMUL_INIT_SPEED;
    strcpy(simulData->slope,"FLAT");
    simulData->isDark = 99;
    simulData->distance = 0;
    strcpy(simulData->isStop,"GO");
    
    return 0;
}

/**********************************************************
 *  Function: simul_mod
 *********************************************************/

int simul_mod(struct simulation_t *simulData)
{
    static int old_gas = 0;
    static int old_brake = 0;
    static int slope = 0;
    static int isStop = 0;
    static int status = 0;
    static int status2 = 0;
    static double old_time = 0.0;
    static double distance = SIMUL_DISTANCE;
    double time;
    double acel;
    double diffTime;
    struct timespec timeWait;
    
    // calcular tiempo pasado
    if (old_time == 0.0) {
        old_time = getClock();;
    }
    // esperar 100 ms para que pase algo de tiempo
    timeWait.tv_sec = 0;
    timeWait.tv_nsec = 100000000;
    nanosleep (&timeWait,NULL);
    
    // calcular el tiempo
    time = getClock();
    diffTime = time - old_time;
    old_time = time;
    // calcular speed
    if (0.0 < diffTime) {
        acel = ((double)old_gas)*0.5 + ((double)old_brake)*(-0.5) + ((double)(slope-1))*(-0.25);
        
        simulData->speed =  simulData->speed + acel*diffTime;
        if (0.0 > simulData->speed) {
            simulData->speed = 0;
        }
    }
    // calcular distance
    if ((0.0 < diffTime) && (distance > 0.0)) {
        distance = distance - simulData->speed * diffTime;
        if (distance <= 0.0) distance = 0.0;
        simulData->distance = (int)(distance);
    }
    // calcular parada
    if ((0.0 == distance) && (0 == isStop)) {
        if (10.0 >= simulData->speed) {
            isStop = 1;
            status2 = SIMUL_STOP_TIME;
            strcpy(simulData->isStop,"STOP");
        } else {
            distance = SIMUL_DISTANCE;
        }
    }
    if (1 == isStop) {
        status2 = status2 -1;
        if (0 == status2) {
            isStop = 0;
            strcpy(simulData->isStop,"GO");
            distance = SIMUL_DISTANCE;
        }
    }
    // calcular slope
    if (0 == (status % SIMUL_SLOPE_TIME)) {
        slope = (slope + 1) % 3;
        if (0 == slope) strcpy (simulData->slope,"DOWN");
        if (1 == slope) strcpy (simulData->slope,"FLAT");
        if (2 == slope) strcpy (simulData->slope,"UP");
    }
    // calcular isdark
    if (0 == (status % SIMUL_ISDARK_TIME)) {
        simulData->isDark = 100 - simulData->isDark;
    }
    // guardar gas y brake
    old_gas = simulData->gas;
    old_brake = simulData->brake;
    
    // run time
    status = (status+1)%1000;
    
    return 0;
}


/**********************************************************
 *  Function: simulator
 *  Parameters: request -> request message
 *   			answer  -> answer message
 *********************************************************/
int simulator(char request[10], char answer[10])
{
    int ret;
    char arg[10];
    static int init = 0;
    static struct simulation_t simulData;
    
    if (0 == init) {
        simul_init(&simulData);
        init=1;
    }
    // peticiones de activacion, no hace nada
    if (1 == sscanf(request,"GAS: %s\n",arg)) {
        if (0 == strcmp(arg,"SET")) {
            // activar acelerador
            simulData.gas=1;
            simul_mod(&simulData);
            strcpy (answer,"GAS:  OK\n");
        } else if (0 == strcmp(arg,"CLR")) {
            // desactivar acelerador
            simulData.gas=0;
            simul_mod(&simulData);
            strcpy (answer,"GAS:  OK\n");
        } else {
            // error
            strcpy (answer,"MSG: ERR\n");
        }
    } else if (1 == sscanf(request,"BRK: %s\n",arg)) {
        if (0 == strcmp(arg,"SET")) {
            // activar freno
            simulData.brake=1;
            simul_mod(&simulData);
            strcpy (answer,"BRK:  OK\n");
        } else if (0 == strcmp(arg,"CLR")) {
            // desactivar freno
            simulData.brake=0;
            simul_mod(&simulData);
            strcpy (answer,"BRK:  OK\n");
        } else {
            // error
            strcpy (answer,"MSG: ERR\n");
        }
    } else if (1 == sscanf(request,"MIX: %s\n",arg)) {
        if (0 == strcmp(arg,"SET")) {
            // activar mixer
            simulData.mix=1;
            simul_mod(&simulData);
            strcpy (answer,"MIX:  OK\n");
        } else if (0 == strcmp(arg,"CLR")) {
            // desactivar mixer
            simulData.mix=0;
            simul_mod(&simulData);
            strcpy (answer,"MIX:  OK\n");
        } else {
            // error
            strcpy (answer,"MSG: ERR\n");
        }
    } else if (1 == sscanf(request,"LAM: %s\n",arg)) {
        if (0 == strcmp(arg,"SET")) {
            // activar focos
            simulData.lamps=1;
            simul_mod(&simulData);
            strcpy (answer,"LAM:  OK\n");
        } else if (0 == strcmp(arg,"CLR")) {
            // desactivar focos
            simulData.lamps=0;
            simul_mod(&simulData);
            strcpy (answer,"LAM:  OK\n");
        } else {
            // error
            strcpy (answer,"MSG: ERR\n");
        }
        
        // peticiones de informaci—n, devolver algo
    } else if (0 == strcmp(request,"SPD: REQ\n")) {
        // devolver velocidad
        simul_mod(&simulData);
        sprintf (answer,"SPD:%04.1f\n",simulData.speed);
    } else if (0 == strcmp(request,"SLP: REQ\n")) {
        // devolver pendiente
        simul_mod(&simulData);
        sprintf (answer,"SLP:%4s\n",simulData.slope);
    } else if (0 == strcmp(request,"LIT: REQ\n")) {
        // devolver hay luz
        simul_mod(&simulData);
        sprintf (answer,"LIT: %2d%%\n",simulData.isDark);
    } else if (0 == strcmp(request,"DS:  REQ\n")) {
        // devolver la distancia en miles de metros
        simul_mod(&simulData);
        sprintf (answer,"DS:%5d\n",simulData.distance);
    } else if (0 == strcmp(request,"STP: REQ\n")) {
        // devolver si esta en una parada o en movimiento
        simul_mod(&simulData);
        sprintf (answer,"STP:%4s\n",simulData.isStop);
        
        // si no coincide con ninguno, error
    } else {
        // error
        strcpy (answer,"MSG: ERR\n");
    }
    return 0;
}


/////
//////////////////
////
#ifdef COMMENT
int main(int argc, char *argv[])
{
    sigset_t alarm_sig;
    int i;
    
    /* Block all real time signals so they can be used for the timers.
     Note: this has to be done in main() before any threads are created
     so they all inherit the same mask. Doing it later is subject to
     race conditions */
    sigemptyset (&alarm_sig);
    for (i = SIGRTMIN; i <= SIGRTMAX; i++) {
        sigaddset (&alarm_sig, i);
    }
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
    
    displayInit(SIGRTMAX);
    sleep(50);
    return (0);
}
#endif



