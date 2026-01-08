//GUI模块
//版本：V0.0
//作者：叶子红
//本模块提供一些通用界面元素，用于构建风格一致的用户界面
//本模块中还包括一些通用函数
//本模块采用屏幕坐标系，左上角为原点
//所有界面元素都基于父元素的相对坐标
//更新记录：
//V0.0  2022/10/26  首次编制
//V0.1  2022/10/31  增加星空效果
//V0.2  2022/12/08  增加流水线效果

//创建直线段
//x1,y1和x2,y2分别为直线的两个端点，用距离父元素左上角百分数表示
//color为线段的颜色
//arrow表示端点处是否绘制箭头，0表示无箭头，1表示x1，y1处有箭头，2表示x2，y2处有箭头，3表示两端均有箭头
//arrow大于3值表示在线段中间有一个箭头，arrow的值表示箭头的位置与X1,Y1的距离占线段总长的比例,arrow为负值表示箭头指向x1,y1
const directionAnimations = ["goRight","goDown","goLeft","goUp"];
function Line(x1,y1,x2,y2,color,arrow,run) {
    for(var i=0;i<arguments.length;i++){
        var v = arguments[i];
        if(/%$/.test(v)) arguments[i]=Number(v.slice(0,v.length-1));
    }
    var left = x1<x2 ? x1 : x2;
    var width = x1<x2 ? x2-x1 : x1-x2;
    var top = y1<y2 ? y1 : y2;
    var height = y1<y2 ? y2-y1 : y1-y2;
    var e = document.createElement("div");
    e.lineColor=color;
    e.run=run;
    e.style.position="absolute";
    e.style.border="2px solid "+color;
    e.style.left=left+"%" ;
    e.style.top=top+"%";
    if(x1==x2) {
        e.style.width="0px";
        e.style.height=height+"%";
    }
    else if(y1==y2) {
        e.style.width=width+"%";
        e.style.height="0px";
    }
    else return;
    if(arrow==1 || arrow==3){
        var ar1 = e.addChild("span","arrow");
        if(x1<x2) {ar1.style.borderRightColor=color;}
        else if(x1>x2) {ar1.style.borderLeftColor=color;ar1.style.left="calc(100% - 10px)";}
        else if(y1<y2) {ar1.style.borderBottomColor=color;}
        else if(y1>y2) {ar1.style.borderTopColor=color;ar1.style.top="calc(100% - 10px)";}
        e.ar1=ar1;
    }
    if(arrow==2 || arrow==3){
        var ar2 = e.addChild("span","arrow");
        if(x2<x1) {ar2.style.borderRightColor=color;}
        else if(x2>x1) {ar2.style.borderLeftColor=color;ar2.style.left="calc(100% - 10px)";}
        else if(y2<y1) ar2.style.borderBottomColor=color;
        else if(y2>y1) {ar2.style.borderTopColor=color;ar2.style.top="calc(100% - 10px)";}
        e.ar2=ar2;
    }
    if(arrow>3 || arrow<-3) {
        var ar1 = e.addChild("span","arrow");
        var d=-1;
        if(arrow>0){
            if(x1<x2) {
                ar1.style.borderLeftColor=color;ar1.style.left="calc("+arrow+"% - 10px)";
                d=0;
            }
            else if(x1>x2) {
                ar1.style.borderRightColor=color;ar1.style.left="calc(100% -"+arrow+"% - 10px)";
                d=2;
            }
            else if(y1<y2) {
                ar1.style.borderTopColor=color;ar1.style.top="calc("+arrow+"% - 10px)";
                d=1;
            }
            else if(y1>y2) {
                ar1.style.borderBottomColor=color;ar1.style.top="calc(100% -"+arrow+"% - 10px)";
                d=3;
            }
        }
        else {
            arrow=-arrow;
            if(x1<x2) {
                ar1.style.borderRightColor=color;ar1.style.left="calc("+arrow+"% - 10px)";
                d=2;
            }
            else if(x1>x2) {
                ar1.style.borderLeftColor=color;ar1.style.left="calc(100% -"+arrow+"% - 10px)";
                d=0;
            }
            else if(y1<y2) {
                ar1.style.borderBottomColor=color;ar1.style.top="calc("+arrow+"% - 10px)";
                d=3;
            }
            else if(y1>y2) {
                ar1.style.borderTopColor=color;ar1.style.top="calc(100% -"+arrow+"% - 10px)";
                d=1;
            }
        }
        e.direct = d;
        e.ar1=ar1;
        if(run){
            if(d>=0) {
                ar1.style.animation=directionAnimations[d]+" 2s linear infinite";
            }
        }
    }
    e.backword = function(flag) {
        var d = this.direct;
        if(flag) d = (d + 2) % 4;
        switch(d){
            case 0:
                this.ar1.style.borderColor="transparent transparent transparent " + this.lineColor;
                break;
            case 1:
                this.ar1.style.borderColor=this.lineColor + " transparent transparent transparent";
                break;
            case 2:
                this.ar1.style.borderColor="transparent " + this.lineColor + " transparent transparent";
                break;
            case 3:
                this.ar1.style.borderColor="transparent transparent " + this.lineColor + " transparent";
                break;
        }
        if(this.run) {
            this.ar1.style.animationName=directionAnimations[d];
        }
    }
    return e;
}
//流水线生成器
//参数x1,y1,x2,y2：直线段的两个端点
//参数color：流水线颜色
//参数flow：流水方向，1正向（向右或向下），-1逆向（向左或向上），0不流动
function FlowLine(x1,y1,x2,y2,color,flow) {
    for(var i=0;i<arguments.length;i++){
        var v = arguments[i];
        if(/%$/.test(v)) arguments[i]=Number(v.slice(0,v.length-1));
    }
    var left = x1<x2 ? x1 : x2;
    var right = x1>x2 ? x1 : x2;
    var width = right - left;
    var top = y1<y2 ? y1 : y2;
    var bottom = y1>y2 ? y1 : y2;
    var height = bottom - top;
    var e = document.createElement("div");
    e.moveH("calc("+left+"% - 3px)",null,"calc("+width+"% + 6px)").moveV("calc("+top+"% - 3px)",null,"calc("+height+"% + 6px)");
    e.className="flowLine";
    e.style.borderRadius="3px";
    e.backword=flow;
    if(height==0) {
        var e1 = e.addChild("span","flowLine").back(color).moveH(0,null,"40em").moveV(0,null,"6px");
        if(flow!=0){
            var e2 = e1.addChild("span","flowLineRight");
            e2.style.webkitAnimation=(flow<0?"flowLeft":"flowRight")+" 2s linear infinite";
            e2.style.animation=(flow<0?"flowLeft":"flowRight")+" 2s linear infinite";
            e.goBack = function(x) {    //x>0正向流动，x<0反向流动，x=0不流动
                var y = 0;
                if(x>0) y=1;
                else if(x<0) y=-1;
                if(this.backword!=0) {
                    if(this.backword<0) y=-y;
                    this.firstChild.firstChild.style.WebkitAnimationName = ["flowLeft","","flowRight"][y+1];    
                    this.firstChild.firstChild.style.animationName = ["flowLeft","","flowRight"][y+1];    
                }
            }    
        } 
    }
    else if(width==0) {
        var e1 = e.addChild("span","flowLine").back(color).moveH(0,null,"6px").moveV(0,null,"40em");
        if(flow!=0){
            var e2 = e1.addChild("span","flowLineDown");
            e2.style.webkitAnimation=(flow<0?"flowUp":"flowDown")+" 2s linear infinite";
            e2.style.animation=(flow<0?"flowUp":"flowDown")+" 2s linear infinite";
            e.goBack = function(x) {
                if(this.backword!=0) {
                    var y = 0;
                    if(x>0) y=1;
                    else if(x<0) y=-1;
                    if(this.bakcword<0) y=!y;
                    this.firstChild.firstChild.style.webkitAnimationName= ["flowUp","","flowDown"][y+1];
                    this.firstChild.firstChild.style.animationName= ["flowUp","","flowDown"][y+1];
                }
            }    
        }
    }
    return e;
}
//在指定元素内创建一个星星
function createStar(e,left,top) {
    var lines=[]
    for(var i=0;i<3;i++){
        lines.push(e.addChild("span","star").moveH(left).moveV(top));
        lines[i].style.webkitTransform="rotate("+i*120+"deg)";
        lines[i].style.transform="rotate("+i*120+"deg)";
    }
    lines.move = function(left,top,size) {
        var t = Math.random();
        for(var i=0;i<3;i++){
            this[i].moveH(left,null,size).moveV(top);
            this[i].style.webkitAnimationDelay=t+"s";
            this[i].style.animationDelay=t+"s";
        }
    }
    lines.rotate = function(angle) {
        for(var i=0;i<3;i++) {
            this[i].style.webkitTransform="rotate(" + (angle + i * 120) + "deg)";
            this[i].style.transform="rotate(" + (angle + i * 120) + "deg)";
        }
    }
    lines.delay = function(t) {
        for(var i=0;i<3;i++) {
            this[i].style.webkitAnimationDelay=t;
            this[i].style.animationDelay=t;
        }
    }
    lines.shown=true;
    lines.show = function() {
        for(var i=0;i<3;i++) this[i].show();
        this.shown=true;
    }
    lines.hide = function() {
        for(var i=0;i<3;i++) this[i].hide();
        this.shown=false;
    }
    return lines;
}
//在指定元素内创建星空
function createStars(e,num) {
    var stars=[];
    for(var i=0;i<num;i++) stars.push(createStar(e,"10%","10%"));
    //定时随机改变星星的位置、大小和角度
    setInterval(function() {
        var x = parseInt(Math.random() * num);
        var d = new Date().getMinute();
        if(d>300 && d<1140) {
            if(stars[x].shown) stars[x].hide();
        }
        else{
            if(!stars[x].show) stars[x].show();
        }
        if(stars[x].shown){
            stars[x].move((Math.random()*90 + 5) + "%", (Math.random()*90 +5) + "%", (Math.random()*20 +2) + "px");
            stars[x].rotate(120 * Math.random());
            stars[x].delay(Math.random()*8+"s");        
        }
},3000);
}
