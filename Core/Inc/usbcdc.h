#ifndef INC_USBCDC_H_
#define INC_USBCDC_H_

/**********************************************************************************/
/*********************************** Includes *************************************/
/**********************************************************************************/
#include <inttypes.h>

#include "system.h"

#include "usbd_cdc_if.h"
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

/**********************************************************************************/
/********************************** Funciones *************************************/
/**********************************************************************************/

/*
 * Funcion de inicio y configuracion del USB
 *
 * Configura e inicializa los bufferes de datos del USB.
 *
 */
void usbcdc_init(void);

/*
 * Funcion de envio de datos del USB
 *
 * Se encarga de veirificar si hay datos disponibles para enviar.
 *
 */
void usbcdc_read_pending(void);

/*
 * Funcion de envio de comandos via USB
 *
 * Esta funcion se encarga de enviar datos por USB.
 *
 */
void usbcdc_write_pending(void);

/*
 * Funcion de timeout de escritura
 *
 * Esta funcion se activa si el paquete tardo mucho en leerse.
 *
 */
void usbcdc_timeout_read(void);
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/

#endif /* INC_USBCDC_H_ */
