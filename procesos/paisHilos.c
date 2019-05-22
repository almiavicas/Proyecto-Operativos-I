#include "structst.c"
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

const char file_exec[] = "Ejecutivo.txt";
const char file_leg[] = "Legislativo.txt";
const char file_jud[] = "Judicial.txt";
const char aprobacion[] = "aprobacion";
const char reprobacion[] = "reprobacion";
const char asignar[] = "asignar";
const char exclusivo[] = "exclusivo";
const char inclusivo[] = "inclusivo";
const char leer[] = "leer";
const char escribir[] = "escribir";
const char anular[] = "anular";
const char nombrar[] = "nombrar";
const char destituir[] = "destituir";
const char disolver[] = "disolver";
const char censurar[] = "censurar";
const char crear[] = "crear";
const char renuncia[] = "renuncia";
const char exito[] = "exito";
const char fracaso[] = "fracaso";
const char presidente[] = "Presidente";
const char congreso[] = "Congreso";
const char tribunal[] = "Tribunal Supremo";
const char end_task[] = "END_TASK";
const char bksp50[] = "                                                  ";
const char bksp100[] = "                                                                                                    ";
const char PERM[] = "PERM";

FILE * EXECUTIVE_F;
FILE * LEGISLATIVE_F;
FILE * JUDICIAL_F;
FILE * MINISTRIES_F;
FILE * PRESIDENT_REQUESTS_F;
FILE * METADATA_F;

pthread_t executive;
pthread_t legislative;
pthread_t judicial;
pthread_mutex_t mutex;


void sig_handler(int signum){

    printf("Received SIGINT. Exit\n");

    pthread_cancel(executive);
    pthread_cancel(legislative);
    pthread_cancel(judicial);

    exit(0);
}

void executive_main(void) {
    while(1) {
        usleep(1000000);
        EXECUTIVE_F = fopen(file_exec, "r+");
        fseek(EXECUTIVE_F, 0, SEEK_SET);
        while (!feof(EXECUTIVE_F)){
            printf("Reading actions from Ejecutivo.txt\n");
            usleep(1000000);
        }
    }
}


void legislative_main(void) {
    while(1) {
        usleep(1000000);
        LEGISLATIVE_F = fopen(file_leg, "r+");
        fseek(LEGISLATIVE_F, 0, SEEK_SET);
        while (!feof(LEGISLATIVE_F)){
            printf("Reading actions from Legislativo.txt\n");
            usleep(1000000);
        }
    }
}

void judicial_main(void) {
    while(1) {
        usleep(1000000);
        JUDICIAL_F = fopen(file_jud, "r+");
        fseek(JUDICIAL_F, 0, SEEK_SET);
        while (!feof(JUDICIAL_F)){
            printf("Reading actions from Judicial.txt\n");
            usleep(1000000);
        }
    }
}

int main(void) {
    pthread_attr_t attr;
    int status;
    EXECUTIVE_F = fopen(file_exec, "r+");
 
    signal(SIGINT, sig_handler);

    pthread_attr_init(&attr);
   
    printf("Creating executive\n");
    status = pthread_create(&executive, &attr, (void*)&executive_main, NULL);   

    printf("Creating legislative\n");
    status = pthread_create(&legislative, &attr, (void*)&legislative_main, NULL); 

    printf("Creating judicial\n");
    status = pthread_create(&judicial, &attr, (void*)&judicial_main, NULL); 

    pthread_join(executive, NULL);
    pthread_join(legislative, NULL);
    pthread_join(judicial, NULL);

    sig_handler(SIGINT);
    
    return 0;
}