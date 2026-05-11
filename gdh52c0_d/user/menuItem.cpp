#include "SetMenuitem.h"
extern s16 currLimitCap;
 
 Item * pMainMenu=NULL;//主菜单页
 Item *pALARMMenu=NULL;//告警页
 Item *pSYSMenu;
 Item *pDCDMenu=NULL;//直流屏页
 Item *pSMRMenu=NULL;//整流模块页
 Item *pSCRMenu=NULL;//光伏模块页
 Item *pACDMenu=NULL;//交流配电页
 Item *pUSRMenu=NULL;
 Item *pUSR2Menu=NULL;
 Item *pUSR3Menu=NULL;
 Item *pPOWERRATEMenu=NULL;
 
 
 Item * pBattParaSetMenu=NULL;//电池参数页
 Item *pBattBootSetMenu;//电池均充页
 Item *pBattTestSetMenu=NULL;//电池测试页
 Item *pBattDownParaSetMenu=NULL;//电池下电设置
 Item * pUsrDownParaSetMenu=NULL;;//电池下电设置
 Item * pUsrDownInfo=NULL;//下电板信息
 Item * pSetDIMenu=NULL;
 Item * pSetDOMenu=NULL;
 Item *pSavePowrSetMenu=NULL;//节能设置页
 Item *pSleepMenu=NULL;; //休眠参数页
 Item *pPeakShiftMenu=NULL;//错峰页
 Item *pPeakShaveMenu=NULL;//削峰页
 Item *pSysParaSetMenu=NULL;//系统参数设置页
 Item *pCalibrationMenu=NULL;//电流校准设置页
 Item *pCalibrationMenu2=NULL;//下电板校准设置页
 Item *pBattParaMenu=NULL;//下电板信息页
 Item *pBattCalibrationMenu;//电池电流校准页
 Item *pCalibrationVMenu=NULL;//压校准设置页
 Item *pUSRCalibrationMenu=NULL;//用户配电单元校准页

 Item * pPeakTimeMenu=NULL;//错峰时段页
 Item *pAlarmParaSetMenu=NULL;//告警参数设置页
 Item *pTimeMenu=NULL;
  Item *pRECORDMenu=NULL;//事件记录页
      Item *pAlarmRecordMenu=NULL;//告警记录页
      Item *pPowrRecordMenu=NULL;//抄表记录页
			Item *pPowrRaTeRecordMenu=NULL;//抄表费用录页
			
			Item *pPowrRecord90Menu=NULL;//90天电量记录页
			Item *pBattRecordMenu=NULL;//90天电量记录页
			Item *pUpDataLogMenu=NULL;//90天电量记录页
			
			
			
       Item *pPASSWORD=NULL;
	Item * pFactorySettingMenu=NULL;
	Item * pSetPara=NULL;
	Item *pSYSINFO=NULL;
	
	Item *pSetBranchMenu=NULL;//分路选择设置
	      Item *pSetBranchParaMenu=NULL;  //分路参数设置页
				Item *pUserDownMenu=NULL;
				Item *pBattBranchMenu=NULL;
				
	Item *pTsetMenu=NULL;
	Item *pBattMenu=NULL;
	Item *pLocalPowerMenu=NULL;
	
	// Item *pBATTERYMenu=NULL;//锂电池信息
   Item *pBATTERYData=NULL;//电池参数
//   Item *pBATTERYAlarm=NULL;//电池告警
	 Item *pBatterySetMenu;//锂电设置界面
	 Item *pBattCellVMenu;// 锂电芯电压界面
	 Item *pBattTmpMenu;//锂电温度界面
	 
	 Item *pBatteryCutMenu =NULL;
	
Item *pAirMenu=NULL;

//	Item *HotFanMenu=NULL;
			 
 Item * pdisPlayItem=NULL;
 u8 * pdisDisPlayData=NULL;
 u8 isSetModule=0;

AIRpara g_astAIRpara[8] = {0}; // 定义结构体数组，存储每个地址的空调数据
AIRpara *pCurrentAir = NULL;  // 当前显示的空调数据指针
u8 downModle=0;
u8 timeDownEnable;
s8 UserDownSetItem[3];
//s8 UserDownSetItem[3]={1,0,1};
u16 downVoltageLimit;
u16 downDelayTimeMin=225;
u8 starDownTime[2];
u8 stopDownTime[2];
u8 LocalInPutNb[2]={1,0};
u8 InitSOC=1;

s16 preAlarmCount=0;
s16 preBattTestCount=0;
s16 preDayCount=0;
s16 preDayCount2=0;
u8 gmoduleSN[17]={"               "};

//u8 airAddr=1;

u8 airOrder[2]={1,8};
u8 battAddr[2]={1,4};

u8 battIndex=0;

extern u8 cmdlist[8];
extern u8 gdownbroadinfo[4];
//extern u32 preDCEnergydata[7];
extern u32 gTotalMrEnergy;

extern AirInterfaceData g_AirInterfaceData[8];		
extern AIRpara *pCurrentAir;


extern s16 mt11Tmp;
extern s16 mt11Humi;

extern u8 errCount;
extern u8 gEnergySle;
extern u8 battComType;

extern u16 g_AirColdData;
extern u16 g_AirTempData; 


void *gShow=NULL;
const u8 * onofflist[]={(const u8 *)"否",(const u8 *)"是",NULL};
void *OnOffLis[]={onofflist};

const u8 * battlist[]={(const u8 *)"智能",(const u8 *)"普通",NULL};
void *BattLis[]={battlist};

const u8 * airstatuslist[]={(const u8 *)"关机",(const u8 *)"待机",(const u8 *)"运行",(const u8 *)"故障",NULL};
void *airstatusLis[]={airstatuslist};

const u8 * battWorkCharacter[] = {(const u8 *)"恒压",(const u8 *)"电池",(const u8 *)"自动",(const u8 *)"自动",NULL};
void * BattWorkCharacter[] = {battWorkCharacter};

/*************************************告警***************************************************/
const u8 * ALarmStatusFlag[]={(const u8 *)"告警",(const u8 *)"告警恢复",NULL};
void * ALarmListStatus[]={ALarmStatusFlag};

const u8 * ALarmList[]={(const u8 *)"无",(const u8 *)"输出电压",(const u8 *)"电池SOC"
	                   ,(const u8 *)"MOS温度",(const u8 *)"电池充电温度",(const u8 *)"电池放电温度"
										 ,(const u8 *)"模块输入电流",(const u8 *)"扩展单元",(const u8 *)"电池单体电压"
										 ,(const u8 *)"交流频率"
										 ,(const u8 *)"缺A相" ,(const u8 *)"缺B相"  ,(const u8 *)"缺C相"    
	                   ,(const u8 *)"分路",(const u8 *)"智能空开",(const u8 *)"模块"
										 ,(const u8 *)"电池温度",(const u8 *)"环境温度"
										 ,(const u8 *)"电池",(const u8 *)"供电"//电池供电
										
										 ,(const u8 *)"门禁",(const u8 *)"水浸",(const u8 *)"烟雾" ,(const u8 *)"防雷"
										 ,(const u8 *)"风机",(const u8 *)"空调"
										 ,(const u8 *)"市电输入",(const u8 *)"油机输入"
										 ,(const u8 *)"防雷开关"
										 ,(const u8 *)"电池熔丝",(const u8 *)"熔丝(1)",(const u8 *)"熔丝(2)",(const u8 *)"无"//,(const u8 *)"交流输出"
                     ,NULL};

const u8 * ALarmnbList[]={(const u8 *)" ",(const u8 *)"1号",(const u8 *)"2号",(const u8 *)"3号",(const u8 *)"4号",										 
										      (const u8 *)"5号",(const u8 *)"6号",(const u8 *)"7号",(const u8 *)"8号",(const u8 *)"9号",	
										      (const u8 *)"10号",(const u8 *)"11号",(const u8 *)"12号",(const u8 *)"13号",(const u8 *)"14号",
                          (const u8 *)"15号",(const u8 *)"16号",(const u8 *)"17号",(const u8 *)"18号",(const u8 *)"19号",
                          (const u8 *)"20号",(const u8 *)"21号",(const u8 *)"22号",(const u8 *)"23号",(const u8 *)"24号",	
                          (const u8 *)"25号",(const u8 *)"26号",(const u8 *)"27号",(const u8 *)"28号",(const u8 *)"29号",															
										      (const u8 *)"30号",(const u8 *)"31号",(const u8 *)"32号",(const u8 *)"33号",(const u8 *)"34号",
													(const u8 *)"35号",(const u8 *)"36号",(const u8 *)"37号",(const u8 *)"38号",(const u8 *)"39号",		
                          (const u8 *)"40号",(const u8 *)"41号",(const u8 *)"42号",(const u8 *)"43号",(const u8 *)"44号",
													(const u8 *)"45号",(const u8 *)"46号",(const u8 *)"47号",(const u8 *)"48号",(const u8 *)"49号",
													(const u8 *)"50号",(const u8 *)"51号",(const u8 *)"52号",(const u8 *)"53号",(const u8 *)"54号",
													(const u8 *)"55号",(const u8 *)"56号",(const u8 *)"57号",(const u8 *)"58号",(const u8 *)"59号",
													(const u8 *)"60号",(const u8 *)"61号",(const u8 *)"62号",(const u8 *)"63号",(const u8 *)"64号",
													(const u8 *)"65号",(const u8 *)"66号",(const u8 *)"67号",(const u8 *)"68号",(const u8 *)"69号",
													(const u8 *)"70号",(const u8 *)"71号",(const u8 *)"72号",(const u8 *)"73号",(const u8 *)"74号",
													(const u8 *)"75号",(const u8 *)"76号",(const u8 *)"77号",(const u8 *)"78号",(const u8 *)"79号",
													(const u8 *)"80号",(const u8 *)"81号",(const u8 *)"82号",(const u8 *)"83号",(const u8 *)"84号",
													(const u8 *)"85号",(const u8 *)"86号",(const u8 *)"87号",(const u8 *)"88号",(const u8 *)"89号",
													(const u8 *)"90号",(const u8 *)"91号",(const u8 *)"92号",(const u8 *)"93号",(const u8 *)"94号",NULL														
};


const u8 * BehaviorList[]={ (const u8 *)" ",(const u8 *)"过低",(const u8 *)"过高",(const u8 *)"异常",(const u8 *)"断开",(const u8 *)"下电"										 
										       ,(const u8 *)"故障", (const u8 *)"通信故障",(const u8 *)"停电",(const u8 *)"过载",(const u8 *)"低压"
                           ,(const u8 *)" FLASH故障",(const u8 *)" 短路",(const u8 *)" 放电过流",(const u8 *)" 充电过流",(const u8 *)" 单体压差大",NULL};
										

void *AlarmMunklis[]={ALarmnbList,ALarmList,BehaviorList};


const u8 *removelist[]={(const u8 *)" ",(const u8 *)"清除",(const u8 *)"0K",NULL};
void *RemoveList[]={removelist};

const u8 *switchFactory[]={(const u8 *)"JG",(const u8 *)"TY",NULL};
void   *SwitchFactory[]={switchFactory};


const u8 *jumpprotocol[]={(const u8 *)"NJP",(const u8 *)"JP",NULL};//跳空， 非跳空
void   *JumpProtocol[]={jumpprotocol};


/*************************************工作模式***************************************************/
const u8 * Work_AC_Mode_Cur_List[]={(const u8 *)"    ",(const u8 *)"尖峰",(const u8 *)"高峰",(const u8 *)"平段",(const u8 *)"低谷",NULL};

const u8 *  workModList[]={(const u8 *)"浮充",(const u8 *)"均充",(const u8 *)"测试",(const u8 *)"放电",(const u8 *)"停电",NULL};
const u8 * workModList2[]={(const u8 *)"    ",(const u8 *)"削峰",NULL};
const u8 * workModList3[]={(const u8 *)"  ",(const u8 *)"CL",NULL};
const u8 * workModList4[]={(const u8 *)"  ",(const u8 *)"SL",NULL};
void *Worklis[]={workModList,Work_AC_Mode_Cur_List,workModList2,workModList3,workModList4};


const u8 *  changemodlist[]={(const u8 *)"浮充",(const u8 *)"均充",(const u8 *)"测试",(const u8 *)"无",NULL};
void * ChangeModList[]={changemodlist};
 
const u8 * swlist[]={(const u8 *)"休眠",(const u8 *)"开机",(const u8 *)"关机",(const u8 *)"--",NULL};
const u8 * limtCurlist[]={(const u8 *)"限流",(const u8 *)"正常",(const u8 *)"故障",(const u8 *)"--",NULL};
void *Workstatuslist[]={limtCurlist,swlist};

void *SwitchModule[]={ &swlist[1]};

/***************************************错峰模式************************************************************************************/
const u8 * WorkACModeCurList[]={(const u8 *)"未定",(const u8 *)"尖峰",(const u8 *)"高峰",(const u8 *)"平段",(const u8 *)"低谷",NULL};
void *ACWorkMode[] ={WorkACModeCurList};


const u8 *DischageModeList[]={(const u8 *)"充电",(const u8 *)"不充不放",(const u8 *)"放电",NULL};
void *DischageMode[]={DischageModeList};


/*************************************电池电流校准***************************************************/
const u8 * calibrationList[]={(const u8 *)"高点",(const u8 *)"低点",NULL};
void *Calilis[]={calibrationList};

const u8 *rangList[]={(const u8 *)"禁用",(const u8 *)"100",(const u8 *)"200",(const u8 *)"300",(const u8 *)"400",NULL};
void *RangList[]={rangList};


const u8 *phaselist[]={(const u8 *)"无",(const u8 *)"C相",(const u8 *)"B相",(const u8 *)"BC相",(const u8 *)"A相",(const u8 *)"AC相",(const u8 *)"AB相",(const u8 *)"ABC相",NULL};
void *PHlist[]={phaselist};

/***************************************电池下电模式************************************************************************************/
const u8 * battDownList[]={(const u8 *)"禁用",(const u8 *)"电压下电",(const u8 *)"时间下电",NULL};
void *BattDownLis[]={battDownList};

/***************************************远程本地下电模式************************************************************/
const u8 * userDownList[]={(const u8 *)"手动上电",(const u8 *)"电压下电",(const u8 *)"时间下电",(const u8 *)"手动下电",NULL};
void *userDownlis[]={userDownList};
const u8 * userDownType[]={(const u8 *)"本地分路1",(const u8 *)"本地分路2",(const u8 *)"扩展分路1",(const u8 *)"扩展分路2",(const u8 *)"扩展分路3",(const u8 *)"扩展分路4",NULL};
void *userDowntype[]={userDownType};


const u8 * userDownList2[]={(const u8 *)"手动上电",(const u8 *)"电压下电",(const u8 *)"时间下电",(const u8 *)"电量下电",NULL};//,(const u8 *)"手动下电"//,(const u8 *)"重置下电",
void *userDownlis2[]={userDownList2}; 

//const u8 *userName2[]={(const u8 *)"未定义",(const u8 *)"移动",(const u8 *)"联通",(const u8 *)"电信",(const u8 *)"广电",(const u8 *)"行业外客户",(const u8 *)"铁塔自用",NULL};
//void *UserName2List={userName2};

const u8 * userlist2[]={ (const u8 *)"全部",
	                      (const u8 *)"移动",(const u8 *)"联通",(const u8 *)"电信",
	                      (const u8 *)"广电",(const u8 *)"行业外",(const u8 *)"铁塔",
                        (const u8 *)"电池分路",NULL};
void *UserListMenu2[]={userlist2};
const u8 * userlist[]={ (const u8 *)"未定义",
	    (const u8 *)"移动5G",(const u8 *)"移动其它",
			(const u8 *)"联通5G",(const u8 *)"联通其它",
      (const u8 *)"电信5G",(const u8 *)"电信其它",
      (const u8 *)"广电5G",(const u8 *)"广电其它",	
      (const u8 *)"铁塔"  ,(const u8 *)"行业外",
			 (const u8 *)"电池", 	
   NULL};				
 void *UserListMenu[]={userlist};
 
 
 const u8 *switchtypelist[]={(const u8 *)"已断开",(const u8 *)"单向",(const u8 *)"双向",NULL};
 void *SwitchTypeList[]={switchtypelist};
 
 const u8 *switchtypelist1[]={(const u8 *)"自动",(const u8 *)"手动下电",(const u8 *)"手动上电",NULL};
 void *SwitchTypeList1[]={switchtypelist1};

