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
var switches = [];
var status_prev;
var status_t = 0;
var train_list = "";
var train_d = 0;

for(var i = 0;i<10;i++){
  switches[i] = [];
  for(var j = 0;j<15;j++){
    switches[i][j] = [];
  }
}

var vara;

var remote_Switch = function(T,M,B,S){
  console.log("http://"+window.location.hostname+":9000/"+T+M+":"+B+":"+S);
  $.ajax({
    dataType: "text",
    cache: false,
    url: "http://"+window.location.hostname+":9000/"+T+M+":"+B+":"+S,
    statusCode: {
    205: function() {
      alert(M+":"+B+":"+S+" is not found");
    }
  }
  });
}

var remote_request = function(D){
  console.log("http://"+window.location.hostname+":9000/"+D);
  $.ajax({
    dataType: "text",
    cache: false,
    url: "http://"+window.location.hostname+":9000/"+D,
    statusCode: {
    205: function() {
      alert("Failed to link train");
    }
  }
  });
}

var remote_addTrain = function(mID,fID,tID){//Message ID, follow ID, Train ID
  console.log("http://"+window.location.hostname+":9000/L"+mID+":"+fID+":"+tID);
  $.ajax({
    dataType: "text",
    cache: false,
    url: "http://"+window.location.hostname+":9000/L"+mID+":"+fID+":"+tID,
    statusCode: {
    205: function() {
      alert("Failed to link train");
    }
  }
  });
}

var remote_gSwitch = function(c,M,B){
  var i = 1;
  console.log(c);
  if(c == '-'){
    while(1){
      if(i == switches[M][B].length){
        i = 1;
        for(var j = 1;j<switches[M][B].length;j++){
          remote_Switch(switches[M][B][j].type,M,B,j);
          switches[M][B][j].state = 0;
        }
        break;
      }
      //console.log(switches[M][B]);
      if(switches[M][B][i].state == switches[M][B][i].len - 1){
        i++;
        continue;
      }else{
        switches[M][B][i].state++;
        remote_Switch(switches[M][B][i].type,M,B,i);
        break;
      }
    }
  }else if(c == '+'){
    var i = switches[M][B].length-1;
    while(1){
      if(i == 1){
        i = switches[M][B].length-1;
        for(var j = 1;j<switches[M][B].length;j++){
          remote_Switch(switches[M][B][j].type,M,B,j);
          switches[M][B][j].state = 1;
        }
        break;
      }
      console.log(i);
      if(switches[M][B][i].state == 0){
        i--;
        continue;
      }else{
        switches[M][B][i].state--;
        remote_Switch(switches[M][B][i].type,M,B,i);
        break;
      }
    }
  }
  console.log(switches[M][B]);

}

function status_arr_comp(A,B){
  if(A.length == B.length){
    $.each(A, function(i){
      if(A[i][0] == B[i][0] && A[i][1] == B[i][1]){

      }else{
        return 0;
      }
    });
    return 1;
  }else{
    return 0;
  }
}

