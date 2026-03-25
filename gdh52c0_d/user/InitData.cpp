#include "globalval.h"
#include "cydt1363.h"
#include "core_cm3.h"


const u8 Ver[]="h52c0_v01";
//*************************工作参数**********************************


//双字节参数
u16 gSetfloatV=5520;           //浮充电压        54V
u16 gSetequalV=5640;           //均充电压        56.4V
u16 gtestV=4900;               //放电测试电压    48V
u16 gSetBattLimitPerC=10;      //电池限流系数    0.1
u16 gSetBattC=600;             //电池容量        600AH
u16 gSetStartBRI=5;            //均充启动系数     0.05C 
u16 gSetEndBRI=1;              //均充结束系数          0.01C
u16 gstartdelay=1;             //均充启动延时      1
u16 gEndDelay=120;               //均充结束延时      1
u16 gInOwrVLimit=18000;        //输入欠压告警值      180V
u16 gInOverVLimit=26000;       //输入过压告警值      250V
u16 gInOverILimit=6000;         //输入过流告警值      4A
u16 gOutOwrVLimit=4800;        //输出欠压告警值      47V
u16 gOutOverVLimit=5800;       //输出过压告警值      59V
u16 gOverFLimit=5200;          //频率过高告警值      52Hz
u16 gOwrFLimit=4850;           //频率过低告警值      48.5Hz
u16 ghtemp=6000;               //高温告警            55
u16 gltemp=1000;               //低温告警            10
u16 gdownV=4320;               //电池下电电压值      43.2V
u16 gbattDownTime=1;          //交流停电电池下电延时单位为分钟           60秒 
u16 gsleepStartDelay=1;        //休眠启动延迟           1  分钟 
u16 gsleepBattTestPeriod=1800;    //休眠电池测试周期     30 分钟 
u16 gRateCurr=110;              //模块限流点
u16 gsafedisV=4500;           //错峰安全放电电压      48V
u16 gdisDeep=6000;               //错峰安全放电深度     60%

u16 gSetBattLimitPerCSpike=0xffCE;     //尖峰时电池限流系数    -50 -0.5
u16 gSetBattLimitPerCPeak=0xffE2;     //峰时电池限流系数    -30 -0.3

u16 gPeakModleDisChargeVol = 4800;      //锂电放电电压
u16 gPeakModleChargeVol = 5700;      //锂电充电电压
//u16 gSetBattLimitPerCNormal=10;     //平时电池限流系数    0.1
//u16 gSetBattLimitPerCTrough=20;   //谷时电池限流系数    0.20
//u16 gSinglePhasePower=300;   //尖峰功率  30KW 取单位为 0.1KW
u16 gBattRecovSoc=2000;  //锂电尖高峰回充SOC
u16 gAPhasePower=300;    //高峰功率 30KW 0.1KW
u16 gBPhasePower=300;   //平段功率  30KW 0.1KW
u16 gCPhasePower=300;   //低谷功率  30KW 0.1KW
u16 gSetILimitCs=200; //0.1A  尖峰时段限流
u16 gSetILimitCp=200; //0.1A  高峰时段限流
u16 gSetILimitCn=200; //0.1A  平时段限流
u16 gSetILimitCt=200; //0.1A  谷时段限流
u16 gBattSafeSoc=2000;//锂电安底容量//gAcRateI=200;    //0.1空调额定电流
//u16 gAcRateP=500;    //0.01KW 5KW空调额定功率
u16 gOcrdiffV = 500; //0.5V  //光伏电压差
u16 gtmpFactor16=52;  //两字节温度保偿  



