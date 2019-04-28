#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include "structs.c"

#define MINISTRY_MUTEX "ministry_mutex"

FILE * EXECUTIVE_F;
FILE * LEGISLATIVE_F;
FILE * JUDICIAL_F;
FILE * MINISTRIES_F;

int main(int argc, char const *argv[]) {
	pid_t master = getpid();
	int ex_jud[2];
	int ex_leg[2];
	int leg_jud[2];
	int jud_leg[2];
	int ex_press[2];
	int leg_press[2];
	int jud_press[2];
	int press_leg[2];
	pipe(ex_jud);
	pipe(ex_leg);
	pipe(leg_jud);
	pipe(jud_leg);
	pipe(ex_press);
	pipe(leg_press);
	pipe(jud_press);
	pipe(press_leg);
	// Mutex shared between processes
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	// sem_init() is used for threads
	sem_close(ministry_mutex);
	EXECUTIVE_F = fopen("Ejecutivo.acc", "r+");
	if (EXECUTIVE_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Ejecutivo.acc");
		return 1;
	}
	LEGISLATIVE_F = fopen("Legislativo.acc", "r+");
	if (LEGISLATIVE_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Legislativo.acc");
		return 1;
	}
	JUDICIAL_F = fopen("Judicial.acc", "r+");
	if (JUDICIAL_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Judicial.acc");
		return 1;
	}
	MINISTRIES_F = fopen("Ministros.txt", "w+");
	if (MINISTRIES_F == NULL) {
		fprintf(stderr, "%s\n", "Cannot create file: Ministros.txt");
		return 1;
	}
	fclose(MINISTRIES_F);

	if (fork() == 0) {
		return press_task(getpid());
	}
	pid_t t1, t2, t3;
	for (int i = 0; i < 3; i++) {
		if (fork() == 0) {
			if (i == 0) {
				return executive_task((t1 = getpid()), ex_jud, ex_leg, ex_press);
			}
			else if (i == 1) {
				return legislative_task((t2 = getpid()), ex_leg, leg_jud, jud_leg, leg_press, press_leg);
			}
			else {
				return judicial_task((t3 = getpid()), ex_jud, leg_jud, jud_leg, jud_press);
			}
		}
	}
	wait(&t1);
	wait(&t2);
	wait(&t3);
	sem_unlink(MINISTRY_MUTEX);
	return 0;
}

static int executive_task(pid_t id, int ex_jud[2], int ex_leg[2], int ex_press[2]){
	// Initial config
	executive president = *create_executive(id);

	// Task management
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT);
	sem_wait(ministry_mutex);
	printf("%s\n", "I have control (P)");
	sleep(5);
	sem_post(ministry_mutex);

	sem_close(ministry_mutex);
	return 0;
}

static int legislative_task(pid_t id, int ex_leg[2], int leg_jud[2], int jud_leg[2], int leg_press[2], int press_leg[2]){
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
	sleep(1);
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT);
	sem_wait(ministry_mutex);
	printf("%s\n", "Now its my turn (C)");
	sleep(3);
	sem_post(ministry_mutex);
	sem_close(ministry_mutex);
	return 0;
}
static int judicial_task(pid_t id, int ex_jud[2], int leg_jud[2], int jud_leg[2], int jud_press[2]){
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