function update(){
	test = $.getJSON( "./../baan.json", function() {
	  console.log( "test success" );
	}).done(function(){
		$.each(test.responseJSON['SW'], function(idxa){
			var Module = test.responseJSON['SW'][idxa];
			M = Module[0];
			B = Module[1];
			S = Module[2];
			state = Module[3];
      var len = Module[4];
      var s_len = Module[5];

              //console.log("#M"+M+"B"+B+'S'+S+"s");
              //console.log("Len: ",len);

      if(len == 2){
  			if(state == 0){
  				//console.log(M+":"+B+":"+S+" Straight:"+".M"+M+" #B"+B+" #S"+S+"D");
  				$(".M"+M+" #B"+B+" #S"+S+"D").css("opacity",0); //Straight
  				$(".M"+M+" #B"+B+" #S"+S+"S").css("opacity",1);
  			}else if(state == 1){
  				//console.log(M+":"+B+":"+S+" Diverging:"+".M"+M+" #B"+B+" #S"+S+"D");
  				$(".M"+M+" #B"+B+" #S"+S+"S").css("opacity",0); //Diverging
  				$(".M"+M+" #B"+B+" #S"+S+"D").css("opacity",1);
        }
      }else if(len > 2){
        for(var i = 1;i<=len;i++){
          $(".M"+M+" #B"+B+" #M"+S+"S"+i).css("opacity",0); //Straight
        }
        //console.log(M+":"+B+":"+S+" State "+state+":"+".M"+M+" #B"+B+" .M"+S+"S"+state);
        $(".M"+M+" #B"+B+" #M"+S+"S"+(state+1)).css("opacity",1);
      }

      if(s_len > 1){
        if(len == 2){
          switches[M][B][S] = {"type":"S","s_len":s_len,"len":2,"state":state}
        }else if(len > 2){
          switches[M][B][S] = {"type":"M","s_len":s_len,"len":len,"state":state}
        }
      }

      if(start == 1){
        if(S == 1){
          $("#switches .content").html($("#switches .content").html() + '<div class="switchbox M'+M+' M'+M+'s" id="M'+M+'B'+B+'S'+S+'s">'+M+":"+B+":"+S+"</div>");

          console.log("active");
          $.ajax({
            async: false,
            type: 'GET',
            cache: true,
            url: "./../modules/"+M+"s.svg",
            success: function(data) {
              $("#M"+M+"B"+B+'S'+S+"s").html(data.documentElement);
              if(s_len == 1){
                var text = "<center><b>"+M+":"+B+':'+S+"</b><br/><img src='./img/switch_button.png' width='30px'";
                text += "onClick=\"remote_Switch('S',"+M+","+B+","+S+")\"";
                text += "onmouseenter=\"this.src = './img/switch_button_h.png'\" onmouseleave=\"this.src = './img/switch_button.png'\"";
                text += "/></center>";
                $("#M"+M+"B"+B+'S'+S+"s svg").attr("viewBox",(80*(B-1))+" "+(80*(S-1))+" 70 70");
              }else if(s_len < 4){
                var text = "<center><b>"+M+":"+B+":x</b><br/><img src='./img/switch_u_button.png' width='30px'";
                text += "onClick=\"remote_gSwitch('-',"+M+","+B+")\"";
                text += "onmouseenter=\"this.src = './img/switch_u_button_h.png'\" onmouseleave=\"this.src = './img/switch_u_button.png'\"/>";
                text += "<img src='./img/switch_d_button.png' width='30px' style='margin-left:5px'";
                text += "onClick=\"remote_gSwitch('+',"+M+","+B+")\"";
                text += "onmouseenter=\"this.src = './img/switch_d_button_h.png'\" onmouseleave=\"this.src = './img/switch_d_button.png'\"/>";
                text += "</center>";
                $("#M"+M+"B"+B+'S'+S+"s svg").attr("viewBox",(80*(B-1))+" "+(80*(S-1))+" 70 70");
              }else{
                var text = "<br/><center><div style='margin-bottom:10px;'><b>"+M+":"+B+":x</b></div><img src='./img/switch_u_button.png' width='30px'";
                text += "onClick=\"remote_gSwitch('-',"+M+","+B+")\"";
                text += "onmouseenter=\"this.src = './img/switch_u_button_h.png'\" onmouseleave=\"this.src = './img/switch_u_button.png'\" style='margin-bottom:10px'/><br/>";
                text += "<img src='./img/switch_d_button.png' width='30px' style='margin-left:5px'";
                text += "onClick=\"remote_gSwitch('+',"+M+","+B+")\"";
                text += "onmouseenter=\"this.src = './img/switch_d_button_h.png'\" onmouseleave=\"this.src = './img/switch_d_button.png'\"/>";
                text += "</center>";
                $("#M"+M+"B"+B+'S'+S+"s").attr("class",$("#M"+M+"B"+B+'S'+S+"s").attr("class")+" big");
                $("#M"+M+"B"+B+'S'+S+"s svg").css("float","left");
                $("#M"+M+"B"+B+'S'+S+"s svg").attr("viewBox",(80*(B-1))+" "+(80*(S-1))+" 70 140");
              }
              $("#M"+M+"B"+B+'S'+S+"s").html($("#M"+M+"B"+B+'S'+S+"s").html() + text);
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
    response = test.responseJSON['Message'];
    if(response.length == 0){
      $('#warning_list').css("display","none");
    }else{
      $('#warning_list').css("display","block");
    }
    if(status_t == 1 && status_arr_comp(response,status_prev)){

    }else{
      status_t = 1;
      console.log("Clear");
      $('#warning_list').empty();
      $.each(response, function(i){
        if(response[i][0] == 1){
          console.log("Emergency "+response[i][1]);
          if($('#warning_list #W'+response[i][1]+"_"+response[i][0]).length == 0){
            var text = "<div id=\"W"+response[i][1]+"_"+response[i][0]+"\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/stop.png\"/></div>";
            text += "<div class=\"photobox r\"><img src=\"./img/stop.png\"/></div><center><h2>EMERGENCY STOP</h2><br/><but onClick=\"remote_request('Er'+"+response[i][1]+")\">";
            text += "<b>Resume</b></but></center></div>";
            $('#warning_list').prepend(text);
/*
            $('#W'+response[i][1]+"_"+response[i][0]+" but").on("touchend", function () {
              remote_request('Er'+response[i][1]);
            });*/
          }

        }else if(response[i][0] == 2){
          console.log("Electrical short "+response[i][1]);
          if($('#warning_list #W'+response[i][1]+"_"+response[i][0]).length == 0){
            var text = "<div id=\"W"+response[i][1]+"_"+response[i][0]+"\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/shortcircuit.png\"/></div>";
            text += "<div class=\"photobox r\"><img src=\"./img/shortcircuit.png\"/></div><center><h2>SHORT CICRUIT</h2><br/><but onClick=\"remote_request('Ec'+"+response[i][1]+")\">";
            text += "<b>Resume</b></but></center></div>";
            $('#warning_list').prepend(text);
/*
            $('#W'+response[i][1]+"_"+response[i][0]+" but").on("touchend", function () {
              remote_request('Er'+response[i][1]);
            });*/
          }

        }else if(response[i][0] == 11){
          console.log("New Train "+response[i][1]);
          if($('#warning_list #W'+response[i][1]+"_"+response[i][0]).length == 0){
            var text = "<div id=\"W"+response[i][1]+"_"+response[i][0]+"\" class=\"warning\" style=\"background-color:grey;\"><div class=\"photobox l\"><img src=\"./img/train.png\"/></div>";
            text += "<div class=\"photobox r\"><img src=\"./img/train.png\"/></div><center><h2>New Train</h2> at "+response[i][2][1];
            text += "<div style=\"width:calc(100% - 100px);height:40px;\"><div style=\"float:left;color:black;width:300px\">"+train_list+"</div><but style=\"float:right;position:relative;top:6px\"";
            text += "onClick=\"var val = $('#'+$(this).parent().parent().parent().attr('id')+' .cs-selected').attr('data-value');if(typeof val != 'undefined'){alert('Sending command');remote_addTrain("+response[i][1]+","+response[i][2][0]+",parseInt(val));}else{alert('Please select a train');}\">";
            text += "<b>Link</b></but></div></center></div>";
            $('#warning_list').append(text);
/*
            $('#W'+response[i][1]+"_"+response[i][0]+" but").on("touchend", function () {
              var val = $('#'+$(this).parent().parent().parent().attr('id')+' .cs-selected').attr('data-value');
              if(typeof val != 'undefined'){
                remote_addTrain(response[i][1]+","+response[i][2][0]+","+parseInt(val));
              }else{
                alert('Please select a train');
              }
            });*/
          }

        }
      });
      redraw_selects();
      status_prev = response;
    }
  });
}

function stop(){
	clearInterval(address);
}

function Start(){
	address = setInterval(update,500);
}

var x = 0,y = 0,r = 0;
var max_x = 0,max_y = 0;
var min_x = 0,min_y = 0;
var anchor3_x = 0, anchor3_y = 0, anchor3_r = 0;

function pad (str, max) {
  str = str.toString();
  return str.length < max ? pad("0" + str, max) : str;
}

$(document).ready(function(){
  segments = $.getJSON( "./../modules/list.txt", function() {
    console.log("GET Segments");
  }).done(function(){
    segments = segments.responseJSON;
    setup = $.getJSON( "./../setup.json", function() {
  	  console.log( "SETUP" );
  	}).done(function(){
      setup = setup.responseJSON;
      console.log(setup);
      $.each(setup[0], function(i){
        if(setup[0][i] == 0){
          //["Name","Width","Height","Rotation","Anchor1_x","Anchor1_y","Anchor2_x","Anchor2_y","Anchor3_rot","Anchor3_x","Anchor3_y"]
          //   0       1       2          3          4           5           6           7             8          9            10
        }else{
          console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
          if(r == 0){
            x += segments[setup[0][i]][4];
            y -= segments[setup[0][i]][5];
            max_x = Math.max(max_x,x+segments[setup[0][i]][1]);
            max_y = Math.max(max_y,y+segments[setup[0][i]][2]);
            min_x = Math.min(min_x,x);
            min_y = Math.min(min_y,y);
          }else if(r == 90){
            x += segments[setup[0][i]][5];
            y += segments[setup[0][i]][4];
            max_x = Math.max(max_x,x);
            max_y = Math.max(max_y,y+segments[setup[0][i]][1]);
            min_x = Math.min(min_x,x-segments[setup[0][i]][2]);
            min_y = Math.min(min_y,y);
          }else if(r == 180){
            x -= segments[setup[0][i]][4];
            y += segments[setup[0][i]][5];
            max_x = Math.max(max_x,x);
            max_y = Math.max(max_y,y);
            min_x = Math.min(min_x,x-segments[setup[0][i]][1]);
            min_y = Math.min(min_y,y-segments[setup[0][i]][2]);
          }
          if(segments[setup[0][i]].length > 10){
            anchor3_r = r + segments[setup[0][i]][8];
            anchor3_x = x + segments[setup[0][i]][9];
            anchor3_y = y + segments[setup[0][i]][10];
          }
          console.log("M"+setup[0][i]+" x:"+x+", y:"+y+", R:"+r);
          console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
          $(".M"+setup[0][i]+"b").css("left",x+"px");
          $(".M"+setup[0][i]+"b").css("top",y+"px");
          $(".M"+setup[0][i]+"b").css("-ms-transform","rotate("+r+"deg)");
          $(".M"+setup[0][i]+"b").css("-webkit-transform","rotate("+r+"deg)");
          $(".M"+setup[0][i]+"b").css("transform","rotate("+r+"deg)");
          $(".M"+setup[0][i]+"b").css("transform-origin","0px 0px");
          if(r == 0){
            x += segments[setup[0][i]][6];
            y += segments[setup[0][i]][7];
          }else if(r == 90){
            x -= segments[setup[0][i]][7];
            y += segments[setup[0][i]][6];
          }else if(r == 180){
            x -= segments[setup[0][i]][6];
            y -= segments[setup[0][i]][7];
          }
          r += segments[setup[0][i]][3];
          if(i == 0 && r == 90){
            r = 0;
          };
          //console.log("M"+setup[0][i]+" x:"+x+", y:"+y+", R:"+r);
        }

        if(i == setup[0].length-1){
          if(setup[1].length > 0){
            x = anchor3_x;
            y = anchor3_y;
            r = anchor3_r;
            $.each(setup[1], function(i){
              if(r == 0){
                x += segments[setup[1][i]][4];
                y -= segments[setup[1][i]][5];
                max_x = Math.max(max_x,x+segments[setup[1][i]][1]);
                max_y = Math.max(max_y,y+segments[setup[1][i]][2]);
                min_x = Math.min(min_x,x);
                min_y = Math.min(min_y,y);
              }else if(r == 90){
                x += segments[setup[1][i]][5];
                y += segments[setup[1][i]][4];
                max_x = Math.max(max_x,x);
                max_y = Math.max(max_y,y+segments[setup[1][i]][1]);
                min_x = Math.min(min_x,x-segments[setup[1][i]][2]);
                min_y = Math.min(min_y,y);
              }else if(r == 180){
                x -= segments[setup[1][i]][4];
                y += segments[setup[1][i]][5];
                max_x = Math.max(max_x,x);
                max_y = Math.max(max_y,y);
                min_x = Math.min(min_x,x-segments[setup[1][i]][1]);
                min_y = Math.min(min_y,y-segments[setup[1][i]][2]);
              }
              if(segments[setup[1][i]].length > 10){
                anchor3_r = r + segments[setup[1][i]][8];
                anchor3_x = x + segments[setup[1][i]][9];
                anchor3_y = y + segments[setup[1][i]][10];
              }
              console.log("M"+setup[1][i]+" x:"+x+", y:"+y+", R:"+r);
              console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
              $(".M"+setup[1][i]+"b").css("left",x+"px");
              $(".M"+setup[1][i]+"b").css("top",y+"px");
              $(".M"+setup[1][i]+"b").css("-ms-transform","rotate("+r+"deg)");
              $(".M"+setup[1][i]+"b").css("-webkit-transform","rotate("+r+"deg)");
              $(".M"+setup[1][i]+"b").css("transform","rotate("+r+"deg)");
              $(".M"+setup[1][i]+"b").css("transform-origin","0px 0px");
              if(r == 0){
                x += segments[setup[1][i]][6];
                y += segments[setup[1][i]][7];
              }else if(r == 90){
                x -= segments[setup[1][i]][7];
                y += segments[setup[1][i]][6];
              }else if(r == 180){
                x -= segments[setup[1][i]][6];
                y -= segments[setup[1][i]][7];
              }
              r += segments[setup[1][i]][3];
            });
          }

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
          $("#Modules").css("height",(max_y+160)+"px");
          $("#Modules").css("width",max_x+"px");
          $("#Modules").css("margin-top",(-min_y)+"px");
          $("#Modules").css("margin-left",(-min_x)+"px");

          if($.urlParam('rot') == "90"){
            console.log("+90deg");
            $("#Modules").css("transform-origin",x+"px 0px");
            $("#Modules").css("left",-x+"px");
            $("#Modules").css("transform","rotate(-90deg)");
            $("#Modules_wrapper").css("height",(max_x-min_x)+"px");
            $("#Modules_wrapper").css("width",max_y+"px");
          }else if($.urlParam('rot') == "-90"){
            console.log("-90deg");
            $("#Modules").css("transform-origin",("0px "+y+"px"));
            $("#Modules").css("top",-y+"px");
            $("#Modules").css("transform","rotate(90deg)");
            $("#Modules_wrapper").css("height",(max_x-min_x)+"px");
            $("#Modules_wrapper").css("width",(max_y-min_y)+"px");
          }else{
            $("#Modules_wrapper").css("height",(max_y+160)+"px");
            $("#Modules_wrapper").css("width",(max_x-min_x)+"px");
          }
        }
      });
    });
  });
  $.ajaxSetup({ cache: false });

  $.getJSON( "./../trains/trainlist.txt", function(response){
    train_list += "<select class=\"cs-selectn cs-select cs-skin-rotate\">";
    train_list += "<option value=\"\" disabled selected>Select train</option>";

    for(var i = 1;i<response.length;i++){
      var id = pad(response[i][1],4);
      train_list += "<option value=\""+response[i][1]+"\">#"+id+"&nbsp;&nbsp;&nbsp;"+response[i][0]+"</option>";
    };

    train_list += "</select>";

    train_d = 1;
  }).done(function(){
  	update();
  });
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
