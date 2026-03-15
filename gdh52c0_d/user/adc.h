#ifndef  __ADC_H
#define  __ADC_H

#include "Tqueue.h"

class adc{
	
private:		



public:	
	u8 m_enableflag;
	adc(uint16_t * adc_value,uint16_t data_size);

uint16_t * m_adc_value;
uint16_t  m_data_size;
void reInit(void);
void startADC(void);
void ReadADResult(void);	
void ReadADResultforLowPassfilter3order(void);
//void average10times(void);	
};

#endif
