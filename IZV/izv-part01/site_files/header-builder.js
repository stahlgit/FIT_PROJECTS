// Reseni postavene na jQuery
// Autor: Jiri Kral, 09.01.2009
// URL: http://www.v2.cz 

$(document).ready(function() {
  $('#rozbaleno').hide();
  
  $('#sekce-rolovat, #sekce-logo').click(function() {
	if($('#sbaleny_holder').hasClass("zavreny")){
		$('#sbaleny_holder').removeClass("zavreny");
		$('#sbaleny_holder').addClass("otevreny");
	} else {
		$('#sbaleny_holder').addClass("zavreny");
		$('#sbaleny_holder').removeClass("otevreny");
	}
    $('#rozbaleno').slideToggle(5);
    return false;            
  });

  $('#akce').click(function() {
    $('#rozbaleno').slideUp('fast');
	$('#sbaleny_holder').addClass("zavreny");
	$('#sbaleny_holder').removeClass("otevreny");
	return false;
  });

  $('#akce').click(function() {             
    $('#sekce-odrolovat').removeClass("otevreno");
    $('#sekce-odrolovat').addClass("zavreno");
  });
  
  if($("#header").innerWidth()>1150){
    $('#ul-list').addClass("w50");
  }else{
    $('#ul-list').addClass("w100");
  };
  
});
