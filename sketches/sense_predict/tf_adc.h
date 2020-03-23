
#ifndef TF_ADC_H
#define TF_ADC_H

#define ADC_EXAMPLE_DEBUG 0 // 1
#define ADC_SAMPLE_RATE 1

#ifdef __cplusplus
extern "C"
{
#endif

int initADC(void);
void triggerAdc(void);
void enableAdcInterrupts(void);

extern volatile uint32_t audioSample;

#ifdef __cplusplus
}
#endif

#endif /* TF_ADC_H */
