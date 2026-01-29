//通信用智能光储系统触摸屏监控程序通信模块
//版本：V1.0
//作者：叶子红
//本模块在后台运行，负责与设备通信
//更新记录：
//V0.0  2022/08/17  首次编制
//V0.1  2022/08/22  增加YD/T1363.3通信协议处理部分
//V0.2  2022/09/05  支持调试模式
//V0.3  2022/09/09  首次试点运行
//V0.4  2022/09/13  实现时间表设置
//V0.5  2022/09/23  实现参数设置，所有设置需要输入密码，实时曲线纵坐标范围按照电池组总容量和光伏方阵容量计算确定
//V0.6  2022/09/29  修改调试窗口显示形式，实现高级参数设置，统一设置代码，支持修改本地口令
//V0.7  2022/10/15  通过单击图例暂时隐藏某条曲线，代码优化，支持单击商标进入全屏显示，修正曲线图背景时段显示错误
//V0.8  2022/10/26  增加设备配置属性，解释设备配置消息，主页改为显示新增的系统框图
//V0.9  2022/11/11  增加了具体电池模块的有关数据，增加获取天气信息的功能，完善站点配置界面，解决全屏显示与非全屏显示不一致等问题
//V1.0  2022/12/14  增加风机和燃料电池的显示界面，增加实时数据主动上报平台功能，用收益曲线取代PUE曲线
 
//常量
const timetxt = "小时,hrs,分,min,秒,s".split(",");
var scanId=0;   //轮询指针
var gid=0;
var xhttp, xhtpReport;
var scanInterval = 250; //轮询间隔时间
var reportTimeOut = 0;  //上报超时计数
let serialPort = null;
let reader = null;
let writer = null;

let currentCID2 = null;

//解析数据按钮
function testa(){
    setCID2();
    const inputContent = document.getElementById('story').value;  
    debug(inputContent,true);//日志显示 
    handleLine(inputContent);   //数据解析   

    // const  commands = calculateChecksum("120142AA600A600000020F")
    // console.log("commands",commands);
} 

//中断显示按钮
function concle(){
      debugEnabled = !debugEnabled;
    const btn = document.getElementById('button');
    if (btn) {
        btn.textContent = debugEnabled ? '取消显示' : '恢复显示';
    }   
}

//发送命令码按钮
function Send() {  
    debugEnabled = true;
    const cmd = document.getElementById('cmd').value + '\r';
    // 检查去除空白字符和换行符后是否还有内容
    if (cmd.trim().replace('\r', '') !== '') {
        currentcmd = cmd;
        debug(0, false);
        sendSerialData(cmd);
    }
}

//设置工作参数命令
function SetParaCmd(setparacmd){
    if(setparacmd)
    {
        console.log("setparacmd",setparacmd);
        currentcmd = setparacmd;//设置当前命令
        debug(0,false);
        sendSerialData(currentcmd);
    }

}

//计算长度码校验
function getLengthCheck(input) {
    // 第一步：将输入转换为有效的十六进制字符串（兼容数字和字符串输入）
    let hexStr;
    // 检查输入是否为有效的十六进制值（0-FF范围，扩展支持3位长度）
    const isHexNumber = (value) => {
        if (typeof value === 'number') {
            return value >= 0 && Number.isInteger(value); // 放宽范围，支持更长的十六进制
        } else if (typeof value === 'string') {
            const num = parseInt(value, 16);
            return !isNaN(num) && num >= 0;
        }
        return false;
    };

    if (isHexNumber(input)) {
        // 转换为十六进制字符串（大写）
        let hexNum;
        if (typeof input === 'number') {
            hexNum = input.toString(16).toUpperCase();
        } else {
            hexNum = input.toUpperCase();
        }
        // 补零处理：不足3位前面补零，确保为3位长度
        hexStr = hexNum.padStart(3, '0');
        // 若超过3位，截取前3位（根据实际需求调整）
        if (hexStr.length > 3) {
            hexStr = hexStr.slice(0, 3);
        }
    } else {
        hexStr = String(input).padStart(3, '0').slice(0, 3);
    }

    // 第二步：将3位十六进制字符对应的数值相加（按十六进制规则）
    let sum = 0;
    for (const c of hexStr) {
        sum += parseInt(c, 16); // 每个字符转为十六进制数值后累加
    }

    // 第三步：对和进行取反加一（补码运算）
    const complement = ~sum + 1;

    // 第四步：取补码结果的十六进制最后一位（低4位）
    // 无论正负，通过与0x0F按位与取低4位，再转为十六进制
    const lastHexDigit = (complement & 0x0F).toString(16).toUpperCase();

    // 第五步：拼接结果（最后一位 + 原3位十六进制字符串）
    const checknum = lastHexDigit + hexStr;

    return checknum;
}




function setCID2(){
    const inputvalue = document.getElementById('cid2').value;
    currentCID2 = inputvalue;
}

