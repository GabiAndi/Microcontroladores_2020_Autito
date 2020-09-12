#ifndef INC_TICKER_H_
#define INC_TICKER_H_

#include "stm32f1xx.h"

#include "stddef.h"
#include "stdlib.h"

// Definiciones
#define MAX_TICKERS_USE                 				6	// Ticker como maximo

#define LOW_PRIORITY									0
#define HIGH_PRIORITY									1

// Typedef
typedef void(*ptrfun)(void);

// Estructuras
typedef struct
{
	volatile uint32_t ms;
	volatile uint32_t count;

	volatile uint16_t calls;

	uint8_t priority;

	ptrfun function;
}ticker_t;

// Variables
ticker_t *user_tickers[MAX_TICKERS_USE];

// Prototipos de funcion
void init_ticker_core(void);

void new_ticker_ms(ptrfun function, uint32_t ms, uint8_t priority);
void delete_ticker(ptrfun function);

void change_ticker_ms(ptrfun function, uint32_t ms);

void change_ticker_priority(ptrfun function, uint8_t priority);

void execute_ticker_pending(void);

#endif /* INC_TICKER_H_ */