InputAlarm gInPutAlarm[13]={
	{0,0,1,6},//门禁 常开 告警号 无1
	{1,0,0,6},//水浸 常开 告警号 无2
	{2,0,0,6},//烟雾 常开 告警号 无3
	{3,1,0,6},//防雷 常闭 告警号 无4
	{4,0,0,6},//风机 常开 告警号 无5
	{5,0,0,6},//空调 常开 告警号 无6
	{6,0,0,6},//市电输入 常开 告警号 无7
	{7,0,0,6},//油机输入 常开 告警号 无8
	{8,0,0,6},//防雷开关 常开 告警号 无9
	{9,0,2,6},//电池熔丝 常开 告警号 无
	{10,0,0,6},//负载熔丝1 常开 告警号 无
	{11,0,0,6},//负载熔丝2 常开 告警号 无
	{11, 0,0,6},//防雷      常开 告警号 无
};
SPeakTimeInteval gsPeakTimeInteval[5]={
	 31,60,120,180,240,300,360,420,480,540,600,660,720,
	 90,60,120,180,240,300,360,420,480,540,600,660,720,
	180,60,120,180,240,300,360,420,480,540,600,660,720,
	270,60,120,180,240,300,360,420,480,540,600,660,720,
	300,60,120,180,240,300,360,420,480,540,600,660,720,
	
};

DCMParam0 gDcParam0[10]={{1,0,0,789,123,4400},{2,1,0,698,123,4400},{1,0,1,789,123,4400},{2,1,1,698,123,4400},{1,0,1,789,123,4400},{2,1,0,698,123,4400},{1,0,1,789,123,5350},{2,1,2,698,123,5452},{2,1,2,698,123,5452},{2,1,2,698,123,5452}};
DCMParam1 gDcParam1[10]={{1,{3,9,2020},1800,1000},{2,{4,10,2019},1800,2000},{1,{3,9,2020},1800,1000},{2,{4,10,2019},1800,2000},{1,{3,9,2020},1800,1000},{2,{4,10,2019},1800,2000},{1,{3,9,2020},30,1000},{2,{4,10,2019},30,2000},{2,{4,10,2019},30,2000},{2,{4,10,2019},30,2000}};
u16 galarmRecordMax=0;
u16 genergyDateRecordMax=0;
u16 gdisBattRecordMax=0;
u16 gdiverter=200;//电池分流量程、
u16 recoverV1=4700;	
u16 recoverV2=4700;	
u16 recoverV3=4700;	
u16 recoverV4=4700;
u16 recoverV5=4700;	
u16 recoverV6=4700;
u16 recoverbat=5000;	
u16 goutPutOwrV=4800;
u16 genergyDateRecord90Max=0;
u16 eleRate[5]={6000,6000,6000,6000,6000};//电单价  0.6元/KWH=>60分/KWH=>6000 * 0.01分/KWH =>6000 * (0.001分/0.1KWH)   ，单位为0.001分/0.1kwh
u16 gbranchBatt[8]={300,300,300,300,300,300,300,300};//分路电池容量
SetBattery gsetbatt={{0,0,0,0,0},5400,1500,0x0001,0x0001,0x0101};
s16 limitCap[5][12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                       };
AirInterfaceData g_AirInterfaceData[8] = {{350,550},{350,550},{350,550},{350,550},
																					{350,550},{350,550},{350,550},{350,550},
																					};								 
u16 Volcut=4830;
u16 recoverVol= 4930;												 
																					
u16 disChargeCount1 = 0;
u16 disChargeCount2 = 0;
u16 disChargeCount3 = 0;
u16 disChargeCount4 = 0;
u16 disChargeCount5 = 0;

u16 chargeCount1 = 0;
u16 chargeCount2 = 0;
u16 chargeCount3 = 0;
u16 chargeCount4 = 0;
u16 chargeCount5 = 0;


u16 *disChargeCount[LI_BATTERY_NUM] = {
    &disChargeCount1,
    &disChargeCount2,
    &disChargeCount3,
    &disChargeCount4,
    &disChargeCount5
};

u16 *chargeCount[LI_BATTERY_NUM] = {
    &chargeCount1,
    &chargeCount2,
    &chargeCount3,
    &chargeCount4,
    &chargeCount5
};
	


//在此处以上增加双字节参数
	