/***************************************DI输入设置************************************************************/
const u8 * TriggerList[]={(const u8 *)"常开",(const u8 *)"常闭",(const u8 *)"禁用",NULL};
const u8 * ALarmStatusFlag2[]={(const u8 *)"未触发",(const u8 *)"已触发",NULL};
void *DiTriggerlist[]={TriggerList};
void *DIsetlist[]={&ALarmList[20]};
void *ReLayerlist[]={ALarmnbList};
void *Behaviorlist[]={BehaviorList};
void *ALarmNblist[]={ALarmnbList};
void *AlarmStatuslist[]={ALarmnbList,ALarmStatusFlag2};

const u8 *outPutAlarmlist[]={(const u8 *)"无",(const u8 *)"输入故障告警",(const u8 *)"输出电压告警",(const u8 *)"模块故障告警",(const u8 *)"熔丝告警",(const u8 *)"环境告警",(const u8 *)"公共告警",
	                           (const u8 *)"燃料不足告警",(const u8 *)"启动降温",(const u8 *)"启动加热",(const u8 *)"启动发电机",(const u8 *)"负载下电",(const u8 *)"电池下电",NULL};
void *outAlarmList[]={outPutAlarmlist};

/***************************************三相单相***********************************************************/

const u8 *threeandonephlist[]={(const u8 *)"三相",(const u8 *)"单相",NULL};
void *ThreeAndOnephlist[]={threeandonephlist};

/***************************************用户配电校准***********************************************************/
const u8 *userAddrList[]={(const u8 *)"本地",(const u8 *)"1",(const u8 *)"2",(const u8 *)"3",(const u8 *)"4",NULL};
const u8 *userCHList[]={(const u8 *)"用户电流1",(const u8 *)"用户电流2",NULL};//(const u8 *)"电池电流1",(const u8 *)"电池电流2",NULL};

void *UserList[]={userCHList};

const u8 *battCHList[]={(const u8 *)"电池电流1",(const u8 *)"电池电流2",NULL};
void *UserList2[]={battCHList};

const u8 *battCHList1[]={(const u8 *)"电流1(A)",(const u8 *)"电流2(A)",(const u8 *)"电压(V)",NULL};
void *BattList1[]={battCHList1};


const u8 * rangelist[]={(const u8 *)"无",(const u8 *)"100",(const u8 *)"200",(const u8 *)"300",(const u8 *)"400",(const u8 *)"500",
                                          (const u8 *)"600",(const u8 *)"700",(const u8 *)"800",(const u8 *)"900",(const u8 *)"1000",
                                          (const u8 *)"1100",(const u8 *)"1200",NULL};
void * RangeList[]={rangelist};

const u8 *PassWordList[]={(const u8 *)"0",(const u8 *)"1",(const u8 *)"2",(const u8 *)"3",(const u8 *)"4",										 
										      (const u8 *)"5",(const u8 *)"6",(const u8 *)"7",(const u8 *)"8",(const u8 *)"9",	
										      (const u8 *)"*",(const u8 *)"*",NULL,	
										
};


void *PassList[]={PassWordList,PassWordList,PassWordList,PassWordList};

const u8 *cmdList[]={(const u8 *)"0",(const u8 *)"1",(const u8 *)"2",(const u8 *)"3",(const u8 *)"4",										 
										      (const u8 *)"5",(const u8 *)"6",(const u8 *)"7",(const u8 *)"8",(const u8 *)"9",	
										      (const u8 *)"A",(const u8 *)"B",(const u8 *)"C",(const u8 *)"D",(const u8 *)"E",
													(const u8 *)"F"	,NULL
														
									  };
void *Cmdlist[]={cmdList,cmdList,cmdList,cmdList,cmdList,cmdList,cmdList,cmdList};


const u8 *moduletPlist[]={(const u8 *)"未绑定",(const u8 *)"已绑定",(const u8 *)"锁定",(const u8 *)"锁定",NULL};
void *ModuletPList[]={moduletPlist};

//const u8 broadlist[]={(const u8 *)"1",(const u8 *)"2",(const u8 *)"3",(const u8 *)"4",										 
//										      (const u8 *)"5",(const u8 *)"6",(const u8 *)"7",(const u8 *)"8",NULL};

const u8 *broadlist[]={(const u8 *)"0",(const u8 *)"1",(const u8 *)"2",(const u8 *)"3",(const u8 *)"4",										 
										      (const u8 *)"5",(const u8 *)"6",(const u8 *)"7",(const u8 *)"8",(const u8 *)"9",	
										      (const u8 *)"10",(const u8 *)"*",NULL};


void *Braodlist[]={broadlist};


const u8 * statuslist[]={(const u8 *)"无",(const u8 *)"预充",(const u8 *)"直充电",(const u8 *)"直放电",
                         (const u8 *)"BUCK充电",(const u8 *)"BOOST充电",(const u8 *)"BUCK放电",(const u8 *)"BOOST放电",(const u8 *)"待机",
                         (const u8 *)"告警",(const u8 *)"保护关机",(const u8 *)"故障关机",(const u8 *)"维护模式",
                         (const u8 *)"测试模式",(const u8 *)"休眠",(const u8 *)"异常",NULL};
void *StatuLis[]={statuslist};
													
u8 *snlist[]={&gmSN[0],NULL};
void * SNList[]={snlist};

u8 *ssnlist[]={&gsSN[0],NULL};
void * SSNList[]={ssnlist};

u8 *msnlist[]={&gmoduleSN[0],NULL};
void *MSNList[]={msnlist};


u8 _sn=0;
u16 passWord=1234;
u16 passWord2=1010;
//u16 passWord=0x2FBD;


 u8 SetScreenItem(Item *pItem,u8 *pContext=NULL,void * plink=NULL,u8 * pParalist=NULL,u8 CtrlType=0 )
{
static	Item *pset=NULL;
       if(pItem!=NULL)
			 {
				 pset=pItem;
			 }
			 if(pset==NULL) return 0;
			 pset->st_pContext=(u32)pContext;
			 pset->st_plink=plink;
			 pset->st_pParalist=pParalist;
			 pset->st_CtrlType=CtrlType;
			 pItem->dec=0;
			 pset++;
			 return 1;

	
	
}


void  SetScreenItemSelectFirst(Item *pItem)
{
	 pItem->dec=1;//设置进入界面后即为选择翻页
}