//转换时间
function formatLocalTime(date) {
    const pad = num => num.toString().padStart(2, '0');
    return `${date.getFullYear()}-${pad(date.getMonth()+1)}-${pad(date.getDate())} ` +
           `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
}

//日志显示
let debugEnabled = true;
function debug(getValue,isFromTesta = false) {
    
    const textarea = document.getElementById('textdebug');
    const now = new Date();
// 格式化时间：YYYY-MM-DD HH:MM:SS
    const timestamp = formatLocalTime(now);

    if (!debugEnabled) return;

    //没有按解析按钮的时候轮询发送命令
    if (!isFromTesta) {
                textarea.value += `[${timestamp}] [发送] ${currentcmd}\n\n`;
    }
    
    //获取接收数据
    if (getValue) {
               console.log("cmd:",currentcmd);
        textarea.value += `[${timestamp}] [接收] ${getValue}\n\n`;
        textarea.scrollTop = textarea.scrollHeight; 
    }

}
//计算检验码
function calculateChecksum(cmd) {
    let sum = 0;
    
    // 每两个字符作为一个字节处理
    for (let i = 0; i < cmd.length; i += 2) {
        const byte = cmd.substr(i, 2);
        // 将每个字符转换为ASCII码并相加
        sum += byte.charCodeAt(0) + byte.charCodeAt(1);
    }
    // console.log("sum",sum);
    // 限制为16位(2字节)
    sum = sum & 0xFFFF;
    
    // 取反再加1
    const checksumValue = (~sum + 1) & 0xFFFF;
    
    // 转换为4位十六进制字符串
    return checksumValue.toString(16).padStart(4, '0').toUpperCase();
}

//生成命令
function generateCommands() {
    const commands = [];
    const ver = "12";      // VER: 1字节
    const addr = "01";     // ADDR: 1字节
    const length = "E002"; // LENGTH: 2字节 
    const cmdGroupDefault = "01"; // 其他CID1的默认COMMAND GROUP
    const cmdGroupSpecial = "FF"; // 特殊COMMAND GROUP（4A+46、E2/4A/E5的其他CID2用）
    
    // CID1数组：40-43、E1、E2、4A（原逻辑保留）
    const cid1Values = [];
    for (let i = 0x40; i <= 0x44; i++) {
        cid1Values.push(i.toString(16).toUpperCase());
    }
    cid1Values.push("E1");
    cid1Values.push("E5");
    cid1Values.push("4A"); // 与E2一起使用特殊COMMAND GROUP
    
    // 1. 原逻辑：CID1=0x42，CID2=0xA7、0xA8（length=0000）
    const targetCid1 = "42";
    const targetCid2s = ["A7", "A8"];
    targetCid2s.forEach((cid2) => {
        const cmdWithoutChecksum = ver + addr + targetCid1 + cid2 + "0000";
        const checksum = calculateChecksum(cmdWithoutChecksum);
        commands.push({
            command: '~' + cmdWithoutChecksum + checksum + '\r',
            cid2: cid2,
        });
    }); 

    const timeCid1 = "E1";
    const timeCid2s = ["4D"];
    timeCid2s.forEach((cid2) => {
        const cmdWithoutChecksum = ver + addr + timeCid1 + cid2 + "0000";
        const checksum = calculateChecksum(cmdWithoutChecksum);
        commands.push({
            command: '~' + cmdWithoutChecksum + checksum + '\r',
            cid2: cid2,
        });
    }); 

    // 新增：CID1=42、CID2=AE（length=E002，cmdGroup=FF）
    const special42Cid2 = "AE";
    const cmdWithoutChecksum42AE = ver + addr + targetCid1 + special42Cid2 + length + cmdGroupSpecial;
    const checksum42AE = calculateChecksum(cmdWithoutChecksum42AE);
    commands.push({
        command: '~' + cmdWithoutChecksum42AE + checksum42AE + '\r',
        cid2: special42Cid2,
    });
    
    
    // 3. 新增：CID1=4A 且 CID2 为 41、43、44 时，commandGroup 从 1 到 16 发送
    const cid14A = "4A";
    const cid2List4A = ["41", "43", "44","46"];
    for (let group = 1; group <= 16; group++) {
        const cmdGroup = group.toString(16).toUpperCase().padStart(2, '0'); // 转换为两位十六进制（如1→01，10→0A）
        cid2List4A.forEach((cid2) => {
            const cmdWithoutChecksum = ver + addr + cid14A + cid2 + length + cmdGroup;
            const checksum = calculateChecksum(cmdWithoutChecksum);
            commands.push({
                command: '~' + cmdWithoutChecksum + checksum + '\r',
                cid2: cid2,
                cmdGroup: cmdGroup,
            });
        });
    }

    const cid1E2 = "E2";
    const cid2ListE2 = ["41", "43", "44","46"];
    for (let group = 1; group <= 64; group++) {
        const cmdGroup = group.toString(16).toUpperCase().padStart(2, '0'); // 转换为两位十六进制（如1→01，10→0A）
        cid2ListE2.forEach((cid2) => {
            const cmdWithoutChecksum = ver + addr + cid1E2 + cid2 + length + cmdGroup;
            const checksum = calculateChecksum(cmdWithoutChecksum);
            commands.push({
                command: '~' + cmdWithoutChecksum + checksum + '\r',
                cid2: cid2,
                cmdGroup: cmdGroup,
            });
        });
    }
    
    // 4. 处理：E2、E5 的所有 CID2（41、43、44、46）（cmdGroup=FF）
    const specialCid1s = ["E5"];
    const cid2ListSpecial = ["41", "43", "44", "46"];
    specialCid1s.forEach((cid1) => {
        cid2ListSpecial.forEach((cid2) => {
            const cmdWithoutChecksum = ver + addr + cid1 + cid2 + length + cmdGroupSpecial;
            const checksum = calculateChecksum(cmdWithoutChecksum);
            commands.push({
                command: '~' + cmdWithoutChecksum + checksum + '\r',
                cid2: cid2,
            });
        });
    });
    
    // 5. 原逻辑：其他CID1（除E2、4A、E5外）生成命令（用默认cmdGroup=01）
    for (const cid1 of cid1Values) {
        if (specialCid1s.includes(cid1) || cid1 === "4A") continue; // 跳过E2、4A、E5，避免重复处理
        
        const cid1Hex = cid1.padStart(2, '0');
        const cid2Values = [0x41, 0x43, 0x44, 0x46]; 
        
        for (const cid2 of cid2Values) {
            const cid2Hex = cid2.toString(16).padStart(2, '0').toUpperCase();
            const cmdWithoutChecksum = ver + addr + cid1Hex + cid2Hex + length + cmdGroupDefault;
            const checksum = calculateChecksum(cmdWithoutChecksum);
            const fullCommand = '~' + cmdWithoutChecksum + checksum + '\r';
            
            commands.push({
                command: fullCommand,
                cid2: cid2Hex
            });
        }
    }   
    
    return commands;
}


async function conneToggle() {
    const btn = document.querySelector('button');
    if (serialPort && serialPort.readable) {
        // 断开连接
        if (reader) {
            await reader.cancel().catch(() => {});
            reader = null;
        }
        if (writer) {
            await writer.close();
            writer = null;
        }
        await serialPort.close();
        serialPort = null;
        btn.textContent = '连接';
        console.log('已断开串口连接');
        return;
    }

    try {
        // 选择串口
        serialPort = await navigator.serial.requestPort();
        
        // 打开串口（9600是最常用波特率，根据设备调整）
        await serialPort.open({ baudRate: 9600 });
        
        // 创建读写流
        writer = serialPort.writable.getWriter();
        reader = serialPort.readable.getReader();
        
        // 更新UI
        btn.textContent = '断开';
        console.log('已连接串口:', serialPort.getInfo());
        
        // 启动通信轮询
        startComm();

        
    } catch (error) {
        console.error('串口连接失败:', error);
        if (writer) {
            await writer.close();
            writer = null;
        }
        if (reader) {
            await reader.cancel();
            reader = null;
        }
        if (serialPort) {
            await serialPort.close();
            serialPort = null;
        }
    }
}

// 全局变量用于存储历史数据
let receivedOrder = []; 
let mergedData = "";
let flag = 0;
function handleLine(param){
 // 1. 数据校验
    if (!param || !param.startsWith('~')) {
        console.error("数据格式错误: 必须以~开头");
        return null;
    }

    // 2. 移除起始符并清洗数据
    let cleanData = param.slice(1).replace(/[^0-9A-Fa-f\s]/g, ''); // 保留空格
    cleanData = cleanData.replace(/\s/g, '0'); // 替换所有空格为零

    
    // 3. 解析数据结构
    const result = {
        header: {},
        len: null,
        dataFlag: null, // 新增DATAFLAG字段
        data: [],
        crc: null,
        specialData: null,
        CID2: parseInt(currentCID2,16)
    };

    // console.log("CID2",CID2);
    let pos = 0;

    // 4. 解析头部 (4字节)
    result.header = {
        VER: cleanData.slice(pos, pos+2),
        ADDR: cleanData.slice(pos+2, pos+4),
        CID1: parseInt(cleanData.slice(pos+4, pos+6),16),
        RTN: cleanData.slice(pos+6, pos+8)
    };
    pos += 8;
    const rtn = result.header.RTN;  
    if(rtn!= '00'){
        console.log("返回值出现错误,返回值：",rtn);
        return null;
    }

    // 5. 解析长度LEN (2字节)
        result.len = parseInt(cleanData.slice(pos+2, pos+4),16);
        pos += 4;
   

    // 提取当前数据
    const currentData = cleanData.slice(pos, -4);
    result.data = currentData;
    // console.log("result.data",result.data);
    // 6. 解析DATAFLAG (1字节)
    result.dataFlag = cleanData.slice(pos, pos+2);
    pos += 2;

                // console.log("result",result.dataFlag);
    var key = cmdKey(result.header.CID1, result.CID2);
                    //  console.log("key",key);
                     
     result.crc = cleanData.slice(-4); // 取最后4字节作为CRC
     //if (result.CID2 == 0xDA) login(parseInt(result.data, 16));
    //  console.log("result.len",result.len);
      if (result.data != undefined){
        //  console.log("result.CID2",result.CID2);
        if (key in eqm.diss && result.len >= 0) {   
                
            //获取等待的命令中的缓冲区
            var cmd = eqm.diss[key].cmd;
                            //console.log("eqm",eqm);
            var id = cmd.bufid();
            var buf = cmd.buffers[id];
            buf.setData(result.data);
            buf.rtn = rtn;
            cmd.next(); //收到数据，准备下一条查询
              
            if ([0x41, 0x43, 0x44, 0x46, 0x4D, 0x51, 0xD2, 0xD5, 0xD6].indexOf(result.CID2) >= 0) {   //自动读取
                        //console.log("eqm['OutCurr']",eqm);
                if(key=="C4241" && eqm["OutCurr"].valueText.length==0) setTimeout(eqm.config.chkConfig,2000);
                                //console.log("eqm['OutCurr']",eqm["OutCurr"]);
                decodeMsg(key, id);

                if(result.CID2==0x41) eqm.realtimeUpdate(result.header.CID1);
                //console.log("result.header.CID1:",result.header.CID1);
                if(result.CID2>=0x41 && result.CID2<=0x44){   //根据DataFlag确定是否再次获取开关状态或告警状态
                    if(eqm.units.unitOfCid1(result.header.CID1).stateChd()) buf.update=new Date("2021/09/29");
                    if(eqm.units.unitOfCid1(result.header.CID1).alarmChd()) buf.update=new Date("2021/09/29");    
                }
                if ([0x46, 0xD2, 0xD6, 0x4D].indexOf(result.CID2) >= 0) {    //收到读取参数命令的响应
                    //updatepage();
                }
            } 
            else if ([0xA7, 0xA8].indexOf(result.CID2) >= 0) {
                // 记录接收顺序
                receivedOrder.push(result.CID2);
                // 只在正确的顺序下才累加数据
                if (
                    (receivedOrder.length === 1 && result.CID2 === 0xA7) || 
                    (receivedOrder.length === 2 && receivedOrder[0] === 0xA7 && result.CID2 === 0xA8)
                ) {
                    mergedData += result.data;
                } else {
                    resetMergeState();
                    return;
                }

                // 当收到第二条数据（A8）时，检查顺序是否正确
                if (receivedOrder.length === 2) {
                    if (receivedOrder[0] === 0xA7 && receivedOrder[1] === 0xA8) {
                        console.log(`合并后的数据: ${mergedData}`);
                        hexToBuffer(mergedData); // 按正确顺序接收后解析
                    } else {
                        console.warn(`数据顺序错误，未执行解析`);
                    }
                    // 重置状态，准备下一次接收
                    resetMergeState();
                }
            }  
            else if([0xAE].indexOf(result.CID2) >= 0){
                parseDischargeCoeff(result.data);
            }
                

        }
        
    }

    return result;
}


// 新增：重置合并状态的辅助函数
function resetMergeState() {
    receivedOrder = [];
    mergedData = '';
    console.log('已重置数据合并状态');
}




// 存储当前正在等待响应的命令信息
let pendingCommand = null;
// 控制扫描状态
let isScanning = false;
// 启动通信
async function startComm(handler) {
    console.log("startComm");
    let accumulatedData = ''; 
    const processData = async () => {
        try {
            while (serialPort && serialPort.readable) {
                const { value, done } = await reader.read();
                if (done) break;
                
                if (value) {        
                    const textDecoder = new TextDecoder('windows-1252');
                    const chunk = textDecoder.decode(value);
                    accumulatedData += chunk;
                    
                    // 按行分割并处理完整行
                    const lines = accumulatedData.split('\n');
                    
                    // 如果最后一行不完整（没有换行符），保留到下一次处理
                    if (accumulatedData.endsWith('\r')) {
                        accumulatedData = '';
                    } else {
                        accumulatedData = lines.pop(); // 最后一个元素可能是不完整的行
                    }
                    


                    // 处理所有完整的行
                    for (const line of lines) {
                        const processedLine = line.trim();
                        if (processedLine) {
                            console.log("接收到完整数据行：", processedLine);
                            // 在这里添加你的业务逻辑，例如解析命令、更新UI等
                        try {
                            // 添加try/catch捕获handleLine中的异常
                            debug(processedLine,false)
                            handleLine(processedLine);




                        } catch (error) {
                            console.error("处理数据行时出错：", error);
                        }
                        }
                    }
                }
            }
        } catch (error) {
            console.error('读取串口数据出错:', error);

        } finally {
            if (reader) reader.releaseLock();
        }
    };

    // 开始处理数据 
    processData();      

    // 开始扫描命令
    startScan();
}


// 提供开始和停止扫描的方法
function startScan() {
    if (!isScanning) {
        
        scan(); // 立即开始第一次扫描
    }
}

function stopScan() {
    isScanning = false;
        clearTimeout(pendingCommand.timeout);

}
async function scan() {
    // 如果不在扫描状态或串口不可用，退出

    if (!isScanning || !serialPort || !serialPort.readable) return;
   
    try {
        // 获取当前要发送的命令
        const cmd = commands[cmdIndex];
        cmdIndex = (cmdIndex + 1) % commands.length;
        
        console.log(`发送命令: ${cmd.command}`);
        
        // 发送命令并等待响应处理完成
        await sendCommandWithResponse(cmd);
        
        // 响应处理完成后，再等待一段时间发送下一个命令
        setTimeout(scan, 500); // 可根据需要调整命令间隔
    } catch (error) {
        console.error('命令处理出错:', error);
        // 出错后仍然继续发送下一个命令
        setTimeout(scan, 1000);
    }
}

// 发送数据到串口
async function sendSerialData(cmd) {
    if (!serialPort || !writer) {
        console.error('串口未连接');
        return;
    }
    
    try {
        const textEncoder = new TextEncoder();
        await writer.write(textEncoder.encode(cmd));
            // console.log('cmd',cmd);
    } catch (error) {
        console.error('发送串口数据出错:', error);
    }
}


const commands = generateCommands();
let cmdIndex = 0;
let cmd  = null;
let currentcmd = null;
// 轮询监控模块
async function scan() {
    if (!serialPort || !serialPort.readable) return;

    cmd = commands[cmdIndex];
    currentCID2 = cmd.cid2;
    currentcmd = cmd.command;
    cmdIndex = (cmdIndex + 1) % commands.length;
    await sendSerialData(currentcmd);
    //console.log(currentcmd);
    //console.log(currentCID2);
    // 设置超时重试
    setTimeout(() => {  
        if (serialPort && serialPort.readable) {
            scan();
        }
    },2400); // 10秒超时
}

// 实时数据主动上报
async function submitData(sJsonDat) {
    if (!setup[1].value) return;
    let accumulatedData = ''; 
    const textDecoder = new TextDecoder('windows-1252');
    const chunk = textDecoder.decode(value);
    accumulatedData += chunk;
    
    // 按行分割并处理完整行
    const lines = accumulatedData.split('\n');
    
    // 如果最后一行不完整（没有换行符），保留到下一次处理
    if (accumulatedData.endsWith('\r')) {
        accumulatedData = '';
    } else {
        accumulatedData = lines.pop(); // 最后一个元素可能是不完整的行
    }
    
    // 处理所有完整的行
    for (const line of lines) {
        const processedLine = line.trim();
        if (processedLine) {
            console.log("接收到完整数据行：", processedLine);
            // 在这里添加你的业务逻辑，例如解析命令、更新UI等
//handleLine(processedLine);
        }
    }
    // if (reportTimeOut === 0) {
    //     try {
    //         const textEncoder = new TextEncoder('windows-1252');
    //         const data = JSON.stringify(sJsonDat);
    //         await writer.write(textEncoder.encode(data));
    //         reportTimeOut++;
    //     } catch (error) {
    //         console.error('上报数据出错:', error);
    //     }
    // } else {
    //     reportTimeOut++;
    //     if (reportTimeOut > 120) reportTimeOut = 0;
    // }
}

//接收到数据包，与命令定义保存在一起。每条命令仅保存最新一个数据包。
function newJsondat(jsonDat) {      
    jsonDat.update = new Date();
    var key = cmdKey(jsonDat.cid1, jsonDat.cid2);
    if (jsonDat.cid2 == 0xDA) login(parseInt(jsonDat.data, 16));
    else if (jsonDat.cid2 == 0xDC && typeof jsonDat.data == "string" && jsonDat.data.length % 4 == 2) {    //处理设备单元配置消息
        eqm.units.forEach(function (v) {v.enable=false;})
        var cid1, count, u;
        for(var i=0;i<parseInt(jsonDat.data.slice(0,2),16);i++){
            cid1 = parseInt(jsonDat.data.slice(i*4+2,i*4+4),16);
            count = parseInt(jsonDat.data.slice(i*4+4,i*4+6),16);
            if(i==0 && count==0) count=1;   //TODO: 第一个单元数量为零代表OEM版本，可能需要隐藏商标
            u = eqm.units.unitOfCid1(cid1);
            u.enable=true;
            u.gNum=count;
        }
        eqm.config.chkConfig(); //收到设备单元配置消息，更新界面配置
    }
    else if (jsonDat.err == 7) {
        //if (scaner.step == 3) scaner.step = 2;
    }
    else if (jsonDat.err == 6) {
        //scaner.rtn = new Date();
    }
    else if (jsonDat.data != undefined){
        if (key in eqm.diss && jsonDat.data.length > 0) {
            //获取等待的命令中的缓冲区
            var cmd = eqm.diss[key].cmd;
            var id = cmd.bufid();
            var buf = cmd.buffers[id];
            buf.setData(jsonDat.data);
            buf.rtn = jsonDat.rtn;
            cmd.next(); //收到数据，准备下一条查询
            if ([0x41, 0x43, 0x44, 0x46, 0x4D, 0x51, 0xD2, 0xD5, 0xD6].indexOf(jsonDat.cid2) >= 0) {   //自动读取
                if(key=="C4241" && eqm["OutCurr"].valueText.length==0) setTimeout(eqm.config.chkConfig,2000);
                decodeMsg(key, id);
                if(eqm["EqmSN"]!=undefined && eqm["EqmSN"].valueText.length>0){
                    jsonDat.sn = eqm["EqmSN"].valueText[0];
                    if(jsonDat.sn.length>0){
                        jsonDat.time = jsonDat.update.getDateTime();
                        jsonDat.gid = id+1;
                        submitData(jsonDat);    
                    }
                }
                if(jsonDat.cid2==0x41) eqm.realtimeUpdate(jsonDat.cid1);
                if(jsonDat.cid2>=0x41 && jsonDat.cid2<=0x44){   //根据DataFlag确定是否再次获取开关状态或告警状态
                    if(eqm.units.unitOfCid1(jsonDat.cid1).stateChd()) buf.update=new Date("2021/09/29");
                    if(eqm.units.unitOfCid1(jsonDat.cid1).alarmChd()) buf.update=new Date("2021/09/29");    
                }
                if ([0x46, 0xD2, 0xD6, 0x4D].indexOf(jsonDat.cid2) >= 0) {    //收到读取参数命令的响应
                    //updatepage();
                }
            }
            else if ([0xD0, 0xD4, 0xDE].indexOf(jsonDat.cid2) >= 0) {//读取并显示历史记录
                //if (scaner.step == 3 && jsonDat.data.substr(0, 2) == "01") {
                //    scaner.step = 2;
                    //setElement("nextp", "none", "D");
                //}
                //var dis = diss[key];
                //if (jsonDat.cid2 == 0xD4) decodeLog(jsonDat, dis);
                //else decodeDataLog(jsonDat, dis);
                //if (lcid % 15 == 0) {   //每15行暂停
                    //scaner.step = 2;
                    //setElement("nextp", "inline-block", "D");
                //}
            }
        }    
    }
}
//实时数据主动向平台上报
function submitData(sJsonDat){
    if(setup[1].value != undefined) {
        var submitBlock = sJsonDat;
        if( reportTimeOut == 0){
            xhtpReport.open("POST","http://"+setup[1].value,true);
            xhtpReport.setRequestHeader("Content-type", "application/json");
            var s = JSON.stringify(submitBlock);
            xhtpReport.send(s);
            reportTimeOut++;
        }
        else {
            reportTimeOut++;
            if(reportTimeOut>120) reportTimeOut=0;
        }
    }    
}

//以下为YD/T 1363.3协议处理程序

//设备单元类
function EqUnit(sDef) {
    var s = sDef.trim().split(":");
    this.dataitems=[];  //本设备单元的数据定义项
    this.supportCid2s=[];    //本设备单元支持的命令集
    this.gid = 0;   //组号，0表示不使用，255表示每次获取所有组的数据，其它正数表示只读取指组号的数据
    this.cgcid = "";    //支持组号的命令
    this.cgids = "";    //支持组号的命令所使用的组号，0按设备单元的组号设置，正数直接指定该命令所用组号而忽略单元组号
    this.gNum = 1;  //设备单元组数量
    switch (s.length) {
        case 11:
            this.cgids = s[10];
        case 10:
            this.cgcid = s[9];
            while (this.cgcid.length > this.cgids.length) {
                this.cgids += "00"; //补全缺省的零字节，使cgids的长度和cgcid的长度一致
            }
        case 9:
            this.bgcolor = parseInt(s[8], 16);
        case 8:
            this.gcountkey = s[7];
        case 7:
            this.width = parseInt(s[6], 10);
        case 6:
            this.table = (s[5] == 1);
        case 5:
            this.gid = getValue(s[4]);
            if (this.gid < -1) this.gNum = -this.gid;
        case 4:
            this.cid1 = getValue(s[3]);
        case 3:
            this.caption = s[2].localText();
        case 2:
            this.code = s[1];
            this.view = 1;  //根据设备单元类型确定显示布局形式，此方法为过渡方法，未来应在设备定义文件中指定
            if(this.code=="ACD") this.view = 2;
            else if(["DCD","SYS"].indexOf(this.caode)>=0) this.view=0;
        case 1:
            this.name = s[0];   
    }
    this.enable = true;
    this.alarms = [];
    this.df = 0x11;
    if (typeof EqUnit._init == "undefined") {
        EqUnit.prototype.alarm = function () { for (var i = 0; i < this.alarms.length; i++) if (this.alarms[i]) return true; return false; }
        EqUnit.prototype.alarmChd = function () { return (this.df & 0x01) == 0 ? false : true; };
        EqUnit.prototype.stateChd = function () { return (this.df & 0x10) == 0 ? false : true; };
        EqUnit.prototype.addDataitem = function (d) {
            this.dataitems.push(d);
            if(this.supportCid2s.indexOf(d.cid2)<0) this.supportCid2s.push(d.cid2);
            // console.log("d.cid2",d.cid2,this.supportCid2s);
            if(this.gcountkey==d.name) this.countField=d;
        }
        EqUnit._init = true;
    }
}
//命令类
function Command(sKey, units) {
    var cid2p = ["414D", "4344", "51D5DC", "46D2D6", "D0D4DE", "484ED3D7", "45D8"];
    this.key = sKey;
    this.CID1 = parseInt(sKey.slice(1, 3), 16);
    this.CID2 = parseInt(sKey.slice(3), 16);
    //根据CID2对命令分类
    for(var i=0;i<cid2p.length;i++) {
        if(cid2p[i].indexOf(sKey.slice(3,5))>=0) {
            this.type=i; 
            break;
        }
    }
    this.u = units.unitOfCid1(this.CID1);
    var j = this.u.cgcid.indexOf(sKey.slice(3));    //查找本命令码在单元支持组号命令列表中的位置
    this.hisCg = (j >= 0);    //本条命令支持组号
    this.gid = this.hisCg ? parseInt(this.u.cgids.substr(j, 2), 16) : 0;    //优先使用命令中的组号
    if (this.gid == 0) this.gid = this.u.gid;   //没有定义则使用单元组号
    if (this.gid < 0) this.gid = 1; //单元组号为负，表示需要遍历各组，将当前组号初始化为1
    this.dat = "";
    this.buffers = [new Buff(this.CID1, this.CID2)];   //命令对应的接收缓存，对于支持多组号的命令，每组一个缓存
    this.bufid = function () {
        if (this.u.gNum > 1 && this.gid > 0 && this.gid < 255) return this.gid - 1;
        else return 0;
    }
    this.accType = -1;
    for (var i = 0; i < cid2p.length; i++) {
        if (cid2p[i].indexOf(sKey.slice(3)) >= 0) {
            this.accType = i;
            break;
        }
    }
    this.goCg = function () {
        return (this.hisCg && this.u.gNum > 1 && this.gid > 0 && this.gid < 255);    //本命令需要遍历组号
    }
    this.next = function () {   //下一个组号
        if (this.goCg()) {  //遍历各组
            this.gid++;
            if (this.gid > this.u.gNum) this.gid = 1;
            while (this.gid>this.buffers.length) this.buffers.push(new Buff(this.CID1,this.CID2));
        }
        return this.gid;    //固定组号
    }
    this.createCmdData = function () {
        this.dat = this.hisCg ? this.gid.toFix(2,0,16) : "";
    }
}
//1363接收缓存类
function Buff(cid1, cid2){
    this.cid1 = cid1;
    this.cid2 = cid2;
    this.rtn = 0;
    this.data = "";
    this.update = new Date("2021/9/29");
    this.newData = 0;
    if(typeof Buff._init == "undefined") {
        Buff.prototype.pass = function () { var d = new Date(); return (d.getTime() - this.update.getTime()) / 1000 };
        Buff.prototype.setData = function (data) {  //设置接收到的数据包
            this.data = data;
            this.update = new Date();
            if (this.cid2 >= 0x41 && this.cid2 <= 0x44) eqm.units.unitOfCid1(this.cid1).df = this.getByte(0) & 0x11;    //获取DataFlag
        };
        Buff.prototype.getByte = function (Id) { return parseInt(this.data.substr(Id , 2), 16) };
        Buff.prototype.getInt = function (Id) { return parseInt(this.data.substr(Id, 4), 16) };
        Buff._init = true;
    }
}

//将数值字符串转换为数值
function getValue(sNum) {
    try {
        if (/H$/i.test(sNum)) return parseInt(sNum, 16);
        else return Number(sNum);
    }
    catch (e) {
        console.log(e.message);
        return 0;
    }
}
//解码设备描述对象
// This function takes in a string with a pipe-separated list of key-value pairs and returns an array of objects with the key and value properties.
// The key is converted to a number and the value is localized.
function decodeUnits(val) {
    return val.split("|").map(function (value) {
        var pair = value.split(":");
        var us = {};
        if (pair.length > 1) {
            us.key = getValue(pair[0]);
            us.value = pair[1].localText();
        }
        else {
            us.key = 0;
            us.value = value.localText();
        }
        return us;
    });
}
//描述设备的定义及其当前状态的类
function Equipment() {
    this.options = [];
    this.units = [];
    this.Caption = eqmDef.Caption.localText();
    this.Company = eqmDef.Company.localText();
    this.Description = eqmDef.Description.localText();
    for(var k in eqmDef){
        if(k.slice(0,3)=="OPT") {
            var opt = {key: k, value: decodeUnits(eqmDef[k])};
            this.options.push(opt);
        }
        else if(k.slice(0,1)=="U") {
            var u = new EqUnit(eqmDef[k]);
            this.units.push(u);
        }
    }
    //console.log("this.Caption:",this.Caption);
    this.units.unitOfName = function(key) {for(var i=0;i<this.length;i++) if(this[i].name==key) return this[i];}
    this.units.unitOfCid1 = function(cid1) {for(var i=0;i<this.length;i++) if(this[i].cid1==cid1) return this[i];}
    //根据定义文本创建数据项对象
    this.createDataItem = function(sDef) {
        var di = {};
        di.def = sDef;
        //console.log("di.Caption:",di);
        var s = sDef.split(",");
        switch (s.length) {
            case 15:
                di.gain = parseInt(s[14]);
            case 14:
                di.wcmd = s[13];
            case 13:
                di.wpos = s[12];
            case 12:
                di.wcid = s[11];
            case 11:
                di.common = s[10];
            case 10:
                di.repeat = s[9];
            case 9:
                di.gtype = s[8].localText();
                if (di.gtype == "" || di.gtype.toUpperCase() == "S") di.cF = "";
                else di.cF = "S";
            case 8:
                di.ln = s[7];
                di.mask = Math.pow(0x100 , di.ln);
            case 7:
                di.factor = getValue(s[6]);
                if (di.factor < 0) {
                     di.factor = -di.factor;
                     di.sign = true;
                }
                else {
                     di.sign = false;
                }
                if (di.factor < 1 && di.factor > 0) di.dec = s[6].length - s[6].lastIndexOf(".") - 1;
                else di.dec = 0;
            case 6:
                di.unitTxt = "";
                if (s[5].indexOf("|") > 0) {
                    di.unittext = decodeUnits(s[5]);
                }
                else if (s[5].indexOf("OPT") == 0) {
                    for (var k = 0; k < this.options.length; k++) {
                        if (this.options[k].key == s[5]) {
                            di.unittext = this.options[k].value;
                            break;
                        }
                    }
                }
                else {
                    var us = {};
                    us.key = 0;
                    if (s[5] == "℃" && langFlag > 0) us.value = "\u00B0" + "C";
                    else us.value = s[5].localText();
                    di.unittext = [];
                    di.unittext.push(us);
                    var ut = us.value;
                    if (ut == ":" || ut == "." || ut == "_" || ut.match(/^(YYMDHMS|YYMD|YYM|MD|HM)$/i)) ut = "";
                    else if (ut.match(/^(hrs|min|s)$/i)) {
                        if (langFlag == 0) {
                            var id = timetxt.indexOf(ut);
                            if (id > 0) ut = timetxt[id - 1];
                        }
                    }
                    di.unitTxt = ut;
                }
            case 5:
                di.vtype = parseInt(s[4]);
            case 4:
                di.cid = getValue(s[3]);
                di.key = "C" + di.cid.toFix(4,0,16);
                di.cid1 = di.cid >> 8;
                di.cid2 = di.cid & 255;
            case 3:
                di.caption = s[2].localText();
            case 2:
                di.name = s[1];
            case 1:
                di.unit = s[0];
        }
        di.valueText = [];
        di.vCode = [];
        di.value = function (id) {
            if(id>=0 && id<this.vCode.length) {
                if([1,2,6,7].indexOf(this.vtype)>=0) {
                    if(this.ln==4 && this.factor==0) return Buf2Float(this.vCode[id]);
                    else {
                        var v = parseInt(this.vCode[id],16);
                        if(this.sign && v>=Math.pow(0x100,this.ln)/2) v=v-Math.pow(0x100,this.ln);
                        return this.factor*v;
                    }
                }
                else return 0;
            }
            else return 0;
        };

        //console.log("sDef:",sDef);  //有问题
        di.sum = function (type) {
            if([1,2,6,7].indexOf(this.vtype)>=0){
                var sum = this.value(0);
                for(var i=1;i<this.vCode.length;i++) {
                    switch (type.toLowerCase()) {
                        case "sum":
                        case "avg":
                            sum += this.value(i);
                            break;
                        case "max":
                            if(this.value(i)>sum) sum=this.value(i);
                            break;
                        case "min":
                            if(this.value(i)<sum) sum=this.value(i);
                            break;
                    }
                }
                if(type.toLowerCase()=="avg") sum /= this.vCode.length;
                return sum;
            }
            return 0;
        }
        di.v = 0;
        di.labels = [];
        if (di.repeat > 1) {
            for (var i = 0; i < di.repeat; i++) {
                if (di.gtype.indexOf("[A]") >= 0) {di.labels.push(di.gtype.replace("[A]", String.fromCharCode(65 + i))); }
                else if (di.gtype.indexOf("[#]") >= 0) {di.labels.push(di.gtype.replace("[#]", i + 1));}
            }
        }

        di.setValue = function (vt, id) {   
            var vs = vt.split(",");
            for (var i = 0; i < vs.length; i++) {
                var ss = vs[i].trim().split(" ");
                console.log("ss",ss);
                if (ss.length > 2) {
                    this.labels[id + i] = ss[0];
                    this.valueText[id + i] = ss[1] + " " + ss[2];
                    //console.log(`第${id + i}个valueText  S0:`, this.valueText[id + i]);
                }
                else if (ss.length == 2) {  
                    if (this.unittext[0].value != "YYMDHMS") {
                        this.labels[id + i] = ss[0];
                        this.valueText[id + i] = ss[1];
                    //console.log(`第${id + i}个valueText    S1:  `, this.valueText[id + i]);
                    }
                    else {
                        this.labels[id + i] = "";
                        this.valueText[id + i] = ss[0] + " " + ss[1];
                       //console.log(`第${id + i}个valueText   S2:`, this.valueText[id + i]);
                    }
                }
                else {
                    this.labels[id + i] = "";
                    this.valueText[id + i] = ss[0];
                    //console.log(`第${id + i}个valueText   S3:`, this.valueText[id + i]);
                }
            }
            if (this.vtype == 1) {
                this.v = 0;
                for (var i = 0; i < this.valueText.length; i++) {
                    var x = parseFloat(this.valueText[i]);
                    if (x.toString() != "NaN") this.v += x;
                    
                }
                //console.log("x",x);
            }
            this.update = new Date();
        };
        //console.log("di",di);
        return di;    
    }
    this.diss = {    //包含所有命令定义和返回包
        cmds: [],   //命令集
        addItem: function (d, units) {         //添加数据项定义的方法
            var x = d.key;
            if (!(x in this)) {
                var a = [];
                this[x] = a;    //一条命令下的数据项集
                a.cmd = new Command(x, units);   //命令
                a.unit = units.unitOfName(d.unit);    //指向相应的设备单元
                a.cgNum = function () { //命令组数
                    if (this.unit.gNum <= 0) return 1;
                    else return this.unit.gNum;
                };
                this.cmds.push(a.cmd);
            }
            this[x].push(d);
        },
        count: function () { return this.cmds.length; }, //获取命令数
        nextCmd: function() {
            var cmd = this.cmds[scanId];
            var nextCommand;
            do{
                scanId++;
                if(scanId>=this.count()) scanId=0; 
                nextCommand=this.cmds[scanId];
                if(nextCommand.buffers[nextCommand.bufid()].pass()<3600 && nextCommand.type > 0){
                    for(var i=0;i<nextCommand.buffers.length;i++) if(nextCommand.buffers[i].pass()>3600) return cmd;
                }
            }
            while (nextCommand.buffers[nextCommand.bufid()].pass()<3600 && nextCommand.type > 0);
            //console.log("cmd",cmd);
            return cmd;
        }
    };
    const eqmDataNames=[
        "ACVolt", 0x40,
        "DCVolt", 0x42,
        "DCVoltAlarm",0x42,
        "LoadCurr", 0x42,
        "LoadEnergy", 0x42,
        "BattCurr", 0x42,
        "RecCurr", 0x42,
        "OutCurr", 0x42,
        "EnvTemp", 0xE1,
        "EnvTempAlarm", 0xE1,
        "SOC", 0xE1,
        "BattLeft", 0x42,
        "Income", 0xE1,
        "PUE", 0xE1,
        "InpEnergy", 0x40, 
        "GenEnergy", 0x42,
        "LoadEnergy", 0x42,
        "EqmSN", 0x42,
        "Capcitys", 0xE1,
        "DiscountSchedule", 0x40,
        "EqmTime", 0x42,
        "SMCount", 0x43,
        "BATGCount", 0x4A
    ];
   // console.log("eqmDef",eqmDef);
    for(var i=0;i<eqmDef.dataitems.length;i++) {
        var d = eqmDef.dataitems[i];
        if(d.length>0){
            var di = this.createDataItem(d);
        //    console.log("di",di);
            this.diss.addItem(di, this.units);
            this.units.unitOfName(di.unit).addDataitem(di);
            var ii = eqmDataNames.indexOf(di.name);
            //console.log("ii",ii);
            if(ii>=0){
                if(di.cid1==eqmDataNames[ii+1]) this[eqmDataNames[ii]]=di;
            }
        }
    }
   // console.log("di",di);
    this.realtimeChartDatas = [
        {cid1: 0x42, caption: "直流电压", unitText: "V", value: 0, name: "DCVolt", id : 0, curveShown:true, cfg:"base"},
        // {cid1: 0x42, caption: "光伏输出功率", unitText: "kWh", value: 0, name: "OutCurr", id : 0, curveShown:true, cfg:"pv"},
        {cid1: 0x42, caption: "电池电流", unitText: "A", value: 0, name: "BattCurr", id : -1, curveShown:true, cfg:"base"},
        {cid1: 0x42, caption: "负载电流", unitText: "A", value: 0, name: "LoadCurr", id : 0, curveShown:true, cfg:"base"},
        // {cid1: 0xE1, caption: "电池电量", unitText: "%", value: 0, name: "SOC", id : 0, curveShown:true, cfg:"base"}
    ];
    this.realtimeUpdate = function(cid1) {
        for(var i=0;i<this.realtimeChartDatas.length;i++) {
            var x = this.realtimeChartDatas[i];
           // console.log("x :",x);
            if(x.cid1==cid1) {
                x.value = x.id<0 ? this[x.name].v : Buf2Float(this[x.name].vCode[x.id]);
                // console.log("x.value 1:",x.value);
                if(x.unitText=="kWh" && this[x.name].unittext[0].value=="A") x.value *= this.DCVolt.v / 1000;
                x.date=this[x.name].update;
                // console.log("x.value 2:",x.value);
                this.realtimeDB.setValue(x.name, x.value, x.date.getMinute());
            }
        }
        for(var i=0;i<this.energyDatas.length;i++) {
            var x = this.energyDatas[i];
            if(x.cid1==cid1) {
                console.log("this[x.name].vCode[x.id]",this[x.name].vCode[x.id]);
                x.value=Buf2Float(this[x.name].vCode[x.id]);
                x.date=this[x.name].update;
            }
        }
        var engLog = [];
        for(var i=0;i<this.energyDatas;i++) {
            var x = this.energyDatas[i];
            if(x.date==undefined && this[x.name]!=undefined) return;
            engLog.push(x.value);
        }
        saveEnergyLogs(this.energyDatas[3].date, engLog);
    }
    this.energyDatas = [
        {cid1: 0x40, caption: "市电用电量", unitText: "kWh", value:0, name: "InpEnergy", id:0, cfg:"rectifier"},
        {cid1: 0x42, caption: "光伏发电量", unitText: "kWh", value:0, name: "GenEnergy", id:0, cfg:"pv"},
        {cid1: 0x42, caption: "负载用电量", unitText: "kWh", value:0, name: "LoadEnergy", id:0, cfg:"load"},
        {cid1: 0xE1, caption: "收益", unitText: "元", value:0, name: "Income", id:0, cfg:"base"}
    ];
        //设备可选配置
    /*
        // 模块启用状态
    rectifier: true,   // 整流器
    pv: true,         // 光伏
    wind: false,      // 风机
    fc: false,        // 燃料电池
    load: true,       // 负载
    base: true,       // 基础模块
    
    // 状态变更标记
    changed: false,
    
     配置检查方法
    chkConfig: function() { }
    */
    this.config = {
        rectifier: true, pv: true, wind: false, fc: false, load: true, base: true,
        changed: false,
        chkConfig: function() {
            var cfg = eqm.config;
            var oldState = cfg.rectifier;
            var u = eqm.units.unitOfCid1(0x41); 
            if(u==undefined) cfg.rectifier=false;
            else cfg.rectifier = u.enable;
            if(eqm["RecCurr"]!=undefined && eqm["RecCurr"].value(0)>0) cfg.rectifier = true;// 有电流则强制启用
            if(oldState!=cfg.rectifier) cfg.changed=true;
            oldState=cfg.pv;
            u = eqm.units.unitOfCid1(0x43);
            if(u==undefined) cfg.pv=false;
            else cfg.pv = u.enable;
            if(oldState!=cfg.pv) cfg.changed=true;
            oldState=cfg.load;
            cfg.load = (setupOf("直接带载").value!=0);
            if(oldState!=cfg.load) cfg.changed=true;
            if(eqm["OutCurr"]!=undefined && eqm["OutCurr"].valueText.length>=3){
                //接收到模拟量数据包后才能分析是否存在风机和燃料电池
                oldState=cfg.wind;
                cfg.wind = eqm["OutCurr"].value(1)>0;
                if(oldState!=cfg.wind) cfg.changed=true;
                oldState=cfg.fc;
                cfg.fc = eqm["OutCurr"].value(2)>0;
                if(oldState!=cfg.fc) cfg.changed=true;                    
            }
            return cfg.changed;
        }
    };
}
//根据设备类型码和命令类型码生成命令键值
function cmdKey(bCID1, bCID2) { return "C" + bCID1.toFix(2,0,16) + bCID2.toFix(2,0,16); }
function names(gt,count) {
    var ns = "";
    for (var i = 1; i <= count; i++) {
        if (i > 1) {ns+="," };
        ns += gt.replace("[1]", i);
    }
    return ns;
}
//解码1363消息数据包
function decodeMsg(key, id) {
    // console.log("decodeMsg key:",key);
    var dis = eqm.diss[key];
    var buf = dis.cmd.buffers[id];
    var cid1 = buf.cid1, cid2 = buf.cid2;
    if(buf.data.length>0){
        if (cid2 == 0x44) dis.unit.alarms[id] = false;
        for (var i = 0; i < dis.length; i++) {
            for (var j = 0; j < (dis[i].vtype == 10 || dis[i].repeat == 0 ? 1 : dis[i].repeat); j++) {
                dis[i].vCode[j + id] = buf.data.substr(j * dis[i].ln * 2, dis[i].ln * 2);
            }

           //console.log("dis:",dis); 
        //    console.log("buf:",buf); 
            vT = decodeItem(buf, dis, i);
            console.log("vT:",vT);
            if (vT != "NA") {
            if (dis[i].vtype < 10 || dis[i].vtype == 11) {
                if (["C4A41", "C4A43", "C4A44","C4A46"].includes(key)) {
                    // 1. 初始化列存储容器 + 循环索引（首次解析时创建）
                    if (!dis[i].columnData) {
                        dis[i].columnData = []; // 存储所有列的数值
                        dis[i].currentColIndex = 0; // 记录当前要写入的列索引（初始为0，对应序号1列）
                    }

                    // 2. 本次解析的单列数值（仅保留纯数值）
                    const currentValue = vT.trim();
                
                    const MAX_COLUMNS = 16; // 定义最大列数为16

                    // 3. 核心逻辑：16列循环覆盖（达到16列后从第一列重新写入）
                    dis[i].columnData[dis[i].currentColIndex] = currentValue;
                    dis[i].currentColIndex = (dis[i].currentColIndex + 1) % MAX_COLUMNS;

                    // 4. 告警逻辑（保留原有逻辑）
                    if (/!$/.test(currentValue) && cid2 == 0x44) {
                        dis.unit.alarms[id] = true;
                    }

                    // 5. 拼接所有列（始终显示16列，空列补空字符串）
                    const fullColumns = Array(MAX_COLUMNS).fill("");
                    dis[i].columnData.forEach((val, idx) => {
                        if (idx < MAX_COLUMNS) {
                            fullColumns[idx] = val || "";
                        }
                    });
                    const displayValue = fullColumns.join(",");

                    // 6. 赋值到表格单元格（核心：与序号1-16精准对齐）
                    dis[i].setValue(displayValue, id);
                    
                    // 新增：将16列数据分别写入表格对应单元格（序号1→列1，序号16→列16）
                    if(dis[i].rowElement) { // 确保表格行元素已存在
                        fullColumns.forEach((val, idx) => {
                            // idx=0 → 序号1列（cells[1]），idx=15 → 序号16列（cells[16]）
                            if(dis[i].rowElement.cells[idx + 1]) {
                                dis[i].rowElement.cells[idx + 1].text(val).txtAlign("center");
                            }
                        });
                    }
                } 
                else if(["CE241", "CE243", "CE244","CE246"].includes(key)){

                    if (!dis[i].columnData) {
                        dis[i].columnData = []; 
                        dis[i].currentColIndex = 0; // 初始为0，对应序号1列
                    }

                    const currentValue = vT.trim();

                    const MAX_COLUMNS = 64; 

                    dis[i].columnData[dis[i].currentColIndex] = currentValue;
                    dis[i].currentColIndex = (dis[i].currentColIndex + 1) % MAX_COLUMNS;

                    if (/!$/.test(currentValue) && cid2 == 0x44) {
                        dis.unit.alarms[id] = true;
                    }

                    // 5. 拼接所有列（始终显示64列，空列补空字符串）
                    const fullColumns = Array(MAX_COLUMNS).fill("");
                    dis[i].columnData.forEach((val, idx) => {
                        if (idx < MAX_COLUMNS) {
                            fullColumns[idx] = val || "";
                        }
                    });
                    const displayValue = fullColumns.join(",");

                    dis[i].setValue(displayValue, id);

                    if(dis[i].rowElement) { // 确保表格行元素已存在
                        fullColumns.forEach((val, idx) => {
                            if(dis[i].rowElement.cells[idx + 1]) {
                                dis[i].rowElement.cells[idx + 1].text(val).txtAlign("center");
                            }
                        });
                    }
                }
                    
                else {
                    // 非指定key：保持原有逻辑，仅显示单次值
                    dis[i].setValue(vT, id);
                    if (/!$/.test(vT) && cid2 == 0x44) dis.unit.alarms[id] = true;
                }
            }

                else if (dis[i].vtype == 10) {
                    var unit = eqm.units.unitOfCid1(cid1);
                    if (dis[i].name == unit.gcountkey && vT > 1) unit.gNum = vT;   //读取动态设备单元数量
                        // console.log("vT:",vT);
                    var repeat = dis[i].repeat;
                    console.log("repeat:",repeat);
                    if (vT > 0) {
                        var vNs = names(dis[i].gtype, vT);
                        dis[i].setValue(vNs, id);
                        vNs = vNs.split(",");
                        console.log("vNs:",vNs);
                        var vTs = [];
                        for (var k = 1; k <= repeat; k++) { vTs.push(""); }
                        for (var j = 1; j <= vT; j++) {
                            for (var k = 1; k <= repeat; k++) {
                                dis[i + k].vCode[j - 1] = buf.data.substr(0, dis[i + k].ln * 2);
                                if (j > 1) { vTs[k - 1] += ","; }
                                vTs[k - 1] += vNs[j - 1] + " " + decodeItem(buf, dis, i + k).trim();
                            }
                        }
                        for (var k = 1; k <= repeat; k++) {
                            if (dis[i + k].vtype < 10 || dis[i + k].vtype == 16) {
                                dis[i + k].setValue(vTs[k - 1], 0);
                                if (vTs[k - 1].indexOf("!") > 0 && cid2 == 0x44) dis.unit.alarms[id] = true;
                            }
                        }
                    }
                    i += Number(repeat);
                }


            }
        }
    }
    else{
        //TODO:返回错误处理
    }
}
function decodeItem(jsD, dis, dId) {
    var def = dis[dId];
    var count=1;
    var gname = def.gtype.localText();
    var gn = "";
    var rtnStr = "";
    var vText = "";
    var buf = "";
    var len = def.ln;
    if (def.vtype!=10 && def.vtype!=12 && def.repeat > 1) {
        count = def.repeat;
    }
    for (var ri = 0; ri < count; ri++) {
        if (gname.indexOf("[A]") >= 0) { gn = gname.replace("[A]", "ABC".charAt(ri)) }
        if (gname.indexOf("[1]") >= 0) { gn = gname.replace("[1]", ri + 1) }
        if (gname.indexOf("|") > 0) { gn = gname.split("|")[ri] }
        if (len < 0) len=-len;
        //console.log("gn",gn);
        buf = jsD.data.substr(0, 2 * len);
        //    console.log("buf",buf);
        jsD.data = jsD.data.substr(2 * len);
                        //console.log("jsD.data",jsD.data);
        if (dId + 1 < dis.length) {
            if (dis[dId + 1].ln < 0) jsD.data = buf + jsD.data;
        }           
        if (buf.length > 0 && buf.match(/\s/g)) {
            v = 0;
            vText = "NA";
        }
        else {

            v = (def.factor == 0 && len == 4) ? Buf2Float(buf) : parseInt(buf, 16);
            switch (def.vtype) {
                case 1:
                    if (def.sign) {
                        var mask = "0x10000000".substr(0, 3 + len * 2);
                        if (v >= mask / 2) { v -= mask; }
                    }
                case 2:
                case 7:
                    // console.log("def:",def)
                    vText = gn + " " + valueText(v, def, false);
                    if (def.vtype == 7 && v > 0) vText += "!";
                    break;
                case 3:
                    if (def.unittext[0].value == "YYMDHMS") {
                        vText = from1363Date(buf);
                    }
                    else {
                        vText = valueText(v, def, false);
                    }
                    break;
                case 4:
                    var s = "";
                    var code;
                    for (var i = 0; i < buf.length; i += 2) {
                        code = parseInt(buf.substr(i, 2), 16);
                        //s += "%" + buf.substr(i, 2);
                        s += String.fromCharCode(code); //仅支持ASCII字符
                    }
                    vText = s;
                    break;
                case 5:
                    var mask = def.unittext[0].value;
                    var s = "", v = "", ch, j = 0;
                    for (var i = 0; i < mask.length; i++) {
                        ch = mask.charAt(i);
                        if (ch == "#") v += buf.charAt(j++);
                        else {
                            if (i > 0) {
                                s += parseInt(v, 16);
                                v = "";
                            }
                            s += ch;
                        }
                    }
                    if (ch == "#") s += parseInt(v, 16);
                    vText = s;
                    break;
                case 6:
                    if (def.sign) {
                        var mask = 0x100 << (8 * (len - 1));
                        if (v >= mask / 2) v -= mask;
                    }
                    vText = valueText(v, def, false);
                    break;
                case 16:
                    vText = buf;
                    break;
                case 11:
                    var s = "";
                    var code;
                    var sp;
                    if (def.unittext[0].value.length > 0) sp = def.unittext[0].value.charAt(0);
                    else sp = "";
                    for (var j = 0; j < buf.length; j += 2) {
                        if (j > 0) s += sp;
                        code = parseInt(buf.substr(j, 2), 16);
                        if (def.unittext[0].value.length >= 2) {
                            if (def.unittext[0].value.charAt(1).toUpperCase() == "H") {
                                s += Hex2(code);
                            }
                            else s += code.toString();
                        }
                        else s += code.toString();
                    }
                    vText = s;
                    break;
                case 10:
                    if (def.factor == 0 && len == 4) v = Buf2Float(buf);
                    // console.log("buf",buf);
                default:
                    vText = v;
            }
        }
        if (ri > 0) { rtnStr += ","; }
        rtnStr += vText;
    }   

    // console.log("rtnStr:",rtnStr);
    return rtnStr;
}
function valueText(value, def, para) {
    var factor = def.factor, unit = def.unittext;
    if (unit.length > 1) {  //枚举量
        if (factor == 0) factor = 255;
        value &= factor;
        while ((factor & 1) == 0) { value = value >> 1; factor = factor >> 1; }
        if (para) {
            return value;
        }
        else {
            for (var i = 0; i < unit.length; i++) {
                if (unit[i].key == value) return unit[i].value;
            }
            return "Unknow:" + (value & factor).toString();
        }
    }
    else {
        if (factor != 0) value *= factor;
        var s = "", ut = unit[0].value;
        if (ut.match(/^(YYMD|YYM|MD|HM)$/i)) {
            var l = ut.length;
            s = (value & 0x3F).toFix(2,0,10);
            if (l != 3) s = ((value >> 8) & 0x3F).toFix(2,0,10) + (ut == "HM" ? ":" : "/") + s;
            if (ut.match(/^YY/i)) s = (value >> (l == 4 ? 16 : 8)).toString() + "/" + s;
            if (ut == "YYMD" && s.substr(7, 3) == "/00") s = s.substr(0, 7);
        }
        else if (ut == "YYMDH") {
            var dv = [];
            for (var i = 0; i < 3; i++) {
                dv.push(value & 0x3F);
                value /= 256;
            }
            dv.push(value);
            s = dv[3].toFix(4,0,10) + "/" + dv[2].toFix(2,0,10) + "/" + dv[1].toFix(2,0,10) + " " + dv[0].toFix(2,0,10) + ":00";
        }
        else if (ut.match(/^(hrs|min|s)$/i)) {
            if (para) {
                s = value.toString();
                if (langFlag == 0) {
                    var id = timetxt.indexOf(unit[0].value); //inArray(unit[0].value, timetxt);
                    if (id >= 0) unit[0].value = timetxt[id - 1];
                }
            }
            else {
                if (unit[0].value == "min") value *= 60;
                else if (unit[0].value == "hrs") value *= 3600;
                if (value >= 3600) {
                    s = Math.floor(value / 3600).toFixed(0) + timetxt[langFlag];
                    value %= 3600;
                }
                if (value >= 60) {
                    s = s + Math.floor(value / 60).toFixed(0) + timetxt[2 + langFlag];
                    value %= 60;
                }
                if (value > 0) s += value.toFixed(0) + timetxt[4 + langFlag];
                else if(value == 0 && s=="") s="0"+timetxt[4+langFlag];
            }
        }
        else if (unit[0].value == ":") {    //时间
            var mv = value % 60;
            s = mv.toFix(2,0,10);
            value -= mv;
            value /= 60;
            mv = value % 60;
            s = mv.toFix(2,0,10) + ":" + s;
            value -= mv;
            value /= 60;
            s = value.toFix(2,0,10) + ":" + s;
        }
        else if (factor == 0 && def.ln == 4) {  //浮点数
            if (para) {
                s = ((unit[0].value.match(/_$/) || unit[0].value == "") ? value.toFixed(0) : value.toPrecision(4));
            }
            else if (unit[0].value == "元") {
                var kk = "";
                if (value >= 100000000) {
                    kk = "亿";
                    value /= 100000000;
                }
                else if (value >= 10000) {
                    kk = "万";
                    value /= 10000;
                }
                s = value.toPrecision(4) + kk + unit[0].value;
            }
            else {
                var kk = "";
                if (value >= 10000) {
                    value /= 1000;
                    kk = "k";
                }
                s = ((unit[0].value.match(/_$/) || unit[0].value == "") ? value.toFixed(0) : value.toPrecision(4)) + kk + (ut=="_" ? "" : ut);
                s = s.replace("kk", "M");
            }
        }
        else {  //定点数
            s = value.toFixed(def.dec) + (para ? "" : (/_$/.test(ut)?ut.slice(0,-1):ut));
        }
        return s;
    }
}

function from1363Date(dStr) {
    var vs = "-- ::", s="";
    for (var i = 2; i < dStr.length; i += 2) {
        if (i == 2) s += (parseInt(dStr.substr(0, 4), 16)).toFix(4,0,10);
        else {
            s += vs[i / 2 - 2] + (parseInt(dStr.substr(i, 2), 16)).toFix(2,0,10);
        }
    }
    return  s;
}

function setPara(cid1, name, id){
    var u = eqm.units.unitOfCid1(cid1);
                // console.log("u",u);
    var title;
    var para="";
    if(name=="EqmTime") para=new Date();
    if(name == 0xD3){   //设置高级参数，采用批量方式
        var jcmd ={};
        jcmd.CID1=cid1;
        jcmd.CID2=name;
        jcmd.dat="";
        for(var i=0;i<u.dataitems.length;i++) {
            var d = u.dataitems[i];
            if(d.cid2==0xD2) {
                var pos = d.wpos * 2, len = d.ln * 2;
                while(jcmd.dat.length<pos) jcmd.dat+="  ";
                for(var j=0;j<d.vCode.length;j++){
                    var c = d.vCode[j];
                    if(d.newCode!=undefined) if(d.newCode.length>j) c = d.newCode[j]
                    jcmd.dat = jcmd.dat.slice(0,pos+j7*len) + c + jcmd.dat.slice(pos+(j+1)*len);
                }
            }
        }
        if(jcmd.dat.length<=0) return;
        title=u.caption+"高级参数";
    }
    else {  //设置其它参数采用逐个方式
        for(var i=0;i<u.dataitems.length;i++) {
            var d = u.dataitems[i];
            if(d.name==name) break;
        }
        if(d==undefined) return;
        else {
            var jcmd ={};
            jcmd.CID1=cid1;
            jcmd.CID2=parseInt(d.wcid.slice(2,4),16);
           // console.log("d",d);
            //eqm.diss[d.key].cmd.createCmdData();
            //jcmd.dat=eqm.diss[d.key].cmd.dat;
            switch(d.vtype){
                case 1:
                case 2:
                case 6:
                case 7:
                    const originalStr = d.wcmd.slice(0, d.wpos * 2);
                    jcmd.dat = originalStr.startsWith("FF") ? "01" + originalStr.slice(2) : originalStr;
                                        console.log("jcmd.dat",jcmd.dat);
                    console.log("d.newCode[id]",d.newCode[id],id, Float2Buf(d.newCode[id]));
                    jcmd.dat += d.newCode[id];
                    jcmd.dat += d.wcmd.slice(d.wpos*2+d.ln*2);

                    if (typeof id === "number") {
                        if (d.wcmd.indexOf("##") >= 0) {
                            jcmd.dat = jcmd.dat.replace("##", id.toHex(2)); // 确保替换为2位
                        } else if (id > 0) {
                            const headVal = parseInt(jcmd.dat.slice(0, 2), 16);
                            const newHeadVal = headVal + id;
                            const newHeadHex = newHeadVal.toString(16).padStart(2, "0").toUpperCase();
                            jcmd.dat = newHeadHex + jcmd.dat.slice(2);
                        }
                    }
                    console.log("dat test: ", d.wcmd, d.wpos, d.ln, d.newCode, jcmd.dat);
                    break;
                case 3:
                    jcmd.dat=para.get1363Time();
                    break;
                // case 4:
                //     jcmd.dat=textValue;
                //     break;
                case 16:
                    jcmd.dat=d.newCode[0];
                    break;
            }
        }
        title=d.caption;    
    }
    var s = JSON.stringify(jcmd);

    const data = jcmd.dat;
    //  console.log("id",d.wcmd.slice(d.wpos*2+d.ln*2));
    // console.log("type",d.vtype);
   let setparacmd = getpara(jcmd.CID1,jcmd.CID2,data);
    console.log("设置"+title+"为"+para+"\n"+s);
    SetParaCmd(setparacmd);

}


function getpara(cid1, cid2, data) {
    const head = "1201";
    // 将cid1转为大写十六进制字符串
    const cid1Hex = cid1.toString(16).toUpperCase();
    // 当cid1为4A、E2、E5时len为400C，其他为600A
    const len = getLengthCheck(data.length.toString(16));
    // console.log("len",data.length.toString(16));
    let checknum = 0;
    const CID1 = cid1Hex;       
    const CID2 = cid2.toString(16).toUpperCase();
    const paracmd = head + CID1 + CID2 + len + data;

    checknum = calculateChecksum(paracmd); 

    const fullcmd = '~' + paracmd + checknum + '\r';
    console.log("checknum", checknum);
    console.log("fullcmd", fullcmd);
    return fullcmd;
}

function SetFactoryPara(cid1,cid2,data){
    const head = "2201"
    const CID1 = cid1.toString(16).toUpperCase();
    const CID2 = cid2.toString(16).toUpperCase();
    const len = getLengthCheck(data.length.toString(16));
      console.log("len:",data.length); 
            // console.log("data:",data); 
    var checknum = 0;

    const paracmd = head+CID1+CID2+len+data;

    checknum = calculateChecksum(paracmd);

    const fullcmd = '~'+ paracmd + checknum +'\r'
    console.log("checknum",checknum);
    console.log("fullcmd",fullcmd);
    return fullcmd;
}

//获取十六进制数
function getTextboxValueAsHex(vbox) {
        const value = vbox.value ? vbox.value.trim() : "";

        return value.split('').reduce((result, char) => {
            // 转换为ASCII码，再转为十六进制，确保两位表示
            const hex = char.charCodeAt(0).toString(16).padStart(2, '0').toUpperCase();
            result.push(hex);
            
            return result;
        }, []);

}

// 处理字段为固定字节长度的数字数组
function processFieldToFixedBytes(vbox, targetLength) {
    // 获取处理后的十六进制数组（已去除原始空格）
    const hexArray = getTextboxValueAsHex(vbox);
    const spaceHex = "20";

    const processedArray = [...hexArray];
    
    while (processedArray.length < targetLength) {
        processedArray.push(spaceHex);
    }
    
    if (processedArray.length > targetLength) {
        processedArray.splice(targetLength);
    }

    return processedArray;
}


// 合并所有字段处理后的数据
function getAllMergedData(inputElements, editableRows) {
    if (!inputElements || !editableRows) {
        console.error("缺少必要的参数：inputElements和editableRows");
        return [];
    }
    
    let merged = [];
    
    editableRows.forEach(item => {
        const input = inputElements[item.key];
        if (input) {
            // 处理每个字段并合并到总数组
            const fieldData = processFieldToFixedBytes(input, item.byteLength);
            merged = merged.concat(fieldData);
        }
    });
    
    return merged.join('');
}
