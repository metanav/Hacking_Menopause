#include <string.h>

#include "wsf_types.h"
#include "att_api.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "svc_cfg.h"
#include "svc_sound.h"

/**************************************************************************************************
 Macros
**************************************************************************************************/

#define SOUND_UUID_SVC           0x3020
#define SOUND_UUID_CHR_DATA      0x3021
#define SOUND_UUID_CHR_CONFIG    0x3022
#define SOUND_UUID_CHR_PERIOD    0x3023

/**************************************************************************************************
 Service variables
**************************************************************************************************/

/* Sound service declaration. */
static const uint8_t  soundValSvc[] = {UINT16_TO_BYTES(SOUND_UUID_SVC)};
static const uint16_t soundLenSvc   = sizeof(soundValSvc);

/* Sound data characteristic. */
static const uint8_t  soundValDataChr[] = {ATT_PROP_READ | ATT_PROP_NOTIFY,
                                          UINT16_TO_BYTES(SOUND_HANDLE_DATA),
                                          UINT16_TO_BYTES(SOUND_UUID_CHR_DATA)};
static const uint16_t soundLenDataChr   = sizeof(soundValDataChr);

/* Sound data. */
static const uint8_t  soundUuidData[] = {UINT16_TO_BYTES(SOUND_UUID_CHR_DATA)};
static       uint8_t  soundValData[]  = {0x00, 0x00};
static const uint16_t soundLenData    = sizeof(soundValData);

/* Sound data client characteristic configuration. */
static       uint8_t  soundValDataClientChrConfig[] = {0x00, 0x00};
static const uint16_t soundLenDataClientChrConfig   = sizeof(soundValDataClientChrConfig);

/* Sound data characteristic user description. */
static const uint8_t  soundValDataChrUsrDescr[] = "Sound Level";
static const uint16_t soundLenDataChrUsrDescr   = sizeof(soundValDataChrUsrDescr) - 1u;

/* Sound config characteristic. */
static const uint8_t  soundValConfigChr[] = {ATT_PROP_READ | ATT_PROP_WRITE,
                                            UINT16_TO_BYTES(SOUND_HANDLE_CONFIG),
                                            UINT16_TO_BYTES(SOUND_UUID_CHR_CONFIG)};
static const uint16_t soundLenConfigChr   = sizeof(soundValConfigChr);

/* Sound config. */
static const uint8_t  soundUuidConfig[] = {UINT16_TO_BYTES(SOUND_UUID_CHR_CONFIG)};
static       uint8_t  soundValConfig[]  = {0x00};
static const uint16_t soundLenConfig    = sizeof(soundValConfig);

/* Sound config characteristic user description. */
static const uint8_t  soundValConfigChrUsrDescr[] = "Sound Level Config";
static const uint16_t soundLenConfigChrUsrDescr   = sizeof(soundValConfigChrUsrDescr) - 1u;

/* Sound period characteristic. */
static const uint8_t  soundValPeriodChr[] = {ATT_PROP_READ | ATT_PROP_WRITE,
                                            UINT16_TO_BYTES(SOUND_HANDLE_PERIOD),
                                            UINT16_TO_BYTES(SOUND_UUID_CHR_PERIOD)};
static const uint16_t soundLenPeriodChr   = sizeof(soundValPeriodChr);

/* Sound period. */
static const uint8_t  soundUuidPeriod[] = {UINT16_TO_BYTES(SOUND_UUID_CHR_PERIOD)};
static       uint8_t  soundValPeriod[]  = {SOUND_ATT_PERIOD_DEFAULT};
static const uint16_t soundLenPeriod    = sizeof(soundValPeriod);

/* Sound period characteristic user description. */
static const uint8_t  soundValPeriodChrUsrDescr[] = "Sound Level Period";
static const uint16_t soundLenPeriodChrUsrDescr   = sizeof(soundValPeriodChrUsrDescr) - 1u;

/* Attribute list for sound group. */
static const attsAttr_t soundList[] =
{
  /* Service declaration. */
  {
    attPrimSvcUuid,
    (uint8_t *) soundValSvc,
    (uint16_t *) &soundLenSvc,
    sizeof(soundValSvc),
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic declaration. */
  {
    attChUuid,
    (uint8_t *) soundValDataChr,
    (uint16_t *) &soundLenDataChr,
    sizeof(soundValDataChr),
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic value. */
  {
    soundUuidData,
    (uint8_t *) soundValData,
    (uint16_t *) &soundLenData,
    sizeof(soundValData),
    0,
    ATTS_PERMIT_READ
  },
  /* Client characteristic configuration. */
  {
    attCliChCfgUuid,
    (uint8_t *) soundValDataClientChrConfig,
    (uint16_t *) &soundLenDataClientChrConfig,
    sizeof(soundValDataClientChrConfig),
    ATTS_SET_CCC,
    ATTS_PERMIT_READ | ATTS_PERMIT_WRITE
  },
  /* Characteristic user description. */
  {
    attChUserDescUuid,
    (uint8_t *) soundValDataChrUsrDescr,
    (uint16_t *) &soundLenDataChrUsrDescr,
    sizeof(soundValDataChrUsrDescr) - 1,
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic declaration. */
  {
    attChUuid,
    (uint8_t *) soundValConfigChr,
    (uint16_t *) &soundLenConfigChr,
    sizeof(soundValConfigChr),
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic value. */
  {
    soundUuidConfig,
    (uint8_t *) soundValConfig,
    (uint16_t *) &soundLenConfig,
    sizeof(soundValConfig),
    ATTS_SET_WRITE_CBACK,
    ATTS_PERMIT_READ | ATTS_PERMIT_WRITE
  },
  /* Characteristic user description. */
  {
    attChUserDescUuid,
    (uint8_t *) soundValConfigChrUsrDescr,
    (uint16_t *) &soundLenConfigChrUsrDescr,
    sizeof(soundValConfigChrUsrDescr),
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic declaration. */
  {
    attChUuid,
    (uint8_t *) soundValPeriodChr,
    (uint16_t *) &soundLenPeriodChr,
    sizeof(soundValPeriodChr),
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic value. */
  {
    soundUuidPeriod,
    (uint8_t *) soundValPeriod,
    (uint16_t *) &soundLenPeriod,
    sizeof(soundValPeriod),
    ATTS_SET_WRITE_CBACK,
    ATTS_PERMIT_READ | ATTS_PERMIT_WRITE
  },
  /* Characteristic user description. */
  {
    attChUserDescUuid,
    (uint8_t *) soundValPeriodChrUsrDescr,
    (uint16_t *) &soundLenPeriodChrUsrDescr,
    sizeof(soundValPeriodChrUsrDescr),
    0,
    ATTS_PERMIT_READ
  }
};

/* Gyro group structure. */
static attsGroup_t soundGroup =
{
  NULL,
  (attsAttr_t *) soundList,
  NULL,
  NULL,
  SOUND_HANDLE_START,
  SOUND_HANDLE_END
};

/*************************************************************************************************/
/*!
 *  \fn     SvcSoundAddGroup
 *
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcSoundAddGroup(void)
{
  AttsAddGroup(&soundGroup);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcSoundRemoveGroup
 *
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcSSoundRemoveGroup(void)
{
  AttsRemoveGroup(SOUND_HANDLE_START);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcSoundCbackRegister
 *
 *  \brief  Register callbacks for the service.
 *
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcSoundCbackRegister(attsWriteCback_t writeCback)
{
  soundGroup.writeCback = writeCback;
}
