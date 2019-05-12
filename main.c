#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "structs.c"


#define METADATA_MUTEX "metadata_mutex"
#define MINISTRY_MUTEX "ministry_mutex"
#define REQUEST_SEM "req_sem"
// Semaphores to activate the press
#define EXECUTIVE_PRINT_SEM "exec_print_sem"
#define LEGISLATIVE_PRINT_SEM "leg_print_sem"
#define JUDICIAL_PRINT_SEM "jud_print_sem"
// Semaphore to manage reccessess
#define EXECUTIVE_REC "exec_reccess"
#define LEGISLATIVE_REC "leg_reccess"
#define JUDICIAL_REC "jud_reccess"


const char file_exec[] = "Ejecutivo.acc";
const char file_leg[] = "Legislativo.acc";
const char file_jud[] = "Judicial.acc";
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

FILE * EXECUTIVE_F;
FILE * LEGISLATIVE_F;
FILE * JUDICIAL_F;
FILE * MINISTRIES_F;
FILE * PRESIDENT_REQUESTS_F;
FILE * METADATA_F;
pid_t master, exec_id, leg_id, jud_id;
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
	// Initial config
	struct sigaction act;
	act.sa_handler = sig_handler_master;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	master = getpid();
	printf("%s %d\n", "Parent id:", master);
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
	// These mutexes are for the press process. When they are unlocked, The
	// press reads from its corresponding pipe and prints
	sem_t *exec_print_sem = sem_open(EXECUTIVE_PRINT_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
	sem_t *leg_print_sem = sem_open(LEGISLATIVE_PRINT_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
	sem_t *jud_print_sem = sem_open(JUDICIAL_PRINT_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
	// Mutex to open the requests file of the president
	sem_t *req_mutex = sem_open(REQUEST_SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	// Mutex to read and write metadata
	sem_t *meta_mutex = sem_open(METADATA_MUTEX, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	// Mutex to go into recess
	sem_t *ex_reccess = sem_open(EXECUTIVE_REC, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	sem_t *leg_reccess = sem_open(LEGISLATIVE_REC, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	sem_t *jud_reccess = sem_open(JUDICIAL_REC, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	
	// sem_init() is used for threads
	sem_close(ministry_mutex);
	sem_close(req_mutex);
	sem_close(meta_mutex);

	
	// Pipes config for press
	close(ex_press[1]);
	close(leg_press[1]);
	close(jud_press[1]);
	close(press_leg[0]);
	close(ex_jud[0]);

	EXECUTIVE_F = fopen(file_exec, "r+");
	if (EXECUTIVE_F == NULL) {
		fprintf(stderr, "%s %s\n", "File not found:", file_exec);
		return 1;
	}
	fclose(EXECUTIVE_F);
	LEGISLATIVE_F = fopen(file_leg, "r+");
	if (LEGISLATIVE_F == NULL) {
		fprintf(stderr, "%s %s\n", "File not found:", file_leg);
		return 1;
	}
	fclose(LEGISLATIVE_F);
	JUDICIAL_F = fopen(file_jud, "r+");
	if (JUDICIAL_F == NULL) {
		fprintf(stderr, "%s %s\n", "File not found:", file_jud);
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
	METADATA_F = fopen("Metadata.txt", "w+");
	if (METADATA_F == NULL) {
		fprintf(stderr, "%s\n", "Cannot create file: Metadata.txt");
		return 1;
	}
	fclose(METADATA_F);
	// Create powers
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
		kill(master, SIGSTOP);
		printf("%s\n", "Received a signal from a child");
	}
	sleep(1);
	printf("%s\n", "My childs are ready for metadata");
	process_metadata();
	kill(exec_id, SIGCONT);
	kill(leg_id, SIGCONT);
	kill(jud_id, SIGCONT);

	// Here starts the press process.
	int num_actions = 0;
	int max_actions = atoi(argv[2]);

	while (num_actions < max_actions) {
		if (sem_trywait(exec_print_sem) == 0) {
			// Read from ex_press pipe
			char * message;
			read(ex_press[0], message, LINE_LEN);
			if (message[0] == 'C' && message[1] == 'C') {
				if (fork()==0){
					return legislative_task((leg_id = getpid()), ex_leg, leg_jud, jud_leg, leg_press, press_leg);
				}
				else {
				// Update metadata 
					kill(master, SIGSTOP);
					process_metadata();
					kill(leg_id, SIGCONT);

					// Tell the president to eliminate some actions

					// Wait for the congress to end
					// Continue congress
					kill(leg_id, SIGCONT);

					write_pipe("UM",ex_jud);
					kill(jud_id, SIGUSR1);
				}	
			}
			else {
				printf("%d: %s\n", num_actions++, message);
				end_reccess(ex_reccess, exec_id);
				end_reccess(leg_reccess, leg_id);
				end_reccess(jud_reccess, jud_id);
			}
			
		}
		if (sem_trywait(leg_print_sem) == 0) {
			// Read from leg_press pipe
			char * message;
			read(leg_press[0], message, LINE_LEN);
			if (message[0] == 'P' && message[1] == 'C') {
				if (fork()==0){
					return executive_task((exec_id = getpid()), ex_jud, ex_leg, ex_press);
				}
				else {
				// Update metadata 
					kill(master, SIGSTOP);
					process_metadata();
					kill(exec_id, SIGCONT);
					kill(exec_id, SIGUSR1);
					// Wait for the president to end
					// Continue congress
					kill(leg_id, SIGCONT);

					write_pipe("UM",ex_jud);
					kill(jud_id, SIGUSR1);
				}	
			}
			else {
				printf("%d: %s\n", num_actions++, message);
				end_reccess(ex_reccess, exec_id);
				end_reccess(leg_reccess, leg_id);
				end_reccess(jud_reccess, jud_id);
			}
		}
		if (sem_trywait(jud_print_sem) == 0) {
			// Read from jud_press pipe
			char * message;
			read(jud_press[0], message, LINE_LEN);
			printf("%d: %s\n", num_actions++, message);
			end_reccess(ex_reccess, exec_id);
			end_reccess(leg_reccess, leg_id);
			end_reccess(jud_reccess, jud_id);
		}
		usleep(50);
	}

	sem_close(exec_print_sem);
	sem_close(leg_print_sem);
	sem_close(jud_print_sem);
	sem_close(ex_reccess);
	sem_close(leg_reccess);
	sem_close(jud_reccess);

	// method to terminate the program
	sig_handler_master(SIGUSR1);
	return 0;
}

static int executive_task(pid_t id, int ex_jud[2], int ex_leg[2], int ex_press[2]){
	// Initial config
	president = *create_executive(id);
	struct sigaction act;
	act.sa_handler = sig_handler_exec_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	sem_t *ministry_mutex = sem_open(MINISTRY_MUTEX, O_CREAT);
	sem_t *ex_reccess = sem_open(EXECUTIVE_REC, O_CREAT);
	sem_t *print_sem = sem_open(EXECUTIVE_PRINT_SEM, O_CREAT);
	close(ex_jud[0]);
	close(ex_leg[0]);
	close(ex_press[0]);
	EXECUTIVE_F = fopen(file_exec, "r+");
	write_metadata('P');
	printf("%s %d %s %d\n", "sending signal from", getpid(), "to", master);
	fflush(stdout);
	kill(master, SIGCONT);
	// We wait until the parent tells us the metadata is ready
	kill(getpid(), SIGSTOP);
	printf("%s\n", "Daddy woke me up");
	process_metadata();
	return 0;

	// Task management
	while (1) {
		int reccess_time = 1;
		int reccess_count = 0;
		fseek(EXECUTIVE_F, 0, SEEK_SET);
		while (feof(EXECUTIVE_F)) {
			action act;
			long int action_start = ftell(EXECUTIVE_F);
			act.name = fgets(act.name, LINE_LEN, EXECUTIVE_F);
			if (!accepted(0.2f)) {
				while (strlen(act.name) > 2 && feof(EXECUTIVE_F)) fgets(act.name, LINE_LEN, EXECUTIVE_F);
				while (strlen(act.name) < 2 && feof(EXECUTIVE_F)) fgets(act.name, LINE_LEN, EXECUTIVE_F);
				if (feof(EXECUTIVE_F)) fseek(EXECUTIVE_F, - strlen(act.name), SEEK_CUR);
				continue;
			}
			int end = 0;
			FILE * opened_file;
			sem_t * mutex;
			// the next int is to manage the mutex behaviour. -1 means
			// uninitialized, 1 means exclusive and 0 means inclusive
			int exclusive_sem = -1;
			// This int tells us if we have written in the file. If that
			// happens, we will have to add a new line at the end of file
			int wrote = 0;
			// The success variable can have 3 states in the executive power:
			// 	0 -> means unsuccessful
			// 	1 -> means successful
			// 	2 -> means the action was sent to a minister, so no printing is needed.
			int success = 1;
			while (!end) {
				char * keyword = read_keyword(EXECUTIVE_F);
				char * value = fgets(value, LINE_LEN, EXECUTIVE_F);
				value[strlen(value)-1] = '\0';
				while(value[0] = ' ') ++value;
				if (keyword == NULL) {
					fprintf(stderr, "%s %s\n", "Error reading file:", file_exec);
					return -1;
				}
				if (!(strcmp(keyword, aprobacion) && strcmp(keyword, reprobacion)) && success) {
					if (!strcmp(value, presidente)) {
						success = accepted(president.success_rate);
					}
					else if(!strcmp(value, congreso)) {
						write_pipe("PP", ex_leg);
						// We interrupt the congress to tell them to answer us
						kill(leg_id, SIGUSR1);
						// We wait until the congress responds. We
						// suspend this process until SIGCONT is caught by it
						kill(exec_id, SIGSTOP);
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
						while (feof(PRESIDENT_REQUESTS_F)) {
							fgets(line, LINE_LEN, PRESIDENT_REQUESTS_F);
							parse_request(line, from_id, to_id, &response);
							int from = atoi(from_id);
							// We are searching the request that was sent by
							// the president.
							if (from == exec_id) {
								if (response == '0') success = 0;
								break;
							} 
						}
						fclose(PRESIDENT_REQUESTS_F);
						sem_post(req_mutex);
						sem_close(req_mutex);
					}
					else if(!strcmp(value, tribunal)) {
						write_pipe("PP", ex_jud);
						kill(jud_id, SIGUSR1);
						sem_t * req_mutex = sem_open(REQUEST_SEM, O_CREAT);
						sem_wait(req_mutex);
						PRESIDENT_REQUESTS_F = fopen("PedidosPresidenciales.txt", "r+");
						char * from_id;
						char * to_id;
						char response;
						char * line;
						while (feof(PRESIDENT_REQUESTS_F)) {
							fgets(line, LINE_LEN, PRESIDENT_REQUESTS_F);
							parse_request(line, from_id, to_id, &response);
							int from = atoi(from_id);
							// We are searching the request that was sent by
							// the president.
							if (from == exec_id) {
								if (response = '0') success = 0;
								break;
							} 
						}
						fclose(PRESIDENT_REQUESTS_F);
						sem_post(req_mutex);
						sem_close(req_mutex);
					}
					else {
						fprintf(stderr, "%s%s\n", "Error parsing power name: ", value);
						return -1;
					}
				}
				else if (!strcmp(keyword, crear) && success) {
					sem_wait(ministry_mutex);
					if (success = accepted(president.success_rate)) {
						init_ministry(value);
						fork();
						if (getpid() != exec_id) {
							sem_close(ministry_mutex);
							return ministry_task(getpid());
						}
					}
					
					sem_post(ministry_mutex);
				}
				else if (!strcmp(keyword, asignar) && success) {
					// Asigna esta tarea a un ministro
					sem_wait(ministry_mutex);
					MINISTRIES_F = fopen("Ministros.txt", "r+");
					int found = 0;
					while(feof(MINISTRIES_F)) {
						char * line;
						fgets(line, LINE_LEN, MINISTRIES_F);
						int equal = 1;
						for (int i = 0; i < strlen(value) && equal; i++) {
							if (value[i] != line[i]) equal = 0;
						}
						if (equal) {
							found = 1;
							// We move the cursor to after the '|' char
							fseek(MINISTRIES_F, -(strlen(bksp100) + strlen(bksp50) + 2), SEEK_CUR);
							fprintf(MINISTRIES_F, "%s", act.name);
							fseek(MINISTRIES_F, strlen(bksp100) - strlen(act.name) + 1, SEEK_CUR);
							fprintf(MINISTRIES_F, "%lo\n", ftell(EXECUTIVE_F));
							break;
						}
					}
					fclose(MINISTRIES_F);
					sem_post(ministry_mutex);
					if (!found) success = 0;
					else success = 2;
					end = 1;
				}
				else if (!strcmp(keyword, exclusivo) && success) {
					if (opened_file != NULL) {
						if (wrote) fprintf(opened_file, "\n");
						fclose(opened_file);
					}
					if (exclusive_sem == 1) {
						// If the last mutex was exclusive, we need to post and close
						sem_post(mutex);
						sem_close(mutex);
					}
					// If the las mutex was inclusive, we need only to close it
					else if (exclusive_sem == 0) sem_close(mutex);

					mutex= sem_open(value, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
					if (mutex == SEM_FAILED) mutex = sem_open(value, O_CREAT);
					// need to wait in case other process is locking the file
					sem_wait(mutex);
					opened_file = fopen(value, "r+");
					if (opened_file == NULL) {
						fprintf(stderr, "%s %s\n", "Error opening file in executive task:", value);
						// TO DO: tell the parent to kill everyone
						return -1;
					}
					exclusive_sem = 1;
				}
				else if (!strcmp(keyword, inclusivo) && success) {
					if (opened_file != NULL) {
						if (wrote) fprintf(opened_file, "\n");
						fclose(opened_file);
					}
					if (exclusive_sem == 1) {
						sem_post(mutex);
						sem_close(mutex);
					}
					else if (exclusive_sem == 0) sem_close(mutex);

					mutex= sem_open(value, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
					if (mutex == SEM_FAILED) mutex = sem_open(value, O_CREAT);
					else {
						// Need to save the semaphore name to unlink at the end of execution
						FILE * sem_file = fopen("Semaforos.txt", "a+");
						fprintf(sem_file, "%s\n", value);
					}
					// need to wait in case other process is locking the file
					sem_wait(mutex);
					// We post inmediatly so anyone else can enter the file
					sem_post(mutex);
					opened_file = fopen(value, "r+");
					if (opened_file == NULL) {
						fprintf(stderr, "%s %s\n", "Error opening file in executive task:", value);
						// TO DO: tell the parent to kill everyone
						return -1;
					}
					exclusive_sem = 0;
				}
				else if (!strcmp(keyword, leer) && success) {
					if (!find_string(value, opened_file)) success = 0;
				}
				else if (!strcmp(keyword, escribir) && success) {
					int curr_cursor = ftell(opened_file);
					fseek(opened_file, 0, SEEK_END);
					fprintf(opened_file, "%s\n", value);
					wrote = 1;
				}
				else if (!strcmp(keyword, anular) && success) {
					if (find_string(value, opened_file)) success = 0;
				}
				else if (!strcmp(keyword, nombrar) && success) {
					// Nombra un ministro o magistrado
					// Se asume que ya se ha pedido permiso al congreso
					if (value[1] = 'a') {
						// magistrado
						write_pipe("PC", ex_jud);
						kill(jud_id, SIGUSR1);
					}
					else {
						// ministro
						while (*(value++) != ' ');
						while (*(value++) != ' ');
						MINISTRIES_F = fopen("Ministros.txt", "r+");
						for (char * l = fgets(l, LINE_LEN, MINISTRIES_F); feof(MINISTRIES_F); l = fgets(l, LINE_LEN, MINISTRIES_F)) {
							while (*(l++) != ' ');
							while (*(l++) != ' ');
							int same = 1;
							for (int i = 0; i < strlen(value) && same; i++) {
								if (l[i] != value[i]) same = 0;
							}
							if (same) {
								// Encontramos el ministerio
								fseek(MINISTRIES_F, strlen(l) + 1, SEEK_CUR);
								fseek(MINISTRIES_F, strlen(bksp100) + 1, SEEK_CUR);
								fprintf(MINISTRIES_F, "%s", value);
								break;
							}
						}
					}
				}
				else if (!strcmp(keyword, destituir) && success) {
					// Destituye un ministro o magistrado
					if (value[1] = 'a') {
						// magistrado
						write_pipe("PE", ex_jud);
						kill(jud_id, SIGUSR1);
					}
					else {
						// ministro
						// Elimina al ministro, y crea una accion en el plan del presidente
						while (*(value++) != ' ');
						while (*(value++) != ' ');
						sem_wait(ministry_mutex);
						MINISTRIES_F = fopen("Ministros.txt", "r+");
						for (char * l = fgets(l, LINE_LEN, MINISTRIES_F); feof(MINISTRIES_F); l = fgets(l, LINE_LEN, MINISTRIES_F)) {
							while (*(l++) != ' ');
							while (*(l++) != ' ');
							int same = 1;
							for (int i = 0; i < strlen(value) && same; i++) {
								same = value[i] == l[i];
							}
							if (same) {
								// Deja sin ministro al ministerio
								fseek(MINISTRIES_F, - strlen(l) + strlen(bksp100) + 1, SEEK_CUR);
								fprintf(MINISTRIES_F, "%s", bksp50);
								break;
							}
						}
						sem_post(ministry_mutex);
						// long int actual_cursor = ftell(EXECUTIVE_F);
						// fseek(EXECUTIVE_F, 0, SEEK_END);
						// // Creamos la accion en el plan del presidente para agregar un ministro
						// fprintf(EXECUTIVE_F, "\n%s\n", "Agregar ministro");
						// fprintf(EXECUTIVE_F, "%s %s\n", "nombrar: Ministro de", value);
						// fprintf(EXECUTIVE_F, "%s %s\n", "exito: nombran nuevo ministro de", value);
						// fprintf(EXECUTIVE_F, "%s %s %s\n", "fracaso: El puesto de ministro de", value, "queda vacante");
						// fseek(EXECUTIVE_F, actual_cursor, SEEK_SET);
					}
				}
				else if (!strcmp(keyword, disolver) && success) {
					// Disuelve el congreso
					kill(leg_id, SIGTERM);
					// Se comunica con el padre para que cree un nuevo congreso
					write_pipe("CC", ex_press);
					sem_post(print_sem);
					kill(exec_id,SIGSTOP);
					process_metadata();
				}
				else if (!strcmp(keyword, exito)) {
					act.success = value;
				}
				else if (!strcmp(keyword, fracaso)) {
					act.failure = value;
					end = 1;
				}
				else {
					fprintf(stderr, "%s %s\n", "Error reading action in file:", file_exec);
					return -1;
				}
				
				if (success == 2) {
					// We did not finish of reading the action, so we need to
					// advance until the next action shows or EOF
					for (char * line = fgets(line, LINE_LEN, EXECUTIVE_F); 
						line[0] != '\n' && line[0] != '\0';
						line = fgets(line, LINE_LEN, EXECUTIVE_F));
				}
				// In this case, we only nead to read the next empty line;
				char * l;
				for(l = fgets(l, LINE_LEN, EXECUTIVE_F); feof(EXECUTIVE_F) && strlen(l) < 2; l = fgets(l, LINE_LEN, EXECUTIVE_F));
				if (feof(EXECUTIVE_F)) fseek(EXECUTIVE_F, - strlen(l), SEEK_CUR);
				
			}
			if (success == 2) continue;
			if (success == 1 && accepted(president.success_rate)) {
				write(ex_press[1], act.success, strlen(act.success) + 1);
			}
			else write(ex_press[1], act.failure, strlen(act.failure) + 1);
			sem_post(print_sem);
		}

		// Before going to reccess we need to check the requests of the president
		sem_t * req_mutex = sem_open(REQUEST_SEM, O_CREAT);
		sem_wait(req_mutex);
		PRESIDENT_REQUESTS_F = fopen("PedidosPresidenciales.txt", "r+");
		char * from_id;
		char * to_id;
		char response;
		char * line;
		while (feof(PRESIDENT_REQUESTS_F)) {
			fgets(line, LINE_LEN, PRESIDENT_REQUESTS_F);
			parse_request(line, from_id, to_id, &response);
			if (atoi(to_id) == exec_id) {
				int ans = accepted(president.success_rate);
				// Responder al congreso
				if (atoi(from_id) == leg_id) {
					if (ans) write_pipe("CS", ex_leg);
					else write_pipe("CN", ex_leg);
					kill(leg_id, SIGCONT);
				}
				else if (atoi(from_id) == jud_id) {
					if (ans) write_pipe("TS", ex_jud);
					else write_pipe("TN", ex_jud);
					kill(jud_id, SIGCONT);
				}
				else {
					// Viene de un ministerio
					// TODO: Definir comportamiento con ministerios
				}
			}
			else if (atoi(to_id) == leg_id) {
				// TO DO: reenviar requests de ministros a otros poderes
				// write_pipe("PP");
				// kill(leg_id, SIGUSR1);
				// // Libero para que me puedan responder
				// sem_post(req_mutex);
				// kill(exec_id, SIGSTOP);
				// sem_wait(req_mutex);
				// long int actual_cursor = ftell(PRESIDENT_REQUESTS_F);

			}
			else if (atoi(to_id) == jud_id) {

			}
		}
		fclose(PRESIDENT_REQUESTS_F);
		sem_post(req_mutex);
		sem_close(req_mutex);

					// Entra en receso
		printf("%s\n", "President going into reccess");
		if (++reccess_count >= 3) {
			reccess_time *= 3;
			reccess_count = 0;
		}
		for(int i = 0; i < reccess_time; i++) {
			sem_wait(ex_reccess);
			kill(exec_id, SIGSTOP);
		}
	}
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
	sem_t *leg_reccess = sem_open(LEGISLATIVE_REC, O_CREAT);
	sem_t *print_sem = sem_open(LEGISLATIVE_PRINT_SEM, O_CREAT);
	close(ex_leg[1]);
	close(leg_jud[0]);
	close(jud_leg[1]);
	close(leg_press[0]);
	close(press_leg[1]);
	LEGISLATIVE_F = fopen(file_leg, "r+");
	write_metadata('C');
	printf("%s %d %s %d\n", "sending signal from", getpid(), "to", master);
	fflush(stdout);
	kill(master, SIGCONT);
	// We wait until the parent tells us the metadata is ready
	kill(getpid(), SIGSTOP);
	printf("%s\n", "Daddy woke me up");
	process_metadata();
	return 0;
	// Task management
	while (1) {
		int reccess_time = 1;
		int reccess_count = 0;
		fseek(LEGISLATIVE_F, 0, SEEK_SET);
		while (feof(LEGISLATIVE_F)) {
			action act;
			act.name = fgets(act.name, LINE_LEN, LEGISLATIVE_F);
			if (!accepted(0.2f)) {
				while (strlen(act.name) > 2 && feof(LEGISLATIVE_F)) fgets(act.name, LINE_LEN, LEGISLATIVE_F);
				while (strlen(act.name) < 2 && feof(LEGISLATIVE_F)) fgets(act.name, LINE_LEN, LEGISLATIVE_F);
				if (feof(LEGISLATIVE_F)) fseek(LEGISLATIVE_F, - strlen(act.name), SEEK_CUR);
				continue;
			}
			int end = 0;
			FILE * opened_file;
			sem_t * mutex;
			int exclusive_sem = -1;
			int wrote = 0;
			int success = 1;
			while (!end) {
				char * keyword = read_keyword(LEGISLATIVE_F);
				char * value = fgets(value, LINE_LEN, LEGISLATIVE_F);
				value[strlen(value)-1] = '\0';
				while (value[0] = ' ') ++value;
				if (keyword == NULL) {
					fprintf(stderr, "%s %s\n", "Error reading file:", file_leg);
					return -1;
				}
				if ((!strcmp(keyword, reprobacion) || !strcmp(keyword, aprobacion)) && success) {
					if (!strcmp(value, congreso)) {
						success = accepted(congress.success_rate);
					}
					else if(!strcmp(value,presidente)){
						send_president_request(leg_id,exec_id,0);
						kill(leg_id,SIGSTOP);
						char response[2];
						read(ex_leg[0], response, 2);
						if (response[1] == 'N') success = 0;
					}
					else if(!strcmp(value,tribunal)){
						write_pipe("CP", leg_jud);
						kill(jud_id, SIGUSR2);
						kill(leg_id, SIGSTOP);
						char response[2];
						read(jud_leg[0], response, 2);
						if (response[1] == 'N') success = 0;
					}
				}
				else if (!strcmp(keyword, exclusivo) && success) {
					if (opened_file != NULL) {
						if (wrote) fprintf(opened_file, "\n");
						fclose(opened_file);
					}
					if (exclusive_sem == 1) {
						// If the last mutex was exclusive, we need to post and close
						sem_post(mutex);
						sem_close(mutex);
					}
					// If the las mutex was inclusive, we need only to close it
					else if (exclusive_sem == 0) sem_close(mutex);

					mutex= sem_open(value, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
					if (mutex == SEM_FAILED) mutex = sem_open(value, O_CREAT);
					// need to wait in case other process is locking the file
					sem_wait(mutex);
					opened_file = fopen(value, "r+");
					if (opened_file == NULL) {
						fprintf(stderr, "%s %s\n", "Error opening file in executive task:", value);
						// TO DO: tell the parent to kill everyone
						return -1;
					}
					exclusive_sem = 1;
				}
				else if (!strcmp(keyword, inclusivo) && success) {
					if (opened_file != NULL) {
						if (wrote) fprintf(opened_file, "\n");
						fclose(opened_file);
					}
					if (exclusive_sem == 1) {
						sem_post(mutex);
						sem_close(mutex);
					}
					else if (exclusive_sem == 0) sem_close(mutex);

					mutex= sem_open(value, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
					if (mutex == SEM_FAILED) mutex = sem_open(value, O_CREAT);
					else {
						// Need to save the semaphore name to unlink at the end of execution
						FILE * sem_file = fopen("Semaforos.txt", "a+");
						fprintf(sem_file, "%s\n", value);
					}
					// need to wait in case other process is locking the file
					sem_wait(mutex);
					// We post inmediatly so anyone else can enter the file
					sem_post(mutex);
					opened_file = fopen(value, "r+");
					if (opened_file == NULL) {
						fprintf(stderr, "%s %s\n", "Error opening file in executive task:", value);
						// TO DO: tell the parent to kill everyone
						return -1;
					}
					exclusive_sem = 0;			
				}
				else if (!strcmp(keyword, leer) && success) {
					if (!find_string(value, opened_file)) success = 0;
				}
				else if (!strcmp(keyword, escribir) && success) {
					int curr_cursor = ftell(opened_file);
					fseek(opened_file, 0, SEEK_END);
					fprintf(opened_file, "%s\n", value);
					wrote = 1;
				}
				else if (!strcmp(keyword, anular) && success) {
					if (find_string(value, opened_file)) success = 0;
				}
				else if (!strcmp(keyword, destituir) && success) {
					if (value[1] = 'a') {
						// magistrado
						write_pipe("PE", leg_jud);
						kill(jud_id, SIGUSR2);
					}
					else {
						// ministro
						// Elimina al ministro, y crea una accion en el plan del presidente
						while (*(value++) != ' ');
						while (*(value++) != ' ');
						sem_wait(ministry_mutex);
						MINISTRIES_F = fopen("Ministros.txt", "r+");
						for (char * l = fgets(l, LINE_LEN, MINISTRIES_F); feof(MINISTRIES_F); l = fgets(l, LINE_LEN, MINISTRIES_F)) {
							while (*(l++) != ' ');
							while (*(l++) != ' ');
							int same = 1;
							for (int i = 0; i < strlen(value) && same; i++) {
								same = value[i] == l[i];
							}
							if (same) {
								// Deja sin ministro al ministerio
								fseek(MINISTRIES_F, - strlen(l) + strlen(bksp100) + 1, SEEK_CUR);
								fprintf(MINISTRIES_F, "%s", bksp50);
								break;
							}
						}
						sem_post(ministry_mutex);

						// long int actual_cursor = ftell(EXECUTIVE_F);
						// fseek(EXECUTIVE_F, 0, SEEK_END);
						// // Creamos la accion en el plan del presidente para agregar un ministro
						// fprintf(EXECUTIVE_F, "\n%s\n", "Agregar ministro");
						// fprintf(EXECUTIVE_F, "%s %s\n", "nombrar: Ministro de", value);
						// fprintf(EXECUTIVE_F, "%s %s\n", "exito: nombran nuevo ministro de", value);
						// fprintf(EXECUTIVE_F, "%s %s %s\n", "fracaso: El puesto de ministro de", value, "queda vacante");
						// fseek(EXECUTIVE_F, actual_cursor, SEEK_SET);
					}
				}
				else if (!strcmp(keyword, censurar) && success) {
					
					kill(exec_id, SIGTERM);
					write_pipe("PC", leg_press);
					sem_post(print_sem);
					kill(leg_id, SIGSTOP);
					process_metadata();


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
		printf("%s\n", "Congress going into reccess");
		if (++reccess_count >= 3) {
			reccess_time *= 3;
			reccess_count = 0;
		}
		for(int i = 0; i < reccess_time; i++) {
			sem_wait(leg_reccess);
			kill(leg_id, SIGSTOP);
		}
	}
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
	JUDICIAL_F = fopen(file_jud, "r+");
	write_metadata('T');
	printf("%s %d %s %d\n", "sending signal from", getpid(), "to", master);
	fflush(stdout);
	kill(master, SIGCONT);
	// We wait until the parent tells us the metadata is ready
	kill(getpid(), SIGSTOP);
	printf("%s\n", "Daddy woke me up");
	process_metadata();
	return 0;
	// Task Management
	while (1) {
		int reccess_time = 1;
		int reccess_count = 0;
		fseek(JUDICIAL_F, 0, SEEK_SET);
		while (feof(JUDICIAL_F)) {
			action act;
			act.name = fgets(act.name, LINE_LEN, JUDICIAL_F);
			if (!accepted(0.2f)) {
				while (strlen(act.name) > 2 && feof(JUDICIAL_F)) fgets(act.name, LINE_LEN, JUDICIAL_F);
				while (strlen(act.name) < 2 && feof(JUDICIAL_F)) fgets(act.name, LINE_LEN, JUDICIAL_F);
				if (feof(JUDICIAL_F)) fseek(JUDICIAL_F, - strlen(act.name), SEEK_CUR);
				continue;
			}
			int end = 0;
			FILE * opened_file;
			sem_t * mutex;
			int exclusive_sem = -1;
			int wrote = 0;
			int success = 1;
			while (!end) {
				char * keyword = read_keyword(JUDICIAL_F);
				char * value = fgets(value, LINE_LEN, JUDICIAL_F);
				value[strlen(value)-1] = '\0';
				while (value[0] = ' ') ++value;
				if (keyword == NULL) {
					fprintf(stderr, "%s\n", "Error reading file Judicial.acc");
					return -1;
				}
				if ((!strcmp(keyword, reprobacion) || !strcmp(keyword, aprobacion)) && success){
					
					if (!strcmp(value, tribunal)){
						success = accepted(tribune.success_rate);
					}
					else if(!strcmp(value,presidente)){
						send_president_request(jud_id,exec_id,0);
						kill(jud_id,SIGSTOP);
						char response[2];
						read(ex_jud[0], response, 2);
						if (response[1] == 'N') success = 0;
					}

					else if(!strcmp(value,congreso)){
						write_pipe("TP", jud_leg);
						kill(leg_id, SIGUSR2);
						kill(jud_id, SIGSTOP);
						char response[2];
						read(leg_jud[0], response, 2);
						if (response[1] == 'N') success = 0;
					}

					else {
						//SOS minister
					}

				}
				else if (!strcmp(keyword, exclusivo) && success) {
					if (opened_file != NULL) {
						if (wrote) fprintf(opened_file, "\n");
						fclose(opened_file);
					}
					if (exclusive_sem == 1) {
						// If the last mutex was exclusive, we need to post and close
						sem_post(mutex);
						sem_close(mutex);
					}
					// If the las mutex was inclusive, we need only to close it
					else if (exclusive_sem == 0) sem_close(mutex);

					mutex= sem_open(value, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
					if (mutex == SEM_FAILED) mutex = sem_open(value, O_CREAT);
					// need to wait in case other process is locking the file
					sem_wait(mutex);
					opened_file = fopen(value, "r+");
					if (opened_file == NULL) {
						fprintf(stderr, "%s %s\n", "Error opening file in executive task:", value);
						// TO DO: tell the parent to kill everyone
						return -1;
					}
					exclusive_sem = 1;
				}
				else if (!strcmp(keyword, inclusivo) && success) {
					if (opened_file != NULL) {
						if (wrote) fprintf(opened_file, "\n");
						fclose(opened_file);
					}
					if (exclusive_sem == 1) {
						sem_post(mutex);
						sem_close(mutex);
					}
					else if (exclusive_sem == 0) sem_close(mutex);

					mutex= sem_open(value, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
					if (mutex == SEM_FAILED) mutex = sem_open(value, O_CREAT);
					else {
						// Need to save the semaphore name to unlink at the end of execution
						FILE * sem_file = fopen("Semaforos.txt", "a+");
						fprintf(sem_file, "%s\n", value);
					}
					// need to wait in case other process is locking the file
					sem_wait(mutex);
					// We post inmediatly so anyone else can enter the file
					sem_post(mutex);
					opened_file = fopen(value, "r+");
					if (opened_file == NULL) {
						fprintf(stderr, "%s %s\n", "Error opening file in executive task:", value);
						// TO DO: tell the parent to kill everyone
						return -1;
					}
					exclusive_sem = 0;			
				}
				else if (!strcmp(keyword, leer) && success) {
					if (!find_string(value, opened_file)) success = 0;
				}
				else if (!strcmp(keyword, escribir) && success) {
					int curr_cursor = ftell(opened_file);
					fseek(opened_file, 0, SEEK_END);
					fprintf(opened_file, "%s\n", value);
					wrote = 1;
				}
				else if (!strcmp(keyword, anular) && success) {
					if (find_string(value, opened_file)) success = 0;
				}
				else if (!strcmp(keyword, destituir) && success) {
					if (value[1] = 'a') {
						// magistrado
						if (tribune.mag_count > 0) {
							if (--tribune.mag_count == 0) {
								// there are no more magisters
								// TO DO
							}
							float accumm = 0;
							for (int i = 0; i < tribune.mag_count; i++) {
								accumm += tribune.magister[i];
							}
							tribune.success_rate = accumm / tribune.mag_count;
						}
					}
					else {
						// ministro
						// Elimina al ministro, y crea una accion en el plan del presidente
						while (*(value++) != ' ');
						while (*(value++) != ' ');
						sem_wait(ministry_mutex);
						MINISTRIES_F = fopen("Ministros.txt", "r+");
						for (char * l = fgets(l, LINE_LEN, MINISTRIES_F); feof(MINISTRIES_F); l = fgets(l, LINE_LEN, MINISTRIES_F)) {
							while (*(l++) != ' ');
							while (*(l++) != ' ');
							int same = 1;
							for (int i = 0; i < strlen(value) && same; i++) {
								same = value[i] == l[i];
							}
							if (same) {
								// Deja sin ministro al ministerio
								fseek(MINISTRIES_F, - strlen(l) + strlen(bksp100) + 1, SEEK_CUR);
								fprintf(MINISTRIES_F, "%s", bksp50);
								break;
							}
						}
						sem_post(ministry_mutex);

						// long int actual_cursor = ftell(EXECUTIVE_F);
						// fseek(EXECUTIVE_F, 0, SEEK_END);
						// // Creamos la accion en el plan del presidente para agregar un ministro
						// fprintf(EXECUTIVE_F, "\n%s\n", "Agregar ministro");
						// fprintf(EXECUTIVE_F, "%s %s\n", "nombrar: Ministro de", value);
						// fprintf(EXECUTIVE_F, "%s %s\n", "exito: nombran nuevo ministro de", value);
						// fprintf(EXECUTIVE_F, "%s %s %s\n", "fracaso: El puesto de ministro de", value, "queda vacante");
						// fseek(EXECUTIVE_F, actual_cursor, SEEK_SET);
					}
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

void init_ministry(char * name) {
	MINISTRIES_F = fopen("Ministros.txt", "a+");
	if (MINISTRIES_F == NULL) {
		fprintf(stderr, "%s\n", "File not found: Ministros.txt");
		// TO DO:
		// Tell the parent to kill everyone
	}
	// El formato de ministro es el siguiente
	// 		100 caracteres para nombre de ministerio
	//  	50 caracteres para nombre de ministro
	//  	100 caracteres para nombre de la accion
	// 		50 caracteres para offset dentro del archivo de Ejecutivo.acc
	fprintf(MINISTRIES_F, "%s|%s|%s|%s\n", bksp100, bksp50, bksp100, bksp50);
	fseek(MINISTRIES_F, -((strlen(bksp100) * 2) + (strlen(bksp50) * 2) + 4), SEEK_CUR);
	fprintf(MINISTRIES_F, "%s\n", name);
	fclose(MINISTRIES_F);
}

static int ministry_task(pid_t id){
	// Initial configuration
	ministry mi = *create_ministry(id);
	MINISTRIES_F = fopen("Ministros.txt", "r");
	char * line;
	while (feof(MINISTRIES_F)) line = fgets(line, LINE_LEN, MINISTRIES_F);
	char * name = line;
	while (line[0] != ' ') line++;
	*line = '\0';
	mi.name = name;
	fclose(MINISTRIES_F);

	// Task management
	while(1) {
		MINISTRIES_F = fopen("Ministros.txt", "r");
		while (feof(MINISTRIES_F)) {
			char * line = fgets(line, LINE_LEN, MINISTRIES_F);
			int same = 1;
			for (int i = 0; i < strlen(mi.name) && same; i++) {
				if (mi.name[i] != line[i]) same = 0;
			}
			if (same) {
				// Check if a task is currently assigned. The char where the
				// task name starts is the one in the 152th char. So we need to
				// Check if that char is a blank space or not
				if (line[152] != ' ') {
					// no action assigned
					continue;
				}
				else {
					char * action = line + 152;
					while (line[152] != ' ') line++;
					line[152] = '\0';
					if (!strcmp(action, end_task)) {
						fseek(MINISTRIES_F, -((strlen(bksp100) * 2) + (strlen(bksp50) * 2) + 4), SEEK_CUR);
						fprintf(MINISTRIES_F, "%c\n", '#');
						return 0;
					}
					else {
						// We have to execute the action that the president assigned for us. 
					}
				}
			}
			fclose(MINISTRIES_F);
		}
	}
	return 1;
}

static void sig_handler_master(int signal) {
	// This handler is to tell the master to kill everyone
	kill(exec_id, SIGTERM);
	kill(leg_id, SIGTERM);
	kill(jud_id, SIGTERM);

	sem_unlink(MINISTRY_MUTEX);
	sem_unlink(EXECUTIVE_PRINT_SEM);
	sem_unlink(LEGISLATIVE_PRINT_SEM);
	sem_unlink(JUDICIAL_PRINT_SEM);
	sem_unlink(REQUEST_SEM);
	sem_unlink(METADATA_MUTEX);
	sem_unlink(EXECUTIVE_REC);
	sem_unlink(LEGISLATIVE_REC);
	sem_unlink(JUDICIAL_REC);
	FILE * sem_file = fopen("Semaforos.txt", "r");
	if (sem_file != NULL) {
		for (char * line = fgets(line, LINE_LEN, sem_file); feof(sem_file); line = fgets(line, LINE_LEN, sem_file)) {
			line[strlen(line)-1] = '\0';
			sem_unlink(line);
		}
	}
	kill(master, SIGTERM);
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
		}
		else if(line[1]=='A'){
			fseek(LEGISLATIVE_F, 0, SEEK_SET);
			char * l;
			for (l = fgets(l, LINE_LEN, LEGISLATIVE_F); feof(LEGISLATIVE_F); l = fgets(l, LINE_LEN, LEGISLATIVE_F)) {
				if (strlen(l) > 2) {
					if (accepted(congress.success_rate)) {
						// borrar la accion
						while (strlen(l) > 2) {
							fseek(LEGISLATIVE_F, -strlen(l), SEEK_CUR);
							for (int i = 0; i < strlen(l); i++) fprintf(LEGISLATIVE_F, " ");
							l = fgets(l, LINE_LEN, LEGISLATIVE_F);	
						}
						
					}
					else {
						// Saltar hasta la nueva accion
						while (strlen(l) <= 1) l = fgets(l, LINE_LEN, LEGISLATIVE_F);
					}
				}
			}
			fseek(LEGISLATIVE_F, 0, SEEK_SET);
		}
		else {
			fprintf(stderr, "%s\n", "Error in pipes comunication");
		}
	}
	else {
		// The congress doesn't exists, the action fails
		send_president_request(exec_id, exec_id, 0);
	}
	// Tell the president we answered
	kill(exec_id, SIGCONT);
}

static void sig_handler_leg_usr2(int signal) {
	// Recibimos del judicial
	if (&congress == NULL) {
		write_pipe("TN", leg_jud);
	}
	else {
		char request[2];
		read(leg_jud[0], request, 2);
		if (accepted(congress.success_rate)) {
			write_pipe("TS", leg_jud);
		}
		else {
			write_pipe("TN", leg_jud);
		}
	}
	kill(jud_id, SIGCONT);
}

static void sig_handler_jud_usr1(int signal) {
	// Recibimos del ejecutivo
	if (&tribune != NULL) {
		char line[2];
		read(ex_jud[0], line, 2);
		if (line[1] == 'C') {
			tribune.mag_count++;
			tribune.magister[tribune.mag_count - 1] = rand();
			float acumm = 0;
			for (int i = 0; i<tribune.mag_count; i++){
				acumm =+tribune.magister[i];
			}
			tribune.success_rate = acumm/tribune.mag_count;


		}
		else if (line[1] == 'E'){
			if (tribune.mag_count>0) {
				if (--tribune.mag_count == 0) {
					// TO DO: there are no magisters left
				}
				float acumm = 0;
				for (int i = 0; i<tribune.mag_count; i++){
					acumm =+tribune.magister[i];
				}
				tribune.success_rate = acumm/tribune.mag_count;

			}
			else if (tribune.mag_count==0){
				return;
			}


		}
		else if(line[1]=='M'){
			process_metadata();
		}
	}
}

static void sig_handler_jud_usr2(int signal) {
	// Recibimos del legislativo
	if (&tribune == NULL) {
		write_pipe("CN", jud_leg);
	} 
	else {
		char request[2];
		read(leg_jud[0], request, 2);
		if (accepted(tribune.success_rate)) {
			write_pipe("CS", jud_leg);
		}
		else {
			write_pipe("CN", jud_leg);
		}
	}

	kill(leg_id, SIGCONT);
}

static void sig_handler_exec_usr1(int signal){

	fseek(EXECUTIVE_F, 0, SEEK_SET);
	char * l;
	for (l = fgets(l, LINE_LEN, EXECUTIVE_F); feof(EXECUTIVE_F); l = fgets(l, LINE_LEN, EXECUTIVE_F)) {
		if (strlen(l) > 2) {
			if (accepted(president.success_rate)) {
				// borrar la accion
				while (strlen(l) > 2) {
					fseek(EXECUTIVE_F, -strlen(l), SEEK_CUR);
					for (int i = 0; i < strlen(l); i++) fprintf(EXECUTIVE_F, " ");
					l = fgets(l, LINE_LEN, EXECUTIVE_F);	
				}
				
			}
			else {
				// Saltar hasta la nueva accion
				while (strlen(l) <= 1) l = fgets(l, LINE_LEN, EXECUTIVE_F);
			}
		}
	}
	fseek(EXECUTIVE_F, 0, SEEK_SET);
}

void send_president_request(pid_t from, pid_t to, int result) {
	sem_t * req_mutex = sem_open(REQUEST_SEM, O_CREAT);
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

void parse_request(char * request, char * from, char * to, char * response) {
	int i = 0;
	int j = 0;
	while ((from[i++] = request[j++]) != ' ');
	j++;
	i = 0;
	while ((to[i++] = request[j++]) != ' ');
	j++;
	*response = request[j];
}

void write_metadata(const char C) {
	sem_t * meta_mutex = sem_open(METADATA_MUTEX, O_CREAT);
	sem_wait(meta_mutex);
	METADATA_F = fopen("Metadata.txt", "a+");
	if (METADATA_F == NULL) {
		fprintf(stderr, "%s\n", "Unable to locate file Metadata.txt");
		exit(1);
	}
	fprintf(METADATA_F, "%c %d\n", C, getpid());
	fclose(METADATA_F);
	sem_post(meta_mutex);
	sem_close(meta_mutex);
}

void process_metadata() {
	METADATA_F = fopen("Metadata.txt", "r");
	if (METADATA_F == NULL) {
		fprintf(stderr, "%s\n", "Could not locate file: Metadata.txt");
		exit(1);
	}
	for(char * line = fgets(line, LINE_LEN, METADATA_F); feof(METADATA_F); line = fgets(line, LINE_LEN, METADATA_F)) {
		if (line[0] = 'P') {
			line += 2;
			exec_id = atoi(line);
		}
		else if (line[0] = 'C') {
			line += 2;
			leg_id = atoi(line);
		}
		else if (line[0] = 'T') {
			line += 2;
			jud_id = atoi(line);
		}
		else {
			fprintf(stderr, "%s\n", "Error in metadata file");
			exit(1);
		}
	}
}