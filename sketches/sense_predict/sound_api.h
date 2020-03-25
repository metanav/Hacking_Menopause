#ifndef SOUND_API_H
#define SOUND_API_H

#include "wsf_types.h"
#include "wsf_os.h"

/*************************************************************************************************/
/*!
 *  \fn     SoundStart
 *
 *  \brief  Start service.
 *
 *  \param  handlerId       Handler ID.
 *  \param  timerEvt        Timer message event.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundStart(wsfHandlerId_t handlerId, uint8_t timerEvt);

/*************************************************************************************************/
/*!
 *  \fn     SoundStop
 *
 *  \brief  Stop service.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundStop(void);

/*************************************************************************************************/
/*!
 *  \fn     SoundMeasStop
 *
 *  \brief  Measurement stop handler.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundMeasStop(void);

/*************************************************************************************************/
/*!
 *  \fn     SoundMeasStart
 *
 *  \brief  Measurement start handler.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundMeasStart(void);

/*************************************************************************************************/
/*!
 *  \fn     SoundMeasComplete
 *
 *  \brief  Measurement complete handler.
 *
 *  \param  connId    Connection ID.
 *  \param  sound      Sound reading.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundMeasComplete(dmConnId_t connId, int16_t sound);

#endif /* SOUND_API_H */
