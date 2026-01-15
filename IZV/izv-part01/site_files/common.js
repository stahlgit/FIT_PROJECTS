
//browser object
var browser=new Object();
browser.isIE=(navigator.appName=="Microsoft Internet Explorer" && navigator.userAgent.indexOf("Opera")<0)?true:false;
browser.isOpera=navigator.userAgent.indexOf("Opera")>0?true:false;
browser.isMozilla=navigator.product=="Gecko"?true:false;

//opens debug window with content
function debugWindow(content){
	var win=window.open("","debug","width=450,height=450,resizable=yes,scrollbars=yes");
	win.document.title="debug";
	win.document.body.innerHTML="<pre>"+content+"</pre>";
}

//returns element
function getEl(elid){
	return document.getElementById(elid);
}

//returns viewport height
function winH(){
	if(window.innerHeight)return parseInt(window.innerHeight);
	else if(document.documentElement&&document.documentElement.clientHeight)return parseInt(document.documentElement.clientHeight);
	else if(document.body&&document.body.clientHeight)return parseInt(document.body.clientHeight);
}

//returns viewport width
function winW(){
	if(window.innerWidth)return parseInt(window.innerWidth);
	else if(document.documentElement&&document.documentElement.clientWidth)return parseInt(document.documentElement.clientWidth);
	else if(document.body&&document.body.clientWidth)return parseInt(document.body.clientWidth);
}

//returns computed height of el with id elid
function getH(elid){
	if (document.all)pupik=document.all.item(elid).offsetHeight;
	else{
		obj=document.getElementById(elid);
		pupik=document.defaultView.getComputedStyle(obj,"").getPropertyValue("height");
	};
	return parseInt(pupik);
}

//returns computed width of el with id elid
function getW(elid){
	if (document.all)pupik=document.all.item(elid).offsetWidth;
	else{
		obj=document.getElementById(elid);
		pupik=document.defaultView.getComputedStyle(obj,"").getPropertyValue("width");
	};
	return parseInt(pupik);
}

//returns computed height of el given as object
function getHObj(obj){
/*
	if (document.all) pupik=obj.offsetHeight;
	else{
		pupik=document.defaultView.getComputedStyle(obj,"").getPropertyValue("height");
	};
*/	
	pupik=obj.offsetHeight;
	return parseInt(pupik);
}

//returns computed width of el given as object
function getWObj(obj){
/*
	if (document.all)pupik=obj.offsetWidth;
	else{
		pupik=document.defaultView.getComputedStyle(obj,"").getPropertyValue("width");
	};
*/	
	pupik=obj.offsetWidth;
	return parseInt(pupik);
}

//finds x position of an element with id elid
function findPosX(elid){
	var obj=getEl(elid);
	var curleft=0;
	if(obj.offsetParent){
		while(obj.offsetParent){
			curleft+=obj.offsetLeft;
			obj=obj.offsetParent;
		}
	}else if(obj.x)curleft+=obj.x;
	return parseInt(curleft);
}

//finds y position of an element with id elid
function findPosY(elid){
	var obj=getEl(elid);
	var curtop=0;
	if(obj.offsetParent){
		while(obj.offsetParent){
			curtop+=obj.offsetTop
			obj=obj.offsetParent;
		}
	}else if(obj.y)curtop+=obj.y;
	return parseInt(curtop);
}

//finds x position of an element given as object
function findPosXObj(obj){
	var curleft=0;
	if(obj.offsetParent){
		while(obj.offsetParent){
			curleft+=obj.offsetLeft;
			obj=obj.offsetParent;
		}
	}else if(obj.x)curleft+=obj.x;
	return parseInt(curleft);
}

//finds y position of an element given as object
function findPosYObj(obj){
	var curtop=0;
	if(obj.offsetParent){
		while(obj.offsetParent){
			curtop+=obj.offsetTop
			obj=obj.offsetParent;
		}
	}else if(obj.y)curtop+=obj.y;
	return parseInt(curtop);
}

//returns array of elements with class name clsname inherent in element with id parel
function getElementsByClassName(parel,clsname){
	var outp=new Array();
	var els=getEl(parel).getElementsByTagName("div");
	for(var i=0;i<els.length;i++){
		if(els[i].className==clsname)outp[outp.length]=els[i];
	};
	return outp;
}

