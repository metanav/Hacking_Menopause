#ifndef SVC_SOUND_H
#define SVC_SOUND_H

/**************************************************************************************************
 Handle Ranges
**************************************************************************************************/

#define SOUND_HANDLE_START  0x70
#define SOUND_HANDLE_END   (SOUND_HANDLE_END_PLUS_ONE - 1)

/**************************************************************************************************
 Handles
**************************************************************************************************/

/* Temperature service handles. */
enum
{
  SOUND_HANDLE_SVC = SOUND_HANDLE_START,

  SOUND_HANDLE_DATA_CHR,
  SOUND_HANDLE_DATA,
  SOUND_HANDLE_DATA_CLIENT_CHR_CONFIG,
  SOUND_HANDLE_DATA_CHR_USR_DESCR,

  SOUND_HANDLE_CONFIG_CHR,
  SOUND_HANDLE_CONFIG,
  SOUND_HANDLE_CONFIG_CHR_USR_DESCR,

  SOUND_HANDLE_PERIOD_CHR,
  SOUND_HANDLE_PERIOD,
  SOUND_HANDLE_PERIOD_CHR_USR_DESCR,

  SOUND_HANDLE_END_PLUS_ONE
};

/**************************************************************************************************
 Macros
**************************************************************************************************/

/* Values for config attributes. */
#define SOUND_ATT_CONFIG_DISABLE  0x00u
#define SOUND_ATT_CONFIG_ENABLE   0x01u

/* Values for period attributes. */
#define SOUND_ATT_PERIOD_MAX      250u
#define SOUND_ATT_PERIOD_DEFAULT  100u
#define SOUND_ATT_PERIOD_MIN       10u

/* Sizes of attributes. */
#define SOUND_SIZE_CONFIG_ATT  1u
#define SOUND_SIZE_PERIOD_ATT  1u
#define SOUND_SIZE_DATA_ATT    6u

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

void SvcSoundAddGroup(void);
void SvcSoundRemoveGroup(void);
void SvcSoundCbackRegister(attsWriteCback_t writeCback);

#endif /* SVC_SOUND_H */
