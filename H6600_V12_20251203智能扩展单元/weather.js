//天气预报模块
//版本：V0.0
//作者：叶子红
//本模块在后台运行，负责获取天气数据
//更新记录：
//V0.0  2022/11/11  首次编制

var wServer = "https://v0.yiketianqi.com/api?unescape=1&version=v62&appid=43656176&appsecret=I42og6Lm&ext=";
var wKeys = [
    ["wea","现在","",0,1],
    ["wea_day","白天","",1,1],
    ["wea_night","夜间","",2,1],
    ["tem","气温","℃",0,3],
    ["tem1","最高","℃",1,3],
    ["tem2","最低","℃",2,3],
    ["humidity","湿度","",0,5],
    ["visibility","能见度","",1,5],
    ["pressure","气压","hPa",2,5],
    ["win","风向","",0,7],
    ["win_speed","风力","",1,7],
    ["win_meter","风速","",2,7],
    ["air","空气质量","",0,9],
    ["air_pm25","PM2.5","",1,9],
    ["air_level","污染等级","",2,9],
    ["sunrise","日出","",1,11],
    ["sunset","日落","",2,11],
    ["air_tips","空气提示","",3,1],
    ["alarm","警报","",4,1]
];
var whKeys = [
    ["hours",""],
    ["wea","天气"],
    ["tem","气温","℃"],
    ["win","风向"],
    ["win_speed","风力"],
    ["aqi","空气"]
];
var weather;
var hourTimer;
var xmlRq;

function getWeather() {
    if (window.XMLHttpRequest) {
        xmlRq = new XMLHttpRequest();
    } 
    else {
        xmlRq = new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlRq.onreadystatechange = function () { if (this.readyState == 4 && this.status==200) { 
        var d = this.responseText;
        if(d!=undefined) decodeWeather(d);
    }};
    reqWeather();
}

function reqWeather() {
    if(hourTimer!=undefined) clearTimeout(hourTimer);
    xmlRq.open("GET",wServer,true);
    xmlRq.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xmlRq.send("");    
    hourTimer=setTimeout(reqWeather, 3600000);
}

function decodeWeather(jsonDat) {
    var wd = JSON.parse(jsonDat);
    if(wd.hours!=undefined){
        weather=wd;
    }
}

function weatherText(hour,item) {
    if(hour>=0) {
        var key = whKeys[item][0];
        var unit = whKeys[item][2];
        var rtn = weather.hours[hour][key];
    }
    else{
        var key = wKeys[item][0];
        var unit = wKeys[item][2];
        var rtn = weather[key];
    }
    if(rtn!=undefined) {
        if(rtn.constructor===Array) {rtn = rtn.join("<br>");}
        if(key=="alarm") {
            var s="";
            s += rtn.alarm_type;
            s += " "+rtn.alarm_level+"预警";
            s += "<br>"+rtn.alarm_content;
            if(rtn.alarm_content.length==0) s="近日本地天气稳定，无天气警报";
            rtn = s;
        }
    if(unit!=undefined && rtn.length>0) rtn+=unit;
        return rtn;
    }
    else return "";
}