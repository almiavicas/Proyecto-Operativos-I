#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include "structs.c"

#define MINISTRY_MUTEX "ministry_mutex"
#define EXECUTIVE_SEM "exec_sem"
#define LEGISLATIVE_SEM "leg_sem"
#define JUDICIAL_SEM "jud_sem"
#define REQUEST_SEM "req_sem"
#define LINE_LEN 100

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
const char renuncia[] = "renuncia";
const char exito[] = "exito";
const char fracaso[] = "fracaso";
const char presidente[] = "Presidente";
const char congreso[] = "Congreso";
const char tribunal[] = "Tribunal Supremo";

FILE * EXECUTIVE_F;
FILE * LEGISLATIVE_F;
FILE * JUDICIAL_F;
FILE * MINISTRIES_F;
FILE * PRESIDENT_REQUESTS_F;
pid_t exec_id, leg_id, jud_id;
executive president;
legislative congress;
judicial tribune;
int ex_jud[2];
int ex_leg[2];
int leg_jud[2];
int jud_leg[2];
int ex_press[2];
int leg_press[2];
int jud_press[2];
int press_leg[2];

int main(int argc, char const *argv[]) {
	pid_t master = getpid();
	pipe(ex_jud);
	pipe(ex_leg);
	pipe(leg_jud);
	pipe(jud_leg);
	pipe(ex_press);
	pipe(leg_press);
	pipe(jud_press);
	pipe(press_leg); 
	list_t president_requests = *new_ordered_list();
	// Mutex shared between processes
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	// These mutexes are for the press process. When they are unlocked, The
	// press reads from its corresponding pipe and prints
	sem_t *exec_mutex = sem_open(EXECUTIVE_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
	sem_t *leg_mutex = sem_open(LEGISLATIVE_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
	sem_t *jud_mutex = sem_open(JUDICIAL_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
	sem_t *req_mutex = sem_open(REQUEST_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	// sem_init() is used for threads
	sem_close(ministry_mutex);
	sem_close(req_mutex);
	
	// Pipes config for press
	close(ex_press[1]);
	close(leg_press[1]);
	close(jud_press[1]);
	close(press_leg[0]);

	EXECUTIVE_F = fopen("Ejecutivo.acc", "r+");
	if (EXECUTIVE_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Ejecutivo.acc");
		return 1;
	}
	fclose(EXECUTIVE_F);
	LEGISLATIVE_F = fopen("Legislativo.acc", "r+");
	if (LEGISLATIVE_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Legislativo.acc");
		return 1;
	}
	fclose(LEGISLATIVE_F);
	JUDICIAL_F = fopen("Judicial.acc", "r+");
	if (JUDICIAL_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Judicial.acc");
		return 1;
	}
	fclose(JUDICIAL_F);
	MINISTRIES_F = fopen("Ministros.txt", "w+");
	if (MINISTRIES_F == NULL) {
		fprintf(stderr, "%s\n", "Cannot create file: Ministros.txt");
		return 1;
	}
	fclose(MINISTRIES_F);
	PRESIDENT_REQUESTS_F = fopen("PedidosPresidenciales.txt", "w+");
	if (PRESIDENT_REQUESTS_F == NULL) {
		fprintf(stderr, "%s\n", "Cannot create file: PedidosPresidenciales.txt");
		return 1;
	}
	fclose(PRESIDENT_REQUESTS_F);

	for (int i = 0; i < 3; i++) {
		if (fork() == 0) {
			if (i == 0) {
				return executive_task((exec_id = getpid()), ex_jud, ex_leg, ex_press);
			}
			else if (i == 1) {
				return legislative_task((leg_id = getpid()), ex_leg, leg_jud, jud_leg, leg_press, press_leg);
			}
			else {
				return judicial_task((jud_id = getpid()), ex_jud, leg_jud, jud_leg, jud_press);
			}
		}
	}

	int num_actions = 0;
	int max_actions = atoi(argv[2]);
	while (num_actions < max_actions) {
		if (sem_trywait(exec_mutex) == 0) {
			// Read from ex_press pipe

		}
		if (sem_trywait(leg_mutex) == 0) {
			// Read from leg_press pipe

		}
		if (sem_trywait(jud_mutex) == 0) {
			// Read from jud_press pipe

		}
		usleep(50);
	}

	kill(exec_id, SIGTERM);
	kill(leg_id, SIGTERM);
	kill(jud_id, SIGTERM);

	sem_close(exec_mutex);
	sem_close(leg_mutex);
	sem_close(jud_mutex);

	sem_unlink(MINISTRY_MUTEX);
	sem_unlink(EXECUTIVE_SEM);
	sem_unlink(LEGISLATIVE_SEM);
	sem_unlink(JUDICIAL_SEM);
	sem_unlink(REQUEST_SEM);
	return 0;
}

static int executive_task(pid_t id, int ex_jud[2], int ex_leg[2], int ex_press[2]){
	// Initial config
	president = *create_executive(id);
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT);
	close(ex_jud[0]);
	close(ex_leg[0]);
	close(ex_press[0]);
	EXECUTIVE_F = fopen("Ejecutivo.acc", "r+");
	sigset_t mask;
	sigfillset(&mask);
	// This is the signal we don't want to block when waiting responses, since
	// it will be the one that the congress and tribune will send
	sigdelset(&mask, SIGUSR1);

	// Task management
	while (1) {
		while (feof(EXECUTIVE_F)) {
			action act;
			act.name = fgets(act.name, LINE_LEN, EXECUTIVE_F);
			int end = 0;
			int success = 1;
			while (1) {
				if (end) break;
				char * keyword = read_keyword(EXECUTIVE_F);
				char * value = fgets(value, LINE_LEN, EXECUTIVE_F);
				while(value[0] = ' ') ++value;
				if (keyword == NULL) {
					fprintf(stderr, "%s\n", "Error reading file Ejecutivo.acc");
					return -1;
				}
				if (!strcmp(keyword, aprobacion)) {
					if (!strcmp(value, presidente)) {
						if (success) success = accepted(president.success_rate);
					}
					else if(!strcmp(value, congreso)) {
						write_pipe("PP", ex_leg);
						// We interrupt the congress to tell them to answer us
						kill(leg_id, SIGUSR1);
						// We wait until the congress responds. We
						// suspend this process until SIGUSR1 is caught by it
						sigsuspend(&mask);
						// At this point, the default sigset_t configuration of
						// the process will restore itself.
						// Now we check what the congress answered
						sem_t * req_mutex = sem_open(REQUEST_SEM, O_CREAT);
						sem_wait(req_mutex);
						PRESIDENT_REQUESTS_F = fopen("PedidosPresidenciales.txt", "r+");
						char * from_id;
						char * to_id;
						char response;
						char * line;
						while (!feof(PRESIDENT_REQUESTS_F)) {
							fgets(PRESIDENT_REQUESTS_F, line, LINE_LEN);
							parse_response(line, from_id, to_id, &response);
							int from = atoi(from_id);
							// We are searching the request that was sent by
							// the president.
							if (from == exec_id && response = '0') success = 0;
						}
						sem_post(req_mutex);
						sem_close(req_mutex);
					}
					else if(!strcmp(value, tribunal)) {

					}
					else {
						fprintf(stderr, "%s%s\n", "Error parsing power name: ", value);
						return -1;
					}
				}
				else if (!strcmp(keyword, reprobacion)) {

				}
				else if (!strcmp(keyword, asignar)) {
					
				}
				else if (!strcmp(keyword, exclusivo)) {
					
				}
				else if (!strcmp(keyword, inclusivo)) {
					
				}
				else if (!strcmp(keyword, leer)) {
					
				}
				else if (!strcmp(keyword, escribir)) {
					
				}
				else if (!strcmp(keyword, anular)) {
					
				}
				else if (!strcmp(keyword, nombrar)) {
					
				}
				else if (!strcmp(keyword, destituir)) {
					
				}
				else if (!strcmp(keyword, disolver)) {
					
				}
				else if (!strcmp(keyword, censurar)) {
					
				}
				else if (!strcmp(keyword, renuncia)) {
					
				}
				else if (!strcmp(keyword, exito)) {
					act.success = value;
				}
				else if (!strcmp(keyword, fracaso)) {
					act.failure = value;
					end = 1;
				}
				else {
					fprintf(stderr, "%s\n", "Error reading file Ejecutivo.acc");
					return -1;
				}
			}
		}
	}
	sem_wait(ministry_mutex);
	//printf("%s\n", "I have control (P)");
	sleep(5);
	sem_post(ministry_mutex);

	sem_close(ministry_mutex);
	return 0;
}

static int legislative_task(pid_t id, int ex_leg[2], int leg_jud[2], int jud_leg[2], int leg_press[2], int press_leg[2]){
	// Initial Config
	congress = *create_legislative(id);
	struct sigaction act;
	act.sa_handler = sig_handler_leg_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	sigaddset(&(act.sa_mask), SIGUSR2);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = sig_handler_leg_usr2;
	sigaction(SIGUSR2, &act, NULL);
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT);
	close(ex_leg[1]);
	close(leg_jud[0]);
	close(jud_leg[1]);
	close(leg_press[0]);
	close(press_leg[1]);
	LEGISLATIVE_F = fopen("Legislativo.acc", "r+");

	// Task management
	while (1) {
		while (feof(LEGISLATIVE_F)) {
			action act;
			act.name = fgets(act.name, LINE_LEN, LEGISLATIVE_F);
			int end = 0;
			while (1) {
				if (end) break;
				char * keyword = read_keyword(LEGISLATIVE_F);
				char * value = fgets(value, LINE_LEN, LEGISLATIVE_F);
				if (keyword == NULL) {
					fprintf(stderr, "%s\n", "Error reading file Legislativo.acc");
					return -1;
				}
				if (!strcmp(keyword, aprobacion)) {

				}
				else if (!strcmp(keyword, reprobacion)) {

				}
				else if (!strcmp(keyword, asignar)) {
					
				}
				else if (!strcmp(keyword, exclusivo)) {
					
				}
				else if (!strcmp(keyword, inclusivo)) {
					
				}
				else if (!strcmp(keyword, leer)) {
					
				}
				else if (!strcmp(keyword, escribir)) {
					
				}
				else if (!strcmp(keyword, anular)) {
					
				}
				else if (!strcmp(keyword, nombrar)) {
					
				}
				else if (!strcmp(keyword, destituir)) {
					
				}
				else if (!strcmp(keyword, disolver)) {
					
				}
				else if (!strcmp(keyword, censurar)) {
					
				}
				else if (!strcmp(keyword, renuncia)) {
					
				}
				else if (!strcmp(keyword, exito)) {
					act.success = value;
				}
				else if (!strcmp(keyword, fracaso)) {
					act.failure = value;
					end = 1;
				}
				else {
					fprintf(stderr, "%s\n", "Error reading file Legislativo.acc");
					return -1;
				}
			}
		}
	}
	sleep(1);
	
	sem_wait(ministry_mutex);
	//printf("%s\n", "Now its my turn (C)");
	sleep(3);
	sem_post(ministry_mutex);
	sem_close(ministry_mutex);
	return 0;
}
static int judicial_task(pid_t id, int ex_jud[2], int leg_jud[2], int jud_leg[2], int jud_press[2]){
	tribune = *create_judicial(id);
	struct sigaction act;
	act.sa_handler = sig_handler_jud_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	sigaddset(&(act.sa_mask), SIGUSR2);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = sig_handler_jud_usr2;
	sigaction(SIGUSR2, &act, NULL);
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT);
	close(ex_jud[1]);
	close(leg_jud[1]);
	close(jud_leg[0]);
	close(jud_press[0]);
	JUDICIAL_F = fopen("Judicial.acc", "r+");

	// Task Management
	while (1) {
		while (feof(JUDICIAL_F)) {
			action act;
			act.name = fgets(act.name, LINE_LEN, JUDICIAL_F);
			int end = 0;
			while (1) {
				if (end) break;
				char * keyword = read_keyword(JUDICIAL_F);
				char * value = fgets(value, LINE_LEN, JUDICIAL_F);
				if (keyword == NULL) {
					fprintf(stderr, "%s\n", "Error reading file Judicial.acc");
					return -1;
				}
				if (!strcmp(keyword, aprobacion)) {

				}
				else if (!strcmp(keyword, reprobacion)) {

				}
				else if (!strcmp(keyword, asignar)) {
					
				}
				else if (!strcmp(keyword, exclusivo)) {
					
				}
				else if (!strcmp(keyword, inclusivo)) {
					
				}
				else if (!strcmp(keyword, leer)) {
					
				}
				else if (!strcmp(keyword, escribir)) {
					
				}
				else if (!strcmp(keyword, anular)) {
					
				}
				else if (!strcmp(keyword, nombrar)) {
					
				}
				else if (!strcmp(keyword, destituir)) {
					
				}
				else if (!strcmp(keyword, disolver)) {
					
				}
				else if (!strcmp(keyword, censurar)) {
					
				}
				else if (!strcmp(keyword, renuncia)) {
					
				}
				else if (!strcmp(keyword, exito)) {
					act.success = value;
				}
				else if (!strcmp(keyword, fracaso)) {
					act.failure = value;
					end = 1;
				}
				else {
					fprintf(stderr, "%s\n", "Error reading file Judicial.acc");
					return -1;
				}
			}
		}
	}
	return 0;
}

static int ministry_task(pid_t id){
	ministry mi = *create_ministry(id);
	return 0;
}

static void sig_handler_leg_usr1(int signal) {
	// Recibimos del ejecutivo
	if (&congress != NULL) {
		char line[2];
		read(ex_leg[0], line, 2);
		if (line[1] = 'P') {
			int success = accepted(congress.success_rate);
			// Send the answer to president
			send_president_request(exec_id, exec_id, success);
			// Tell the president we answered
			kill(exec_id, SIGUSR1);
		}
		else if (line[1] = 'S') {

		}
		else if (line[1] = 'N') {

		}
		else {
			fprintf(stderr, "%s\n", "Error in pipes communication");
		}
	}
	else {
		// The congress doesn't exists, the action fails
		send_president_request(exec_id, exec_id, 0);
		kill(exec_id, SIGUSR1);
	}
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

void send_president_request(pid_t from, pid_t to, int result) {
	sem_t * req_mutex = sem_open(REQUEST_SEM, O_CREAT);
	request req = *create_request(from, to, result);
	sem_wait(req_mutex);
	PRESIDENT_REQUESTS_F = fopen("PedidosPresidenciales.txt", "a+");
	if (PRESIDENT_REQUESTS_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: PedidosPresidenciales.txt");
		return;
	}
	fprintf(PRESIDENT_REQUESTS_F, "%d %d %d\n", from, to, result);
	fclose(PRESIDENT_REQUESTS_F);
	sem_post(req_mutex);
	sem_close(req_mutex);
}

void parse_response(char * request, char * from, char * to, char * response) {
	int i = 0;
	int j = 0;
	while ((from[i++] = request[j++]) != ' ');
	j++;
	i = 0;
	while ((to[i++] = request[j++]) != ' ');
	j++;
	*response = request[j];
}