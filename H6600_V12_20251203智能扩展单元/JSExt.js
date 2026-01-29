//JS 内置对象扩展模块
//版本：V0.1
//作者：叶子红
//本模块对js内置对象进行通用扩展，以提供常用功能，缩减重复代码
//本模块中还包括一些通用函数，用以执行一些常见操作
//更新记录：
//V0.0  2022/08/02  首次编制
//V0.1  2022/10/13  增加手动全屏显示的方法和浮点数与字节缓冲区互转方法
//V0.2  2022/11/29  增加将日期时间转换为14位十进制数字的方法

//扩展日期对象方法，获取日期时间从零时开始的分钟数
Date.prototype.getMinute = function() {
    return this.getHours()*60 + this.getMinutes();
}
//扩展日期对象方法，获取8位数字格式的日期代码
Date.prototype.getDateText = function() {
    return this.getFullYear().toFix(4,0)+(this.getMonth()+1).toFix(2,0)+this.getDate().toFix(2,0);
}
//扩展日期对象方法，将日期转为为14位十进制数
Date.prototype.getDateTime = function() {
    return this.getFullYear().toFix(4,0)+(this.getMonth()+1).toFix(2,0)+this.getDate().toFix(2,0)+this.getHours().toFix(2,0)+this.getMinutes().toFix(2,0)+this.getSeconds().toFix(2,0);
}
//扩展日期对象方法，获取1363格式的日期时间
Date.prototype.get1363Time = function() {
    return this.getFullYear().toHex(4)+(this.getMonth()+1).toHex(2)+this.getDate().toHex(2)+this.getHours().toHex(2)+this.getMinutes().toHex(2)+this.getSeconds().toHex(2);
}
//扩展数字对象方法，将数值转为为固定长度的字符串，具有最小总位数和指定小数位数，可以指定进位制
Number.prototype.toFix = function(len, dec, radix) {
    if(len==undefined) len=2;
    if(dec==undefined) dec=0;
    if(radix==undefined) radix=10;
    var s = this.toString(radix).toUpperCase().split(".");
    while(s[0].length<len-dec) {s[0]="0"+s[0]}
    if(s.length>1 && dec>0){
        while(s[1].length<dec) {s[1]+="0"}
        if(s[1].length>dec) s[1]=s[1].slice(0,dec);
    }
    else if(dec>0) {
        s.push("");
        while(s[1].length<dec) s[1]+="0";
    }
    else return s[0];
    return s[0]+"."+s[1];
}
Number.prototype.toHex = function(len) {return this.toFix(len,0,16);}
//扩展字符串对象方法，将文本插入到字符串的指定位置，替换指定数量的字符
String.prototype.replaceAt = function (pos, count, txt) {
    return this.slice(0,pos)+txt+this.slice(pos+count);
}
//扩展字符串对象方法，在多语言字符串中找出当前语言字符串
//多语言字符串用下划线字符分隔多个语言的相同文本，并按照语言编码顺序排列，位于尾端的下划线字符总是返回
String.prototype.localText = function() {
    var t = "";
    var txt = this;
    if (txt.match(/_$/)) {
        txt = txt.slice(0, txt.length - 1);
        t = "_";
    }
    var s = txt.split("_");
    if (langFlag < s.length) return s[langFlag] + t;
    else if (s.length >= 1) return s[0] + t;
    else return "";
}

