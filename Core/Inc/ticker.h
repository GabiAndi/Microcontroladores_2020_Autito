#ifndef INC_TICKER_H_
#define INC_TICKER_H_

#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>

// Definiciones
#define TICKER_MAX_USE                 					10	// Ticker como maximo

#define TICKER_LOW_PRIORITY								0
#define TICKER_HIGH_PRIORITY							1

// Typedef
typedef void(*tick_ptrfun)(void);

// Estructuras
typedef struct
{
	volatile uint32_t ms;
	volatile uint32_t count;

	volatile uint16_t calls;

	uint8_t priority;

	tick_ptrfun function;
}ticker_t;

// Variables
ticker_t *tickers_user[TICKER_MAX_USE];

// Prototipos de funcion
void ticker_init_core(void);

void ticker_new(tick_ptrfun function, uint32_t ms, uint8_t priority);
void ticker_delete(tick_ptrfun function);

void ticker_change_period(tick_ptrfun function, uint32_t ms);

void ticker_change_priority(tick_ptrfun function, uint8_t priority);

void ticker_execute_pending(void);

#endif /* INC_TICKER_H_ */
