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

	list_t * requests = new_ordered_list();
	if (requests == NULL) {
		free(success_rate);
		free(ex);
	}
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