//clips an element with id elid (left, top, right, bottom)
function clipEl(elid,l,t,r,b){
	getEl(elid).style.clip="rect("+t+"px,"+(getW(elid)-r)+"px,"+(getH(elid)-b)+"px,"+l+"px)";
}

//sets display none for an element with id elid 
function setDN(elid){
	getEl(elid).style.display="none";
}

//sets display block for an element with id elid
function setDB(elid){
	getEl(elid).style.display="block";
}

//sets display inline for an element with id elid
function setDI(elid){
	getEl(elid).style.display="inline";
}

//sets visibility hidden for an element with id elid
function setVH(elid){
	getEl(elid).style.visibility="hidden";
}

//sets visibility visible for an element with id elid
function setVV(elid){
	getEl(elid).style.visibility="visible";
}

//clips an element given as object 
function clipElObj(el,l,t,r,b){
	el.style.clip="rect("+t+"px,"+(getW(elid)-r)+"px,"+(getH(elid)-b)+"px,"+l+"px)";
}

//sets display none for an element el
function setDNObj(el){
	el.style.display="none";
}

//sets display block for an element el
function setDBObj(el){
	el.style.display="block";
}

//sets display inline for an element el
function setDIObj(el){
	el.style.display="inline";
}

//sets visiblity hidden for an element el
function setVHObj(el){
	el.style.visibility="hidden";
}

//sets visiblity visible for an element el
function setVVObj(el){
	el.style.visibility="visible";
}

//php trim() analogue
function trim(inpstr){
	inpstr=inpstr.replace(/^[\s]+/g,"");
	inpstr=inpstr.replace(/[\s]+$/g,"");
	return inpstr;
}

//opens window with image
function picWin(im,wi,he,titl,mess,winpars){
	var wL = (screen.width-wi) / 2;
	var wT = (screen.height-he) / 2;
	var wp;
	var img=new Image();
	img.src=im;
	wp="width="+wi+",height="+he+",left="+wL+",top="+wT+(typeof(winpars)=="undefined"?"":winpars);
	var vokno=window.open("","",wp);
	vokno.document.open();
	vokno.document.write('<html><head><title>'+(typeof(titl)=="undefined"?"":titl)+'</title><meta http-equiv="content-type" content="text/html; charset=iso-8859-2"></head><body style="margin:0;padding:0;background:#fff"><img src="'+im+'" alt="'+(typeof(mess)=="undefined"?"Kliknutim zavrete okno":mess)+'" onclick="window.close()" style="display:block;cursor:pointer;cursor:hand"></body></html>');
	vokno.document.close();
	return typeof(vokno)=="object"?true:false;
}

//opens window with image
function picWin2(im,wi,he,php,parname,winpars){
	if(typeof(php)=="undefined" || php=="" || php==null)php="image.php";
	if(typeof(parname)=="undefined" || parname=="" || parname==null)parname="im";
	var wp="width="+wi+",height="+he;
	if(typeof(winpars)!="undefined")wp+=","+winpars;
	var vokno=window.open(php+"?"+parname+"="+im,"",wp);
	return typeof(vokno)=="object"?true:false;
}

//kills teckos
function killTeckos(){
	window.focus();
}

//swaps display property of an element with elid - none->block / block->none
function swapDisplay(elid){
	el=getEl(elid);
	el.style.display=el.style.display=="none"?"block":"none";
}

//swaps display property of an element el - none->block / block->none
function swapDisplayObj(el){
	el.style.display=el.style.display=="none"?"block":"none";
}

// open anchor in new window
function targetBlank(e){
	if(browser.isIE)el=event.srcElement; else el=e.target;
	while(el.tagName.toLowerCase()!="a")el=el.parentNode;
	var hrf=el.href;
	var newwin=window.open(hrf);
	return typeof(newwin)=="object"?false:true;
}

// select anchors with class=targetblank
function addTB(){
	var lnks=document.getElementsByTagName("a");
	for(var i=0;i<lnks.length;i++){
		if(/\ ?targetblank\ ?/.test(lnks[i].className)){
			lnks[i].onclick=targetBlank;
			lnks[i].title="Adresa "+lnks[i].href+" bude otevrena v novem okne";
		}
	}
}
