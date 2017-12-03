$.ajaxSetup({ cache: false });

function wait(ms){
   var start = new Date().getTime();
   var end = start;
   while(end < start + ms) {
     end = new Date().getTime();
  }
}

$.urlParam = function(name){
    var results = new RegExp('[\?&]' + name + '=([^&#]*)').exec(window.location.href);
    if (results==null){
       return null;
    }
    else{
       return results[1] || 0;
    }
}

var tablet;

if($.urlParam('tablet') == 'y'){
	tablet = 1;
}else{
	tablet = 0;
}

var start = 0;

var address;
var test;
var t;
var text;

function update(){
	test = $.getJSON( "./baan.json", function() {
	  console.log( "success" );
	}).done(function(){
		$.each(test.responseJSON['M'], function(idxa){
			var Module = test.responseJSON['M'][idxa];
			M = Module[0];			
			B = Module[1];			
			S = Module[2];
			D = Module[3];
			blocked = Module[4];
			state = Module[5];
			tID = Module[6];

			var color;

			if(blocked == 1){
				color = "#900";
			}else{
				if(state == 0){
					color = "#0f0";
				}else if(state == 1){ //SLOW
					color = "#ff7f00";
				}else if(state == 2){	//Red
					color = "#f00";
				}else if(state == 3){	//UNKOWN
					color = "#ff0";
				}else if(state == 4){	//GHOST
					color = "#888";
				}else if(state == 5){ //RESERVED
					color = "#00f";
				}
			}

			if(tID != 0){
				console.log(M+":"+B+":"+S+"\tD:"+D+"\tBlocked:"+blocked+"\tState:"+state+"\ttID:"+tID);
			}
			
			$(".L","#M"+M+" #B"+B+".S"+S).css('stroke',color);


			text = "";
			if(D == 0){
				text = "&lt;"+tID;
			}else if(D == 1){
				text = tID+">";
			}
			$(".T","#M"+M+"B"+B+"S"+S).html(text);
			if(tablet){
				$("#M"+M+"B"+B+"S"+S).attr('style','display:block');
			}
		});
		$.each(test.responseJSON['SW'], function(idxa){
			var Module = test.responseJSON['SW'][idxa];
			M = Module[0];
			B = Module[1];
			S = Module[2];
			state = Module[3];

			if(state == 0){
				console.log(M+":"+B+":"+S+" Straight:"+"#M"+M+" #B"+B+" #S"+S+"D");
				$("#M"+M+" #B"+B+" #S"+S+"D").css("opacity",0); //Straight
				$("#M"+M+" #B"+B+" #S"+S+"S").css("opacity",1);
			}else if(state == 1){
				console.log(M+":"+B+":"+S+" Diverging:"+"#M"+M+" #B"+B+" #S"+S+"D");
				$("#M"+M+" #B"+B+" #S"+S+"S").css("opacity",0); //Diverging
				$("#M"+M+" #B"+B+" #S"+S+"D").css("opacity",1);
			}


			//$("#M"+M+" #B"+B+" #S"+S).css('stroke',color);

			//console.log("SW: "+M+":"+B+":"+S+"  state:"+state);
		});
	});
}

function stop(){
	clearInterval(address);
}

function Start(){
	address = setInterval(update,500);
}

$(document).ready(function(){
	update();
	address = setInterval(update,500);
	stop();
});

function Hide(object1,object2){
	console.log("Hide: #"+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class'));
	if(tablet == 0){
		$("#"+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class')).attr('style','display:none');
	}
}

function Show(object1,object2){
	console.log("Show: #"+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class'));
	if(tablet == 0){
		$("#"+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class')).attr('style','display:block');
	}
}
