// --- ConfigCS --- //

var cookieTitleCs = "Cookies."; // Title

var cookieDescCs = "Tyto webové stránky používají k poskytování svých služeb soubory Cookies. Používáním těchto webových stránek souhlasíte s použitím souborů Cookies."; // Description

var cookieButtonCs = "Rozumím"; // Button text

var cookieLinkCs = '<a href="/files/portal/docs/ruzne/GDPR/c-Zasady_prace_s_cookies_zakladni_2021_verze_12072021.pdf" target="_blank">Zásady práce s cookies</a>'; // Cookiepolicy link





// ---        --- //



// --- Config --- //

var cookieTitleEn = "Cookies."; // Title

var cookieDescEn = "This website uses cookies to provide its services. By using this website, you agree to the use of cookies."; // Description

var cookieButtonEn = "Understood"; // Button text

var cookieLinkEn = '<a href="/files/portal/docs/ruzne/GDPR/c-Zasady_prace_s_cookies_zakladni_2021_verze_12072021.pdf" target="_blank">Principles of working with cookies</a>'; // Cookiepolicy link

// ---        --- //





function fadeInCookie(elem, display) {

  var el = document.getElementById(elem);

  el.style.opacity = 1;

  el.style.display = display || "block";



  (function fade() {

    var val = parseFloat(el.style.opacity);

    if (!((val += .02) > 1)) {

      el.style.opacity = val;

      requestAnimationFrame(fade);

    }

  })();

};



function fadeOutCookie(elem) {

  var el = document.getElementById(elem);

  el.style.opacity = 0;



  (function fade() {

    if ((el.style.opacity -= .02) < 0) {

      el.style.display = "none";

    } else {

      requestAnimationFrame(fade);

    }

  })();

};



function setCookie(name, value, days) {

  var expires = "";

  if (days) {

    var date = new Date();

    date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));

    expires = "; expires=" + date.toUTCString();

  }

  document.cookie = name + "=" + (value || "") + expires + "; path=/; SameSite=Lax";

}



function getCookie(name) {

  var nameEQ = name + "=";

  var ca = document.cookie.split(';');

  for (var i = 0; i < ca.length; i++) {

    var c = ca[i];

    while (c.charAt(0) == ' ') c = c.substring(1, c.length);

    if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length, c.length);

  }

  return null;

}



function eraseCookie(name) {

  document.cookie = name + '=; Max-Age=-99999999;';

}



function cookieConsent() {

  if (!getCookie('cookieDismiss')) {

    var lang = document.documentElement.lang;
    var template = document.createElement('template');

    var html ='<div class="cookieConsentContainer" id="cookieConsentContainer"><div class="cookieTitle"><a>'

      + (lang === 'cs' ? cookieTitleCs : cookieTitleEn) + '</a></div><div class="cookieDesc"><p>'

      + (lang === 'cs' ? cookieDescCs : cookieDescEn) +

      '</p> <p>' + (lang === 'cs' ? cookieLinkCs : cookieLinkEn) + '</p></div><div class="cookieButton"><a onClick="cookieDismiss();">'

      + (lang === 'cs' ? cookieButtonCs : cookieButtonEn) + '</a></div></div>';
    html = html.trim();
    template.innerHTML = html;
    document.body.appendChild(template.content.firstChild);
    fadeInCookie("cookieConsentContainer");

  }

}



function cookieDismiss() {

  setCookie('cookieDismiss', '1', 365);

  fadeOutCookie("cookieConsentContainer");

}