//单字节参数
u8  gAutoBootEnable=1;       //自动均充使能      禁能
u8  gtestPeriodEnable=0;     //周期放电测试使能   禁能
u8  gbeeEnable=0;               //蜂鸣器使能     使能
u8  gSleepOnoff=0;             //休眠使能        禁能  
u8  gWorkShiftMode=0;        //错峰使能          禁能
u8  gPeakShaveOnoff=0;         //削峰使能        禁能
u8  gStopTest;               //停电测试       gSinglePhase; //单相使能
u8  gtmpCompEn=0;            //温度补偿使能
u8  gGetEnergyDay=1;           //抄表日          每月4日
u8  gbattDownMod=0;             //下电模式       无下电
u8  gsleepMinCount=1;          //休眠基数        1个
u8  gsleepLoadRate=40;         //模块休眠带载率  45%
u8  gsleepWakeupLoadRate=90;   //唤醒带载率      85%
u8  gsleepBattTestTime=1;      //休眠测试时长    默认为0分钟，若为0分钟最小有30秒
u8  gsleepTurnPeriod=2;        //轮换时长            2小时 
u8  gAutoPeriod=182;         //均充周期         180 天2592000 s
u8  gtestPeriod=182;       //周期放电天  180天 2592000 s
u8 gEnergySle=0;
u8  gAcPhase=7;       //在线相位  0 无 1 C 2 B 3 BC 4 A 5 AC 6AB 7ABC
u8  gtmpFactor=58;   //mV/度
u8 gequalTimeMax=12;        //均充最大时长12小时
u8 gtestTimeout=12;          //周期测试时长          12小时 以秒为单位
u8 gSpeakFlag[30]=
{
	0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,

};//60个错峰时段的错峰标志

//DCMParam2 gDcParam2[4]={{1,0,0,1,0,2},{2,0,0,1,0,2},{1,0,0,1,0,2},{2,0,0,1,0,2}};//,{1,0,0,1,0,2}};//,{2,0,0,1,0,2},{1,0,0,1,0,2},{2,0,0,1,0,2},{2,0,0,1,0,2},{2,0,0,1,0,2}};
//DCMParam3 gDcParam3[4]={{1,1,1,1,1,1},{2,2,2,2,2,2},{1,1,1,1,1,1},{2,2,2,2,2,2}};//,{1,1,1,1,1,1}};//,{2,2,2,2,2,2},{1,1,1,1,1,1},{2,2,2,2,2,2},{2,2,2,2,2,2},{2,2,2,2,2,2}};
u8 battGroud=1;



ChEnable chenble={1,1,1,1,1,1,1,1};	
u8 monitor_addr=1;
u8 gcheckBatt=0;//电池在线检测位
u8 gbootBattCap=30;//85%
u8 branch[10]={2,2,2,2,2,3,3,2,3,3};
u8 genableTP=0xFF;//模块防盗使能
u8 battComType = YDT1363_COM;		//电池通讯方式 0：采用MODBUS 1: 1363

//在此处以上增加单字节参数
u8 EndParaData=0xFF;
//u8 genableTP=2;








u8 *userName=NULL;
u8 gcourrentWorkStatus;
u8 genableTimeCount=10;
u8 *gpSysData=NULL;
DATABASE  *pModuledata=NULL;

u8 *NCspace=NULL;
u8 glimitflag;
u8 gAcFlag=1;       //交流有电标志
SpeakFlag ACRunMode=RUNNULL;//错峰模式
u8 gsleepSucceedFlag=0;
u8 genableTimeFlag=0;
u8 sleepWait=0;
u8 gSPD=0xFF; //0无防雷告警，0x20有防雷告警，0xFF交流配电不在线，防雷标习无效
u16 gMaxModuleCurr=550;

u8 gmoduleInputLowV;
uint64_t  gWarnbit=0;
ACdistribution * pgACmointor=NULL;
DCdistribution gDCdistribution;
u32 * palarmtable=NULL;
u16  gbattDownCount;
u16  EnergyDataWriteConunt=0;
u16  EnergyDataWriteConunt90=0;
u8  gbattDownDelay=30;
u8 gtestSaveFlag;
u16 gbattTestCount;
u8 *updatamem=NULL;

