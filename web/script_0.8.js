$.ajaxSetup({ cache: false });

function wait(ms){
   var start = new Date().getTime();
   var end = start;
   while(end < start + ms) {
     end = new Date().getTime();
  }
}

var start = 0;

var address;
var test;
var t;
var text;
var setup;
var start = 1;

var vara;

function update(){
	test = $.getJSON( "./../baan.json", function() {
	  console.log( "success" );
	}).done(function(){

		$.each(test.responseJSON['SW'], function(idxa){
			var Module = test.responseJSON['SW'][idxa];
			M = Module[0];
			B = Module[1];
			S = Module[2];
			state = Module[3];
			var len = Module[4];

			if(state == 0){
				//console.log(M+":"+B+":"+S+" Straight:"+".M"+M+" #B"+B+" #S"+S+"D");
				$(".M"+M+" #B"+B+" #S"+S+"D").css("opacity",0); //Straight
				$(".M"+M+" #B"+B+" #S"+S+"S").css("opacity",1);
			}else if(state == 1){
				//console.log(M+":"+B+":"+S+" Diverging:"+".M"+M+" #B"+B+" #S"+S+"D");
				$(".M"+M+" #B"+B+" #S"+S+"S").css("opacity",0); //Diverging
				$(".M"+M+" #B"+B+" #S"+S+"D").css("opacity",1);
       }

      if(start == 1){
        if(S == 1){
          $("#switches .content").html($("#switches .content").html() + '<div class="switchbox M'+M+'" id="M'+M+'B'+B+'S'+S+'s">'+M+":"+B+":"+S+"</div>");

          console.log("active");
          $.ajax({
            async: false,
            type: 'GET',
            cache: true,
            url: "./../modules/"+M+"s.svg",
            success: function(data) {
              console.log("#M"+M+"B"+B+'S'+S+"s");
              console.log("Len: ",len);
              $("#M"+M+"B"+B+'S'+S+"s").html(data.documentElement);
              if(len == 1){
                var text = "<center><b>"+M+":"+B+':'+S+"</b><br/><img src='./img/switch_button.png' width='30px'";
                text += "onmouseenter=\"this.src = './img/switch_button_h.png'\" onmouseleave=\"this.src = './img/switch_button.png'\"";
                text += "/></center>";
              }else{
                var text = "<center><b>"+M+":"+B+":x</b><br/><img src='./img/switch_u_button.png' width='30px'";
                text += "onmouseenter=\"this.src = './img/switch_u_button_h.png'\" onmouseleave=\"this.src = './img/switch_u_button.png'\"/>";
                text += "<img src='./img/switch_d_button.png' width='30px' style='margin-left:5px'";
                text += "onmouseenter=\"this.src = './img/switch_d_button_h.png'\" onmouseleave=\"this.src = './img/switch_d_button.png'\"/>";
                text += "</center>";
              }
              $("#M"+M+"B"+B+'S'+S+"s").html($("#M"+M+"B"+B+'S'+S+"s").html() + text);
              $("#M"+M+"B"+B+'S'+S+"s svg").attr("viewBox",(80*(B-1))+" "+(80*(S-1))+" 70 70");
            }
          });
        }
      }

      if(start == 1 && test.responseJSON['SW'].length - 1 == idxa){
        start = 0;
      }
			//$(".M"+M+" #B"+B+" #S"+S).css('stroke',color);

			//console.log("SW: "+M+":"+B+":"+S+"  state:"+state);
		});
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
					color = "#f70";
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
				//console.log(M+":"+B+":"+S+"\tD:"+D+"\tBlocked:"+blocked+"\tState:"+state+"\ttID:"+tID);
			}

			$(".L",".M"+M+" #B"+B+".S"+S).css('stroke',color);

      $(".L",".M"+M+" #B"+B+".S"+S).each(function(index){
        var attr = $(this).attr('nswitch');
        if (typeof attr !== typeof undefined && attr !== false) {
            var nColor = "#";
            a = Math.floor(parseInt(color[1],16)/2);
            nColor += a.toString(16);
            b = Math.floor(parseInt(color[2],16)/2);
            nColor += b.toString(16);
            c = Math.floor(parseInt(color[3],16)/2);
            nColor += c.toString(16);
            $(this).css('stroke',nColor);
        }else{
          $(this).css('stroke',color);
        }
      });
      //console.log(".L",".M"+M+" #B"+B+".S"+S);


			text = "";
			if(D == 0){
				text = "&lt;"+tID;
			}else if(D == 1){
				text = tID+">";
			}
			$(".T",".M"+M+"B"+B+"S"+S).html(text);
			if(tablet){
				$(".M"+M+"B"+B+"S"+S).attr('style','display:block');
			}
		});
		$.each(test.responseJSON['MO'], function(idxa){

			var Module = test.responseJSON['MO'][idxa];
			M = Module[0];
			B = Module[1];
			S = Module[2];
			length = Module[3];
			state = Module[4];

			for(var i = 0;i<length;i++){
				$(".M"+M+" #B"+B+" .M"+S+"S"+i).css("opacity",0); //Straight
			}
			//console.log(M+":"+B+":"+S+" State "+state+":"+".M"+M+" #B"+B+" .M"+S+"S"+state);
			$(".M"+M+" #B"+B+" .M"+S+"S"+state).css("opacity",1);


			//$(".M"+M+" #B"+B+" #S"+S).css('stroke',color);

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

var x = 0,y = 0,r = 0;
var width = 0,height = 0;

$(document).ready(function(){
  segments = $.getJSON( "./../modules/list.txt", function() {
    console.log("GET Segments");
  }).done(function(){
    segments = segments.responseJSON;
    setup = $.getJSON( "./../setup.json", function() {
  	  console.log( "SETUP" );
  	}).done(function(){
      setup = setup.responseJSON;
      $.each(setup, function(i){
        if(setup[i] == 0){

        }else{
          console.log("M"+setup[i]+" x:"+x+", y:"+y);
          $(".M"+setup[i]).css("left",x+"px");
          $(".M"+setup[i]).css("top",y+"px");
          $(".M"+setup[i]).css("-ms-transform","rotate("+r+"deg)");
          $(".M"+setup[i]).css("-webkit-transform","rotate("+r+"deg)");
          $(".M"+setup[i]).css("transform","rotate("+r+"deg)");
          $(".M"+setup[i]).css("transform-origin","0px 0px");
          r += segments[setup[i]-1][3];
          if(i == 0 && r == 90){
            r = 0;
          };
          if(r == 0){
            x += segments[setup[i]-1][2];
            y += segments[setup[i]-1][1];
          }else if(r == 90){
            x += segments[setup[i]-1][1];
            y += segments[setup[i]-1][2];
          }else if(r == 180){
            x -= segments[setup[i]-1][2];
            y += segments[setup[i]-1][1];
          }
        }

        if(i == setup.length-1){
          console.log("R:"+r);
          if(r == 90){
            x += 140;
          }else if(r == 0){
            y += 140;
          }else if(r == -90){
            x -=140;
          }else if(r == 180 || r == -180){
            console.log("R");
            x += 140;
            y += 140;
          }
          $("#Modules").css("height",y+"px");
          $("#Modules").css("width",x+"px");

          if($.urlParam('rot') == "90"){
            console.log("+90deg");
            $("#Modules").css("transform-origin",x+"px 0px");
            $("#Modules").css("left",-x+"px");
            $("#Modules").css("transform","rotate(-90deg)");
            $("#Modules_wrapper").css("height",x+"px");
            $("#Modules_wrapper").css("width",y+"px");
          }else if($.urlParam('rot') == "-90"){
            console.log("-90deg");
            $("#Modules").css("transform-origin",("0px "+y+"px"));
            $("#Modules").css("top",-y+"px");
            $("#Modules").css("transform","rotate(90deg)");
            $("#Modules_wrapper").css("height",x+"px");
            $("#Modules_wrapper").css("width",y+"px");
          }else{
            $("#Modules_wrapper").css("height",y+"px");
            $("#Modules_wrapper").css("width",x+"px");
          }
        }
      });
    });
  });
	update();
});

function Hide(object1,object2){
	//console.log("Hide: #"+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class'));
	if(tablet == 0){
		$("."+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class')).attr('style','display:none');
	}
}

function Show(object1,object2){
	//console.log("Show: #"+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class'));
	if(tablet == 0){
		$("."+object2.parent().parent().parent().attr('id')+object1.attr('id')+object1.attr('class')).attr('style','display:block');
	}
}