//扩展DOM元素对象方法，移动元素的位置、设置文本、对齐、颜色、背景，显示或隐藏元素
Element.prototype.moveH = function(left, right, width) {
    if(typeof left == "string") this.style.left = left;
    if(typeof right == "string") this.style.right = right;
    if(typeof width == "string") this.style.width = width;
    return this;
}
Element.prototype.moveV = function(top, bottom, height) {
    if(typeof top == "string") this.style.top = top;
    if(typeof bottom == "string") this.style.bottom = bottom;
    if(typeof height == "string") this.style.height = height;
    return this;
}
Element.prototype.text = function(s) {
    this.innerText=s;
    return this;
}
Element.prototype.txtAlign = function(a) {
    this.style.textAlign=a;
    return this;
}
Element.prototype.color = function(color) {
    this.style.color=color;
    return this;
}
Element.prototype.back = function(bg) {
    this.style.background=bg;
    return this;
}
Element.prototype.show = function() {this.style.display="";}
Element.prototype.hide = function() {this.style.display="none";}
Element.prototype.clear = function() {this.innerHTML="";}
//为元素添加子元素，可以指定元素名称、类、Id以及元素内的文本
Element.prototype.addChild = function(tag, cl, id, txt) {
    var e = document.createElement(tag);
    if(typeof cl == "string" && cl.length>0) e.className=cl;
    if(typeof id == "string" && id.length>0) e.id=id;
    if(typeof txt == "string" || typeof txt == "number") e.innerText=txt;
    this.appendChild(e);
    return e;
}
//为元素添加按钮
Element.prototype.button = function(text, callback, id) {
    var e = this.addChild("span","button",id,text);
    e.onclick = callback;
    return this;
}
//将元素指定为可编辑框
Element.prototype.editable = function(){
    this.addEventListener("mouseenter", function(){
        this.style.border="1px solid white";
        this.style.background="black";
        this.contentEditable="true";    
    })
    this.addEventListener("mouseleave", function(){
        this.style.border="";
        this.style.background="";
        this.contentEditable="false";    
    })
    return this;
}
 //设定表格行的列数
Element.prototype.setCols = function(num, align) {
    if(this.tagName=="TR"){
        while(num>this.cells.length) {
            this.insertCell().txtAlign(typeof(align)=="undefined"?(this.cells.length==0?"left":"right"):align)
            .text(typeof(this.label)=="undefined"?"":this.label.replace("[1]",this.cells.length-1));
        }
        while(num<this.cells.length) this.deleteCell(this.cells.length-1);
    }
    else if(this.tagName="TABLE") {
        for(var i=0;i<this.rows.length;i++) {this.rows[i].setCols(num, align)}
    }
}
//为表格添加新行，如果不指定列数，新行采用原来最后一行的列数
Element.prototype.addRows = function(num, cols) {
    if(this.tagName=="TABLE"){
        var count = typeof(cols)=="number" ? cols : (this.rows.length>0 ? this.rows[this.rows.length-1].cells.length : 1); 
        for(var i=0;i<num;i++){
            var r = this.insertRow();
            r.className = "list";
            for(var j=0;j<count;j++) r.insertCell();
        }
    }
}
//一下为通用函数
//根据选择器获取DOM元素，给定Id则返回具体元素，否则返回一组符合条件的元素
function el(selector) {
    if(selector.charAt(0)=="#") return document.getElementById(selector.substr(1));
    else if(selector.charAt(0)==".") return document.getElementsByClassName(selector.substr(1));
    else if(!(/[\.#:\s]/.test(selector))) return document.getElementsByTagName(selector.substr(1));
    else return document.querySelectorAll(selector);
}
//设置全屏显示的方法，仅可手动触发
function setFullscreen() {
    if(document.documentElement.requestFullscreen) {document.documentElement.requestFullscreen();}
    else if(document.documentElement.mozRequestFullscreen) {document.documentElement.mozRequestFullscreen();}
    else if(document.documentElement.webkitRequestFullscreen) {document.documentElement.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);}
}

//以下代码用于在字节数组和32位浮点数之间相互转换
var buffer = new ArrayBuffer(4);    
var f32 = new Float32Array(buffer); // [0]
var ui8 = new Uint8Array(buffer);   // [0, 0, 0, 0]
function Buf2Float(buf) {    //从数据包中读取一个浮点数
    for (var i=0; i < 4; i++) ui8[i] = parseInt(buf.substr(i * 2, 2), 16);
    return f32[0];
}
function Float2Buf(v32) {
    var buf="";
    f32[0]=v32;
    for (var i=0; i<4; i++) buf += ui8[i].toHex();
    return buf;
}
