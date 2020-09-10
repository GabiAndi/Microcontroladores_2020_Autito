#include "ticker.h"

void init_ticker_core(void)
{
	for (uint8_t i = 0 ; i < MAX_TICKERS_USE ; i++)
	{
		user_tickers[i] = NULL;
	}
}

void new_ticker_ms(ptrfun function, uint32_t ms, uint8_t priority)
{
	for (uint8_t i = 0 ; i < MAX_TICKERS_USE ; i++)
	{
		if (user_tickers[i] == NULL)
		{
			user_tickers[i] = (ticker_t *)(malloc(sizeof(ticker_t)));

			user_tickers[i]->ms = ms;
			user_tickers[i]->count = 0;
			user_tickers[i]->priority = priority;
			user_tickers[i]->calls = 0;
			user_tickers[i]->function = function;

			break;
		}
	}
}

void delete_ticker(ptrfun function)
{
	for (uint8_t i = 0 ; i < MAX_TICKERS_USE ; i++)
	{
		if (user_tickers[i] != NULL)
		{
			if (user_tickers[i]->function == function)
			{
				free(user_tickers[i]);

				user_tickers[i] = NULL;

				break;
			}
		}
	}
}

void change_ticker_ms(ptrfun function, uint32_t ms)
{
	for (uint8_t i = 0 ; i < MAX_TICKERS_USE ; i++)
	{
		if (user_tickers[i] != NULL)
		{
			if (user_tickers[i]->function == function)
			{
				user_tickers[i]->ms = ms;

				break;
			}
		}
	}
}

void change_ticker_priority(ptrfun function, uint8_t priority)
{
	for (uint8_t i = 0 ; i < MAX_TICKERS_USE ; i++)
	{
		if (user_tickers[i] != NULL)
		{
			if (user_tickers[i]->function == function)
			{
				user_tickers[i]->priority = priority;

				break;
			}
		}
	}
}

void execute_ticker_pending(void)
{
	for (uint8_t i = 0 ; i < MAX_TICKERS_USE ; i++)
	{
		if (user_tickers[i] != NULL)
		{
			if ((user_tickers[i]->count >= user_tickers[i]->ms) && (user_tickers[i]->priority == LOW_PRIORITY))
			{
				user_tickers[i]->count = 0;

				user_tickers[i]->function();

				user_tickers[i]->calls++;
			}
		}
	}
}
