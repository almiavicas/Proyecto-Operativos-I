#include "structs.h"

executive * create_executive(pid_t id) {
	executive * ex = malloc(sizeof(executive));
	if (ex == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	ex->id = id;
	float * success_rate = malloc(sizeof(float));
	if (success_rate == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(ex);
		return NULL;
	}
	*success_rate = rand();
	ex->success_rate = *success_rate;
	return ex;
}

legislative * create_legislative(pid_t id) {
	legislative * leg = malloc(sizeof(legislative));
	if (leg == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	leg->id = id;
	float * success_rate = malloc(sizeof(float));
	if (success_rate == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(leg);
		return NULL;
	}
	*success_rate = rand();
	leg->success_rate = *success_rate;
	return leg;
}

judicial * create_judicial(pid_t id) {
	judicial * jud = malloc(sizeof(judicial));
	if (jud == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	jud->id = id;
	float * success_rate = malloc(sizeof(float));
	if (success_rate == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(jud);
		return NULL;
	}
	struct sigaction act;
	act.sa_handler = sig_handler_jud_usr1;
	sigemptyset(&(act.sa_mask));
	sigaddset(&(act.sa_mask), SIGUSR1);
	sigaddset(&(act.sa_mask), SIGUSR2);
	act.sa_flags = SA_INTERRUPT;
	sigaction(SIGUSR1, &act, NULL);
	act.sa_handler = sig_handler_jud_usr2;
	sigaction(SIGUSR2, &act, NULL);

	float * magister = calloc(sizeof(float), 8);
	if (magister == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(&(jud->success_rate));
		free(jud);
		return NULL;
	}
	float accum = 0;
	for(int i = 0; i < 8; i++) {
		magister[i] = rand();
		accum += magister[i];
	}
	*success_rate = accum / 8.0f;
	jud->success_rate = *success_rate;
	jud->magister = magister;

	int * magister_count = malloc(sizeof(int));
	if (magister_count == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(jud->magister);
		free(&(jud->success_rate));
		free(jud);
		return NULL;
	}
	*magister_count = 8;
	jud->mag_count = *magister_count;
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

	float * success_rate = malloc(sizeof(float));
	if (success_rate == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		free(min->minister);
		free(min->name);
		free(min);
		return NULL;
	}
	*success_rate = rand();
	min->success_rate = *success_rate;
	return min;
}

request * create_request(pid_t from, pid_t to, int value) {
	request * req = malloc(sizeof(request));
	if (req = NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	req->from = from;
	req->to = to;
	req->value = value;
	return req;
}

char * read_keyword(FILE * f) {
	char * word = malloc(sizeof(char) * 15);
	if (word == NULL) {
		fprintf(stderr, "%s\n", "Not enought memory");
		return NULL;
	}
	int i = 0;
	char c;
	while ((c = fgetc(f)) != ':') {
		if (c == '\n') {
			free(word);
			return NULL;
		}
		word[i++] = c;
	}
	return word;
}

int accepted(float success_rate) {
	float f = (float) rand();
	if (success_rate >= f) {
		return 1;
	}
	return 0;
}

void write_pipe(const char msg[], int pipe[2]) {
	write(pipe[1], msg, strlen(msg) + 1);
}