u16 delayPassWord=0;
u8 gmSN[17]={"123456789ABCDEF"};
u8 gsSN[17]={"123456789ABCDEF"};
u8 gMAC[6]={66,43,64,65,64,67};
u8 gIP[4]={192,168,1,112};
u8 gGW[4]={192,168,1,1};
u8 gCustom[17]={"123456789ABCDEF"};
u8 gDNS[4]={192,168,1,1};
u8 gMask[4]={255,255,255,1};
u16 *pgBattBranch[6]={NULL,NULL,NULL,NULL,NULL,NULL};


u16 VerNB=100 ;
u8 gsw[7]={0x20,0x20,0x20,0x20,0x20,0x20,0x20};


Battery batt[LI_BATTERY_NUM];

u8 detectTimer1=0;
u8 detectTimer2=0;

u32 gcounter32=0;			
u8 showSpecilMenu=0;			
			 
extern Queue<CanBuf> gCan2SendBuf2;

uTIME gtestStarTimer ,gtestStopTimer ;//保存测试起止时间 结束时间

u8 send1363buf[2048];




void InitData()
{ 	
	
	  
   	pdisPlayItem=(Item *)pvPortMalloc(sizeof(Item)*(40));//存储 40 个界面项目
	  pdisDisPlayData=(u8 *)pvPortMalloc(sizeof(u8)*(40));//存储 40 个界面显示数据
	  gpSysData=(u8 *)pvPortMalloc(sizeof(u8)*(48));
	  pgACmointor=(ACdistribution *)pvPortMalloc(sizeof(ACdistribution));
//		pCurrentAir=(AIRpara *)pvPortMalloc(sizeof(AIRpara));
		
	  palarmtable=(u32 *)pvPortMalloc(404);	//暂存100个告警值
	  memset(palarmtable,0,404);

	  gDCdistribution.pst_I          = (u16 *)pvPortMalloc(sizeof(u16)*(TOTAL_USER));		
	  gDCdistribution.pst_status     = (u8 *)pvPortMalloc(sizeof(u8)*(TOTAL_USER ));	
	  gDCdistribution.pst_switchtype	= (u8 *)pvPortMalloc(sizeof(u8)*(TOTAL_USER ));		 
    gDCdistribution.pst_enerqy     = (u32 *)pvPortMalloc(sizeof(u32)*(TOTAL_USER ));
	
	  gDCdistribution.pst_userI=(u16 *)pvPortMalloc(sizeof(u16)*6);
	  gDCdistribution.pst_userStatus=(u8 *)(u8 *)pvPortMalloc(sizeof(u8)*6);	
	  gDCdistribution.pst_userEnerqy=(u32 *)pvPortMalloc(sizeof(u32)*6); 
	
	
	

	 dNew(pModuledata,DATABASE,REC_MODULE,30);
   pModuledata->Append(VOLTAGE);//模块输出电压
	 pModuledata->Append(CURRENT);//模块输出电流
   pModuledata->Append(VOLTAGE_AC);//输块输入电压
   pModuledata->Append(CURRENT_AC);//输块输入电流 
	 pModuledata->Append(PHASEPOSITION);//物理地址
	 pModuledata->Append(LIMITCURRENT);//限流值 
	 pModuledata->Append(FREQUENCY);//交流频率
	 pModuledata->Append(WRANNING);//告警
	 pModuledata->Append(ONOFF);//开关状态
	 pModuledata->Append(LIMIT_CURR_STATUS);//限流状态
   pModuledata->Append(TEMP);//温度	 
	 pModuledata->Append(MODULETYPE);//模块类型	
	 pModuledata->Append(MODULE_RATE_I);//模块额定电流
	 pModuledata->Append(MODULE_EFFICIEN);//模块效率
	 pModuledata->Append(POWER);//功率
	 pModuledata->Append(ENERGY1);//电能

	 
	 NCspace=(u8 *)pvPortMalloc(sizeof(u8)*(8));

     u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING); 

	 // 预设模块告警状态初始值（0x30 对应 ASCII 字符 '0'，表示初始无告警）
    memset(pModuleWRANNING,0x30,30);
    sprintf((char *)gsw,(const char *)"V%4.2f",(float)VerNB/100);
    
		  for(u8 i=i;i<7;i++)
					{
						if(gsw[i]==0)  gsw[i]=0x20;
					}
					
		  u8 *plimitdat=((u8 *)pModuledata->GetDataAddr(LIMIT_CURR_STATUS));	
       for(u8 i=0;i<30;i++)	
			{
				 plimitdat[i]=1;
			}	
					


		
			
