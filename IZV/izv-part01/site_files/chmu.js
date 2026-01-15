window.onload = function() {
	initNav();

	// Vlozeni vizitek
	vlozVizitky();

        // Cookie
	/** cookieConsent(); */
}


var timers=new Array();
var menuFall=50;
function initNav() {
	var menuElements=getEl('mnu').getElementsByTagName("ul");
	for(var i=0;i<menuElements.length;i++) {
		if(browser.isIE) {
			menuElements[i].onmouseover=function() {
				if(timers[this.parentNode.id]) clearTimeout(timers[this.parentNode.id]);
					if(this.parentNode.parentNode.parentNode.id=="mnu") {
						this.parentNode.parentNode.firstChild.style.color="#fff";
						this.parentNode.parentNode.firstChild.lastChild.style.backgroundPosition="0px -32px";
					} else {
						this.parentNode.parentNode.firstChild.style.color="#1078b9";
						this.parentNode.parentNode.firstChild.style.backgroundColor="#98c8e8";
						this.parentNode.parentNode.firstChild.style.fontWeight="bold";
					}
			}
			
			menuElements[i].parentNode.parentNode.onmouseover=function() {
				if(timers[this.lastChild.id]) clearTimeout(timers[this.lastChild.id]);							
				setVVObj(this.lastChild);
			}

			menuElements[i].parentNode.parentNode.onmouseout=function() {
				timers[this.lastChild.id]=setTimeout('setVH("'+this.lastChild.id+'");var liEl=getEl("'+this.id+'");liEl.firstChild.style.color="#fff";liEl.firstChild.style.fontWeight="normal";liEl.firstChild.style.backgroundColor="#1078b9";if(liEl.parentNode.id=="mnu")liEl.firstChild.lastChild.style.backgroundPosition="0px 0px";',menuFall);
			}

		}
	}
}

function toggleTree(liNode){
  var cls = liNode.className;
  if ( cls ){
       liNode.className = ( cls == 'expanded' ? 'collapsed' : 'expanded' );
  }
}


// Vlozeni vizitek

VIZITKY_DATA_FILE="/portal/chmu/vizitky.txt"
VIZITKY_PREFIX='vizitka_';
VIZITKY_PREFIX_LENGTH=VIZITKY_PREFIX.length;
ROLE_VIZITKA_SEPARATOR='=';

// Vizitky zamestnancu
function vlozVizitky() {
	
	// Nalezeni vsech elementu, ktere jsou vizitkami
	var vizitkyElems = findVizitkyElements();
	
	var vizitkyNum = vizitkyElems.length;
	//alert("Pocet vizitek k nahrazeni: " + vizitkyNum);
	
	if (vizitkyNum == 0) {
		return;
	}
	
	loadAndTranslateVizitky(translateVizitky, vizitkyElems);

}            

//Nalezeni vsech elementu, ktere jsou vizitkami
function findVizitkyElements() {
	var spanElems = document.getElementsByTagName('span');
	var vizitkyElems = new Array();
	var i;
	for (i in spanElems) {
		// Element muze mit vice trid
		if ((' ' + spanElems[i].className + ' ').indexOf(' ' + VIZITKY_PREFIX) > -1) {
			vizitkyElems.push(spanElems[i]);
		}
	}
	return vizitkyElems;
}

//Nacteni souboru s vizitkami a spusteni handleru na jejich prelozeni
function loadAndTranslateVizitky(handler, vizitkyElems) {
	var xmlhttp; 
	if (window.XMLHttpRequest) {
		// code for IE7+, Firefox, Chrome, Opera, Safari
		xmlhttp = new XMLHttpRequest();
	} else {
		return;
	}
	
	xmlhttp.onreadystatechange = function() {
		if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
			handler(vizitkyElems, xmlhttp.responseText);
		}
	}
	xmlhttp.open("GET", VIZITKY_DATA_FILE, true);
	xmlhttp.send();
}

//Prelozeni souboru do mapy vizitka -> skutecny text
function createVizitkyMap(vizitkyData) {

	var vizitkyMap = {};
	
	if (vizitkyData == null) {
		return vizitkyMap;
	}
	
	var lines = vizitkyData.split("\n");
	
	var i;
	for (i in lines) {
		var parts = lines[i].split(ROLE_VIZITKA_SEPARATOR);
		if (parts.length == 0 || parts[0].length == 0) {
			continue;
		}
		if (parts.length == 1) {
			parts[1] = "";
		}
		// Prvni cast je klic do mapy, kvuli ProxyPortletu davame na mala pismena
		var key = parts.shift().toLowerCase();
		// V dalsich castech je hodnota do mapy
		var value = parts.join(ROLE_VIZITKA_SEPARATOR);
		vizitkyMap[key] = value;
	}
	
	return vizitkyMap;
}


function translateVizitky(vizitkyElems, vizitkyData) {
	
	// Prelozeni souboru do mapy vizitka -> skutecny text
	var vizitkyMap = createVizitkyMap(vizitkyData);
	
	// Nahrazeni vizitek skutecnym textem
	var i, className;
	for (i in vizitkyElems) {
		className = vizitkyElems[i].className;
		var roleIndex = className.indexOf(VIZITKY_PREFIX) + VIZITKY_PREFIX_LENGTH;
		var roleEndIndex = className.indexOf(" ", roleIndex);
		if (roleEndIndex == -1) {
			roleEndIndex = className.length;
		}
		var roleName = className.substr(roleIndex, roleEndIndex-roleIndex).toLowerCase();
		var realVizitka = vizitkyMap[roleName];
		
		if (realVizitka == null) {
			//alert('Vizitka nenalezena for class ' + roleName);
			continue;
		}
		
		vizitkyElems[i].innerHTML = realVizitka;		
	}
}

