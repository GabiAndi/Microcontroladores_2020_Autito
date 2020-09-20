#include "ticker.h"

void ticker_init_core(void)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		tickers_user[i] = NULL;
	}
}

void ticker_new(tick_ptrfun function, uint32_t ms, uint8_t priority)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers_user[i] == NULL)
		{
			tickers_user[i] = (ticker_t *)(malloc(sizeof(ticker_t)));

			tickers_user[i]->ms = ms;
			tickers_user[i]->count = 0;
			tickers_user[i]->priority = priority;
			tickers_user[i]->calls = 0;
			tickers_user[i]->function = function;

			break;
		}
	}
}

void ticker_delete(tick_ptrfun function)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers_user[i] != NULL)
		{
			if (tickers_user[i]->function == function)
			{
				free(tickers_user[i]);

				tickers_user[i] = NULL;

				break;
			}
		}
	}
}

void ticker_change_period(tick_ptrfun function, uint32_t ms)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers_user[i] != NULL)
		{
			if (tickers_user[i]->function == function)
			{
				tickers_user[i]->ms = ms;

				break;
			}
		}
	}
}

void ticker_change_priority(tick_ptrfun function, uint8_t priority)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers_user[i] != NULL)
		{
			if (tickers_user[i]->function == function)
			{
				tickers_user[i]->priority = priority;

				break;
			}
		}
	}
}

void ticker_execute_pending(void)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers_user[i] != NULL)
		{
			if ((tickers_user[i]->count >= tickers_user[i]->ms) && (tickers_user[i]->priority == TICKER_LOW_PRIORITY))
			{
				tickers_user[i]->count = 0;

				tickers_user[i]->function();

				tickers_user[i]->calls++;
			}
		}
	}
}
