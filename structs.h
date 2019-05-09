#include <signal.h>
#include <semaphore.h>
#include "ordered_list.c"
#define NAME_LEN 40
#define LINE_LEN 250

typedef struct {
	pid_t from;
	pid_t to;
	int response;
} request;

typedef struct {
	pid_t id;
	float success_rate;
	// lista de requests
} executive;

typedef struct {
	pid_t id;
	float success_rate;
} legislative;

typedef struct {
	pid_t id;
	float success_rate;
	float * magister;
	int mag_count;
} judicial;

typedef struct {
	pid_t id;
	// Nombre del ministerio. como maximo tiene NAME_LEN caracteres
	char * name;
	// Nombre del ministro. Como maximo tiene NAME_LEN caracteres
	char * minister;
	float success_rate;
} ministry;

typedef struct {
	char * name;
	char * success;
	char * failure;
} action;

executive * create_executive(pid_t id);

legislative * create_legislative(pid_t id);

judicial * create_judicial(pid_t id);

void init_ministry(char * name);

ministry * create_ministry(pid_t id);

request * create_request(pid_t from, pid_t to, pid_t value);

static int executive_task(pid_t id, int ex_jud[2], int ex_leg[2], int ex_press[2]);

static int legislative_task(pid_t id, int ex_leg[2], int leg_jud[2], int jud_leg[2], int leg_press[2], int press_leg[2]);

static int judicial_task(pid_t id, int ex_jud[2], int leg_jud[2], int jud_leg[2], int jud_press[2]);

static int ministry_task(pid_t id);

static void sig_handler_leg_usr1(int signal);

static void sig_handler_leg_usr2(int signal);

static void sig_handler_jud_usr1(int signal);

static void sig_handler_jud_usr2(int signal);

char * read_keyword(FILE * f);

int accepted(float success_rate);

void write_pipe(const char msg[], int pipe[2]);

void send_president_request(pid_t from, pid_t to, int response);

void parse_request(char * request, char * from, char * to, char *response);

void write_metadata(const char C);

void process_metadata(void);

int find_string(char * string, FILE * f);

void end_reccess(sem_t * t1, pid_t id);