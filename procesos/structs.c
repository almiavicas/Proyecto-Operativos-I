#include "structs.h"

executive * create_executive(pid_t id) {
	executive * ex = malloc(sizeof(executive));
	if (ex == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	ex->id = id;
	ex->success_rate = rand() / RAND_MAX;
	return ex;
}

legislative * create_legislative(pid_t id) {
	legislative * leg = malloc(sizeof(legislative));
	if (leg == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	leg->id = id;
	leg->success_rate = rand() / RAND_MAX;
	return leg;
}

judicial * create_judicial(pid_t id) {
	judicial * jud = malloc(sizeof(judicial));
	if (jud == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	jud->id = id;
	struct sigaction act;
	act.sa_handler = sig_handler_jud_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	sigaddset(&(act.sa_mask), SIGUSR2);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = sig_handler_jud_usr2;
	sigaction(SIGUSR2, &act, NULL);

	float * magister = calloc(sizeof(float), 100);
	if (magister == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(&(jud->success_rate));
		free(jud);
		return NULL;
	}
	float accum = 0;
	for(int i = 0; i < 8; i++) {
		magister[i] = 2.0f/3;
		accum += magister[i];
	}
	jud->success_rate = accum / 8.0f;
	jud->magister = magister;

	jud->mag_count = 8;
	return jud;
}

ministry * create_ministry(pid_t id) {
	ministry * min = malloc(sizeof(ministry));
	if (min == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	min->id = id;
	char * name = malloc(sizeof(char) * NAME_LEN);
	if (name == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(min);
		return NULL;
	}
	min->name = name;

	char * minister = malloc(sizeof(char) * NAME_LEN);
	if (minister == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(min->name);
		free(min);
		return NULL;
	}

	min->minister = minister;
	min->success_rate = rand();
	return min;
}

request * create_request(pid_t from, pid_t to, int response) {
	request * req = malloc(sizeof(request));
	if (req = NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	req->from = from;
	req->to = to;
	req->response = response;
	return req;
}

action * create_action() {
	action * act = malloc(sizeof(action));
	if (act == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	act->name = malloc(sizeof(char) * NAME_LEN);
	if (act->name == NULL) {
		fprintf(stderr, "Not enought memory\n");
		free(act);
		return NULL;
	}
	act->success = malloc(sizeof(char) * LINE_LEN);
	if (act->success == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(act->name);
		free(act);
		return NULL;
	}
	act->failure = malloc(sizeof(char) * LINE_LEN);
	if (act->failure == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(act->success);
		free(act->name);
		free(act);
		return NULL;
	}
	return act;
}

void read_keyword(FILE * f, char * word) {
	int i = 0;
	char c;
	while ((c = fgetc(f)) != ':') {
		if (c == '\n') {
			printf("print de read_keyword\n" );
		}
		word[i++] = c;
	}
	word[i] = '\0';
}

int accepted(float success_rate) {
	float f = (float) rand() / (float)RAND_MAX;
	if (success_rate >= f) {
		return 1;
	}
	return 0;
}

void write_pipe(const char msg[], int pipe[2]) {
	write(pipe[1], msg, strlen(msg) + 1);
}

int find_string(char * string, FILE * f) {
	long int current_cursor = ftell(f);
	fseek(f, 0, SEEK_SET);
	for (char * line = fgets(line, LINE_LEN, f); feof(f); line = fgets(line, LINE_LEN, f)) {
		for (int i = 0; i < strlen(line); i++) {
			if (strlen(line) - i < strlen(string)) break;
			int same = line[i] == string[0];
			for (int j = 1; j < strlen(string) && same; j++) {
				same = line[i + j] == string[j];
			}
			if (same) {
				fseek(f, current_cursor, SEEK_SET);
				return 1;
			}
		}
	}
	fseek(f, current_cursor, SEEK_SET);
	return 0;
}

void end_reccess(sem_t * t1, pid_t id) {
	int * sval;
	if (sem_getvalue(t1, sval) <= 0) {
		sem_post(t1);
		kill(id, SIGCONT);
	}
}