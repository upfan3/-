#ifndef SET_MENUITEM_H
#define SET_MENUITEM_H

#include "globalval.h"
#include "powercan.h"

void setOnRemoteCtrl(Screen *pCurrentScreen);
void  SetDCDMenu(Screen * pCurrentScreen);
void SetModuleTP(Screen * pCurrentScreen);
void setSMRONOFF(Screen *pCurrentScreen);
void SetBootMenu(Screen * pCurrentScreen);	
void SetBattTestMenu(Screen * pCurrentScreen);
void SetBattDownPara(Screen *pCurrentScreen);
void SetSleepPara(Screen *pCurrentScreen);
void SetPeakShiftPara(Screen *pCurrentScreen);
void SetPeakPara(Screen *pCurrentScreen);
void SetPeakShavePara(Screen *pCurrentScreen);
void SetCalibrationBattI(Screen *pCurrentScreen);
void SetCalibrationBattV(Screen *pCurrentScreen);
void ShowALARM(Screen *pCurrentScreen);
void setDiPara(Screen *pCurrentScreen);
void setDoPara(Screen *pCurrentScreen);
void SetUserDownPara(Screen *pCurrentScreen);//设置用户下电
void SetLocalPower(Screen *pCurrentScreen);
void SetTimePara(Screen *pCurrentScreen);
void SetAlarmPara(Screen *pCurrentScreen);
void SetOter(Screen *pCurrentScreen);
void SetBattParaSetMenu(Screen *pCurrentScreen);
void GetAlarmRecordMenu(Screen *pCurrentScreen);
void GetEenergyRecordMenu(Screen *pCurrentScreen);
void GetEenergy90dayRecordMenu(Screen *pCurrentScreen);
void GetBattTestRecordMenu(Screen *pCurrentScreen);
void ComparaPassWord(Screen * pCurrentScreen);
void FactorySettingPara(Screen *pCurrentScreen);
void SetUSRPara(Screen * pCurrentScreen);
void SetBranchDownParaMenu(Screen *pCurrentScreen);
void SetBranchBroadMenu(Screen *pCurrentScreen);
void SetBranchParaMenu(Screen *pCurrentScreen);

void SetBranchDownIMenu(Screen *pCurrentScreen);
void SetBranchCalibrationIMenu(Screen *pCurrentScreen);
void SetBranchCalibrationVMenu(Screen *pCurrentScreen);
void SetUserDownMenu(Screen *pCurrentScreen);
void SetUSR2Menu(Screen *pCurrentScreen);
void SetUSR3Menu(Screen *pCurrentScreen);
void SetPOWERRATEMenu(Screen *pCurrentScreen);
void GetEenergyRateRecordMenu(Screen *pCurrentScreen);
void SetBattBranchMenu(Screen *pCurrentScreen);
void GetUpDataLogMenu(Screen *pCurrentScreen);
void ComparapFactoryPassWord(Screen * pCurrentScreen);
void SetBttCalibration(Screen *pCurrentScreen);
void SetCalibrationMenu2(Screen *pCurrentScreen);

//void SetHotFanPara(Screen *pCurrentScreen);

extern u8 setPeakKey0;
extern u8 setPeakKey1;
extern u8 setSpecialParaFlag;




//extern u8 gDcmStatus[10];
#endif

