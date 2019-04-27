#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "structs.c"

FILE * EXECUTIVE_F;
FILE * LEGISLATIVE_F;
FILE * JUDICIAL_F;
list_t * ministries;

int main(int argc, char const *argv[]) {
	pid_t master = getpid();
	int ex_jud[2];
	int ex_leg[2];
	int leg_jud[2];
	int jud_leg[2];
	int ex_press[2];
	int leg_press[2];
	int jud_press[2];
	pipe(ex_jud);
	pipe(ex_leg);
	pipe(leg_jud);
	pipe(jud_leg);
	pipe(ex_press);
	pipe(leg_press);
	pipe(jud_press);
	pipe(press_leg);
	ministries = create_ordered_list();

	if (fork() == 0) {
		return press_task(getpid());
	}
	for (int i = 0; i < 3; i++) {
		if (fork() == 0) {
			if (i == 0) {
				return executive_task(getpid());
			}
			else if (i == 1) {
				return legislative_task(getpid());
			}
			else {
				return judicial_task(getpid());
			}
		}
	}

	return 0;
}

static int executive_task(pid_t id){
	// Initial config
	executive president = *create_executive(id);

	// Task management

	return 0;
}

static int legislative_task(pid_t id){
	// Initial Config
	legislative congress = *create_legislative(id);
	struct sigaction act;
	act.sa_handler = sig_handler_leg_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	sigaddset(&(act.sa_mask), SIGUSR2);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = sig_handler_leg_usr2;
	sigaction(SIGUSR2, &act, NULL);

	// Task management

	return 0;
}
static int judicial_task(pid_t id){
	judicial tribune = *create_judicial(id);
	struct sigaction act;
	act.sa_handler = sig_handler_jud_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	sigaddset(&(act.sa_mask), SIGUSR2);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = sig_handler_jud_usr2;
	sigaction(SIGUSR2, &act, NULL);

	// Task Management

	return 0;
}

static int ministry_task(pid_t id){
	return 0;
}

static int press_task(pid_t id) {
	return 0;
}

static void sig_handler_leg_usr1(int signal) {
	// Recibimos del ejecutivo
}

static void sig_handler_leg_usr2(int signal) {
	// Recibimos del judicial
}


static void sig_handler_jud_usr1(int signal) {
	// Recibimos del ejecutivo
}

static void sig_handler_jud_usr2(int signal) {
	// Recibimos del legislativo
}