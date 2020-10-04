#include "ticker.h"

// Variables
ticker_t *tickers[TICKER_MAX_USE];

void ticker_init_core(void)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		tickers[i] = 0;
	}
}

void ticker_new(ticker_t *ticker)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers[i] == ticker)
		{
			return;
		}
	}

	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers[i] == 0)
		{
			tickers[i] = ticker;

			break;
		}
	}
}

void ticker_delete(ticker_t *ticker)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers[i] == ticker)
		{
			tickers[i] = 0;

			break;
		}
	}
}

void ticker_execute_pending(void)
{
	for (uint8_t i = 0 ; i < TICKER_MAX_USE ; i++)
	{
		if (tickers[i] != 0)
		{
			if ((tickers[i]->ms_count >= tickers[i]->ms_max) && (tickers[i]->priority == TICKER_LOW_PRIORITY)
					&& (tickers[i]->active == TICKER_ACTIVE))
			{
				tickers[i]->ms_count = 0;

				tickers[i]->ticker_function();

				tickers[i]->calls++;
			}
		}
	}
}
