#include <signal.h>
#define NAME_LEN 40

typedef struct {
	pid_t id;
	float success_rate;
	// Lleva la cuenta de los requests que le han hecho al presidente.
	// Dichos requests se responden cuando el presidente deja de estar ocupado
	int requests;
} executive;

typedef struct {
	pid_t id;
	float success_rate;
	// Define la accion del congreso para atender requests.
	// Los requests se responden inmediatamente
	struct sigaction act;
} legislative;

typedef struct {
	pid_t id;
	float succes_rate;
	int[8] magister;
	int mag_count;
	// Define la accion del tribunal para responder requests
	// Los requests se responden inmediatamente.
	struct sigaction act;
} judicial;

typedef struct {
	pid_t id;
	// Nombre del ministerio. como maximo tiene NAME_LEN caracteres
	char * name;
	// Nombre del ministro. Como maximo tiene NAME_LEN caracteres
	char * minister;
	float succes_rate;
} ministry;