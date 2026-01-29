//数据库模块
//版本 V0.1
//作者：叶子红
//本模块包含用于表示设备及其内部数据的所有对象以及其本地存储方法
//更新记录：
//V0.0  2022/08/03  首次编制
//V0.1  2022/10/08  更正读取电能记录函数中的错误
//V0.2  2022/12/14  日记录和月记录都包含当日的值

//数据记录点，记录一项数据在一个采集周期内的最大值和最小值、最新值，该类没有方法，以便于存储
function DataLogPoint() {
    this.Max = 0;
    this.Min = 1;
    this.Last = 0;
}
function setPointValue (point, value) {
    if (!isNaN(value)) {
        if (point.Max < point.Min) {
            point.Max = value;
            point.Min = value;
        }
        else if (value > point.Max) {
            point.Max = value;
        }
        else if (value < point.Min) {
            point.Min = value;
        }
        point.Last = value;
    }
};

//数据记录，包含一项数据一天的1440个数据点，即每分钟一个数据点，该类没有方法，以便于存储
function DatalogSet(name, caption, unit) {
    this.start = 0;
    this.end = -1;
    this.completed = -1;
    this.points = [];
    this.name = name;
    this.caption = caption;
    this.unit = unit;
}
function getLastValue(datalog){
    return datalog.points[datalog.end].Last.toFix(2,0,10);
}
//数据集，包含一组数据的历史记录
function DataSets() {
    var d = new Date();
    this.logDate = d.toLocaleDateString();// 记录当前日期
    this.datalogs = []; // 存储所有数据集的数组（每个元素是一个DatalogSet对象） 
     // 检查本地存储的日期是否与当前日期一致
    if (window.localStorage.getItem("logDate") != this.logDate) {
        window.localStorage.setItem("logDate", this.logDate);
    }
    else {
        for (var i=0;i<eqm.realtimeChartDatas.length;i++) {
            var d = eqm.realtimeChartDatas[i];// 从配置中获取数据集信息（如名称、标题）
            var x = window.localStorage.getItem(d.name);
            if (x == undefined || x == "") break;
            this.datalogs.push(JSON.parse(x));
            this.datalogs[this.datalogs.length-1].completed = -1;
        }
    }
    this.addData = function(name, caption, unit) {
        // 检查数据集是否已存在（通过name判断）
        if(this.datalogs.some(function (x) {return x.name==name;}));
        else{
            var dat = new DatalogSet(name, caption, unit);
            this.datalogs.push(dat);    
        }
    };
    this.datalog = function(name) {
        for(var i=0;i<this.datalogs.length;i++) {
            var x = this.datalogs[i];
            if(x.name==name) return x;
        }
    };
    this.setValue = function(name, value, tid) {
        var d = new Date();
        var id = typeof(tid)=="number" ? tid : d.getMinute();
        //跨日
        if (window.localStorage.getItem("logDate") != d.toLocaleDateString()) {
            for (var i=0;i<this.datalogs.length;i++) this.datalogs[i].points=[]; //清除昨日数据
            window.localStorage.setItem("logDate", d.toLocaleDateString());
        }
        var datalog = this.datalog(name);
        if (datalog.points[id] == undefined) {
            datalog.points[id] = new DataLogPoint;
        }
        setPointValue(datalog.points[id],value);
        if (datalog.end < datalog.start) datalog.start = id;
        datalog.end = id;
        window.localStorage.setItem(name, JSON.stringify(datalog));
    };
}
//创建数据库
function createDatabase(){
    var db = new DataSets();
    for(var i=0;i<eqm.realtimeChartDatas.length;i++) {
        var d = eqm.realtimeChartDatas[i];
        db.addData(d.name, d.caption, d.unitText);
    }
    return db;
}
//保存电能记录，d为记录日期，省略则为当日。每日的最后一次记录会被保留下来。
function saveEnergyLogs(d, values){
    if(d==undefined) d = new Date();
    window.localStorage.setItem("E"+ d.getDateText(), values);  //更新当日电量读数
    window.localStorage.setItem("E"+ d.getDateText().slice(0,6), values);   //更新当月电量读数
}
//读取电能记录，d指定搜索日期，m=true为月数据，缺省或否为日数据
function readEnergyLogs(d, m){
    var logs = [];
    var eLogs ={};
    var sD = d;
    if(m) {
        sD.setMonth(sD.getMonth()-12,1);   //从指定日期之前12个月的1日开始搜索
        for(var i=0;i<13;i++) {
            if(i==0) eLogs.start=new Date(sD.getTime());    //记下开始日期
            var log = window.localStorage.getItem("E"+ sD.getDateText().slice(0,6));
            if(i==0 && log==undefined) i--; //未找到开头
            else if(log!=undefined) logs[i]=log.split(",").map(function (v) {return Number(v);});
            sD.setMonth(sD.getMonth()+1);   //下个月1日
            if(sD > new Date()) break;
        }
    }
    else {
        sD.setDate(sD.getDate()-31);    //从指定日期前31天开始搜索
        for(var i=0;i<32;i++) {
            if(i==0) eLogs.start=new Date(sD.getTime());    //记下开始日期
            var log = window.localStorage.getItem("E"+ sD.getDateText());
            if(i==0 && log==undefined) i--; //未找到开头
            else if(log!=undefined) logs[i]=log.split(",").map(function (v) {return Number(v);});
            sD.setDate(sD.getDate()+1);  //下一天
            if(sD > new Date()) break;
        }
    }
    if(logs.length>1){  //超过一个记录,计算相邻记录的差值作为后一记录时段的电量
        for(var i=logs.length-1;i>0;i--) {
            if(logs[i]!=undefined){
                var k=i-1;
                while(logs[k]==undefined && k>0) {k--;}
                if(logs[k]!=undefined) {
                    for(var j=0;j<logs[0].length;j++) {
                        logs[i][j]=logs[i][j]-logs[k][j];
                    }    
                }
            }
        }
        if(m) {
            while(logs.length>12) {   //只返回最近12个月的记录
                eLogs.start.setMonth(eLogs.start.getMonth()+1);
                logs.shift();
            }
        } 
        else {
            while(logs.length>31) {   //只返回最近31天的记录
                eLogs.start.setDate(eLogs.start.getDate()+1);
                logs.shift();
            }
        } 
    }
    eLogs.log=logs;
    return eLogs;
}
