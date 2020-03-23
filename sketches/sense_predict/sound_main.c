#include <stdlib.h>

#include "wsf_types.h"
#include "app_api.h"
#include "att_api.h"
#include "wsf_trace.h"
#include "bstream.h"

#include "sound_api.h"
#include "svc_sound.h"

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! Control block. */
static struct
{
  wsfTimer_t  measTimer;
  bool_t      measTimerStarted;
} soundCb;

/*************************************************************************************************/
/*!
 *  \fn       soundUpdateTimer
 *
 *  \brief    Update measurement timer.
 *
 *  \return   None.
 */
/*************************************************************************************************/
static void soundUpdateTimer(void)
{
  uint8_t  config;
  uint8_t *pConfig = NULL;
  uint8_t  period;
  uint8_t *pPeriod = NULL;
  uint16_t attLen = 0;

  /* Get config & period. */
  AttsGetAttr(SOUND_HANDLE_CONFIG, &attLen, &pConfig);
  if (pConfig == NULL)
  {
    WSF_TRACE_ERR0("sound: unable to read config");
    return;
  }
  config = *pConfig;
  AttsGetAttr(SOUND_HANDLE_PERIOD, &attLen, &pPeriod);
  if (pPeriod == NULL)
  {
    WSF_TRACE_ERR0("sound: unable to read period");
    return;
  }
  period = *pPeriod;
  if (period < SOUND_ATT_PERIOD_MIN)
  {
    period = SOUND_ATT_PERIOD_MIN;
  }

  if (config == SOUND_ATT_CONFIG_ENABLE)
  {
    if (!soundCb.measTimerStarted)
    {
      soundCb.measTimerStarted = TRUE;
      WsfTimerStartMs(&soundCb.measTimer, period * 10u);
    }
  }
  else
  {
    if (soundCb.measTimerStarted)
    {
      soundCb.measTimerStarted = FALSE;
      WsfTimerStop(&soundCb.measTimer);
    }
  }
}

/*************************************************************************************************/
/*!
 *  \fn     soundWriteCback
 *
 *  \brief  ATTS write callback for sound profile.
 *
 *  \return ATT status.
 */
/*************************************************************************************************/
static uint8_t soundWriteCback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                              uint16_t offset, uint16_t len, uint8_t *pValue,
                              attsAttr_t *pAttr)
{
  switch (handle)
  {
    case SOUND_HANDLE_CONFIG:
    {
      uint8_t config;

      /* Check attribute value. */
      if (len != 1)
      {
        return ATT_ERR_LENGTH;
      }
      config = *pValue;
      if ((config != SOUND_ATT_CONFIG_DISABLE) && (config != SOUND_ATT_CONFIG_ENABLE))
      {
        return ATT_ERR_RANGE;
      }

      /* Save value. */
      AttsSetAttr(SOUND_HANDLE_CONFIG, len, pValue);

      /* Enable or disable timer. */
      soundUpdateTimer();
      return ATT_SUCCESS;
    }
    case SOUND_HANDLE_PERIOD:
    {
      uint8_t period;

      if (len != 1)
      {
        return ATT_ERR_LENGTH;
      }
      period = *pValue;
      if ((period < SOUND_ATT_PERIOD_MIN) || (period > SOUND_ATT_PERIOD_MAX))
      {
        return ATT_ERR_RANGE;
      }
      AttsSetAttr(SOUND_HANDLE_PERIOD, len, pValue);
      return ATT_SUCCESS;
    }
  }
  return ATT_ERR_NOT_SUP;
}

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
void SoundStart(wsfHandlerId_t handlerId, uint8_t timerEvt)
{
  SvcSoundAddGroup();
  SvcSoundCbackRegister(soundWriteCback);

  soundCb.measTimer.handlerId = handlerId;
  soundCb.measTimer.msg.event = timerEvt;
  soundCb.measTimerStarted    = FALSE;
}

/*************************************************************************************************/
/*!
 *  \fn     SoundStop
 *
 *  \brief  Stop service.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundStop(void)
{
  SoundMeasStop();
  SvcSoundRemoveGroup();
}

/*************************************************************************************************/
/*!
 *  \fn     SoundMeasStop
 *
 *  \brief  Measurement stop handler.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundMeasStop(void)
{
  soundCb.measTimerStarted = FALSE;
  WsfTimerStop(&soundCb.measTimer);
}

/*************************************************************************************************/
/*!
 *  \fn     SoundMeasStart
 *
 *  \brief  Measurement start handler.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SoundMeasStart(void)
{
  soundUpdateTimer();
}

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
void SoundMeasComplete(dmConnId_t connId, int32_t sound)
{
  soundCb.measTimerStarted = FALSE;

  uint8_t soundData[2] = {UINT32_TO_BYTES(sound)};
  AttsSetAttr(SOUND_HANDLE_DATA, sizeof(soundData), soundData);
  AttsHandleValueNtf(connId, SOUND_HANDLE_DATA, sizeof(soundData), soundData);

  soundUpdateTimer();
}