u8 setMenu(Item **ppItem)
{
	     if(ppItem==&pMainMenu)//菜单界面
		  {
							  pMainMenu=pdisPlayItem;
				 if(pMainMenu!=NULL)
				 { 
                     SetScreenItem(pMainMenu,   (u8 *)"设置"      ,&passWord,(u8 *)(&pSetPara),NEXT_SCREEN);
					     			 SetScreenItem(NULL,        (u8 *)"实时告警"  ,NULL,(u8 *)(&pALARMMenu),NEXT_SCREEN);//pSetPara
//				             SetScreenItem(NULL,        (u8 *)"交流配电"  ,NULL,(u8 *)(&pACDMenu),NEXT_SCREEN);
//				             SetScreenItem(NULL,        (u8 *)"整流模块"  ,NULL,(u8 *)(&pSMRMenu),NEXT_SCREEN);
//					           SetScreenItem(NULL,        (u8 *)"光伏模块"  ,NULL,(u8 *)(&pSCRMenu),NEXT_SCREEN);
					           SetScreenItem(NULL,        (u8 *)"锂电信息"  ,NULL,(u8 *)(&pBATTERYData),NEXT_SCREEN);
					           SetScreenItem(NULL,        (u8 *)"配电信息"  ,NULL,(u8 *)(&pUSRMenu),NEXT_SCREEN);
//					           SetScreenItem(NULL,        (u8 *)"空调信息 ", NULL,(u8 *)(&pAirMenu),NEXT_SCREEN);
//					           SetScreenItem(NULL,        (u8 *)"扩展分路信息"  ,NULL,(u8 *)(&pUSR3Menu),NEXT_SCREEN);
											SetScreenItem(NULL,        (u8 *)"用户分路信息"  ,NULL,(u8 *)(&pUSR2Menu),NEXT_SCREEN);
					           SetScreenItem(NULL,        (u8 *)"历史记录"  ,NULL,(u8 *)(&pRECORDMenu),NEXT_SCREEN);				
					           	SetScreenItem(NULL,       (u8 *)"系统信息 ", NULL,(u8 *)(&pSYSINFO),NEXT_SCREEN);
											
					         
					           SetScreenItem(NULL);//结束
					          
					 
					 
					 

					 return 1;
				 }
				 else
				 {
						return 0;
				 }			
				
				
			}
				else if(ppItem==&pSetPara)//设置界面
			{
				
				
					  pgh52c0->SaveFlashData16(sysPara,72);
				 
				
					pSetPara=pdisPlayItem;
				if(pSetPara!=NULL)
				{
				   
				  SetScreenItem(pSetPara,   (u8 *)"参数设置" ,NULL,(u8 *)(&pSYSMenu),NEXT_SCREEN);
					SetScreenItem(NULL,       (u8 *)"工厂设置 ",NULL,(u8 *)(&pSysParaSetMenu),NEXT_SCREEN);
					SetScreenItem(NULL,       (u8 *)"锂电设置 ",NULL,(u8 *)(&pBatterySetMenu),NEXT_SCREEN);
					SetScreenItem(NULL,       (u8 *)"备电设置 ",NULL,(u8 *)(&pBatteryCutMenu),NEXT_SCREEN);
					SetScreenItem(NULL,       (u8 *)"智能空开设置 ",NULL,(u8 *)(&pSetBranchMenu),NEXT_SCREEN);
//					SetScreenItem(NULL,       (u8 *)"扩展单元设置",NULL,(u8 *)(&pBattMenu),NEXT_SCREEN);//
//					SetScreenItem(NULL,       (u8 *)"分路下电设置",NULL,(u8 *)(&pLocalPowerMenu),NEXT_SCREEN);//						
					SetScreenItem(NULL,       (u8 *)"清除记录 ", NULL,(u8 *)(&pFactorySettingMenu),NEXT_SCREEN);
					
					if(showSpecilMenu==1)
					{
						showSpecilMenu=0;
						SetScreenItem(NULL,       (u8 *)"工厂密码; %x%x%x%x",PassList,passwordshow,MODIFI_PARA);		
						SetScreenItem(NULL,       (u8 *)"TSET ",     NULL,(u8 *)(&pTsetMenu),NEXT_SCREEN);
					}
					SetScreenItem(NULL);//结束//14
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pLocalPowerMenu)
			{
				
				pdisDisPlayData[0]=0;
				u16 *precoverV=&recoverV1;
				pdisDisPlayData[1]=gDcParam0[pdisDisPlayData[0]].st_downModle; 
			 (*(u16 *)(&pdisDisPlayData[2]))=gDcParam0[pdisDisPlayData[0]].st_LDVoltage;
			 (*(u16 *)(&pdisDisPlayData[4]))=gDcParam1[pdisDisPlayData[0]].st_LDDelay;
				(*(u16 *)(&pdisDisPlayData[13]))=recoverV1;
				setBattParaflag=3;
				pLocalPowerMenu=pdisPlayItem;
				if(pLocalPowerMenu!=NULL)
				{					
					SetScreenItem(pLocalPowerMenu , (u8 *)"%l"       ,  userDowntype, &pdisDisPlayData[0] ,MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"%l"                 ,  userDownlis,  &pdisDisPlayData[1] ,MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"下电电压    %F.2V"   , &pdisDisPlayData[2],0,SCREEN_TWO_BYTE|MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"恢复电压    %F.2V"   , &pdisDisPlayData[13],0,SCREEN_TWO_BYTE|MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"下电时间    %F.1分钟", &pdisDisPlayData[4],0,SCREEN_TWO_BYTE|MODIFI_PARA);
					SetScreenItem(NULL);//结束
					SetScreenItemSelectFirst(pALARMMenu);//设置为选择翻页
					return 1;
				}
				else 
				   return 0;
				
				
			}
			else if(ppItem==&pBatteryCutMenu)//备电切换设置
			{
					
				(*(u16 *)&pdisDisPlayData[0])=	Volcut;		
				(*(u16 *)&pdisDisPlayData[2])=	recoverVol;	
				pBatteryCutMenu=pdisPlayItem;
				if(pBatteryCutMenu!=NULL)
				{
					SetScreenItem(pBatteryCutMenu,   (u8 *)"备电切换电压  %F.2V"   , &pdisDisPlayData[0],0,SCREEN_TWO_BYTE|MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"恢复电压    %F.2V"   , &pdisDisPlayData[2],0,SCREEN_TWO_BYTE|MODIFI_PARA);
					SetScreenItem(NULL);//结束
					return 1;
				}
				else 
				   return 0;
			}
			else if(ppItem==&pSYSMenu)//参数设置界面
			{
				
					pSYSMenu=pdisPlayItem;
				if(pSYSMenu!=NULL)
				{
				   
					
//					SetScreenItem(pSYSMenu,(u8 *)"电池参数设置 ",NULL,(u8 *)(&pBattParaSetMenu),NEXT_SCREEN);//
//					SetScreenItem(NULL,(u8 *)"电池均充设置 ",NULL,(u8 *)(&pBattBootSetMenu),NEXT_SCREEN);//
//					SetScreenItem(NULL,(u8 *)"电池测试设置 ",NULL,(u8 *)(&pBattTestSetMenu),NEXT_SCREEN);//
//					SetScreenItem(NULL,(u8 *)"节能参数设置 ",NULL,(u8 *)(&pSavePowrSetMenu),NEXT_SCREEN);//
//					SetScreenItem(NULL,(u8 *)"告警参数设置 ",NULL,(u8 *)(&pAlarmParaSetMenu),NEXT_SCREEN);//
					SetScreenItem(pSYSMenu,(u8 *)"输入告警设置 ",NULL,(u8 *)(&pSetDIMenu),NEXT_SCREEN);
					SetScreenItem(NULL,(u8 *)"输出告警设置 ",NULL,(u8 *)(&pSetDOMenu),NEXT_SCREEN);
					SetScreenItem(NULL);//结束//14
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pSysParaSetMenu)//工厂设置界面
			{
				   pdisDisPlayData[0]=gGetEnergyDay;
				   pdisDisPlayData[1]=monitor_addr;
				
				    (*(u16*)&pdisDisPlayData[2])=gRateCurr;
				   pdisDisPlayData[4]= sysPara[SWITCH_FACT];
				   pdisDisPlayData[5]= sysPara[PROL_OLD_FLAG];
				
				  
				   pdisDisPlayData[6]=gbeeEnable;
				   pdisDisPlayData[7]=gStopTest;
				   pdisDisPlayData[8]=gcheckBatt;
					for(u8 i=0;i<4;i++)
				   { 
						 pdisDisPlayData[9+i]=gIP[i];
						 pdisDisPlayData[13+i]=gGW[i];
					 }
				   if(genableTP==0xFF)
						 genableTP=0;
				   pdisDisPlayData[17]=genableTP;
					 pdisDisPlayData[18]=battComType;
					 pdisDisPlayData[19]=gEnergySle;
					 pdisDisPlayData[20]=gAcPhase;
					 pdisDisPlayData[21]=sysPara[BATT_MOS_OFF];
			     pdisDisPlayData[22]=sysPara[DELAY_VOLT_UP];
				   pdisDisPlayData[23]=sysPara[SELECT_FACT];
					 
				 pSysParaSetMenu=pdisPlayItem;
				if(pSysParaSetMenu!=NULL)
				{
				   				
			    SetScreenItem(pSysParaSetMenu,    (u8 *)"时间设置        ",NULL,(u8 *)(&pTimeMenu),NEXT_SCREEN);	
					
		     
					SetScreenItem(NULL,               (u8 *)"电流校准      ", NULL,(u8 *)(&pBattCalibrationMenu),NEXT_SCREEN);
					SetScreenItem(NULL,               (u8 *)"电压校准      ", NULL,(u8 *)(&pCalibrationVMenu),NEXT_SCREEN);//
					SetScreenItem(NULL,               (u8 *)"抄表日        %D日", &pdisDisPlayData[0],NULL,MODIFI_PARA); 
					SetScreenItem(NULL,               (u8 *)"禁用蜂鸣器    %l",  OnOffLis,         &pdisDisPlayData[6],MODIFI_PARA);
					
					SetScreenItem(NULL,               (u8 *)"在线相位      %l",  PHlist,           &pdisDisPlayData[20] ,MODIFI_PARA);//空调交流相位
					SetScreenItem(NULL,               (u8 *)"通信地址      %D",  &pdisDisPlayData[1],NULL,MODIFI_PARA);			
					SetScreenItem(NULL,               (u8 *)"模块限流点    %d%", &pdisDisPlayData[2],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,               (u8 *)"电池在线检测  %l",  OnOffLis,         &pdisDisPlayData[8],MODIFI_PARA);	
					SetScreenItem(NULL,               (u8 *)"锂电类型      %l",  BattLis,          &pdisDisPlayData[18],MODIFI_PARA);	
					SetScreenItem(NULL,               (u8 *)"模块防盗      %l",  OnOffLis,         &pdisDisPlayData[17],MODIFI_PARA);	
					
				if(showSpecilMenu==1)
				{	
					SetScreenItem(NULL,               (u8 *)"关停电延迟    %l",  OnOffLis,         &pdisDisPlayData[7]     ,MODIFI_PARA);
					SetScreenItem(NULL,               (u8 *)"开机限流      %l",  OnOffLis,         &pdisDisPlayData[22]     ,MODIFI_PARA);
					SetScreenItem(NULL,               (u8 *)"峰关充电MOS   %l",  OnOffLis,         &pdisDisPlayData[21],MODIFI_PARA);	
					SetScreenItem(NULL,               (u8 *)"空开厂家      %l", SwitchFactory,     &pdisDisPlayData[4],MODIFI_PARA);			
					SetScreenItem(NULL,               (u8 *)"槽位次序      %l", JumpProtocol,      &pdisDisPlayData[5],MODIFI_PARA); 
					SetScreenItem(NULL,               (u8 *)"电能2         %l",  OnOffLis,         &pdisDisPlayData[19],MODIFI_PARA);	
					SetScreenItem(NULL,               (u8 *)"Mentech       %l",  OnOffLis,         &pdisDisPlayData[23],MODIFI_PARA);	
				}
					   


          SetScreenItem(NULL);//结束
					return 1; 
			  }
			  else 
				   return 0;
			
			}
			else if(ppItem==&pSYSINFO)//监控版本信息界面
			{
				  for(u8 i=0;i<22;i++)
				{
					pdisDisPlayData[i]=0;
				}
				
				
				sprintf((char *)pdisDisPlayData,( const char *)"MAC:%x-%x-%x-%x-%x-%x",gMAC[0],gMAC[1],gMAC[2],gMAC[3],gMAC[4],gMAC[5]);
				
				 pSYSINFO=pdisPlayItem;
				if(pSYSINFO!=NULL)
				{
				   				
			    SetScreenItem(pSYSINFO,           (u8 *)"硬件版本 V1.0", NULL,NULL,NONE_DO);
					SetScreenItem(NULL,               (u8 *)"软件版本 V%F.2", (u8*)&VerNB,NULL,NONE_DO|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,               (u8 *)"字库版本 V%O.%O",(u8*)0x8040002,NULL,NONE_DO);
					SetScreenItem(NULL,               (u8 *)"C/SN:%l",              SNList,         &_sn,                      0); 
          SetScreenItem(NULL,               (u8 *)"S/SN:%l",              SSNList,         &_sn,                      0);
 					SetScreenItem(NULL,               pdisDisPlayData,              0,         0,                      0); 
//          SetScreenItem(NULL,               (u8 *)"P/SN:%l",              CuList,         &_sn,                      0);
					SetScreenItem(NULL);//结束
					return 1; 
			  }
			  else 
				   return 0;
			
			}
			
			else if(ppItem==&pSavePowrSetMenu)//节能界面
			{
				
				 pSavePowrSetMenu=pdisPlayItem;
				if(pSavePowrSetMenu!=NULL)
				{
				   				
					SetScreenItem(pSavePowrSetMenu,(u8 *)"休眠         ", NULL,(u8*)(&pSleepMenu),NEXT_SCREEN);//
					SetScreenItem(NULL,            (u8 *)"错峰用电     ", NULL,(u8*)(&pPeakShiftMenu),NEXT_SCREEN);//
					SetScreenItem(NULL,            (u8 *)"市电削峰     ", NULL,(u8*)(&pPeakShaveMenu),NEXT_SCREEN);//
					SetScreenItem(NULL);//结束//14
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pALARMMenu)//告警界面
			{
//				   u8 *para8=(u8 *)tmpPara;
				    pdisDisPlayData[0]=0;
				    alarmCount[1]= alarmCount[0];
				   pALARMMenu=pdisPlayItem;
				   if(pALARMMenu!=NULL)
					 {
						  SetScreenItem(pALARMMenu,(u8 *)"告警数 :%O/$O"  ,&alarmCount[1],NULL,MODIFI_PARA);
						  SetScreenItem(NULL, (u8 *)" %l%l%l"   ,AlarmMunklis,alarmdisplay,0);
						  SetScreenItem(NULL, (u8 *)"%l"   ,RemoveList,&pdisDisPlayData[0],MODIFI_PARA);
		          SetScreenItem(NULL);//结束
						  SetScreenItemSelectFirst(pALARMMenu);//设置为选择翻页
					 	 return 1;
			     }
			     else 
				   return 0;
				  
			}	
			 else if(ppItem==&pDCDMenu)//直流配电界面（主界面）
		  {
                 
				   pdisDisPlayData[0]=gcourrentWorkStatus;
				   pdisDisPlayData[1]=ACRunMode;                  
				   pdisDisPlayData[2]=gPeakShaveOnoff;//削峰
				   pdisDisPlayData[3]=gChargeStatus;
				   pdisDisPlayData[4]=gsleepSucceedFlag;//休眠
				  *((u16*)&pdisDisPlayData[13])=0;
				 pDCDMenu=pdisPlayItem;
			 if(pDCDMenu!=NULL)
			 {
		
				  SetScreenItem(pDCDMenu,(u8 *)"%l  %l %l %l %l",      Worklis, pdisDisPlayData,NONE_DO); 
					SetScreenItem(NULL,   (u8 *)"直流电压    %F.2V",    &gpSysData[DCVOLTAGE], 0,SCREEN_TWO_BYTE);
					SetScreenItem(NULL, (u8 *)"负载电流    %F.1A", &gpSysData[USER_CURR], 0, SCREEN_TWO_BYTE);
  			  SetScreenItem(NULL,   (u8 *)"电池电流    %F.1A" ,   &totalBattI,0,SCREEN_TWO_BYTE);//BATT_SOC				
				  SetScreenItem(NULL,   (u8 *)"电池SOC     %F.2%",     &totalSOC,0,SCREEN_FOUR_BYTE);//
					SetScreenItem(NULL,   (u8 *)"电池剩余容量 %F.2Ah",     &remainCapSum,0,SCREEN_FOUR_BYTE);//
					SetScreenItem(NULL,   (u8 *)"电池续航 %dmin",		  &battRuntime,0,SCREEN_TWO_BYTE);//gChargeStatus;	
				 //					SetScreenItem(NULL,   (u8 *)"总电流      %F.1A",    &gpSysData[LOADCURR],  0,SCREEN_TWO_BYTE);


//				  SetScreenItem(NULL,   (u8 *)"备电电流    %F.1A" ,   &gpSysData[TOTAL_BATTI],0,SCREEN_TWO_BYTE);//BATT_SOC
//				 	SetScreenItem(NULL,   (u8 *)"备电SOC     %F.2%",     &gpSysData[BATT_SOC],0,SCREEN_TWO_BYTE);//

//  			  SetScreenItem(NULL,   (u8 *)"电池温度    %F.2℃",		  &gpSysData[ENV_TEMP],0,SCREEN_TWO_BYTE);//gChargeStatus;	

//				  SetScreenItem(NULL,   (u8 *)"调节电压   %G.3V",		  &gModuleVoltage,0,SCREEN_TWO_BYTE);
//				  SetScreenItem(NULL,   (u8 *)"电芯电压   %F.2V",		  &vagBatVolt,0,SCREEN_TWO_BYTE);//currdiscap
//				  SetScreenItem(NULL,   (u8 *)"电芯放电量   %G.0AH",		  &currdiscap,0,SCREEN_TWO_BYTE);
//				  SetScreenItem(NULL,   (u8 *)"设定放电量   %G.0AH",		  (u8*)&currLimitCap,0,SCREEN_TWO_BYTE);
//				  SetScreenItem(NULL,   (u8 *)"总放电量:%G.0AH",		  (u8*)&totalBattdisCap,0,SCREEN_FOUR_BYTE);//u32 prtotalBattdisCap = 0;
				     //SetScreenItem(NULL, (u8 *)"前总放电量   %G.0",		  (u8*)&prtotalBattdisCap,0,SCREEN_FOUR_BYTE);
					SetScreenItem(NULL);//结束
				 return 1;
			 }
			 else 
				 return 0;
		 }
			else if(ppItem==&pSMRMenu)//整流模块界面
			{
				  pSMRMenu=pdisPlayItem;
				  pdisDisPlayData[13]=0;//开关机电动标志，由第6条菜单触发
				for(u8 i=0;i<22;i++)
				{
					pdisDisPlayData[i]=0;

				}
				pdisDisPlayData[1]=2;
				pdisDisPlayData[30]=glockDelayTime;
				   if(pSMRMenu!=NULL)
			 {


				  SetScreenItem(pSMRMenu,(u8 *)"当前模块        %O/$O",  &ModuleOnlineMessage[4],0,MODIFI_PARA);
          SetScreenItem(NULL,   (u8 *)"工作状态  %l/%l",      Workstatuslist,&pdisDisPlayData[0],NONE_DO);	//0 1	
          SetScreenItem(NULL,   (u8 *)"输出 %F.2V  %F.2A", (u8*)&pdisDisPlayData[2],  NULL,SCREEN_TWO_BYTE);//2 3 4 5	
				  SetScreenItem(NULL,   (u8 *)"输入 %F.1V  %F.1Hz", (u8*)&pdisDisPlayData[6],  NULL,SCREEN_TWO_BYTE);// 6 7 8 9	2022-12-2
				  SetScreenItem(NULL,   (u8 *)"温度     %F.2℃",		    (u8*)&pdisDisPlayData[10],0,SCREEN_TWO_BYTE);		
          SetScreenItem(NULL,   (u8 *)" %l",           SwitchModule,&pdisDisPlayData[12],MODIFI_PARA);	//0 1
         if(genableTP==1)//启用防盗功能时，才显才解绑口令
				 { 
					 SetScreenItem(NULL,   (u8 *)"M/SN:%l"                  ,MSNList,NULL,NONE_DO);
           SetScreenItem(NULL,   (u8 *)"解绑状态:%l" 	            ,ModuletPList,&pdisDisPlayData[31],NONE_DO);				 
				   SetScreenItem(NULL,   (u8 *)"解绑口令:%l%l%l%l%l%l%l%l",           Cmdlist,&pdisDisPlayData[14],MODIFI_PARA);	
					 SetScreenItem(NULL,   (u8 *)"锁定延时 %O小时"               ,(u8*)&pdisDisPlayData[30],NULL,MODIFI_PARA);	
				 }//
					SetScreenItem(NULL);//结束
				 SetScreenItemSelectFirst(pSMRMenu);//设置为选择翻页
			 	 return 1;
			 }
			  else 
				{
				 return 0;
				}
		  }
			else if(ppItem==&pSCRMenu)//光伏模块界面
			{
				  pSCRMenu=pdisPlayItem;
				  pdisDisPlayData[13]=0;//开关机电动标志，由第6条菜单触发
				for(u8 i=0;i<12;i++)
				{
					pdisDisPlayData[i]=0;

				}
				pdisDisPlayData[1]=2;
				   if(pSCRMenu!=NULL)
			 {


				  SetScreenItem(pSCRMenu,(u8 *)"当前模块        %O/$O",  &ModuleOnlineMessage[4],0,MODIFI_PARA);
          SetScreenItem(NULL,   (u8 *)"工作状态  %l/%l",      Workstatuslist,&pdisDisPlayData[0],NONE_DO);	//0 1	
          SetScreenItem(NULL,   (u8 *)"输出 %F.2V  %F.2A", (u8*)&pdisDisPlayData[2],  NULL,SCREEN_TWO_BYTE);//2 3 4 5	
				  SetScreenItem(NULL,   (u8 *)"输入 %F.1V  %F.2A", (u8*)&pdisDisPlayData[6],  NULL,SCREEN_TWO_BYTE);// 6 7 8 9	
				  SetScreenItem(NULL,   (u8 *)"温度     %F.2℃",		    (u8*)&pdisDisPlayData[10],0,SCREEN_TWO_BYTE);		
          SetScreenItem(NULL,   (u8 *)" %l",           SwitchModule,&pdisDisPlayData[12],MODIFI_PARA);	//0 1					 
					SetScreenItem(NULL);//结束
				 SetScreenItemSelectFirst(pSCRMenu);//设置为选择翻页
			 	 return 1;
			 }
			  else 
				{
				 return 0;
				}
		  }
			 
			
			
			
			
			else if(ppItem==&pACDMenu)//交流配电界面
		  {    
				

				 pACDMenu=pdisPlayItem;
			 if(pACDMenu!=NULL)
			 {
                  if((gAcPhase&0x04)==0x04)
									{
										SetScreenItem(pACDMenu,(u8 *)"A相电压   %F.2V",    &pgACmointor->st_Ua,NULL,SCREEN_TWO_BYTE);
									  SetScreenItem(NULL,   (u8 *)"A相电流   %F.2A",     &pgACmointor->st_Ia,NULL,SCREEN_TWO_BYTE);
										SetScreenItem(NULL,   (u8 *)"A相功率   %F.2KW",    &pgACmointor->st_Pa,NULL,SCREEN_TWO_BYTE);
									}
									else
									{
									 SetScreenItem(pACDMenu,(u8 *)"A相电压   ------V",     NULL,NULL,NONE_DO);
										SetScreenItem(NULL,   (u8 *)"A相电流   ------A",     NULL,NULL,NONE_DO);
										SetScreenItem(NULL,   (u8 *)"A相功率   ------KW",    NULL,NULL,NONE_DO);
									}
									
									
									 if((gAcPhase&0x02)==0x02)
									{
										SetScreenItem(NULL,   (u8 *)"B相电压   %F.2V",     &pgACmointor->st_Ub,NULL,SCREEN_TWO_BYTE);
									  SetScreenItem(NULL,   (u8 *)"B相电流   %F.2A",     &pgACmointor->st_Ib,NULL,SCREEN_TWO_BYTE);
										SetScreenItem(NULL,   (u8 *)"B相功率   %F.2KW",    &pgACmointor->st_Pb,NULL,SCREEN_TWO_BYTE);
									}
									else
									{
									  SetScreenItem(NULL,   (u8 *)"B相电压   ------V",   NULL,NULL,NONE_DO);
										SetScreenItem(NULL,   (u8 *)"B相电流   ------A",   NULL,NULL,NONE_DO);
										SetScreenItem(NULL,   (u8 *)"B相功率   ------KW",  NULL,NULL,NONE_DO);
									}
									
									 if((gAcPhase&0x01)==0x01)
									{
										SetScreenItem(NULL,   (u8 *)"C相电压   %F.2V",     &pgACmointor->st_Uc,NULL,SCREEN_TWO_BYTE);
									  SetScreenItem(NULL,   (u8 *)"C相电流   %F.2A",     &pgACmointor->st_Ic,NULL,SCREEN_TWO_BYTE);
										SetScreenItem(NULL,   (u8 *)"C相功率   %F.2KW",    &pgACmointor->st_Pc,NULL,SCREEN_TWO_BYTE);
									}
									else
									{
									  SetScreenItem(NULL,   (u8 *)"C相电压   ------V",   NULL,NULL,NONE_DO);
										SetScreenItem(NULL,   (u8 *)"C相电流   ------A",   NULL,NULL,NONE_DO);
										SetScreenItem(NULL,   (u8 *)"C相功率   ------KW",  NULL,NULL,NONE_DO);
									}
									
									
						    	 SetScreenItem(NULL,   (u8 *)"频率   %F.1Hz",    &pgACmointor->st_Frq,NULL,SCREEN_TWO_BYTE); 
							 
							 
							  if(gWorkShiftMode==0)
								{
	
									   SetScreenItem(NULL,   (u8 *)"电能:%F.1KWH",   &pgACmointor->st_totalenergy,NULL,SCREEN_FOUR_BYTE);//errCount
									   SetScreenItem(NULL,   (u8 *)"电能:%G.3KWH",   &gTotalMrEnergy,NULL,SCREEN_FOUR_BYTE);
								}
								else
								{
									 SetScreenItem(NULL,   (u8 *)"错峰电能",        NULL,NULL,NONE_DO);// totalEnergy[6]
									 SetScreenItem(NULL,   (u8 *)"尖峰:%F.1KWH",   &totalEnergy[SpikeMode],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"高峰:%F.1KWH",   &totalEnergy[PeakMode],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"平段:%F.1KWH",   &totalEnergy[NormalMode],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"低谷:%F.1KWH",   &totalEnergy[TroughMode],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"总共:%F.1KWH",   &totalEnergy[5],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"其它:%F.1KWH",   &totalEnergy[0],NULL,SCREEN_FOUR_BYTE);
								}
							 
               SetScreenItem(NULL);//结束     
				 return 1;
			 }
			  else 
				 return 0;
			 
		 }
			 else if(ppItem==&pUSRMenu)//用智能空开信息界面
			{
				
				
				 *((u16 *)&pdisDisPlayData[0])=0;
				 pdisDisPlayData[2]=1;//选择分路
				 pdisDisPlayData[3]=0;// 分路总数

				
				 for( u8 i=0;i<TOTAL_USER;i++)
										 {
													if(SwitchOnlineCount[i]>0)//查找已出现过的用户分路
													{
														  pdisDisPlayData[3]++;
														
													}
										 }
				
					*((u16 *)&pdisDisPlayData[4])=0;//gDCdistribution.pst_I[pdisDisPlayData[2]-1];
					*((u32 *)&pdisDisPlayData[6])=0;//gDCdistribution.pst_enerqy[pdisDisPlayData[2]-1];
					*((u16 *)&pdisDisPlayData[10])=0;
					*((u32 *)&pdisDisPlayData[12])=0;
					
				
				
				pUSRMenu=pdisPlayItem;
				   if(pUSRMenu!=NULL)
			 {
				 
				 
				 SetScreenItem(pUSRMenu,   (u8 *)"配电分路     %O/$D",             &pdisDisPlayData[2],0,MODIFI_PARA);
				    SetScreenItem(NULL,    (u8 *)"电流  %F.1A",     &pdisDisPlayData[4],0,SCREEN_TWO_BYTE); 	
						SetScreenItem(NULL,    (u8 *)"电量  %G.2KWH",   &pdisDisPlayData[6],0,SCREEN_FOUR_BYTE); 
						SetScreenItem(NULL,    (u8 *)"电压  %F.2V",     &pdisDisPlayData[10],0,SCREEN_TWO_BYTE); 
				    SetScreenItem(NULL,    (u8 *)"功率  %F.2W",     &pdisDisPlayData[12],0,SCREEN_FOUR_BYTE); 
				 
				 	SetScreenItem(NULL);
				 SetScreenItemSelectFirst(pUSRMenu);
				 		 return 1;
			 }
			  else 
				 return 0;
			 }//pUSR2Menu
			 else if(ppItem==&pUSR2Menu)//租户界面
			{
				
				

				 pdisDisPlayData[2]=1;//选择分路
				 pdisDisPlayData[3]=6;// 分路总数


					*((u16 *)&pdisDisPlayData[4])=0;
					*((u32 *)&pdisDisPlayData[6])=0;
				  *((u32 *)&pdisDisPlayData[10])=0;
				

				pUSR2Menu=pdisPlayItem;
				   if(pUSR2Menu!=NULL)
			 {
				 
				 
				    //SetScreenItem(pUSR2Menu,   (u8 *)"用户分路      %O/$D",             &pdisDisPlayData[2],0,MODIFI_PARA);
				    SetScreenItem(pUSR2Menu,   (u8 *)"租户  %l" ,      UserListMenu2 ,  &pdisDisPlayData[2],MODIFI_PARA);
				    SetScreenItem(NULL,        (u8 *)"电流  %F.1A",     &pdisDisPlayData[4],0,SCREEN_TWO_BYTE); 	
						SetScreenItem(NULL,        (u8 *)"电量  %G.2KWH",   &pdisDisPlayData[6],0,SCREEN_FOUR_BYTE); 
					  SetScreenItem(NULL,        (u8 *)"百分比 %F.1%",       &pdisDisPlayData[0],0,SCREEN_TWO_BYTE); 
            SetScreenItem(NULL,        (u8 *)"电量2  %G.1KWH",   &pdisDisPlayData[10],0,SCREEN_FOUR_BYTE); //20221229
				 	SetScreenItem(NULL);
				 SetScreenItemSelectFirst(pUSR2Menu);
				 		 return 1;
			 }
			  else 
				 return 0;
			 }//pUSR2Menu
			 else if(ppItem==&pUSR3Menu)//用户单元界面
			{

				 pdisDisPlayData[0]=1;//选择分路
				 pdisDisPlayData[1]=4;// 分路总数
			
				pUSR3Menu=pdisPlayItem;
				   if(pUSR3Menu!=NULL)
			 {
				 
				    SetScreenItem(pUSR3Menu,   (u8 *)"用户分路      %O/$D",&pdisDisPlayData[0],0,MODIFI_PARA);
						SetScreenItem(NULL,        (u8 *)"电压  %F.1V",     &pdisDisPlayData[2],0,SCREEN_TWO_BYTE); 	
				    SetScreenItem(NULL,        (u8 *)"电流  %F.1A",     &pdisDisPlayData[4],0,SCREEN_TWO_BYTE); 	
						SetScreenItem(NULL,        (u8 *)"电量  %G.3KWH",   &pdisDisPlayData[6],0,SCREEN_FOUR_BYTE); 

				 	SetScreenItem(NULL);
				 		 return 1;
			 }
			  else 
				 return 0;
			 }//pUSR2Menu
			 
			 
			else if(ppItem==&pBATTERYData)//锂电池参数界面
			{
			
				battID[0]=1;
				pBATTERYData=pdisPlayItem;
				
				if(pBATTERYData!=NULL)
				{
					
						SetScreenItem(pBATTERYData,(u8 *)"当前电池        %O/$O",  &battID,0,MODIFI_PARA);
//												SetScreenItem(NULL,(u8 *)"总线电压    %F.2V",	 &pdisDisPlayData[0],  NULL,SCREEN_TWO_BYTE);
												SetScreenItem(NULL,(u8 *)"电池电压    %F.2V",	 &pdisDisPlayData[2],  NULL,SCREEN_TWO_BYTE);
												SetScreenItem(NULL,(u8 *)"电池电流    %F.2A",	 &pdisDisPlayData[4],  NULL,SCREEN_TWO_BYTE);
//					              SetScreenItem(NULL,(u8 *)"Ibat        %F.2A",	 &pdisDisPlayData[32],  NULL,SCREEN_TWO_BYTE);
												SetScreenItem(NULL,(u8 *)"充放状态    %l"  , StatuLis,&pdisDisPlayData[26],MODIFI_PARA);
												SetScreenItem(NULL,(u8 *)"电池温度    %d℃",    &pdisDisPlayData[8],  (u8 *)(&pBattTmpMenu),NEXT_SCREEN|SCREEN_TWO_BYTE);			
												SetScreenItem(NULL,(u8 *)"电池芯数    %D个",  &pdisDisPlayData[10], NULL,MODIFI_PARA);				
												SetScreenItem(NULL,(u8 *)"SOC         %F.2%",    &pdisDisPlayData[12], NULL,SCREEN_TWO_BYTE);		
												SetScreenItem(NULL,(u8 *)"SOH         %F.2%",	  &pdisDisPlayData[14],  NULL,SCREEN_TWO_BYTE);
					              SetScreenItem(NULL,(u8 *)"单体电压 %F.3-%F.3V",	 &pdisDisPlayData[28],  (u8 *)(&pBattCellVMenu),NEXT_SCREEN|SCREEN_TWO_BYTE);
												SetScreenItem(NULL,(u8 *)"额定容量    %F.2AH",   &pdisDisPlayData[16],  NULL,SCREEN_TWO_BYTE);	
												SetScreenItem(NULL,(u8 *)"MOS充闭合   %l",OnOffLis,&pdisDisPlayData[18],MODIFI_PARA);
												SetScreenItem(NULL,(u8 *)"MOS放闭合   %l",OnOffLis,&pdisDisPlayData[20],MODIFI_PARA);
												SetScreenItem(NULL,(u8 *)"放电电流    %F.2A",&pdisDisPlayData[22],NULL,SCREEN_TWO_BYTE);
												SetScreenItem(NULL,(u8 *)"电池循环次数    %d",&pdisDisPlayData[34],NULL,SCREEN_TWO_BYTE);

				//	SetScreenItem(NULL,(u8 *)"剩余时间      %dMIN",    &pdisDisPlayData[24], NULL,SCREEN_TWO_BYTE);	
					//SetScreenItem(NULL,(u8 *)"充放状态         %D"  , &pdisDisPlayData[6],  NULL,MODIFI_PARA);						
						
					SetScreenItem(NULL);//结束
					 SetScreenItemSelectFirst(pBATTERYData);//设置为选择翻页
				return 1;
				}
				else
				{
					return 0;
				}
			}
	/*		
			 else if(ppItem==&pPOWERRATEMenu)//电费显示界面{
		  {
			 

			  pPOWERRATEMenu=pdisPlayItem;
			
			    //单位转换，从0.001分转为 1分 以0.01元显示
			    *((u32 *)&pdisDisPlayData[0])=powerRate[0]/1000;
			    *((u32 *)&pdisDisPlayData[4])=powerRate[1]/1000;
			    *((u32 *)&pdisDisPlayData[8])=powerRate[2]/1000;
		    	*((u32 *)&pdisDisPlayData[12])=powerRate[3]/1000;
			    *((u32 *)&pdisDisPlayData[16])=powerRate[4]/1000;
			    *((u32 *)&pdisDisPlayData[20])=powerRate[5]/1000;
			
			
			if(pPOWERRATEMenu!=NULL)
			 {
			    
         
				 SetScreenItem(pPOWERRATEMenu,   (u8 *)"常态电费: %F.2元",    &pdisDisPlayData[0],0,SCREEN_FOUR_BYTE);
         SetScreenItem(NULL,             (u8 *)"尖峰电费: %F.2元",    &pdisDisPlayData[4],0,SCREEN_FOUR_BYTE);
				 SetScreenItem(NULL,             (u8 *)"高峰电费: %F.2元",    &pdisDisPlayData[8],0,SCREEN_FOUR_BYTE);
				 SetScreenItem(NULL,             (u8 *)"平峰电费: %F.2元",    &pdisDisPlayData[12],0,SCREEN_FOUR_BYTE);
				 SetScreenItem(NULL,             (u8 *)"低峰电费: %F.2元",    &pdisDisPlayData[16],0,SCREEN_FOUR_BYTE);
				 SetScreenItem(NULL,             (u8 *)"谷平峰费: %F.2元",    &pdisDisPlayData[20],0,SCREEN_FOUR_BYTE);
         	SetScreenItem(NULL);
				 return 1;
			 }
			 else
				  return 0;
			
			
		}
	*/		
			
			else if(ppItem==&pBattTmpMenu)//锂电单体电温度界面
		  {
			 

			  pBattTmpMenu=pdisPlayItem;
			
			   
			
			
			if(pBattTmpMenu!=NULL)
			 {
			    
         
				 SetScreenItem(pBattTmpMenu,   (u8 *)"单体温度 4个",   NULL,NULL,NONE_DO);
				 for(u8 i=0 ;i<4;i++)
				 {
					  pdisDisPlayData[i] =i+1;
				    SetScreenItem(NULL,             (u8 *)"单体温度    %D",  &pdisDisPlayData[i],NULL,NONE_DO);
            SetScreenItem(NULL,             (u8 *)"%d℃",  (u8 *)&batt[battID[0]-1].CellTemp[i],NULL,SCREEN_TWO_BYTE);
				 }
				 
         	SetScreenItem(NULL);
				 return 1;
			 }
			 else
				  return 0;
			
			
		}
			else if(ppItem==&pBattCellVMenu)//锂电单体电压界面
		  {
			 

			  pBattCellVMenu=pdisPlayItem;
			
			   
			
			
			if(pBattCellVMenu!=NULL)
			 {
			    
         
				 SetScreenItem(pBattCellVMenu,   (u8 *)"单体电压 %D个",   (u8 *)&batt[battID[0]-1].Cell_Num,NULL,NONE_DO);
				 for(u8 i=0 ;i<batt[battID[0]-1].Cell_Num;i++)
				 {
					  pdisDisPlayData[i] =i+1;
				    SetScreenItem(NULL,             (u8 *)"单体电压    %D",  &pdisDisPlayData[i],NULL,NONE_DO);
            SetScreenItem(NULL,             (u8 *)"%F.3V",  (u8 *)&batt[battID[0]-1].CellV[i],NULL,SCREEN_TWO_BYTE);
				 }
				 
         	SetScreenItem(NULL);
				 return 1;
			 }
			 else
				  return 0;
			
			
		}
	
		  else if(ppItem==&pBattParaSetMenu)//电池参数界面
			{


				
				
				
			   u16 *para16=(u16 *)pdisDisPlayData;
				 para16[0]=gSetfloatV;
				 para16[1]=gSetequalV;
				 para16[2]=gtmpFactor16;
			   para16[3]=gSetBattC;
         para16[4]=gSetBattLimitPerC;
				
				 u8 *para8=(u8 *)&para16[5];
				 para8[0]=pMonitor->m_workMode;
				 para8[1]=gtmpCompEn;
				 para8[2]=battGroud;
				 para8[3]=gbattGroud; 
				
				
				 para16[7]=gbranchBatt[0];
				 para16[8]=gbranchBatt[1];
				 para16[9]=gbranchBatt[2];
				 para16[10]=gbranchBatt[3];
				 para16[11]=gbranchBatt[4];
				 para16[12]=gbranchBatt[5];

				 para16[13] = gOcrdiffV;
			
		
				
				 pBattParaSetMenu=pdisPlayItem;
				if(pBattParaSetMenu!=NULL)
				{
				   
					SetScreenItem(pBattParaSetMenu,(u8 *)"手动工作模式 %l ",ChangeModList,&para8[0],MODIFI_PARA);//
					SetScreenItem(NULL,           (u8 *)"浮充电压      %F.2V",& para16[0],(u8*)&gSetfloatV,MODIFI_PARA|SCREEN_TWO_BYTE);//
					SetScreenItem(NULL,           (u8 *)"均充电压      %F.2V",& para16[1],(u8*)&gSetequalV,MODIFI_PARA|SCREEN_TWO_BYTE);//
					SetScreenItem(NULL,           (u8 *)"光伏电压差    %F.3V",& para16[13],(u8*)&gOcrdiffV,MODIFI_PARA|SCREEN_TWO_BYTE);//
					SetScreenItem(NULL,           (u8 *)"温度补偿使能  %l",   OnOffLis,  &para8[1],MODIFI_PARA|SCREEN_TWO_BYTE);
         	SetScreenItem(NULL,           (u8 *)"温补系数      %dmV/℃",& para16[2],(u8*)&gtmpFactor16,MODIFI_PARA|SCREEN_TWO_BYTE);
					
				
					SetScreenItem(NULL            ,(u8 *)"总电池限流   %F.2C",&para16[4],(u8 *)&gSetBattLimitPerC,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,            (u8 *)"总电池容量   %dAh" ,&para16[3],(u8 *)&gSetBattC,NONE_DO);
					if( gbattGroud >0 )
					{
					  SetScreenItem(NULL,            (u8 *)"空开电池分路数  %D"   ,&para8[3] , NULL,NONE_DO);
					}
					else if( gbattGroud == 0 )
					{
							SetScreenItem(NULL,            (u8 *)"铅酸单元电池数  %D"   ,&para8[2] , &battGroud,MODIFI_PARA);					
							if(battGroud>0)
							{
								for(u8 i=0;i<battGroud;i++)
								SetScreenItem(NULL,          (u8 *)"铅酸电池容量  %dAh" ,&para16[7+i],(u8 *)&gbranchBatt[i],MODIFI_PARA); 
					
							}	
				  }
					SetScreenItem(NULL);//结束//14
					return 1;
			  }
			  else 
				   return 0;
			}
		 
		 
		  else if(ppItem==&pBattTestSetMenu)//电池测试界面
			{
				
				

				    pdisDisPlayData[0]=gtestPeriodEnable;
				    pdisDisPlayData[1]=gtestPeriod;
				    pdisDisPlayData[2]=gtestTimeout;
				    (*(u16 *)(&pdisDisPlayData[3]))=gtestV;
				
				
				 pBattTestSetMenu=pdisPlayItem;
				if(pBattTestSetMenu!=NULL)
				{
				   				
					SetScreenItem(pBattTestSetMenu,       (u8 *)"周期放电测试   %l ", OnOffLis,&pdisDisPlayData[0],MODIFI_PARA);//
					SetScreenItem(NULL,                   (u8 *)"测试周期      %D天",    & pdisDisPlayData[1],NULL,MODIFI_PARA);//
					SetScreenItem(NULL,                   (u8 *)"测试时长     %D小时",   & pdisDisPlayData[2],NULL,MODIFI_PARA);//
					SetScreenItem(NULL,                   (u8 *)"测试结束电压 %F.2V",   & pdisDisPlayData[3],(u8*)&gtestV,MODIFI_PARA|SCREEN_TWO_BYTE);//	
					SetScreenItem(NULL);//结束//14
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pBattBootSetMenu)//电池均充界面
			{
				
						

				   
				     pdisDisPlayData[0]=gAutoPeriod;
				     pdisDisPlayData[1]=gstartdelay;
				     //pdisDisPlayData[2]=gEndDelay;
				
				    (*(u16 *)&pdisDisPlayData[3])=gSetStartBRI;
				    (*(u16 *)(&pdisDisPlayData[5]))=gSetEndBRI;
				    (*(u16 *)&pdisDisPlayData[12])=gEndDelay;
				      pdisDisPlayData[7]=gequalTimeMax;
				     pdisDisPlayData[9]=gAutoBootEnable;
		         pdisDisPlayData[10]=gbootBattCap;
				  pBattBootSetMenu=pdisPlayItem;
				if(pBattBootSetMenu!=NULL)
				{
				 
					
				  SetScreenItem(pBattBootSetMenu,(u8 *)"自动均充     %l ", OnOffLis,&pdisDisPlayData[9],MODIFI_PARA);//遥控//19
					SetScreenItem(NULL,            (u8 *)"均充周期     %D天",&pdisDisPlayData[0],(u8 *)&gAutoPeriod,MODIFI_PARA);
					SetScreenItem(NULL,            (u8 *)"均充时长     %D小时",& pdisDisPlayData[7],NULL,MODIFI_PARA);
					SetScreenItem(NULL,            (u8 *)"最小放电时长 %D分",  & pdisDisPlayData[10],NULL,MODIFI_PARA);
					SetScreenItem(NULL,            (u8 *)"均充启动系数 %F.2C" ,& pdisDisPlayData[3],(u8 *)&gSetStartBRI,  MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,            (u8 *)"均充结束系数 %F.2C", & pdisDisPlayData[5],(u8 *)&gSetEndBRI,  MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,            (u8 *)"均充启动延时 %D分",& pdisDisPlayData[1],(u8 *)&gstartdelay,MODIFI_PARA);
					SetScreenItem(NULL,            (u8 *)"均充结束延时 %d分",& pdisDisPlayData[12],(u8 *)&gEndDelay,MODIFI_PARA);			
					SetScreenItem(NULL);//结束//14
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pAlarmParaSetMenu)//告警参数设置界面
			{
								

				(*(u16 *)&pdisDisPlayData[0])=gInOwrVLimit;
				(*(u16 *)&pdisDisPlayData[2])=gInOverVLimit;      
				(*(u16 *)&pdisDisPlayData[4])=gInOverILimit;		 
        (*(u16 *)&pdisDisPlayData[6])=gOutOwrVLimit;
				(*(u16 *)&pdisDisPlayData[8])=gOutOverVLimit;
				(*(u16 *)&pdisDisPlayData[10])=gOverFLimit;
				(*(u16 *)&pdisDisPlayData[12])=gOwrFLimit;
				(*(u16 *)&pdisDisPlayData[14])=ghtemp;
				(*(u16 *)&pdisDisPlayData[16])=gltemp;
				(*(u16 *)&pdisDisPlayData[18])=goutPutOwrV;
			 pAlarmParaSetMenu=pdisPlayItem;
				if(pAlarmParaSetMenu!=NULL)
				{
				   				
					SetScreenItem(pAlarmParaSetMenu ,   (u8 *)"输入欠压    %F.2V",&pdisDisPlayData[0],(u8*)&gInOwrVLimit,MODIFI_PARA|SCREEN_TWO_BYTE);
		      SetScreenItem(NULL,   (u8 *)"输入过压    %F.2V",&pdisDisPlayData[2],(u8*)&gInOverVLimit,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL,   (u8 *)"输入过流      %F.2A",&pdisDisPlayData[4],(u8*)&gInOverILimit,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"电池低压      %F.2V",&pdisDisPlayData[6],(u8*)&gOutOwrVLimit,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL,   (u8 *)"输出过压      %F.2V",&pdisDisPlayData[8],(u8*)&gOutOverVLimit,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"输出欠压      %F.2V",&pdisDisPlayData[18],(u8*)&goutPutOwrV,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"频率上限      %F.2Hz",&pdisDisPlayData[10],(u8*)&gOverFLimit,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL,   (u8 *)"频率下限      %F.2Hz",&pdisDisPlayData[12],(u8*)&gOwrFLimit,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"电池温度高  %F.2℃",&pdisDisPlayData[14],(u8*)&ghtemp,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL,   (u8 *)"电池温度低  %F.2℃",&pdisDisPlayData[16],(u8*)&gltemp,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			}
	/*		
			 else if(ppItem==&pBattDownParaSetMenu)//电池下电参数设置界面
			{

								

				      pdisDisPlayData[0]=gbattDownMod;
              *((u16 *)(&pdisDisPlayData[1]))=gdownV;
				       *((u16 *)(&pdisDisPlayData[3]))=gbattDownTime;
				        *((u16 *)(&pdisDisPlayData[5]))=recoverbat;
						  pBattDownParaSetMenu=pdisPlayItem;
				if(pBattDownParaSetMenu!=NULL)
				{
				   				
					SetScreenItem(pBattDownParaSetMenu ,   (u8 *)"%l",BattDownLis,&pdisDisPlayData[0],MODIFI_PARA);//gbattDownMod
		      SetScreenItem(NULL,   (u8 *)"下电电压  %F.2V" ,&pdisDisPlayData[1],(u8*)&gdownV,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"恢复电压  %F.2V" ,&pdisDisPlayData[5],(u8*)&recoverbat,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL,   (u8 *)"下电延迟  %d分钟",&pdisDisPlayData[3], (u8*)&gbattDownTime,MODIFI_PARA);
				  //SetScreenItem(NULL,   (u8 *)"下电计时  %d分钟",   &gbattDownCount,NULL,SCREEN_TWO_BYTE);
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			}
			*/
		   	else if(ppItem==&pUsrDownParaSetMenu)//用户下电参数设置界面
			{   
				  pdisDisPlayData[0]=1;
				  u16 *precoverV=&recoverV1;
				  pdisDisPlayData[1]=gDcParam0[pdisDisPlayData[0]+1].st_downModle; 
				 (*(u16 *)(&pdisDisPlayData[2]))=gDcParam0[pdisDisPlayData[0]+1].st_LDVoltage;
				 (*(u16 *)(&pdisDisPlayData[4]))=gDcParam1[pdisDisPlayData[0]+1].st_LDDelay;
          //(*(u16 *)(&pdisDisPlayData[13]))=recoverV1;
				 pdisDisPlayData[13] = 0;
//				  setBattParaflag=3;
					
				 pUsrDownParaSetMenu=pdisPlayItem;
				if(pUsrDownParaSetMenu!=NULL)
				{
				   				
					SetScreenItem(pUsrDownParaSetMenu ,   (u8 *)"用户分路    %D"      , &pdisDisPlayData[0],0,MODIFI_PARA);
												 	SetScreenItem(NULL,   (u8 *)"%l"                 , userDownlis, &pdisDisPlayData[1] ,MODIFI_PARA);
													SetScreenItem(NULL,   (u8 *)"下电电压    %F.2V"   , &pdisDisPlayData[2],0,SCREEN_TWO_BYTE|MODIFI_PARA);
													//SetScreenItem(NULL,   (u8 *)"恢复电压    %F.2V"   , &pdisDisPlayData[13],0,SCREEN_TWO_BYTE|MODIFI_PARA);
													SetScreenItem(NULL,   (u8 *)"下电时间    %F.1分钟",&pdisDisPlayData[4],0,SCREEN_TWO_BYTE|MODIFI_PARA);

									
					 SetScreenItemSelectFirst(pUsrDownParaSetMenu);//设置为选择翻页
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			
			}	
			else if(ppItem == &pUsrDownInfo)
			{
				  pUsrDownInfo = pdisPlayItem;
				

				   setBattParaflag = 5;
				   if(pUsrDownInfo!=NULL)
				{
				   				
					SetScreenItem(pUsrDownInfo ,   (u8 *)"软件版本  V%D.%D"      , &gdownbroadinfo[0],0,NULL);
				  	      SetScreenItem(NULL ,   (u8 *)"硬件版本  V%D.%D"      , &gdownbroadinfo[2],0,NULL);								 
															
					 
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pSetDIMenu)//DI设置页面
			{
 				        pdisDisPlayData[0]=gInPutAlarm[LocalInPutNb[0]-1].st_AlarmType;
  				      pdisDisPlayData[1]=gInPutAlarm[LocalInPutNb[0]-1].st_AlarmTrigger;
                pdisDisPlayData[2]=gInPutAlarm[LocalInPutNb[0]-1].st_InputNb;
                pdisDisPlayData[3]=gInPutAlarm[LocalInPutNb[0]-1].st_behavior%6;	
				   u16 diMask=1;
				       if((gDiInPutFlag&(diMask<<(LocalInPutNb[0]-1)))==0)
				             LocalInPutNb[1]=0;
							 else
				             LocalInPutNb[1]=1;
				   pSetDIMenu=pdisPlayItem;
				   if(pSetDIMenu!=NULL)
					 {
						 SetScreenItem(pSetDIMenu    , (u8 *)"DI:%l 告警:$l", AlarmStatuslist,LocalInPutNb,MODIFI_PARA);//注DI序号从1开始，到12
						    SetScreenItem(NULL       , (u8 *)"告警类型    %l", DIsetlist,&pdisDisPlayData[0],MODIFI_PARA);
						    SetScreenItem(NULL       , (u8 *)"触发型式    %l", DiTriggerlist,&pdisDisPlayData[1],MODIFI_PARA);
						    SetScreenItem(NULL       , (u8 *)"告警号  %l", ALarmNblist, &pdisDisPlayData[2],MODIFI_PARA);
						    //SetScreenItem(NULL       , (u8 *)"继电器 %l"        ,outAlarmList,&pdisDisPlayData[3],MODIFI_PARA);
				        SetScreenItem(NULL);//结束
						 SetScreenItemSelectFirst(pSetDIMenu);//设置为选择翻页
						return 1;
			  }
			  else 
				   return 0;
			}
			
			else if(ppItem==&pSetDOMenu)//DO设置页面
			{
				       pdisDisPlayData[0]=1;
               pdisDisPlayData[1]=gInPutAlarm[0].st_behavior;	
				
				   pSetDOMenu=pdisPlayItem;
				   if(pSetDOMenu!=NULL)
					 {
						 SetScreenItem(pSetDOMenu    , (u8 *)"继电器:%D "  , &pdisDisPlayData[0],0,MODIFI_PARA);//注DI序号从1开始，到12
		
						    SetScreenItem(NULL       , (u8 *)"%l"        ,outAlarmList,&pdisDisPlayData[1],MODIFI_PARA);
				        SetScreenItem(NULL);//结束
						 //SetScreenItemSelectFirst(pSetDIMenu);//设置为选择翻页
						return 1;
			  }
			  else 
				   return 0;
			}
			
			
			
			
			
			
			else if(ppItem==&pSleepMenu)//休眠参数界面  
			{
				

				      pdisDisPlayData[0]=gSleepOnoff;
				      pdisDisPlayData[1]=sleepWait;
				      pdisDisPlayData[2]=gsleepMinCount;
				      pdisDisPlayData[3]=gsleepLoadRate;
				      pdisDisPlayData[4]=gsleepWakeupLoadRate;
				      pdisDisPlayData[5]=gsleepTurnPeriod;
//							pdisDisPlayData[6]=gsleepBattTestTime;
				     *((u16 *)(&pdisDisPlayData[7]))=gsleepBattTestPeriod;
				
				  pSleepMenu=pdisPlayItem;
					if(pSleepMenu!=NULL)
				{
				   
          SetScreenItem(pSleepMenu,(u8 *)"使能          %l" , OnOffLis,&pdisDisPlayData[0],MODIFI_PARA);//	
          SetScreenItem(NULL,      (u8 *)"休眠暂停      %l"  ,OnOffLis,&pdisDisPlayData[1],MODIFI_PARA);						
					SetScreenItem(NULL,      (u8 *)"基数          %D" ,  &pdisDisPlayData[2],(u8*)&gsleepMinCount,MODIFI_PARA);
					SetScreenItem(NULL,      (u8 *)"休眠带载率    %D%" , &pdisDisPlayData[3],(u8*)&gsleepLoadRate,MODIFI_PARA);
					SetScreenItem(NULL,      (u8 *)"唤醒带载率    %D%" , &pdisDisPlayData[4],(u8*)&gsleepWakeupLoadRate,MODIFI_PARA);				
					SetScreenItem(NULL,      (u8 *)"轮换周期      %D小时" , &pdisDisPlayData[5],(u8*)&gsleepTurnPeriod,MODIFI_PARA);
					//SetScreenItem(NULL,      (u8 *)"测试时长      %D分" , &pdisDisPlayData[6],(u8*)&gsleepBattTestTime,MODIFI_PARA);
					SetScreenItem(NULL,      (u8 *)"测试周期      %d秒" , &pdisDisPlayData[7],(u8*)&gsleepBattTestPeriod,MODIFI_PARA);
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pPeakShiftMenu)//错峰界面
			{
							
				pdisDisPlayData[0]=gWorkShiftMode;
				*((u16 *)(&pdisDisPlayData[1]))=gsafedisV;
				*((u16 *)(&pdisDisPlayData[3]))=gdisDeep;
				*((u16 *)(&pdisDisPlayData[5]))=gBattSafeSoc;
				*((u16 *)(&pdisDisPlayData[7]))=gBattRecovSoc;

				*((u16 *)(&pdisDisPlayData[9]))=gPeakModleDisChargeVol;
				*((u16 *)(&pdisDisPlayData[11]))=gPeakModleChargeVol;
				
		   *((u16 *)(&pdisDisPlayData[13]))= eleRate[TroughMode];
			 *((u16 *)(&pdisDisPlayData[15]))= eleRate[PeakMode];
			 *((u16 *)(&pdisDisPlayData[17]))= eleRate[SpikeMode];	
			 *((u16 *)(&pdisDisPlayData[19]))= eleRate[NormalMode];
     	
				 
				  pPeakShiftMenu=pdisPlayItem;
				if(pPeakShiftMenu!=NULL)
				{
					
					

				   				
					SetScreenItem(pPeakShiftMenu,   (u8 *)"错峰使能   %l" ,OnOffLis, &pdisDisPlayData[0],MODIFI_PARA);
		      SetScreenItem(NULL,   (u8 *)"安全放电电压  %F.2V"     , &pdisDisPlayData[1],(u8*)&gsafedisV ,MODIFI_PARA|SCREEN_TWO_BYTE);
 //         SetScreenItem(NULL,   (u8 *)"电池放电深度  %F.2%"     , &pdisDisPlayData[3],(u8*)&gdisDeep,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"锂电保底容量  %F.2%"     , &pdisDisPlayData[5],(u8*)&gBattSafeSoc,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"锂电回充容量  %F.2%"     , &pdisDisPlayData[7],(u8*)&gBattRecovSoc,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"锂电放电电压  %F.2V"     , &pdisDisPlayData[9],(u8*)&gPeakModleDisChargeVol,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"锂电充电电压  %F.2V"     , &pdisDisPlayData[11],(u8*)&gPeakModleChargeVol,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"时段设置"                ,NULL,(u8*)(&pPeakTimeMenu),NEXT_SCREEN);
//					SetScreenItem(NULL   ,(u8 *)"低谷限流系数 %F.2C",& pdisDisPlayData[5],(u8 *)&gSetBattLimitPerCTrough,MODIFI_PARA|SCREEN_TWO_BYTE);
//					SetScreenItem(NULL   ,(u8 *)"高峰限流系数 %F.2C",& pdisDisPlayData[7],(u8 *)&gSetBattLimitPerCPeak,MODIFI_PARA|SCREEN_TWO_BYTE);
//					SetScreenItem(NULL   ,(u8 *)"尖峰限流系数 %F.2C",& pdisDisPlayData[9],(u8 *)&gSetBattLimitPerCSpike,MODIFI_PARA|SCREEN_TWO_BYTE);
//					SetScreenItem(NULL   ,(u8 *)"平段限流系数 %F.2C",& pdisDisPlayData[11],(u8 *)&gSetBattLimitPerCNormal,MODIFI_PARA|SCREEN_TWO_BYTE);  
					
//					SetScreenItem(NULL   ,(u8 *)"低谷电价 %G.4元/度",& pdisDisPlayData[13],(u8 *)&eleRate[TroughMode],MODIFI_PARA|SCREEN_TWO_BYTE); 
//					SetScreenItem(NULL   ,(u8 *)"高峰电价 %G.4元/度",& pdisDisPlayData[15],(u8 *)&eleRate[PeakMode],MODIFI_PARA|SCREEN_TWO_BYTE);
//          SetScreenItem(NULL   ,(u8 *)"尖峰电价 %G.4元/度",& pdisDisPlayData[17],(u8 *)&eleRate[SpikeMode],MODIFI_PARA|SCREEN_TWO_BYTE);
//					SetScreenItem(NULL   ,(u8 *)"平段电价 %G.4元/度",& pdisDisPlayData[19],(u8 *)&eleRate[NormalMode],MODIFI_PARA|SCREEN_TWO_BYTE);
					  




					
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			}
				else if(ppItem==&pPeakTimeMenu)//错峰时段界面
			{

        pdisDisPlayData[0]=setPeakKey0;
				pdisDisPlayData[1]=setPeakKey1;
				pPeakTimeMenu=pdisPlayItem;
				if(pPeakTimeMenu!=NULL)
				{
				  
					SetScreenItem(pPeakTimeMenu ,   (u8 *)"当前时段       %O/%O" 	,pdisDisPlayData, NULL,MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"日期段  %O/%O-$O/$O"    ,&pdisDisPlayData[2],NULL,MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"时间段  %O:%O-$O:$O"    ,&pdisDisPlayData[6],NULL,MODIFI_PARA);
          SetScreenItem(NULL,   (u8 *)"模式    %l"   ,ACWorkMode, &pdisDisPlayData[10],MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"充放电模式   %l",DischageMode, &pdisDisPlayData[11],MODIFI_PARA);
					SetScreenItem(NULL,   (u8 *)"放电量  %F.0AH"   ,&pdisDisPlayData[12], NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
          SetScreenItem(NULL);//结束
					SetScreenItemSelectFirst(pPeakTimeMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}	
			else if(ppItem==&pPeakShaveMenu)//削峰界面
			{
				
								

								 
								 pdisDisPlayData[0]=gPeakShaveOnoff;
								 //	*((u16 *)(&pdisDisPlayData[1]))=gSinglePhasePower;//单相总功率
									*((u16 *)(&pdisDisPlayData[3]))=gAPhasePower;//L1相功率
									*((u16 *)(&pdisDisPlayData[5]))=gBPhasePower;//L2相功率
									*((u16 *)(&pdisDisPlayData[7]))=gCPhasePower;//L3相功率
								  //*((u16 *)(&pdisDisPlayData[9]))=gAcRateP;
									pdisDisPlayData[11]=gAcPhase;
									
								 
						 pPeakShaveMenu=pdisPlayItem;
				
				if(pPeakShaveMenu!=NULL)
				{
				   				
					          SetScreenItem(pPeakShaveMenu ,   (u8 *)"启用             %l"     ,OnOffLis,& pdisDisPlayData[0],MODIFI_PARA);
		               // SetScreenItem(NULL,   (u8 *)"单相总功率  %F.1Kw"    ,&pdisDisPlayData[1],(u8*)&gSinglePhasePower ,MODIFI_PARA|SCREEN_TWO_BYTE);//尖峰时段交流限功率值
                    SetScreenItem(NULL,   (u8 *)"L1相功率    %F.1Kw"    ,&pdisDisPlayData[3],(u8*)&gAPhasePower ,MODIFI_PARA|SCREEN_TWO_BYTE);//高峰时段交流限功率值
					          SetScreenItem(NULL,   (u8 *)"L2相功率    %F.1Kw"    ,&pdisDisPlayData[5],(u8*)&gBPhasePower ,MODIFI_PARA|SCREEN_TWO_BYTE);//平段时段交流限功率值
					          SetScreenItem(NULL,   (u8 *)"L3相功率    %F.1Kw"    ,&pdisDisPlayData[7],(u8*)&gCPhasePower ,MODIFI_PARA|SCREEN_TWO_BYTE);//低谷时段交流限功率值
//					          SetScreenItem(NULL,   (u8 *)"空调功率    %F.2Kw"     ,&pdisDisPlayData[9],(u8*)&gAcRateP ,MODIFI_PARA|SCREEN_TWO_BYTE);//空调额定功率
//					          SetScreenItem(NULL,   (u8 *)"空调相位    %l"        , PHlist,&pdisDisPlayData[11] ,MODIFI_PARA);//空调交流相位

          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pBattCalibrationMenu)//电池电流校准（分流器）
			{
						 pBattCalibrationMenu=pdisPlayItem;
				
				
				      pdisDisPlayData[0]=0;	
						//	*(u16 *)&pdisDisPlayData[1]=gk[0];
							pdisDisPlayData[3]=0;
							pdisDisPlayData[4]=0;
						  pdisDisPlayData[5]=0;
				      pdisDisPlayData[6]=0;
				      pdisDisPlayData[7]=0;
				      pdisDisPlayData[8]=0;
						  pdisDisPlayData[9]=0;
				      pdisDisPlayData[10]=0;
				       pdisDisPlayData[11]=0;
				       pdisDisPlayData[12]=0;
							
							if(pBattCalibrationMenu!=NULL)
							{
												
							
								 SetScreenItem(pBattCalibrationMenu ,(u8 *)"%l",UserList2, &pdisDisPlayData[3],MODIFI_PARA);
								 SetScreenItem(NULL,   (u8 *)"分流器量程: %l", RangeList,&pdisDisPlayData[4],MODIFI_PARA);
								 SetScreenItem(NULL   ,(u8 *)"校准电流:  %F.1A",&pdisDisPlayData[5],0,SCREEN_TWO_BYTE);//
								 SetScreenItem(NULL,   (u8 *)"模块电流:  %F.1A",        &gpSysData[LOADCURR],0,SCREEN_TWO_BYTE);
								 //SetScreenItem(NULL,   (u8 *)"增益:  %F.2",&pdisDisPlayData[1],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
								 SetScreenItem(NULL,   (u8 *)"零点:  %l", OnOffLis,&pdisDisPlayData[0],MODIFI_PARA);
								 SetScreenItem(NULL,   (u8 *)"%F.3V/%F.3V", &pdisDisPlayData[7],NULL,SCREEN_TWO_BYTE|NONE_DO);
								 SetScreenItem(NULL,   (u8 *)"实测电流:  %F.1A", &pdisDisPlayData[11],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
								 SetScreenItem(NULL);//结束
								return 1;
							}
							else 
								 return 0;

				
			}
			else if(ppItem==&pCalibrationMenu)//电流传感器校准界面（扩展铅酸单元）
			{
				    
				     pdisDisPlayData[3]=0;//校准通道 
				    *((u16 *)(&pdisDisPlayData[4]))=*((s16 *)&gpSysData[BATT_CURR1]);
             pdisDisPlayData[6]=0;//使能通道 
				     pdisDisPlayData[7]=0xFF;//ch 
				      *((u16 *)(&pdisDisPlayData[8]))=10000;
				
				 pCalibrationMenu=pdisPlayItem;
				if(pCalibrationMenu!=NULL)
				{
				   				
					 SetScreenItem(pCalibrationMenu ,(u8 *)"%l",UserList2, &pdisDisPlayData[3],MODIFI_PARA);
					 SetScreenItem(NULL,   (u8 *)"模块电流      %F.1A",        &gpSysData[LOADCURR],0,SCREEN_TWO_BYTE);
					
		       SetScreenItem(NULL   ,(u8 *)"校准电流: %F.1A",&pdisDisPlayData[4],0,SCREEN_TWO_BYTE);//
					 SetScreenItem(NULL,   (u8 *)"量程: %l", RangList,&pdisDisPlayData[0],MODIFI_PARA);
					 SetScreenItem(NULL,   (u8 *)"零点: %d ",&pdisDisPlayData[8],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);  
          // SetScreenItem(NULL,   (u8 *)"系数: %F.3",&pdisDisPlayData[1],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
           SetScreenItem(NULL);//结束
					 	SetScreenItemSelectFirst(pCalibrationMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			
			}
			
			else if(ppItem==&pCalibrationVMenu)//电压校准界面
			{
				
				    pdisDisPlayData[0]=0;//进入设校准界面，设置为低点
				    *((u16 *)(&pdisDisPlayData[1]))=5000;//设置初始校准值
				   
				 pCalibrationVMenu=pdisPlayItem;
				if(pCalibrationVMenu!=NULL)
				{
				   				
					 SetScreenItem(pCalibrationVMenu, (u8 *)"校准电压: %F.2V",   &pgh52c0->m_result[3],  0,                   SCREEN_TWO_BYTE);
           SetScreenItem(NULL,              (u8 *)"校准值:   %F.2V",   &pdisDisPlayData[1],    NULL,                MODIFI_PARA|SCREEN_TWO_BYTE);
//					 SetScreenItem(NULL,              (u8 *)"温度修正     %l",   OnOffLis,               &pdisDisPlayData[0], MODIFI_PARA);
//					  SetScreenItem(NULL,             (u8 *)"温度修正值   %d",  &sysPara[TEMP_CORR],     NULL               , NONE_DO);
//					 SetScreenItem(NULL,              (u8 *)"温度AD       %d",  (u8 *)&tempAD ,          NULL,                NULL);
//					 SetScreenItem(NULL,              (u8 *)"温度电V   %F.3V",  (u8 *)&tempV  ,          NULL,                SCREEN_TWO_BYTE);
//					 SetScreenItem(NULL,              (u8 *)"温度RevC %F.2uA",  (u8 *)&tempI  ,          NULL,                SCREEN_TWO_BYTE);	
           SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			
			}
		   else if(ppItem==&pTimeMenu)//时间设置界面
			{
				
				 pTimeMenu=pdisPlayItem;
				if(pTimeMenu!=NULL)
				{
				   				
					SetScreenItem(pTimeMenu ,   (u8 *)"%d年%d月%d日 ",pdisDisPlayData,NULL,MODIFI_PARA);
		      SetScreenItem(NULL,   (u8 *)"%d时%d分%d秒 ",&pdisDisPlayData[6],NULL,MODIFI_PARA);
          SetScreenItem(NULL);//结束
					return 1;
			  }
			  else 
				   return 0;
			
			}
				else if(ppItem==&pRECORDMenu)//事件记录界面
			{
				
				pRECORDMenu=pdisPlayItem;
				if(pRECORDMenu!=NULL)
				{
					

						SetScreenItem(pRECORDMenu,(u8 *)"告警记录 ",NULL,(u8 *)(&pAlarmRecordMenu),NEXT_SCREEN);//0
		        SetScreenItem(NULL,   (u8 *)"抄表记录 ",NULL,(u8 *)(&pPowrRecordMenu),NEXT_SCREEN);//1 
					  SetScreenItem(NULL,   (u8 *)"用户电量记录 ",NULL,(u8 *)(&pPowrRecord90Menu),NEXT_SCREEN);//1
//					  SetScreenItem(NULL,   (u8 *)"电池测试记录 ",NULL,(u8 *)(&pBattRecordMenu),NEXT_SCREEN);
					  SetScreenItem(NULL,   (u8 *)"升级日志 "     ,NULL,(u8 *)(&pUpDataLogMenu),NEXT_SCREEN);
					 
		        SetScreenItem(NULL);//结束//2
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pAlarmRecordMenu)//告警事件记录界面
			{
				
				pAlarmRecordMenu=pdisPlayItem;
				if(pAlarmRecordMenu!=NULL)
				{
				   u16 *para16=(u16 *)pdisDisPlayData;
					 u16 *recordTime=&para16[2];
					 u8 *alarmRecordType=(u8 *)&recordTime[6];
					
					
					
					 preAlarmCount=-1;
					  para16[0]=1;//设置进入告警记录界面时都指向第一条

    				para16[1]=galarmRecordMax;//
					
					for(u8 i=0;i<6;i++)//12byte
					 recordTime[i]=0;
					
						for(u8 i=0;i<4;i++)
					 alarmRecordType[i]=0;//4byte
						
					
						SetScreenItem(pAlarmRecordMenu,(u8 *)"当前记录    %o/$o",para16,NULL,MODIFI_PARA);//3告警记录条目控制
  		      SetScreenItem(NULL,(u8 *)"%d/%d/%d %d:%d:%d",recordTime,NULL,0);//4告警记录日期时间
					  SetScreenItem(NULL,(u8 *)"%l",ALarmListStatus,&alarmRecordType[3],0);//告警状态
					  SetScreenItem(NULL,(u8 *)"%l%l%l",AlarmMunklis,alarmRecordType,0);//告警记录
			      SetScreenItem(NULL);//结束//6
						SetScreenItemSelectFirst(pAlarmRecordMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}
      else if(ppItem==&pPowrRecordMenu)//抄表事件记录界面
			{


				for(u8 i=0;i<32;i++)
				{
					 pdisDisPlayData[i]=0;
				}
				 u16 *para16=(u16 *)pdisDisPlayData;
				  para16[0]=1;
				  para16[1]=genergyDateRecordMax;
				
				preDayCount=-1;
				
				
				pPowrRecordMenu=pdisPlayItem;
				if(pPowrRecordMenu!=NULL)
				{
				   
					SetScreenItem(pPowrRecordMenu,(u8 *)"当前记录     %o/$o",&para16[0],NULL,MODIFI_PARA);//7记录条目控制0123
					SetScreenItem(NULL,(u8 *)"%d/%D/%D %D:%D:%D",&pdisDisPlayData[4],NULL,0);//记录日期时间 456789A
					//SetScreenItem(NULL,(u8 *)"总电量: %F.1KWH",&pdisDisPlayData[11],NULL,SCREEN_FOUR_BYTE);//9交流电能11 12 13 14
					
					  if(gWorkShiftMode==0)
								{
									 SetScreenItem(NULL,   (u8 *)"电能:%F.1KWH",   &pdisDisPlayData[11],NULL,SCREEN_FOUR_BYTE);
								}
								else
								{
									 SetScreenItem(NULL,   (u8 *)"错峰电能",        NULL,NULL,NONE_DO);// totalEnergy[6]
									 SetScreenItem(NULL,   (u8 *)"尖峰:%F.1KWH",  &pdisDisPlayData[15],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"高峰:%F.1KWH",  &pdisDisPlayData[19],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"平段:%F.1KWH",  &pdisDisPlayData[23],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"低谷:%F.1KWH",  &pdisDisPlayData[27],NULL,SCREEN_FOUR_BYTE);
									 SetScreenItem(NULL,   (u8 *)"总共:%F.1KWH",  &pdisDisPlayData[31],NULL,SCREEN_FOUR_BYTE);//31 32 33 34
									 SetScreenItem(NULL,   (u8 *)"其它:%F.1KWH",  &pdisDisPlayData[11],NULL,SCREEN_FOUR_BYTE);									
									
								}
					SetScreenItem(NULL);//结束//14
					SetScreenItemSelectFirst(pPowrRecordMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}
			
			
			else if(ppItem==&pPowrRaTeRecordMenu)//抄表费用录页
			{
				   for(u8 i=0;i<32;i++)
						{
							 pdisDisPlayData[i]=0;
						}
						 u16 *para16=(u16 *)pdisDisPlayData;
							para16[0]=1;
							para16[1]=genergyDateRecordMax;
						
						preDayCount=-1;
						
							pPowrRaTeRecordMenu=pdisPlayItem;
						if(pPowrRaTeRecordMenu!=NULL)
						{
								 
										SetScreenItem(pPowrRaTeRecordMenu,(u8 *)"当前记录     %o/$o",&para16[0],NULL,MODIFI_PARA);//7记录条目控制0123
									                 SetScreenItem(NULL,(u8 *)"%d/%D/%D %D:%D:%D",&pdisDisPlayData[4],NULL,0);//记录日期时间 456789A
																	 SetScreenItem(NULL,   (u8 *)"常态电费: %F.2元",    &pdisDisPlayData[11],0,SCREEN_FOUR_BYTE);//35 36 37 38
																	 SetScreenItem(NULL,   (u8 *)"尖峰电费: %F.2元",    &pdisDisPlayData[15],0,SCREEN_FOUR_BYTE);
																	 SetScreenItem(NULL,   (u8 *)"高峰电费: %F.2元",    &pdisDisPlayData[19],0,SCREEN_FOUR_BYTE);
																	 SetScreenItem(NULL,   (u8 *)"平峰电费: %F.2元",    &pdisDisPlayData[23],0,SCREEN_FOUR_BYTE);
																	 SetScreenItem(NULL,   (u8 *)"低峰电费: %F.2元",    &pdisDisPlayData[27],0,SCREEN_FOUR_BYTE);
																	 SetScreenItem(NULL,   (u8 *)"谷平峰费: %F.2元",    &pdisDisPlayData[31],0,SCREEN_FOUR_BYTE); 
								                	SetScreenItem(NULL);//结束//14
					                         SetScreenItemSelectFirst(pPowrRecordMenu);//设置为选择翻页
							return 1;
			    }
			    else 
				   return 0;	
						
						
				
			}
			 else if(ppItem==&pPowrRecord90Menu)//抄表事件记录界面
			{

         for(u8 i=0;i<19;i++)
         {pdisDisPlayData[i]=0;}
				
				 u16 *para16=(u16 *)pdisDisPlayData;
				  para16[0]=1;
				  para16[1]=genergyDateRecord90Max;//0123 //最大记录数
				  para16[2]=1;
				  para16[3]=6;//4567 总分路数
				preDayCount=-1;
				preDayCount2=-1;
				
				pPowrRecord90Menu=pdisPlayItem;
				if(pPowrRecord90Menu!=NULL)
				{
				   
					SetScreenItem(pPowrRecord90Menu,(u8 *)"当前记录       %o/$o",&para16[0],NULL,MODIFI_PARA);//7记录条目控制0123
					             SetScreenItem(NULL,(u8 *)"当前用户       %o/$o",&para16[2],NULL,MODIFI_PARA);//7记录条目控制4567
					             SetScreenItem(NULL,(u8 *)"%d/%D/%D %D:%D:%D",&pdisDisPlayData[8],NULL,0);//记录日期时间 8 9 10 11 12 13 14  
              				 SetScreenItem(NULL,(u8 *)"电量: %F.1KWH",   &pdisDisPlayData[15],NULL,SCREEN_FOUR_BYTE);//9交流电能15 16 17 18

					SetScreenItem(NULL);//结束//14
					SetScreenItemSelectFirst(pPowrRecordMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}
			
			
			
			
			
			
			
				else if(ppItem==&pBattRecordMenu)//电池测试记录界面
			{

				
				 u16 *para16=(u16 *)pdisDisPlayData;
				  para16[0]=1;
				  para16[1]=gdisBattRecordMax;
				
				preBattTestCount=-1;
				
				pBattRecordMenu=pdisPlayItem;
				if(pBattRecordMenu!=NULL)
				{
				   
					SetScreenItem(pBattRecordMenu,(u8 *)"当前记录     %o/$o",&para16[0],NULL,MODIFI_PARA);//7记录条目控制
					SetScreenItem(NULL,(u8 *)"S:%d/%D/%D %D:%D:%D",&pdisDisPlayData[4],NULL,0);//8记录日期时间
					SetScreenItem(NULL,(u8 *)"E:%d/%D/%D %D:%D:%D",&pdisDisPlayData[11],NULL,0);//8记录日期时间
					SetScreenItem(NULL,   (u8 *)"放电电量     %F.1Ah",		&pdisDisPlayData[18],0,SCREEN_TWO_BYTE);
					SetScreenItem(NULL);//结束//14
					SetScreenItemSelectFirst(pBattRecordMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}//pUpDataLogMenu
			else if(ppItem==&pUpDataLogMenu)//升级记录界面
			{

				
				 u16 *para16=(u16 *)pdisDisPlayData;
				for(u8 i=0;i<18;i++)
				  pdisDisPlayData[i]=0;
				para16[0]=0;
				para16[1]=0xff;
				
				
				sprintf((char *)&pdisDisPlayData[18],( const char *)"");
			   pgh52c0->readI2C2(UPDATA_LOG+2,(u8 *)&para16[0],2);
			
			
				pUpDataLogMenu=pdisPlayItem;
				if(pUpDataLogMenu!=NULL)
				{
				   
					SetScreenItem(pUpDataLogMenu,(u8 *)"当前记录     %o",&para16[0],NULL,MODIFI_PARA);//7记录条目控制
					SetScreenItem(NULL,(u8 *)"S:%d/%D/%D %D:%D:%D",&pdisDisPlayData[4],NULL,0);//8记录日期时间
					SetScreenItem(NULL,(u8 *)"E:%d/%D/%D %D:%D:%D",&pdisDisPlayData[11],NULL,0);//8记录日期时间
					SetScreenItem(NULL,   &pdisDisPlayData[18],		           0,0,0);
				
					SetScreenItem(NULL);//结束//14
					SetScreenItemSelectFirst(pUpDataLogMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}
			 else if(ppItem==&pPASSWORD)//密码
			{
				  
				   pPASSWORD=pdisPlayItem;
				   if(pPASSWORD!=NULL)
					 {

             SetScreenItem(pPASSWORD,         (u8*)"密码: %x%x%x%x",PassList,passwordshow,MODIFI_PARA);					 
				     SetScreenItem(NULL);//结束
						 
						     return 1;
			     }
			  else 
				   return 0;
			}//Item *pFactorySettingMenu
			 else if(ppItem==&pFactorySettingMenu)//记录清除
			{
				  
				   pFactorySettingMenu=pdisPlayItem;
				
				    	pdisDisPlayData[0]=0;
				      pdisDisPlayData[1]=0;
				      pdisDisPlayData[2]=0;
				      pdisDisPlayData[3]=0;
				      pdisDisPlayData[4]=0;
				      pdisDisPlayData[5]=0;
				      pdisDisPlayData[6]=0;
				      pdisDisPlayData[7]=0;
				      pdisDisPlayData[8]=0;
				 
				   if(pFactorySettingMenu!=NULL)
					 {

						  SetScreenItem(pFactorySettingMenu,  (u8 *)"恢复出厂设置     %l",  OnOffLis, &pdisDisPlayData[0],setSpecialParaFlag);					 
						  SetScreenItem(NULL,                 (u8 *)"清除告警记录     %l",  OnOffLis, &pdisDisPlayData[1], MODIFI_PARA);//1 
   						//SetScreenItem(NULL,                 (u8 *)"用户电量清除     %l",  OnOffLis, &pdisDisPlayData[2],MODIFI_PARA);
   						SetScreenItem(NULL,                 (u8 *)"抄表记录清除     %l",  OnOffLis, &pdisDisPlayData[3],MODIFI_PARA);
  						SetScreenItem(NULL,                 (u8 *)"用户电量清除     %l",  OnOffLis, &pdisDisPlayData[4],MODIFI_PARA);
//  						SetScreenItem(NULL,                 (u8 *)"电池测试记录清除 %l", OnOffLis, &pdisDisPlayData[5],MODIFI_PARA);
						  SetScreenItem(NULL,                 (u8 *)"恢复配电单元参数 %l", OnOffLis, &pdisDisPlayData[6],MODIFI_PARA);
//						  SetScreenItem(NULL,                 (u8 *)"同步配电单元     %l", OnOffLis, &pdisDisPlayData[7],MODIFI_PARA);
						  SetScreenItem(NULL,                 (u8 *)"同步租户电能     %l", OnOffLis, &pdisDisPlayData[8],MODIFI_PARA);
						   SetScreenItem(NULL,                (u8 *)"                %D", &gsysParaflag, NULL              ,NONE_DO);
						
						 //setSpecialParaFlag=0;
						 
  						SetScreenItem(NULL);//结束
						     return 1;
			     }
			  else 
				   return 0;
			}
			 else if(ppItem==&pTsetMenu)//测试界面
			 {
				       pTsetMenu=pdisPlayItem;
				       SetScreenItem(pTsetMenu,          (u8 *)"test1        %D"  , &detectTimer1, NULL              ,NONE_DO);
						   SetScreenItem(NULL,               (u8 *)"test2        %D"  , &detectTimer2, NULL              ,NONE_DO);
				       SetScreenItem(NULL,               (u8 *)"test3        %F.1", &gcounter32    , NULL            ,SCREEN_FOUR_BYTE);
				       SetScreenItem(NULL,               (u8 *)"M1           %D",   &ModuleOnlineMessage[1]    , NULL            ,NONE_DO);
               SetScreenItem(NULL,               (u8 *)"M2           %D",   &ModuleOnlineMessage[2]    , NULL            ,NONE_DO);
				       SetScreenItem(NULL);//结束
						     return 1;
			 }
      else if(ppItem==&pSetBranchMenu)//配电设置界面
			{
				   pSetBranchMenu=pdisPlayItem;
				
				   
				
				 if(pSetBranchMenu!=NULL)
					 {
				                 SetScreenItem(pSetBranchMenu,  (u8 *)"配电分路设置",   NULL, (u8 *)(&pSetBranchParaMenu),NEXT_SCREEN);	
//						             SetScreenItem(NULL,            (u8 *)"电池参数设置",   NULL, (u8 *)(&pUserDownMenu),NEXT_SCREEN); 
//						             SetScreenItem(NULL,            (u8 *)"电池分路设置",   NULL, (u8 *)(&pBattBranchMenu),NEXT_SCREEN); 
						            
						             SetScreenItem(NULL);//结束
						                return 1;
					 }
					 else
					 return 0;	 
				
				
				
			}
			else if(ppItem==&pSetBranchParaMenu)//智能空开参数设置
			{
				  
				   pSetBranchParaMenu=pdisPlayItem;

     
					       for(u8 i=0;i<40;i++)
                      pdisDisPlayData[i]=0;
				            pdisDisPlayData[0]=1;
				
				
				     
	
                
	
	                	for( u8 i=0;i<TOTAL_USER;i++)
										 {
													if(SwitchOnlineCount[i]>0)//查找已出现过的用户分路
													{
														  pdisDisPlayData[1]++;
														
													}
										 }
				
				
				
				   if(pSetBranchParaMenu!=NULL)
					 {
  
						   
						 SetScreenItem(pSetBranchParaMenu,    (u8 *)"分路:%O/$O  槽位:$O   "  , &pdisDisPlayData[0],NULL,MODIFI_PARA);
						                SetScreenItem(NULL,   (u8 *)"开关类型    %l" ,      SwitchTypeList ,&pdisDisPlayData[32],NONE_DO);
						              //  SetScreenItem(NULL,   (u8 *)"控制方式    %l" ,      SwitchTypeList1 ,&pdisDisPlayData[33],NONE_DO);
                            SetScreenItem(NULL,   (u8 *)"下电模式    %l" ,      userDownlis2 ,&pdisDisPlayData[3],SCREEN_TWO_BYTE|MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"分路用户    %l" ,    UserListMenu ,  &pdisDisPlayData[4],MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"过载电流    %F.1A" ,    &pdisDisPlayData[5] ,0,SCREEN_TWO_BYTE|setSpecialParaFlag);
											
														SetScreenItem(NULL,   (u8 *)"下电电压    %F.2V" ,    &pdisDisPlayData[7] ,0,SCREEN_TWO_BYTE|MODIFI_PARA);
													 
														SetScreenItem(NULL,   (u8 *)"恢复电压    %F.2V" ,    &pdisDisPlayData[9],0,SCREEN_TWO_BYTE|MODIFI_PARA);	
                            SetScreenItem(NULL,   (u8 *)"授权        %l"   ,     &OnOffLis   ,&pdisDisPlayData[11],setSpecialParaFlag);						 
														//SetScreenItem(NULL ,  (u8 *)"止于 %d年%d月%d日"    ,    &pdisDisPlayData[11],NULL, setSpecialParaFlag);
														SetScreenItem(NULL,   (u8 *)"下电时间    %d分钟" ,     &pdisDisPlayData[17],0,SCREEN_TWO_BYTE|MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"下电电量    %G.2KWH" ,     &pdisDisPlayData[19],0,SCREEN_TWO_BYTE|MODIFI_PARA);	 
														SetScreenItem(NULL,   (u8 *)"定时下电使能     %l" ,   OnOffLis,  & pdisDisPlayData[21],MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"开始定时    %DH:%DM" ,     &pdisDisPlayData[22],0,MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"定时时长    %DH:%DM" ,     &pdisDisPlayData[24],0,MODIFI_PARA);	
														SetScreenItem(NULL,   (u8 *)"免责使能        %l" ,    OnOffLis,&pdisDisPlayData[26],MODIFI_PARA);	 
														SetScreenItem(NULL,   (u8 *)"开始免责    %DH:%DM" ,  &pdisDisPlayData[27],0,MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"免责时长    %DH:%DM" ,  &pdisDisPlayData[29],0,MODIFI_PARA);
					                  SetScreenItem(NULL,   (u8 *)"额定电流    %DA" ,    &pdisDisPlayData[35] ,0,MODIFI_PARA);
														SetScreenItem(NULL,   (u8 *)"电量  %G.2KWH" ,    &pdisDisPlayData[36] ,0,SCREEN_FOUR_BYTE|setSpecialParaFlag);

					//	setSpecialParaFlag=0;
						 
						 
						
						 
						 
  						SetScreenItem(NULL);//结束
						     return 1;
			     }
			  else 
				   return 0;
			}
			else if(ppItem==&pUserDownMenu)//电池分路参数
			{
                 pdisDisPlayData[1] = 255; 
				 
				        pdisDisPlayData[3]=gSwitchPara[TOTAL_USER+7].st_downMode&0x0F;
				        *(u16 *)&pdisDisPlayData[7]=gSwitchPara[TOTAL_USER+7].st_downV;
				        *(u16 *)&pdisDisPlayData[9]=gSwitchPara[TOTAL_USER+7].st_recoverV;
                *(u16 *)&pdisDisPlayData[17]=gSwitchPara[TOTAL_USER+pdisDisPlayData[0]].st_downT;


				
				 pUserDownMenu=pdisPlayItem;
				
				
				   if(pUserDownMenu!=NULL)
					 {
						  
						         
											SetScreenItem(pUserDownMenu,   (u8 *)"下电模式   %l" ,      userDownlis2 ,&pdisDisPlayData[3],SCREEN_TWO_BYTE|MODIFI_PARA);   
											SetScreenItem(NULL,   (u8 *)"下电电压   %F.2V" ,     &pdisDisPlayData[7] ,0,SCREEN_TWO_BYTE|MODIFI_PARA);
										  SetScreenItem(NULL,   (u8 *)"恢复电压    %F.2V" ,    &pdisDisPlayData[9] ,0,SCREEN_TWO_BYTE|MODIFI_PARA);
										  SetScreenItem(NULL,   (u8 *)"下电时间   %d分钟" ,     &pdisDisPlayData[17],0,SCREEN_TWO_BYTE|MODIFI_PARA);


						 
  						SetScreenItem(NULL);//结束
						     return 1;
			     }
			  else 
				   return 0;

			}
			
			
			
			else if(ppItem==&pBattBranchMenu)//智能空开电池配置信息
			{
				   for(u8 i=0;i<6;i++)
				{
				   pdisDisPlayData[i]=gUseToBattBranch[i]+1;
				    
				   
				   (*(u16 *)&pdisDisPlayData[2*i+6])=gbranchBatt[i];
				}
				
				   pBattBranchMenu=pdisPlayItem;
				   if(pBattBranchMenu!=NULL)
					 {
						 SetScreenItem(pBattBranchMenu, (u8 *)"电池1<-->第%O槽位",&pdisDisPlayData[0],NULL,MODIFI_PARA);
						 SetScreenItem(     			NULL, (u8 *)"电池2<-->第%O槽位",&pdisDisPlayData[1],NULL,MODIFI_PARA);
						 SetScreenItem(						NULL, (u8 *)"电池3<-->第%O槽位",&pdisDisPlayData[2],NULL,MODIFI_PARA);
						 SetScreenItem(						NULL, (u8 *)"电池4<-->第%O槽位",&pdisDisPlayData[3],NULL,MODIFI_PARA);
						 SetScreenItem(						NULL, (u8 *)"电池5<-->第%O槽位",&pdisDisPlayData[4],NULL,MODIFI_PARA);
						 SetScreenItem(						NULL, (u8 *)"电池6<-->第%O槽位",&pdisDisPlayData[5],NULL,MODIFI_PARA);
						 SetScreenItem(     			NULL, (u8 *)"电池1容量: %dAH",   &pdisDisPlayData[6],NULL,MODIFI_PARA);	 
						 SetScreenItem(     			NULL, (u8 *)"电池2容量: %dAH",   &pdisDisPlayData[8],NULL,MODIFI_PARA);					 
						 SetScreenItem(     			NULL, (u8 *)"电池3容量: %dAH",   &pdisDisPlayData[10],NULL,MODIFI_PARA);					
						 SetScreenItem(     			NULL, (u8 *)"电池4容量: %dAH",   &pdisDisPlayData[12],NULL,MODIFI_PARA);					 
						 SetScreenItem(     			NULL, (u8 *)"电池5容量: %dAH",   &pdisDisPlayData[14],NULL,MODIFI_PARA);						 
						 SetScreenItem(     			NULL, (u8 *)"电池6容量: %dAH",   &pdisDisPlayData[16],NULL,MODIFI_PARA);
						 SetScreenItem(NULL);//结束
						 return 1;
					 }
					   else 
				   return 0;
			}
			else if(ppItem==&pBattMenu)//铅酸单元设置
			{
				  pBattMenu=pdisPlayItem;

				  SetScreenItem(pBattMenu,          (u8 *)"校准 ", NULL,(u8 *)(&pCalibrationMenu2),NEXT_SCREEN);
				//  SetScreenItem(NULL,          (u8 *)"电池下电设置 ", NULL,(u8 *)(&pUsrDownParaSetMenu),NEXT_SCREEN);//
				  SetScreenItem(NULL,          (u8 *)"版本信息     ", NULL,(u8 *)(&pUsrDownInfo),NEXT_SCREEN);//
				   SetScreenItem(NULL);//结束
				return 1;
			}
			else if(ppItem==&pCalibrationMenu2)//铅酸单元下电板校准
			{
				 pCalibrationMenu2=pdisPlayItem;
				
				 for(u8 i=0;i<14;i++)
				   pdisDisPlayData[i]=0;
				
					if(battAddr[0] > 4 || battAddr[0] == 0)
					{
							battAddr[0] = 1;  // 只有当索引无效时才重置为1
					}			
						
					
				 SetScreenItem(pCalibrationMenu2,(u8 *)"电池地址    %O/$O" ,&battAddr,NULL,MODIFI_PARA);
				 SetScreenItem(NULL, (u8 *)" %l"          ,BattList1             ,& pdisDisPlayData[0],MODIFI_PARA);
				 SetScreenItem(NULL,              (u8 *)"校准值: %F.1"  , & pdisDisPlayData[2] ,NULL,SCREEN_TWO_BYTE);
				 SetScreenItem(NULL,              (u8 *)"%l"           , Calilis              ,& pdisDisPlayData[1],MODIFI_PARA);
				 SetScreenItem(NULL,              (u8 *)"设置值: %F.1"  , & pdisDisPlayData[4] ,NULL,SCREEN_TWO_BYTE|MODIFI_PARA);
				 SetScreenItem(NULL);//结束
					return 1;
			}
			else if(ppItem==&pBatterySetMenu)//锂电设置界面
			{
				
				
				(*(u16 *)&pdisDisPlayData[0])=batt[0].Set_BattV;
				(*(u16 *)&pdisDisPlayData[2])=batt[0].Set_Batt_ChargA;
				(*(u16 *)&pdisDisPlayData[4])=batt[0].MOS_Charg;
				(*(u16 *)&pdisDisPlayData[6])=batt[0].MOS_DisCharg;
				 pdisDisPlayData[8]  = batt[0].WorkMode-1;


				 if(gsetbatt.WORK_MODE == 0x0101)
				 {
						pdisDisPlayData[9] = 0;
				 }
				 else  if(gsetbatt.WORK_MODE == 0x0303)
				 {
						pdisDisPlayData[9] = 1;
				 }
				 else  if(gsetbatt.WORK_MODE == 0x0707)
				 {
						pdisDisPlayData[9] = 2;
				 }
				
				
				
				battID[0]=1;
					pBatterySetMenu=pdisPlayItem;
				if(pBatterySetMenu!=NULL)
				{
				   
					SetScreenItem(pBatterySetMenu,(u8 *)"当前模块        %O/$O",  &battID,0,MODIFI_PARA);
					SetScreenItem(NULL,(u8 *)"放电电压      %F.2V",&pdisDisPlayData[0],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,(u8 *)"充电系数      %F.2%",&pdisDisPlayData[2],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
//					SetScreenItem(NULL,(u8 *)"MOS充闭合      %l",OnOffLis,&pdisDisPlayData[4],MODIFI_PARA);
//					SetScreenItem(NULL,(u8 *)"MOS放闭合      %l",OnOffLis,&pdisDisPlayData[6],MODIFI_PARA);
//				  SetScreenItem(NULL,(u8 *)"锂电工作特性   %l",BattWorkCharacter,&pdisDisPlayData[8],MODIFI_PARA);
//          SetScreenItem(NULL,(u8 *)"设定工作特性   %l",BattWorkCharacter,&pdisDisPlayData[9],MODIFI_PARA);

					SetScreenItem(NULL);//结束
					SetScreenItemSelectFirst(pBatterySetMenu);//设置为选择翻页
					return 1;
			  }
			  else 
				   return 0;
			}
			else if(ppItem==&pAirMenu)//空调设置界面
			{
				pAirMenu=pdisPlayItem;
//				InitAirSimData();
				if(airOrder[0] > 8 || airOrder[0] == 0)
				{
						airOrder[0] = 1;  // 只有当索引无效时才重置为1
				}
				
				for(u8 i=0;i<20;i++)
				{
					pdisDisPlayData[i]=0;
				}
				
				pCurrentAir=&g_astAIRpara[airIdx];
				
				pdisDisPlayData[0] = pCurrentAir->ast_workStatus;
				pdisDisPlayData[1] = pCurrentAir->ast_compressorstatus;
				(*(s16 *)&pdisDisPlayData[2]) = pCurrentAir->ast_cabinetTemp;
				(*(u16 *)&pdisDisPlayData[4]) = pCurrentAir->ast_infanSpeed;
				(*(u16 *)&pdisDisPlayData[6]) = pCurrentAir->ast_exfanSpeed;
				(*(u16 *)&pdisDisPlayData[8]) = pCurrentAir->ast_airColdData;// g_AirInterfaceData[airIdx].g_AirColdData;
				(*(u16 *)&pdisDisPlayData[10]) =pCurrentAir->ast_airTempData; //g_AirInterfaceData[airIdx].g_AirTempData;
				pdisDisPlayData[12] = airIdx;  // 初始化分路标记
				(*(u16 *)&pdisDisPlayData[13]) = mt11Humi;
				(*(u16 *)&pdisDisPlayData[15]) = mt11Tmp;
				(*(s16 *)&pdisDisPlayData[17]) = pCurrentAir->ast_humi;
			
				
				if(pAirMenu!=NULL)
				{
					SetScreenItem(pAirMenu,(u8 *)"空调模块    %O/$O" ,&airOrder,NULL,MODIFI_PARA);
					SetScreenItem(NULL,(u8 *)"空调工作状态    %l" , airstatusLis,&pdisDisPlayData[0],SCREEN_ONE_BYTE);
					SetScreenItem(NULL,(u8 *)"压缩机状态      %l" , airstatusLis,&pdisDisPlayData[1],SCREEN_ONE_BYTE);		
					SetScreenItem(NULL,   (u8 *)"柜内回风温度  %F.1℃",&pdisDisPlayData[2],0,SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"回风湿度  %F.1%RH",&pdisDisPlayData[17],0,SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"内风机转速   %dRpm",&pdisDisPlayData[4],NULL,SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"外风机转速   %dRpm",&pdisDisPlayData[6],NULL,SCREEN_TWO_BYTE);
					SetScreenItem(NULL,(u8 *)"压缩机制冷点  %F.1℃",&pdisDisPlayData[8],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,(u8 *)"高温告警点    %F.1℃",&pdisDisPlayData[10],NULL,MODIFI_PARA|SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"湿度  	      %F.1%RH",&pdisDisPlayData[13],NULL,SCREEN_TWO_BYTE);
					SetScreenItem(NULL,   (u8 *)"温度  	      %F.1℃",&pdisDisPlayData[15],NULL,SCREEN_TWO_BYTE);
					SetScreenItem(NULL);//结束
					SetScreenItemSelectFirst(pAirMenu);//设置为选择翻页
					return 1;
					
				}
				else 
					return 0;
			}
			return 0;//
}






void OnTickSetPara(Screen *pCurrentScreen)
{
	   Item ** CurrentPpItem=pCurrentScreen->m_layer[pCurrentScreen->m_LayerCount].st_ppItem;
	
	             if(CurrentPpItem==&pDCDMenu)//直流配电单元界面处理
							{
								 
								  SetDCDMenu(pCurrentScreen);
							}
	            else if(CurrentPpItem==&pSMRMenu)//模块开关机
							{
								 						
								  setSMRONOFF(pCurrentScreen);
							
								  isSetModule=0x11;
								
								if(genableTP==1)
								{
									SetModuleTP(pCurrentScreen);
								}
							}
							 else if(CurrentPpItem==&pSCRMenu)//光伏开关机
							{
								   setSMRONOFF(pCurrentScreen);
								  isSetModule=0x21;
							}
							else if(CurrentPpItem==&pBattBootSetMenu)//均充设置
							{
								SetBootMenu(pCurrentScreen);
							}
							else if(CurrentPpItem==&pBattTestSetMenu)//电池测试设置
							{
								SetBattTestMenu(pCurrentScreen);
							}
							/*
							else if(CurrentPpItem==&pBattDownParaSetMenu)//电池下电参数设置
							{
								SetBattDownPara(pCurrentScreen);//
							}	
							*/
               else if(CurrentPpItem==&pUSRMenu)//（分路电流）配电单元百分比计算
							{
								  SetUSRPara(pCurrentScreen);
							}
							 if(CurrentPpItem==&pUSR2Menu)//租户数据
							{
								 
								  SetUSR2Menu(pCurrentScreen);
							}
							if(CurrentPpItem==&pUSR3Menu)//负载分路电流
							{
								 
								  SetUSR3Menu(pCurrentScreen);
							}
							
							
							
							 else if(CurrentPpItem==&pSleepMenu)//休眠界面动作
							{
								  SetSleepPara(pCurrentScreen);
							}
								else if(CurrentPpItem==&pPeakShiftMenu)//错峰参数界面动作 
							{
								 SetPeakShiftPara(pCurrentScreen);
							}
							else if(CurrentPpItem==&pPeakTimeMenu)//错峰时段设置 
							{
								 SetPeakPara(pCurrentScreen);
							}
							else if(CurrentPpItem==&pPeakShaveMenu)//削峰参数界面动作 
							{
								 SetPeakShavePara(pCurrentScreen);
							}	
              else if(CurrentPpItem==&pBattCalibrationMenu)//校准分流器电池电流界面动作
							{
								 SetBttCalibration(pCurrentScreen);
							}							
              else if(CurrentPpItem==&pCalibrationMenu)//校准传感器电池电流界面动作
							{
								  SetCalibrationBattI(pCurrentScreen);//校准电流
							}
							else if(CurrentPpItem==&pCalibrationVMenu)//校准电池电压界面动作
							{
								  SetCalibrationBattV(pCurrentScreen);//校准电压 
							}				
	            else if(CurrentPpItem==&pALARMMenu)
							{
								  ShowALARM(pCurrentScreen);//告警
							}
							else if(CurrentPpItem==&pSetDIMenu)
							{
								  setDiPara(pCurrentScreen);//DI设置界面
							}
								else if(CurrentPpItem==&pSetDOMenu)
							{
								  setDoPara(pCurrentScreen);//DI设置界面
							}
							
							else if(CurrentPpItem==&pUsrDownParaSetMenu)
							{
								SetUserDownPara(pCurrentScreen);
							}
							
							else if(CurrentPpItem==&pTimeMenu)//时间界面动作
							{
								 SetTimePara(pCurrentScreen);
							}
							else if(CurrentPpItem==&pAlarmParaSetMenu)
							{
								SetAlarmPara(pCurrentScreen);
							}
							else if(CurrentPpItem==&pSysParaSetMenu)
							{
								SetOter(pCurrentScreen);//工厂参数设置
							}
							else if(CurrentPpItem==&pBattParaSetMenu)
							{
								SetBattParaSetMenu(pCurrentScreen);
							}
								else if(CurrentPpItem==&pAlarmRecordMenu)
							{
								
								GetAlarmRecordMenu(pCurrentScreen);
							}
							else if(CurrentPpItem==&pPowrRecordMenu)//交流电能抄表记录
							{
								GetEenergyRecordMenu(pCurrentScreen);
							}//
							else if(CurrentPpItem==&pPowrRaTeRecordMenu)//交流电费记录
							{
								  GetEenergyRateRecordMenu(pCurrentScreen);
							}
							else if(CurrentPpItem==&pPowrRecord90Menu)//
							{
								GetEenergy90dayRecordMenu(pCurrentScreen);
							}
								else if(CurrentPpItem==&pBattRecordMenu)//
							{
								GetBattTestRecordMenu(pCurrentScreen);
							}//pUpDataLogMenu
							else if(CurrentPpItem==&pUpDataLogMenu)//
							{
								GetUpDataLogMenu(pCurrentScreen);
							}
							 else if(CurrentPpItem==&pPASSWORD)//DI设置界面
							{
								 
								  ComparaPassWord(pCurrentScreen);
							}
								else if(CurrentPpItem==&pFactorySettingMenu)//愎恢默认参数
							{
								FactorySettingPara(pCurrentScreen);
							}
								else if(CurrentPpItem==&pSetBranchParaMenu)//扩展分路参配置页
							{
								SetBranchParaMenu(pCurrentScreen);
							}					
							else if(CurrentPpItem==&pUserDownMenu)
							{
								SetUserDownMenu(pCurrentScreen);
							}
							else if(CurrentPpItem==&pPOWERRATEMenu)
							{
								SetPOWERRATEMenu(pCurrentScreen);
							}
							else if(CurrentPpItem==&pBattBranchMenu)
							{
								 SetBattBranchMenu(pCurrentScreen);
							}
							else if(CurrentPpItem==&pSetPara)
							{
								ComparapFactoryPassWord(pCurrentScreen);
							}
							else if(CurrentPpItem==&pCalibrationMenu2)
							{
								  SetCalibrationMenu2(pCurrentScreen);//1
							}
								else if(CurrentPpItem==&pBATTERYData )	//锂电数据信息	
							{
								
								Update_BattData(pCurrentScreen);//更新锂电信息（切换锂电时）	
							}
							else if(CurrentPpItem==&pBatterySetMenu)//锂电设置
							{
								SetBrainBatt(pCurrentScreen);
							}
							else if(CurrentPpItem==&pAirMenu)//空调设置
							{
								SetAirData(pCurrentScreen);
							}
							else if(CurrentPpItem==&pLocalPowerMenu)
							{
								SetLocalPower(pCurrentScreen);
							}
							else if(CurrentPpItem==&pUSR3Menu)
							{
								GetBattData(pCurrentScreen);
							}
							else if(CurrentPpItem==&pBatteryCutMenu)
							{
								SetBattCut(pCurrentScreen);
							}


}