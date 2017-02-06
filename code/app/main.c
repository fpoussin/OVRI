/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"

#define SAMPLES 2048

static adcsample_t samples1[SAMPLES];
static adcsample_t samples2[SAMPLES];

/*
 * ADC streaming callback.
 */
size_t nx = 0, ny = 0;
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;

  // Todo: Check peak amplitude
}

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  if (err == ADC_ERR_AWD1) // Peak detection (12 bits)
  {
    // Validate peak went above requirements based on frequency
    // Then disable comparator blanking to enable output
    adcp->adcm->ISR &= ~ADC_ISR_AWD1; // Clear interrupt error flag.
  }

  else if (err == ADC_ERR_AWD2) // Zero crossing? Unused. (Limited to 8 bits)
  {
    adcp->adcm->ISR &= ~ADC_ISR_AWD2; // Clear interrupt error flag.
  }

  else if (err == ADC_ERR_AWD3) // Unused. (Limited to 8 bits)
  {
    adcp->adcm->ISR &= ~ADC_ISR_AWD3; // Clear interrupt error flag.
  }
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, N samples of 1 channels, continuous.
 * Channels:    IN1.
 */
static const ADCConversionGroup adcgrpcfg1 = {
  TRUE,
  1,
  adccallback,
  adcerrorcallback,
  ADC_CFGR_CONT | ADC_CFGR_AWD1_SINGLE(1),            /* CFGR    */
  ADC_TR(0, 4095),          /* TR1 for peak detection  */
  {                         /* SMPR[2] */
    ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_19P5),   /* Sampling rate = 36000000/(19.5+12.5) = 1.125Mhz  */
    0
  },
  {                         /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
    0,
    0,
    0
  }
};

/*
 * ADC conversion group.
 * Mode:        Linear buffer, N samples of 1 channels, continuous.
 * Channels:    IN3.
 */
static const ADCConversionGroup adcgrpcfg2 = {
  TRUE,
  1,
  adccallback,
  adcerrorcallback,
  ADC_CFGR_CONT | ADC_CFGR_AWD1_SINGLE(3),            /* CFGR    */
  ADC_TR(0, 4095),          /* TR1 for peak detection */
  {                         /* SMPR[2] */
    ADC_SMPR1_SMP_AN3(ADC_SMPR_SMP_19P5),  /* Sampling rate = 36000000/(19.5+12.5) = 1.125Mhz  */
    0
  },
  {                         /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3),
    0,
    0,
    0
  }
};

static const DACConfig dac1cfg1 = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  dacStart(&DACD1, &dac1cfg1);
  chThdSleepMilliseconds(10);

  adcStart(&ADCD1, NULL);
  adcStart(&ADCD2, NULL);

  dacPutChannelX(&DACD1, 1, 2047);
  adcStartConversion(&ADCD1, &adcgrpcfg1, samples1, SAMPLES);
  adcStartConversion(&ADCD2, &adcgrpcfg2, samples2, SAMPLES);

  /* Normal main() thread activity, nothing in this test.*/
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