//		 u8 *ptmpdc=(u8 *)pvPortMalloc(sizeof(8)*424);
//    gDCdistribution.pst_I=(u16 *)ptmpdc;//分路电流44个 两字节
//	  gDCdistribution.pst_V=(u16 *)&gDCdistribution.pst_I[44];//分路电压0个 两字节
//	  gDCdistribution.pst_status=(u8 *)&gDCdistribution.pst_V[0];//分路状态 熔丝及下电状态 32 个，单字节
//	  gDCdistribution.pst_user=(u8 *)&gDCdistribution.pst_status[44];//分路用户名44个，用户（1~8）或电池9，0未定义，单字节
//		gDCdistribution.pst_enerqy=(u32 *)&gDCdistribution.pst_user[44];//分路电能44个，4字节
//		gDCdistribution.pst_userEnerqy=(u32 *)&gDCdistribution.pst_enerqy[44];//用户电能 8个，4字节
//		gDCdistribution.pst_userI=(u16 *)&gDCdistribution.pst_userEnerqy[8];// 用户电流8个，2字节
//		gDCdistribution.pst_userStatus=(u8 *)&gDCdistribution.pst_userI[8];//用户状态，熔丝，下电，单字节
//		gDCdistribution.pst_switchtype=(u8 *)&gDCdistribution.pst_userStatus[8];//分路开关类型44
			
		

	 
  memset(SwitchOnlineCount,0,TOTAL_USER);



	   
	  pgh52c0->InitPara();
		
	
		pcan2->SetCanFilterAndFIFO1buf(&gCan2SendBuf2);
		pScreen->SetScreenFun((PTRFUN)setMenu);//绑定显示函数处更理函数
		pScreen->SetKey(&pgh52c0->m_io.m_vkey);//绑定按键
	  pModule->SetPara((u16*)(&gpSysData[RECT_CURR]),&glimitflag,&isSetModule,pdisDisPlayData);//绑定模块参数
 
	  pMonitor->Init();
		pgh52c0->Init();
	
	//	gnetwork.Init();//网络初始化
	  gOled.Init();//OLED初始化
	  pScreen->SetOled(&gOled);    //绑定显示OLED
		TIM1_Init();
		
    com1363.Init(send1363buf);
		com1363.SetVersion(4);
			
			setbatt=gsetbatt;
	for(u8 j=0;j<TOTAL_USER;j++)
{	
			
								 
		gSwitchData[j].authorizeflag=0xff;	// 授权标志：0xff表示完全授权			 
		gSwitchData[j].setAuthorize=0xff;// 设定授权
	  gSwitchData[j].st_stopSupplyCount=0x00ff;	// 断电计数
			
 }		
				
 
// 关联电池分路：将指定的直流分路绑定为电池分路，用于电池电流采集
		 for(u8 i=0;i<6;i++)
			{
					 if(gUseToBattBranch[i]<TOTAL_USER)
					 {
							 pgBattBranch[i]=&gDCdistribution.pst_I[gUseToBattBranch[i]];
						 
								gSwitchPara[ gUseToBattBranch[i]].st_userGroud = 0x7B;
								gbattGroud++;
						 
						 
						 
						 
					 }
						
			}
	

// 初始化电池容量设定值：若未关联实际电池分路，按预设值累加
      gSetBattC = 0; 
      if(gbattGroud == 0)
			{
					for(u8 i=0;i<battGroud;i++)
						{
								
										
							 gSetBattC+=gbranchBatt[i];
							
									
						}
			}
		 pbattCap->Init();
}
 
  



void SoftReset(void)
{
	
	__set_FAULTMASK(1);
	NVIC_SystemReset();
	
}