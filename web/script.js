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
var train_d = 0;
var socket_tries = 0;
var active_trains = [];
var rot = 0;

for(var i = 0;i<10;i++){
  switches[i] = [];
  for(var j = 0;j<15;j++){
    switches[i][j] = [];
  }
}

var vara;

var remote_Switch = function(T,M,B,S){
  ws.send(T+M+":"+B+":"+S);
}

var remote_request = function(D){
  ws.send(D);
}

var remote_addTrain = function(mID,fID,tID){//Message ID, follow ID, Train ID
  ws.send("L"+mID+":"+fID+":"+tID);
  console.log("L"+mID+":"+fID+":"+tID);
  setTimeout(function () {
    if($('#W'+mID+"_11").length != 0){
      alert("Train is already in use!!\nSelect another train");
    }
  }, 500);
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

var x = 0,y = 0,r = 0;
var max_x = 0,max_y = 0;
var min_x = 0,min_y = 0;
var anchor3_x = 0, anchor3_y = 0, anchor3_r = 0;

function pad (str, max) {
  str = str.toString();
  return str.length < max ? pad("0" + str, max) : str;
}

var x = 0;
var y = 0;
var r = 0;

function create_track(setup){
  //Reset all values
  x = 0,y = 0,r = 0;
  max_x = 0,max_y = 0;
  min_x = 0,min_y = 0;
  anchor3_x = 0, anchor3_y = 0, anchor3_r = 0;
  var blocks_load = 0;

  $('#Modules').empty();
  $('#Modules').css('display','none');

  console.log(setup);
  $.each(setup[0], function(i){
    console.log(setup[0][i]);
    if(setup[0][i] == 0){
      //["Name","Width","Height","Rotation","Anchor1_x","Anchor1_y","Anchor2_x","Anchor2_y","Anchor3_rot","Anchor3_x","Anchor3_y"]
      //   0       1       2          3          4           5           6           7             8          9            10
    }else{
      //Add Switches to switch list
      if(segments[setup[0][i]][1] != 0){
        var M = setup[0][i];
        $.each(segments[setup[0][i]][1], function(j){
          console.log(segments[setup[0][i]][1][j]);
          var B = segments[setup[0][i]][1][j][0];
          var S = segments[setup[0][i]][1][j][1];
          $("#switches .content").html($("#switches .content").html() + '<div class="switchbox M'+M+' M'+M+'s" id="M'+M+'B'+B+'S'+S+'s">'+M+":"+B+":"+S+"</div>");
          $.ajax({
            async: false,
            type: 'GET',
            cache: true,
            url: "./../modules/"+M+"s.svg",
            success: function(data) {
              $("#M"+M+"B"+B+'S'+S+"s").html(data.documentElement);
              var text = "<center><b>"+M+":"+B+':'+S+"</b><br/><img src='./img/switch_button.png' width='30px'";
              text += "onClick=\"remote_Switch('S',"+M+","+B+","+S+")\"";
              text += "onmouseenter=\"this.src = './img/switch_button_h.png'\" onmouseleave=\"this.src = './img/switch_button.png'\"";
              text += "/></center>";
              $("#M"+M+"B"+B+'S'+S+"s svg").attr("viewBox",(80*(B-1))+" "+(80*(S-1))+" 70 70");
            }
          });
        });
      }


      //Create track
      console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
      $('#Modules').append("<div class=\"M"+setup[0][i]+" Module M"+setup[0][i]+"b\"></div>");
      blocks_load++;
      $('.M'+setup[0][i]+'b').load('./../modules/'+setup[0][i]+'.svg',function (){
        blocks_load--;
        if(blocks_load == 0){$('#Modules').css('display','block');ws.send("Ready");}
      });
      if(r == 0){
        x += segments[setup[0][i]][5];
        y -= segments[setup[0][i]][6];
        max_x = Math.max(max_x,x+segments[setup[0][i]][2]);
        max_y = Math.max(max_y,y+segments[setup[0][i]][3]);
        min_x = Math.min(min_x,x);
        min_y = Math.min(min_y,y);
      }else if(r == 90){
        x += segments[setup[0][i]][6];
        y += segments[setup[0][i]][5];
        max_x = Math.max(max_x,x);
        max_y = Math.max(max_y,y+segments[setup[0][i]][2]);
        min_x = Math.min(min_x,x-segments[setup[0][i]][3]);
        min_y = Math.min(min_y,y);
      }else if(r == 180){
        x -= segments[setup[0][i]][5];
        y += segments[setup[0][i]][6];
        max_x = Math.max(max_x,x);
        max_y = Math.max(max_y,y);
        min_x = Math.min(min_x,x-segments[setup[0][i]][2]);
        min_y = Math.min(min_y,y-segments[setup[0][i]][3]);
      }
      if(segments[setup[0][i]].length > 11){
        anchor3_r = r + segments[setup[0][i]][9];
        anchor3_x = x + segments[setup[0][i]][10];
        anchor3_y = y + segments[setup[0][i]][11];
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
        x += segments[setup[0][i]][7];
        y += segments[setup[0][i]][8];
      }else if(r == 90){
        x -= segments[setup[0][i]][8];
        y += segments[setup[0][i]][7];
      }else if(r == 180){
        x -= segments[setup[0][i]][7];
        y -= segments[setup[0][i]][8];
      }
      r += segments[setup[0][i]][4];
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
          $('#Modules').append("<div class=\"M"+setup[1][i]+" Module M"+setup[1][i]+"b\"></div>");
          blocks_load++;
          $('.M'+setup[1][i]+'b').load('./../modules/'+setup[1][i]+'.svg',function (){
            blocks_load--;
            if(blocks_load == 0){$('#Modules').css('display','block');ws.send("Ready");}
          });
          if(r == 0){
            x += segments[setup[1][i]][5];
            y -= segments[setup[1][i]][6];
            max_x = Math.max(max_x,x+segments[setup[1][i]][2]);
            max_y = Math.max(max_y,y+segments[setup[1][i]][3]);
            min_x = Math.min(min_x,x);
            min_y = Math.min(min_y,y);
          }else if(r == 90){
            x += segments[setup[1][i]][6];
            y += segments[setup[1][i]][5];
            max_x = Math.max(max_x,x);
            max_y = Math.max(max_y,y+segments[setup[1][i]][2]);
            min_x = Math.min(min_x,x-segments[setup[1][i]][3]);
            min_y = Math.min(min_y,y);
          }else if(r == 180){
            x -= segments[setup[1][i]][5];
            y += segments[setup[1][i]][6];
            max_x = Math.max(max_x,x);
            max_y = Math.max(max_y,y);
            min_x = Math.min(min_x,x-segments[setup[1][i]][2]);
            min_y = Math.min(min_y,y-segments[setup[1][i]][3]);
          }
          if(segments[setup[1][i]].length > 11){
            anchor3_r = r + segments[setup[1][i]][9];
            anchor3_x = x + segments[setup[1][i]][10];
            anchor3_y = y + segments[setup[1][i]][11];
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
            x += segments[setup[1][i]][7];
            y += segments[setup[1][i]][8];
          }else if(r == 90){
            x -= segments[setup[1][i]][8];
            y += segments[setup[1][i]][7];
          }else if(r == 180){
            x -= segments[setup[1][i]][7];
            y -= segments[setup[1][i]][8];
          }
          r += segments[setup[1][i]][4];
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
      $("#Modules").css("height",(max_y)+"px");
      $("#Modules").css("width",max_x+"px");
      $("#Modules").css("margin-top",(-min_y)+"px");
      $("#Modules").css("margin-left",(-min_x)+"px");

      rotate('');
    }
  });
}

function rotate(R){
  if(R == '+'){
    rot += 90;
  }else if(R == '-'){
    rot -= 90;
  }
  if(rot == 90){
    console.log("+90deg");
    $("#Modules").css("transform-origin",x+"px 0px");
    $("#Modules").css("left",-x+"px");
    $("#Modules").css("transform","rotate(-90deg)");
    $("#Modules_wrapper").css("height",(max_x-min_x)+"px");
    $("#Modules_wrapper").css("width",max_y+"px");
  }else if(rot == -90){
    console.log("-90deg");
    $("#Modules").css("transform-origin",("0px "+y+"px"));
    $("#Modules").css("top",-y+"px");
    $("#Modules").css("left","0px");
    $("#Modules").css("transform","rotate(90deg)");
    $("#Modules_wrapper").css("height",(max_x-min_x)+"px");
    $("#Modules_wrapper").css("width",(max_y-min_y)+"px");
  }else{
    $("#Modules_wrapper").css("height",(max_y)+"px");
    $("#Modules_wrapper").css("width",(max_x-min_x)+"px");
    $("#Modules").css("transform","rotate(0deg)");
    $("#Modules").css("transform-origin",("0px "+y+"px"));
    $("#Modules").css("top","0px");
    $("#Modules").css("left","0px");
  }
}

function Link_train(list,r1,r2){
  var val;
  if(tablet == 0){
    val = $('#'+$(list).parent().parent().parent().attr('id')+' .cs-selected').attr('data-value');
  }else{
    console.log("tablet == 1");
    console.log($(list).parent().parent().parent().attr('id'));
    val = $('#'+$(list).parent().parent().parent().attr('id')+' .cs-select').val();
  }
  if(typeof val != 'undefined'){
    console.log(val);
    remote_addTrain(r1,r2,parseInt(val));
  }else{
    alert('Please select a train');
  }
}

function switch_update(data){
  $.each(data, function(idxa){
    var Module = data[idxa];
    M = Module[0];
    B = Module[1];
    S = Module[2];
    state = Module[3];

    if(state == 0){
      console.log(M+":"+B+":"+S+" Straight:"+".M"+M+" #B"+B+" #S"+S+"D");
      $(".M"+M+" #B"+B+" #S"+S+"D").css("opacity",0); //Straight
      $(".M"+M+" #B"+B+" #S"+S+"S").css("opacity",1);
    }else if(state == 1){
      console.log(M+":"+B+":"+S+" Diverging:"+".M"+M+" #B"+B+" #S"+S+"D");
      $(".M"+M+" #B"+B+" #S"+S+"S").css("opacity",0); //Diverging
      $(".M"+M+" #B"+B+" #S"+S+"D").css("opacity",1);
    }

    //switches[M][B][S] = {"type":"S","s_len":1,"len":2,"state":state}

    /*
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

    if(start == 1 && data.length - 1 == idxa){
      start = 0;
    }
    */
    //$(".M"+M+" #B"+B+" #S"+S).css('stroke',color);

    //console.log("SW: "+M+":"+B+":"+S+"  state:"+state);
  });
}

function ms_switch_update(data){
  $.each(data, function(idxa){
    var Module = data[idxa];
    M = Module[0];
    B = Module[1];
    S = Module[2];
    state = Module[3];
    var len = Module[4];

    for(var i = 1;i<=len;i++){
      $(".M"+M+" #B"+B+" #M"+S+"S"+i).css("opacity",0); //Straight
    }
    //console.log(M+":"+B+":"+S+" State "+state+":"+".M"+M+" #B"+B+" .M"+S+"S"+state);
    $(".M"+M+" #B"+B+" #M"+S+"S"+(state+1)).css("opacity",1);

    //switches[M][B][S] = {"type":"M","s_len":s_len,"len":len,"state":state}

  });
}

function track_update(data){
  $.each(data, function(idxa){
    var Module = data[idxa];
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
    if(state == 5 || blocked){
      $(".Switch",".M"+M+" #B"+B+".S"+S).css('cursor','not-allowed');
    }else{
      $(".Switch",".M"+M+" #B"+B+".S"+S).css('cursor','pointer');
    }

    if(M == 4 && B == 3 && S == 1){
      console.log(".L\t.M"+M+" #B"+B+".S"+S);
      console.log($(".L",".M"+M+" #B"+B+".S"+S));
    }

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
}

function message_update(response){
  if(response.length == 0){
    $('#warning_list').css("display","none");
  }else{
    $('#warning_list').css("display","block");
  }
  status_t = 1;
  console.log("Clear");
  $.each(response, function(i){
    if(response[i][0] == 1){
      console.log("Emergency "+response[i][1]);
      console.log('#warning_list #W'+response[i][1]+"_"+response[i][0]);
      if($('#warning_list #W'+response[i][1]+"_"+response[i][0]).length == 0){
        var text = "<div id=\"W"+response[i][1]+"_"+response[i][0]+"\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/stop.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/stop.png\"/></div><center><h2>EMERGENCY STOP</h2><br/><but onClick=\"remote_request('Er'+"+response[i][1]+")\">";
        text += "<b>Resume</b></but></center></div>";
        $('#warning_list').prepend(text);
      }
    }else if(response[i][0] == 2 || response[i][0] == 4){
      if(response[i][0] == 2){
        console.log("Emergency "+response[i][1]+" release");
      }else{
        console.log("Electrical short "+response[i][1]+" release");
      }
      console.log('#warning_list #W'+response[i][1]+"_"+response[i][0]);
      if($('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1)).length != 0){
        $('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1)).remove();
      }
    }else if(response[i][0] == 3){
      console.log("Electrical short "+response[i][1]);
      console.log('#warning_list #W'+response[i][1]+"_3"+(response[i][0]-1));
      if($('#warning_list #W'+response[i][1]+"_3").length == 0){
        var text = "<div id=\"W"+response[i][1]+"_"+response[i][0]+"\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/shortcircuit.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/shortcircuit.png\"/></div><center><h2>SHORT CICRUIT</h2><br/><but onClick=\"remote_request('Ec'+"+response[i][1]+")\">";
        text += "<b>Resume</b></but></center></div>";
        $('#warning_list').prepend(text);
      }
    }else if(response[i][0] == 11){
      console.log("New Train "+response[i][1]);
      console.log('#warning_list #W'+response[i][1]+"_11");
      if($('#warning_list #W'+response[i][1]+"_11").length == 0){
        var text = "<div id=\"W"+response[i][1]+"_11\" class=\"warning\" style=\"background-color:grey;\"><div class=\"photobox l\"><img src=\"./img/train.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/train.png\"/></div><center><h2>New Train</h2> at "+response[i][2][1];
        text += "<div style=\"width:calc(100% - 100px);height:40px;\"><div style=\"float:left;color:black;width:300px\"><select class=\"cs-selectn cs-select cs-skin-rotate\"><option value=\"\" disabled selected>Select train</option>";
        text += train_list_t+"</select></div><but style=\"float:right;position:relative;top:6px\"";
        text += "onClick=\"Link_train(this,"+response[i][1]+","+response[i][2][0]+")\">";
        text += "<b>Link</b></but></div></center></div>";
        $('#warning_list').append(text);
        if(tablet == 0){
          redraw_selects('#W'+response[i][1]+"_"+response[i][0]);
        }
      }
    }else if(response[i][0] == 12){
      console.log("New train "+response[i][1]+" remove");
      console.log('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1));
      if($('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1)).length != 0){
        $('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1)).remove();
      }

      console.log("Adding train "+response[i][2][1]+" (L"+response[i][2][0]+") to active trains at "+active_trains.length);
      active_trains[active_trains.length] = [response[i][2][0], response[i][2][1]];

    }
  });

  if($("#warning_list").html() == ""){
    $('#warning_list').css("display","none");
  }
}

function socket(adress){
    $('#status').attr('onClick','');
    $('#status').css('cursor','default');
    // Let us open a web socket
    ws = new WebSocket("ws://"+adress+"/");

    ws.onopen = function(){
      //alert("Connected");
      $('#warning_list').empty();
      socket_tries = 0;
      $('#status').attr('src','./img/status_g.png');
    };

    ws.onmessage = function (evt){
      var received_msg = evt.data;
      //console.log(received_msg);
      data = JSON.parse(received_msg);
      if(data.hasOwnProperty('M')){
        console.log("Track update");
        track_update(data['M']);
      }else if(data.hasOwnProperty('SW')){
        console.log("Switch update");
        switch_update(data['SW']);
      }else if(data.hasOwnProperty('Mod')){
        console.log("MSSwitch update");
        ms_switch_update(data['Mod']);
      }else if(data.hasOwnProperty('Message')){
        console.log("Message update");
        console.log(data['Message']);
        message_update(data['Message']);
      }else if(data.hasOwnProperty('Setup')){
        console.log("Setup update");
        create_track(data['Setup']);
      }else if(data.hasOwnProperty('RNTrain')){
        console.log("Request for new train");
        if(data['RNTrain'][0] == "Confirmed"){
          //Start upload file and reload list
          succ_add_train(data['RNTrain'][1]);
          train_list_t += "<option value=\""+data['RNTrain'][1][0]+"\">#"+data['RNTrain'][1][2]+"&nbsp;&nbsp;&nbsp;"+data['RNTrain'][1][1]+"</option>"
        }else{
          alert("Error\n"+data['RNTrain'][0]);
          fail_add_train();
        }
      }else{
        console.log(data);
      }
      //  writeToScreen(received_msg);
      //alert("Message is received...\n"+received_msg);
    };

    ws.onclose = function(event){
      // websocket is closed.
      console.log("Connection closed" + event.code);
      if(socket_tries == 0){
        console.log("Connection Closed\nRetrying....");
        $('#status').attr('src','./img/status_ow.gif');
        $('#warning_list').css('display','none');
        //Resetting values
        active_trains = [];
      }
      if(socket_tries < 5){
        setTimeout(function(){
          console.log("Reconnecting...");
          socket(adress);
          socket_tries++;
        }, 5000);
      }else{
        console.log("No Connection posseble\nIs the server on?");
        $('#status').attr('src','./img/status_r.png');
        $('#status').css('cursor','pointer');
        $('#status').attr('onClick','socket_tries = 0;socket(window.location.host+":9000")');
      }
    };

}

function train_add_request(){

}

function new_train(){

}

$(document).ready(function(){
  $.ajax({
    url: './../modules/list.txt',
    success: function (result) {
        segments = JSON.parse(result);
    },
    async: false
  });

  if ("WebSocket" in window){
    socket(window.location.host+':9000');
  }else{
    // The browser doesn't support WebSocket
    alert("WebSocket NOT supported by your Browser!\nBIG PROBLEM!!");
    $('#status').attr('src','./img/status_w.png');
  }

  //$.ajaxSetup({ cache: false });
});
