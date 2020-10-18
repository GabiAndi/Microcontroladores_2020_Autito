#ifndef INC_ADC_H_
#define INC_ADC_H_

/**********************************************************************************/
/*********************************** Includes *************************************/
/**********************************************************************************/
#include <inttypes.h>
#include <string.h>

#include "stm32f1xx_hal.h"

#include "datatypes.h"

#include "ticker.h"
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/

/*
 * Funcion que inicia el ADC
 *
 */
void adc_init(void);

/*
 * Funcion inicia la captura de datos
 *
 */
void adc_capture(void);
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

#endif /* INC_ADC_H_ */
