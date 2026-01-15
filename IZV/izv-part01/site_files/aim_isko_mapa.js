 function aim_isko_mapa(kolik) {
          if(kolik < 8) {
            document.getElementById('mapa').style.backgroundPosition = "0px -"+(kolik*160)+"px";
          } else {
            document.getElementById('mapa').style.backgroundPosition = "-200px -"+((kolik-7)*160)+"px";
          }
}
