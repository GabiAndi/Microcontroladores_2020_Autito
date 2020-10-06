#ifndef INC_TICKER_H_
#define INC_TICKER_H_

#include <stdint.h>

// Definiciones
#define TICKER_MAX_USE                 					15	// Tickers como maximo

#define TICKER_LOW_PRIORITY								0	// Prioridad baja (se ejecuta en blucle principal)
#define TICKER_HIGH_PRIORITY							1	// Prioridad alta (se ejecuta en interrupcion)

#define TICKER_DEACTIVATE								0
#define TICKER_ACTIVE									1

// Tipos de variables
typedef void(*ticker_ptrfun)(void);	// Puntero a funcion a ejecutar en el ticker

typedef struct
{
	volatile uint32_t ms_max;
	volatile uint32_t ms_count;

	volatile uint16_t calls;

	volatile uint8_t active;

	uint8_t priority;

	ticker_ptrfun ticker_function;
}ticker_t;

// Prototipos de funcion
void ticker_init_core(void);

void ticker_new(ticker_t *ticker);
void ticker_delete(ticker_t *ticker);

void ticker_execute_pending(void);

#endif /* INC_TICKER_H_ */
