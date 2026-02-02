#include "globalval.h"
u32 ab;
void Swtmr2_Callback(void *parameter)
{
	
	 pgh52c0->GetAdc();
	 ab++;
	 detectTimer2=0;

}
