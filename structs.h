#include <signal.h>
#include "ordered_list.c"
#define NAME_LEN 40

typedef struct {
	pid_t from;
	pid_t to;
} request;

typedef struct {
	pid_t id;
	float success_rate;
	// lista de requests
	list_t requests;
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

executive * create_executive(pid_t id);

legislative * create_legislative(pid_t id);

judicial * create_judicial(pid_t id);

ministry * create_ministry(pid_t id);

static int press_task(pid_t id);

static int executive_task(pid_t id);

static int legislative_task(pid_t id);

static int judicial_task(pid_t id);

static int ministry_task(pid_t id);

static void sig_handler_leg_usr1(int signal);

static void sig_handler_leg_usr2(int signal);

static void sig_handler_jud_usr1(int signal);

static void sig_handler_jud_usr2(int signal);