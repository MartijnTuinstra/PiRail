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
var rot = 0;

for(var i = 0;i<10;i++){
  switches[i] = [];
  for(var j = 0;j<15;j++){
    switches[i][j] = [];
  }
}

var vara;

var remote_Switch2 = function(evt){
  console.log("remote_Switch2");
  console.log(evt);
  var target = $(evt.currentTarget);

  SwNr = parseInt(target.attr("class").split(' ')[0].slice(2));
  Module = parseInt(target.parent().parent().parent().attr("class").split(' ')[0].slice(1))
  console.log(Module+":"+SwNr);

  ws.send(String.fromCharCode(40)+String.fromCharCode(Module)+String.fromCharCode(SwNr));
}



function dir_train(obj,TrainNr,dir){
	if(!($(obj).hasClass('selected'))){
		var str = [32,(TrainNr >> 8),(TrainNr & 0xFF),dir];

		var data2 = new Int8Array(str);

		ws.send(data2);
	}
}

var remote_request = function(D){
  ws.send(D);
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

function train_funtion_svg(type,NR){
  var text = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 26 26" style="width:40px;margin:5px;">';
  if(type.charAt(0) == 'D'){
    text += '<rect class="dbox" x="0.5" y="0.5" width="25" height="25" rx="7.42" ry="7.42"/>';
  }
  else if(type.charAt(0) == 'E'){
    text += '<rect class="ebox" x="0.5" y="0.5" width="25" height="25" rx="7.42" ry="7.42"/>';
  }
  else if(type.charAt(0) == 'U'){
    text += '<rect class="line" x="0.5" y="0.5" width="25" height="25" rx="7.42" ry="7.42"/>';
  }

  if(type == "D_default" || type == "E_default"){
    text += '<text class="dtext" text-anchor="middle" x="13" y="16.52">F'+NR+'</text>';
  }
  else if(type == "U_default"){
    text += '<text class="dutext" text-anchor="middle" x="13" y="16.52">F'+NR+'</text>';
  }
  else if(type == "D_light" || type == "E_light"){
    text += '<text class="text" transform="translate(9.22 23.71)">F'+NR+'</text><path class="img" d="M8.48,14a.68.68,0,0,1-.3.27l-1.26.57a.54.54,0,0,1-.41,0,.41.41,0,0,1-.28-.23.42.42,0,0,1,0-.36A.59.59,0,0,1,6.53,14l1.28-.57a.51.51,0,0,1,.39,0,.41.41,0,0,1,.27.23A.37.37,0,0,1,8.48,14Z"/>';
    text += '<path class="img" d="M13.44,18.18a.57.57,0,0,1-.14.38.42.42,0,0,1-.33.16.39.39,0,0,1-.32-.16.61.61,0,0,1-.12-.38V16.8a.61.61,0,0,1,.12-.38.39.39,0,0,1,.32-.16.42.42,0,0,1,.33.16.57.57,0,0,1,.14.38Z"/>';
    text += '<path class="img" d="M19.76,14.6a.41.41,0,0,1-.28.23.54.54,0,0,1-.41,0l-1.26-.57a.68.68,0,0,1-.3-.27.37.37,0,0,1,0-.37.41.41,0,0,1,.27-.23.51.51,0,0,1,.39,0l1.28.57a.59.59,0,0,1,.28.28A.42.42,0,0,1,19.76,14.6Z"/>';
    text += '<path class="img" d="M17.45,10.38a.67.67,0,0,1,.36-.2L19.19,10a.54.54,0,0,1,.38.09.51.51,0,0,1,.21.31.34.34,0,0,1-.1.35.58.58,0,0,1-.35.17L18,11.1a.51.51,0,0,1-.62-.37A.41.41,0,0,1,17.45,10.38Z"/>';
    text += '<path class="img" d="M17.28,17.42a.53.53,0,0,1-.19.32.48.48,0,0,1-.37,0,.52.52,0,0,1-.32-.25l-.74-1.18a.57.57,0,0,1-.1-.38.47.47,0,0,1,.17-.31.4.4,0,0,1,.37-.06.58.58,0,0,1,.32.26L17.2,17A.54.54,0,0,1,17.28,17.42Z"/>';
    text += '<rect class="img" x="11.12" y="4.88" width="3.75" height="1.06" rx="0.25" ry="0.25"/>';
    text += '<path class="img" d="M9.31,12.28a.11.11,0,0,0,0,0v0a2.2,2.2,0,0,1,0-.33l0-.33a5.34,5.34,0,0,1,.44-1.31q.32-.67.72-1.43a4.88,4.88,0,0,0,.35-.93q.1-.41.15-.7a2.09,2.09,0,0,1,.12-.47.3.3,0,0,1,.25-.2h3.16a.29.29,0,0,1,.26.2,3.68,3.68,0,0,1,.12.47q.06.3.16.7a4.86,4.86,0,0,0,.35.93l.69,1.43a5.35,5.35,0,0,1,.44,1.31l0,.33a2.2,2.2,0,0,1,0,.33.07.07,0,0,1,0,0v.1a2.88,2.88,0,0,1-1,2.41,4.12,4.12,0,0,1-2.63.83h0a4.94,4.94,0,0,1-1.47-.21,3.39,3.39,0,0,1-1.17-.62,2.87,2.87,0,0,1-.78-1,3.25,3.25,0,0,1-.28-1.39A.11.11,0,0,1,9.31,12.28Z"/>';
    text += '<path class="img" d="M8.8,17l.76-1.16a.58.58,0,0,1,.32-.26.4.4,0,0,1,.37.06.47.47,0,0,1,.17.31.57.57,0,0,1-.1.38l-.74,1.18a.52.52,0,0,1-.32.25.48.48,0,0,1-.37,0A.5.5,0,0,1,8.8,17Z"/>';
    text += '<path class="img" d="M6.66,10.88,8,11.1a.51.51,0,0,0,.62-.37.41.41,0,0,0-.11-.35.67.67,0,0,0-.36-.2L6.81,10a.54.54,0,0,0-.38.09.51.51,0,0,0-.21.31.34.34,0,0,0,.1.35A.58.58,0,0,0,6.66,10.88Z"/>';
    text += '<path class="img" d="M11.7,4.36h2.6q.2,0,.2-.27V3.52q0-.25-.2-.25H11.7q-.2,0-.2.25v.57Q11.5,4.36,11.7,4.36Z"/>';
  }
  else if(type == "U_light"){
    text += '<text class="utext" transform="translate(9.22 23.71)">F'+NR+'</text><path class="img" d="M8.48,14a.68.68,0,0,1-.3.27l-1.26.57a.54.54,0,0,1-.41,0,.41.41,0,0,1-.28-.23.42.42,0,0,1,0-.36A.59.59,0,0,1,6.53,14l1.28-.57a.51.51,0,0,1,.39,0,.41.41,0,0,1,.27.23A.37.37,0,0,1,8.48,14Z"/>';
    text += '<path class="uimg" d="M13.44,18.18a.57.57,0,0,1-.14.38.42.42,0,0,1-.33.16.39.39,0,0,1-.32-.16.61.61,0,0,1-.12-.38V16.8a.61.61,0,0,1,.12-.38.39.39,0,0,1,.32-.16.42.42,0,0,1,.33.16.57.57,0,0,1,.14.38Z"/>';
    text += '<path class="uimg" d="M19.76,14.6a.41.41,0,0,1-.28.23.54.54,0,0,1-.41,0l-1.26-.57a.68.68,0,0,1-.3-.27.37.37,0,0,1,0-.37.41.41,0,0,1,.27-.23.51.51,0,0,1,.39,0l1.28.57a.59.59,0,0,1,.28.28A.42.42,0,0,1,19.76,14.6Z"/>';
    text += '<path class="uimg" d="M17.45,10.38a.67.67,0,0,1,.36-.2L19.19,10a.54.54,0,0,1,.38.09.51.51,0,0,1,.21.31.34.34,0,0,1-.1.35.58.58,0,0,1-.35.17L18,11.1a.51.51,0,0,1-.62-.37A.41.41,0,0,1,17.45,10.38Z"/>';
    text += '<path class="uimg" d="M17.28,17.42a.53.53,0,0,1-.19.32.48.48,0,0,1-.37,0,.52.52,0,0,1-.32-.25l-.74-1.18a.57.57,0,0,1-.1-.38.47.47,0,0,1,.17-.31.4.4,0,0,1,.37-.06.58.58,0,0,1,.32.26L17.2,17A.54.54,0,0,1,17.28,17.42Z"/>';
    text += '<rect class="uimg" x="11.12" y="4.88" width="3.75" height="1.06" rx="0.25" ry="0.25"/>';
    text += '<path class="uimg" d="M9.31,12.28a.11.11,0,0,0,0,0v0a2.2,2.2,0,0,1,0-.33l0-.33a5.34,5.34,0,0,1,.44-1.31q.32-.67.72-1.43a4.88,4.88,0,0,0,.35-.93q.1-.41.15-.7a2.09,2.09,0,0,1,.12-.47.3.3,0,0,1,.25-.2h3.16a.29.29,0,0,1,.26.2,3.68,3.68,0,0,1,.12.47q.06.3.16.7a4.86,4.86,0,0,0,.35.93l.69,1.43a5.35,5.35,0,0,1,.44,1.31l0,.33a2.2,2.2,0,0,1,0,.33.07.07,0,0,1,0,0v.1a2.88,2.88,0,0,1-1,2.41,4.12,4.12,0,0,1-2.63.83h0a4.94,4.94,0,0,1-1.47-.21,3.39,3.39,0,0,1-1.17-.62,2.87,2.87,0,0,1-.78-1,3.25,3.25,0,0,1-.28-1.39A.11.11,0,0,1,9.31,12.28Z"/>';
    text += '<path class="uimg" d="M8.8,17l.76-1.16a.58.58,0,0,1,.32-.26.4.4,0,0,1,.37.06.47.47,0,0,1,.17.31.57.57,0,0,1-.1.38l-.74,1.18a.52.52,0,0,1-.32.25.48.48,0,0,1-.37,0A.5.5,0,0,1,8.8,17Z"/>';
    text += '<path class="uimg" d="M6.66,10.88,8,11.1a.51.51,0,0,0,.62-.37.41.41,0,0,0-.11-.35.67.67,0,0,0-.36-.2L6.81,10a.54.54,0,0,0-.38.09.51.51,0,0,0-.21.31.34.34,0,0,0,.1.35A.58.58,0,0,0,6.66,10.88Z"/>';
    text += '<path class="uimg" d="M11.7,4.36h2.6q.2,0,.2-.27V3.52q0-.25-.2-.25H11.7q-.2,0-.2.25v.57Q11.5,4.36,11.7,4.36Z"/>';
  }
  else if(type == "D_Cab_light" || type == "E_Cab_light"){
    text += '<text class="text" transform="translate(9.22 23.71)">F'+NR+'</text><path class="img" d="M15.39,6.43a.51.51,0,0,1,.65-.3l4.36,1.61a.51.51,0,0,1-.17,1,.46.46,0,0,1-.17,0L15.69,7.08a.51.51,0,0,1-.3-.65Zm-4.8-1.2c-3.38.7-5.85,2.87-5.85,5.45s2.47,4.75,5.85,5.45Zm9.82,6.59L16,10.21a.51.51,0,1,0-.35.95l4.36,1.61a.52.52,0,0,0,.17,0,.51.51,0,0,0,.17-1ZM12.91,6.45V5a11.4,11.4,0,0,0-1.82.15V16.22a11.76,11.76,0,0,0,1.82.14V14.92c.84-.14,1.51-2,1.51-4.24s-.67-4.1-1.51-4.24Zm7.49,9.45L16,14.28a.51.51,0,1,0-.35.95l4.36,1.61a.52.52,0,0,0,.17,0,.51.51,0,0,0,.17-1Zm0,0"/>';
  }
  else if(type == "U_Cab_light"){
    text += '<text class="utext" transform="translate(9.22 23.71)">F'+NR+'</text><path class="uimg" d="M15.39,6.43a.51.51,0,0,1,.65-.3l4.36,1.61a.51.51,0,0,1-.17,1,.46.46,0,0,1-.17,0L15.69,7.08a.51.51,0,0,1-.3-.65Zm-4.8-1.2c-3.38.7-5.85,2.87-5.85,5.45s2.47,4.75,5.85,5.45Zm9.82,6.59L16,10.21a.51.51,0,1,0-.35.95l4.36,1.61a.52.52,0,0,0,.17,0,.51.51,0,0,0,.17-1ZM12.91,6.45V5a11.4,11.4,0,0,0-1.82.15V16.22a11.76,11.76,0,0,0,1.82.14V14.92c.84-.14,1.51-2,1.51-4.24s-.67-4.1-1.51-4.24Zm7.49,9.45L16,14.28a.51.51,0,1,0-.35.95l4.36,1.61a.52.52,0,0,0,.17,0,.51.51,0,0,0,.17-1Zm0,0"/>';
  }
  text += '</svg>';
  return text;
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

  var end1;
  if(setup[1] == 0){
    end1 = setup.length;
  }else{
    end1 = 2+setup[1];
    console.log("Extra track");
  }

  console.log(setup);
  for(var i = 2;i<end1;i++){
    console.log(setup[i]);
    if(setup[i] == 0){
      //["Name","Width","Height","Rotation","Anchor1_x","Anchor1_y","Anchor2_x","Anchor2_y","Anchor3_rot","Anchor3_x","Anchor3_y"]
      //   0       1       2          3          4           5           6           7             8          9            10
    }else{
      var M = setup[i];

      //Create track
      //console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
      $('#Modules').append("<div class=\"M"+M+" Module M"+M+"b\"></div>");
      blocks_load++;
      $('.M'+M+'b').load('./../modules/'+M+'/layout.svg',function (evt){
        blocks_load--;
        if(blocks_load == 0){ //Done loading all modules?
          $('#Modules').css('display','block');
          ws.send("Ready");
          $('g.SwGroup','.Module').on("click",ev_throw_switch);  //Attach click event to all switch and mswitches
        }
      });
      if(r == 0){
        x -= segments[M]['anchors'][0][0];
        y -= segments[M]['anchors'][0][1];
        max_x = Math.max(max_x,x+segments[M]['width']);
        max_y = Math.max(max_y,y+segments[M]['height']);
        min_x = Math.min(min_x,x);
        min_y = Math.min(min_y,y);
      }else if(r == 90){
        x -= segments[M]['anchors'][0][1];
        y += segments[M]['anchors'][0][0];
        max_x = Math.max(max_x,x);
        max_y = Math.max(max_y,y+segments[M]['width']);
        min_x = Math.min(min_x,x-segments[M]['height']);
        min_y = Math.min(min_y,y);
      }else if(r == 180){
        x += segments[M]['anchors'][0][0];
        y += segments[M]['anchors'][0][1];
        max_x = Math.max(max_x,x);
        max_y = Math.max(max_y,y);
        min_x = Math.min(min_x,x-segments[M]['width']);
        min_y = Math.min(min_y,y-segments[M]['height']);
      }
      if(segments[M]['anchors'].length > 2){
        anchor3_x = x + segments[M]['anchors'][2][0];
        anchor3_y = y + segments[M]['anchors'][2][1];
        anchor3_r = r + segments[M]['anchors'][2][2];
      }
      //console.log("M"+setup[i]+" x:"+x+", y:"+y+", R:"+r);
      //console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
      $(".M"+setup[i]+"b").css("left",x+"px");
      $(".M"+setup[i]+"b").css("top",y+"px");
      $(".M"+setup[i]+"b").css("-ms-transform","rotate("+r+"deg)");
      $(".M"+setup[i]+"b").css("-webkit-transform","rotate("+r+"deg)");
      $(".M"+setup[i]+"b").css("transform","rotate("+r+"deg)");
      $(".M"+setup[i]+"b").css("transform-origin","0px 0px");

      if(r == 0){
        x += segments[M]['anchors'][1][0];
        y += segments[M]['anchors'][1][1];
      }else if(r == 90){
        x -= segments[M]['anchors'][1][1];
        y += segments[M]['anchors'][1][0];
      }else if(r == 180){
        x -= segments[M]['anchors'][1][0];
        y -= segments[M]['anchors'][1][1];
      }
      r += segments[M]['angle'];
      if(i == 2 && r == 90){
        r = 0;
      };
      //console.log("M"+setup[i]+" x:"+x+", y:"+y+", R:"+r);
    }

  }

  if(setup[1] != 0){
    x = anchor3_x;
    y = anchor3_y;
    r = anchor3_r;
    for(var i = end1;i<setup.length;i++){
      $('#Modules').append("<div class=\"M"+setup[i]+" Module M"+setup[i]+"b\"></div>");
      blocks_load++;
      $('.M'+setup[i]+'b').load('./../modules/'+setup[i]+'.svg',function (evt){
        blocks_load--;
        if(blocks_load == 0){
          $('#Modules').css('display','block');
          ws.send("Ready");
          $('g.SwGroup','.Module').on("click",ev_throw_switch); //Attach click event to all switch and mswitches
        }
      });
      if(r == 0){
        x += segments[M]['anchors'][0][0];
        y -= segments[M]['anchors'][0][1];
        max_x = Math.max(max_x,x+segments[M]['width']);
        max_y = Math.max(max_y,y+segments[M]['height']);
        min_x = Math.min(min_x,x);
        min_y = Math.min(min_y,y);
      }else if(r == 90){
        x += segments[M]['anchors'][0][1];
        y += segments[M]['anchors'][0][0];
        max_x = Math.max(max_x,x);
        max_y = Math.max(max_y,y+segments[M]['width']);
        min_x = Math.min(min_x,x-segments[M]['height']);
        min_y = Math.min(min_y,y);
      }else if(r == 180){
        x -= segments[M]['anchors'][0][0];
        y += segments[M]['anchors'][0][1];
        max_x = Math.max(max_x,x);
        max_y = Math.max(max_y,y);
        min_x = Math.min(min_x,x-segments[M]['width']);
        min_y = Math.min(min_y,y-segments[M]['height']);
      }
      console.log("M"+setup[i]+" x:"+x+", y:"+y+", R:"+r);
      console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
      $(".M"+setup[i]+"b").css("left",x+"px");
      $(".M"+setup[i]+"b").css("top",y+"px");
      $(".M"+setup[i]+"b").css("-ms-transform","rotate("+r+"deg)");
      $(".M"+setup[i]+"b").css("-webkit-transform","rotate("+r+"deg)");
      $(".M"+setup[i]+"b").css("transform","rotate("+r+"deg)");
      $(".M"+setup[i]+"b").css("transform-origin","0px 0px");

      if(r == 0){
        x += segments[M]['anchors'][1][0];
        y += segments[M]['anchors'][1][1];
      }else if(r == 90){
        x -= segments[M]['anchors'][1][1];
        y += segments[M]['anchors'][1][0];
      }else if(r == 180){
        x -= segments[M]['anchors'][1][0];
        y -= segments[M]['anchors'][1][1];
      }
      r += segments[M]['angle'];
      if(i == 2 && r == 90){
        r = 0;
      };
    }
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
/*
function Link_train(list,tID){
  var val;
  if(tablet == 0){
    val = $('#W'+$(list).attr('Did')+'_11 .cs-selected').attr('data-value');
  }else{
    console.log("tablet == 1");
    console.log($(list).attr('Did')+'_11');
    val = $('#W'+$(list).attr('Did')+'_11 .cs-select').val();
  }
  if(typeof val != 'undefined'){
    console.log(val);
    remote_addTrain(tID,parseInt(val));
  }else{
    alert('Please select a train');
  }
}
*/
function station_list_update(data){
  $('#Station_List select').empty();
  var x = 0;
  for(var i = 1;i<(data.length-1);){
    var M = data[i++];
    var S = data[i++];
    var L = data[i++];
    var text = "";
    for(var j = 0;j<L;j++){
      text += String.fromCharCode(data[i++]);
    }
    console.log("Station "+i+": "+text+" at "+M);
	Station_list[x] = {M,S,L,text};
	Station_list_t += "<option value=\""+x+"\">"+text+"</option>";
	x++;
  }
  $('#Station_List select').append(Station_list_t);
}
/*
function message_update(response){
  /*
  if(response.length != 0){
    $('#warning_list').css("display","block");
    $('#notify').attr('src','./img/notification_y.png');
  }*/
  /*if(response[1] == 1){ //Emergency Stop
    console.log("Emergency Stop");
    console.log('#warning_list #W_EmS');
    if($('#warning_list #W_EmS').length == 0){
      var text = "<div id=\"W_EmS\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/stop.png\"/></div>";
      text += "<div class=\"photobox r\"><img src=\"./img/stop.png\"/></div><center><h2>EMERGENCY STOP</h2><br/><but onClick=\"remote_request('Er')\">";
      text += "<b>Resume</b></but></center></div>";
      $('#warning_list').prepend(text);
      $('#warning_list').css("display","block");
    }
  }
  else if(response[1] == 2){ //Emergency stop release
    console.log("Emergency stop release");
    console.log('#warning_list #W_EmS');
    if($('#warning_list #W_EmS').length != 0){
      $('#warning_list #W_EmS').remove();
    }
  }
  else if(response[1] == 3){ //Electrical short
    console.log("Electrical short EsS");
    console.log('#warning_list #W_EsS');
    if($('#warning_list #W_EsS').length == 0){
      var text = "<div id=\"W_EsS\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/shortcircuit.png\"/></div>";
      text += "<div class=\"photobox r\"><img src=\"./img/shortcircuit.png\"/></div><center><h2>SHORT CICRUIT</h2><br/><but onClick=\"remote_request('Ec')\">";
      text += "<b>Resume</b></but></center></div>";
      $('#warning_list').prepend(text);
      $('#warning_list').css("display","block");
    }
  }
  else if(response[1] == 4){ //Electrical short stop release
    console.log("Electrical short stop release");
    console.log('#warning_list #W_EsS');
    if($('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1)).length != 0){
      $('#warning_list #W'+response[i][1]+"_"+(response[i][0]-1)).remove();
    }
  }
  else *//*if(response[1] == 5){ //Split train
    //2 = Train follow id
    //3 = Split Address M
    //4 =              B
    //5 =             S

    console.log("A Train (#"+response[2]+") has split.");
    console.log('#warning_list #W5_'+response[2]);
  }
  else if(response[1] == 6){ //Split train release
    //2 = Train follow id

    console.log('#warning_list #W5_'+response[2]);
    if($('#warning_list #W5_'+response[2]).length != 0){
      $('#warning_list #W5_'+response[2]).remove();
    }
  }
  else if(response[1] == 12){//New train_link
    //2 = Train follow id
    //3 = Train ID high byte
    //4 = Train ID low byte
    var DCC_T = (response[4] << 8) + response[5];
    var train_ID = response[3];
    var follow_ID = response[2];

    console.log("New train "+response[2]+" remove");
    console.log('#warning_list #W'+response[2]+"_11");
    if($('#warning_list #W'+response[2]+"_11").length != 0){
      $('#warning_list #W'+response[2]+"_11").remove();
    }

    console.log("Adding train t"+train_ID+" (#"+DCC_T+"=f"+follow_ID+") to active trains");
    //active_trains[active_trains.length] = [response[2], train_ID];
    active_trains[DCC_T] = {"f":follow_ID,"t":train_ID};
    //$('#CTrain').append("<div id=\"T  "+response[2]+"\" class=\"trainbox\"><img src=\"./../trains/"+train_ID+".jpg\" style=\"max-width:100%\"/>"+train_ID+"|"+train_ID+"</div>");

  }

  if($("#warning_list").html() == ""){
    $('#warning_list').css("display","none");
    $('#notify').attr('src','./img/notification.png');
  }else{
    $('#notify').attr('src','./img/notification_y.png');
  }
}
*/
function create_train_slider(DCC_ID,max_speed,speed_step){
	var handle = $("#T"+DCC_ID+" .slider-handle" );

	$( "#T"+DCC_ID+" .slider" ).slider({
		orientation: "vertical",
		range: "min",
		min: 0,
		max: max_speed,
		step: max_speed/127,
		value: ((speed_step / 127) * max_speed),
		create: function(event, ui) {
			handle.html(Math.round(ui.value));
			//handle.text( $( this ).slider( "value" ) );
		},
		slide: function( event, ui ) {
			handle.html(Math.round(ui.value));
		},
		change: (function(event, ui){
			if(event.originalEvent){
				handle.html(Math.round(ui.value));
				console.log("User Change speed");

				var ID = parseInt($(this).attr("tid"));

        ev_Train_speed(ID,(Math.round((ui.value*127)/max_speed)));
			}
		})
	});
}
/*
function train_data_update(data){
  console.log('Train_Data_Update');
  console.log(active_trains);
  if(data.length > 5){
    var DCC_ID = (data[1] << 8) + data[2];

    if($('#T'+DCC_ID).length != 0){
      console.log('Known');
      console.log(DCC_ID);

      var speed_step = (data[4] & 0x7F);
      var max_speed = parseInt(train_list[train_list_c[active_trains[DCC_ID]['t']]][4]);

	  console.log('Speed: '+speed_step);

      $("#T"+DCC_ID+" .slider" ).slider('value',((speed_step / 127)*max_speed));
	  $("#T"+DCC_ID+" .slider-handle" ).html(Math.round((speed_step / 127)*max_speed));

	  if((data[4] & 0x80) == 0){
		  //Forward
		  $('#T'+DCC_ID+" .dir_right").addClass('selected');
		  $('#T'+DCC_ID+" .dir_left").removeClass('selected');
	  }else{
		  //Reverse
		  $('#T'+DCC_ID+" .dir_left").addClass('selected');
		  $('#T'+DCC_ID+" .dir_right").removeClass('selected');
	  }

    }
    else{
      //console.log(data[6]+'<<+'+data[7]+'=='+DCC_ID);
	  console.log("New train");
      console.log(active_trains[DCC_ID]);
      var text = "";
      try{
      text += '<div id="T'+DCC_ID+'" class="trainbox">';
      text += '<div style="width:calc(100% - 60px);height:250px;float:left;">';
      text += '<div class="image_box">';
      text += '<img src="./../trains/'+train_list[train_list_c[active_trains[DCC_ID]['t']]][0]+'.jpg">';
      text += '</div>';
      text += '<div class="name_tag">';
      text += '<span class="title">'+train_list[train_list_c[active_trains[DCC_ID]['t']]][1]+'</span>';
      text += '<span class="value">#'+DCC_ID+'</span>';
      text += '<br/></div>';
      text += '<div class="control_tag">';
      text += '<span class="title">Control</span>';
      text += '<span class="value">Manual</span>';
      text += '<br/></div>';
      text += '<div class="route_tag">';
      text += '<span class="title">Route</span>';
      text += '<span class="value">None</span>';
      text += '<svg viewBox="0 0 16 16" class="route_plus" onClick="open_Route(this);"><circle fill="#bbb" cx="8" cy="8" r="6.11"/><path d="M8,0a8,8,0,1,0,8,8A8,8,0,0,0,8,0Zm4,8.8H8.8V12H7.2V8.8H4V7.2H7.2V4H8.8V7.2H12Zm0,0"/></svg>';
      text += '<br/></div>';
      text += '<div class="dir_tag">';
      if((data[4] & 0x80) == 0){
        text += '<svg viewBox="0 0 830.48 233.25" class="dir_arrow"><path onClick="dir_train(self,'+DCC_ID+',1)" class="dir_left selected" d="M385.87,60.27h-238V5.07a2.14,2.14,0,0,0-3.44-1.62L3.79,115a2.07,2.07,0,0,0,0,3.24L144.44,229.8a2.09,2.09,0,0,0,1.29.45,2.33,2.33,0,0,0,1-.2,2.13,2.13,0,0,0,1.23-1.87V173H385.8a2.05,2.05,0,0,0,2.14-2V62.52a2.35,2.35,0,0,0-2.2-2.26"/><path onClick="dir_train(self,'+DCC_ID+',0)" class="dir_right" d="M444.62,173h238v55.2A2.14,2.14,0,0,0,686,229.8L826.7,118.25a2.07,2.07,0,0,0,0-3.24L686,3.45A2.09,2.09,0,0,0,684.75,3a2.33,2.33,0,0,0-1,.2,2.13,2.13,0,0,0-1.23,1.87v55.2H444.69a2.05,2.05,0,0,0-2.14,2V170.73a2.35,2.35,0,0,0,2.2,2.26"/></svg>';
      }else{
        text += '<svg viewBox="0 0 830.48 233.25" class="dir_arrow"><path onClick="dir_train(self,'+DCC_ID+',1)" class="dir_left" d="M385.87,60.27h-238V5.07a2.14,2.14,0,0,0-3.44-1.62L3.79,115a2.07,2.07,0,0,0,0,3.24L144.44,229.8a2.09,2.09,0,0,0,1.29.45,2.33,2.33,0,0,0,1-.2,2.13,2.13,0,0,0,1.23-1.87V173H385.8a2.05,2.05,0,0,0,2.14-2V62.52a2.35,2.35,0,0,0-2.2-2.26"/><path onClick="dir_train(self,'+DCC_ID+',0)" class="dir_right selected" d="M444.62,173h238v55.2A2.14,2.14,0,0,0,686,229.8L826.7,118.25a2.07,2.07,0,0,0,0-3.24L686,3.45A2.09,2.09,0,0,0,684.75,3a2.33,2.33,0,0,0-1,.2,2.13,2.13,0,0,0-1.23,1.87v55.2H444.69a2.05,2.05,0,0,0-2.14,2V170.73a2.35,2.35,0,0,0,2.2,2.26"/></svg>';
      }
	    text += '</div>';

      if((data[3] & 0b1000) != 0){
        text += 'Andere X-BUS Handregler<br/>';
      }
      if((data[3] & 0b111) == 0){
        text += '14 steps<br/>';
      }
      else if((data[3] & 0b111) == 2){
        text += '28 steps<br/>';
      }
      else if((data[3] & 0b111) == 4){
        text += '128 steps<br/>';
      }

      text += 'Fahrstufen KKK: ' + (data[5] & 0x7F) + '<br/>'

      text += 'Options active:<br/>';

      if((data[5] & 0x40) == 1){
        text += 'Doppeltraktion<br/>';
      }

      if((data[5] & 0x20) == 1){
        text += 'Smartsearch<br/>';
      }

      if((data[5] & 0x10) == 1){
        text += 'F0<br/>';
      }

      if((data[5] & 0x8) == 1){
        text += 'F4<br/>';
      }

      if((data[5] & 0x4) == 1){
        text += 'F3<br/>';
      }

      if((data[5] & 0x2) == 1){
        text += 'F2<br/>';
      }

      if((data[5] & 0x1) == 1){
        text += 'F1<br/>';
      }

      text += '<br/></div>';

      text += '<div style="width:50px;margin-left:10px;height:250px;float:left;">';
      text += '<div class="slider" Did="T'+DCC_ID+'" style="height:210px;margin:0px auto;margin-top:20px;">';
      text += '<div class="ui-slider-handle slider-handle"></div></div></div>';
      text += '<div class="prim_fn">';

        text += train_funtion_svg("D_light",0);
        text += train_funtion_svg("D_Cab_light",1);
        text += train_funtion_svg("D_Cab_light",2);
        text += train_funtion_svg("D_default",3);
        text += train_funtion_svg("D_default",4);
        text += train_funtion_svg("D_default",5);

      text += '</div><div class="secu_fn"><div class="D1"><div class="D2">';
        for(var i = 6;i<29;i++){
          text += train_funtion_svg("U_default",i);
        }
      text += '</div></div></div></div>';

      $('#CTrain').append(text);

      var speed_step = (data[4] & 0x7F);
      var max_speed = parseInt(train_list[train_list_c[active_trains[DCC_ID]['t']]][4]);

    }catch(e){
      
    }


      create_train_slider(DCC_ID,max_speed,speed_step);

    }
  }
}
*/
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

  var bg = $('#T1200 .prim_fn')[0];

  $.ajax({ type: "GET",
     url: './img/Functions/F_Cab_Light_Disable.svg',
     success : function(text)
     {
         $('#T1200 .prim_fn').append(text.documentElement);
     }
  });
  $.ajax({ type: "GET",
     url: './img/Functions/F_Cab_Light_Enable.svg',
     success : function(text)
     {
         $('#T1200 .prim_fn').append(text.documentElement);
     }
  });
  $.ajax({ type: "GET",
     url: './img/Functions/F_Cab_Light_Unavailable.svg',
     success : function(text)
     {
         $('#T1200 .prim_fn').append(text.documentElement);
     }
  });
  $.ajax({ type: "GET",
     url: './img/Functions/F_Default_Disable.svg',
     success : function(text)
     {
         $('#T1200 .prim_fn').append(text.documentElement);
     }
  });
  $.ajax({ type: "GET",
     url: './img/Functions/F_Default_Enable.svg',
     success : function(text)
     {
         $('#T1200 .prim_fn').append(text.documentElement);
     }
  });

    var bg = $('#T1200 .secu_fn div')[0];

    $.ajax({ type: "GET",
       url: './img/Functions/F_Default_Unavailable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Horn_Disable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Horn_Enable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Horn_Unavailable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Light_Disable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Light_Enable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Light_Unavailable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Whistle_Disable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Whistle_Enable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Whistle_Unavailable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Whistle_Disable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Whistle_Enable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });
    $.ajax({ type: "GET",
       url: './img/Functions/F_Whistle_Unavailable.svg',
       success : function(text)
       {
           $('#T1200 .secu_fn div div').append(text.documentElement);
       }
    });


  /*
  $('<img src="'+ imgPath +'">').load(function() {
    $(this).width(some).height(some).appendTo('#some_target');
  });*/

  //$.ajaxSetup({ cache: false });
});

function LoadImgManageTrains(){
  $('#MTrain').children().each(function (index,element){
    var classes = $(element).attr('class');
    if(typeof classes !== "undefined"){
      classes = classes.split(/\s+/);
      n = parseInt(classes[1].substring(4,classes[1].length));

      var img = $("<img />").attr('src', './../trains/'+(train_list[n][0])+'.jpg').on('load', function() {
          if (!this.complete || typeof this.naturalWidth == "undefined" || this.naturalWidth == 0) {
              alert('broken image!');
          } else {
              $(".img",element).append(img);
          }
      });
    }
  });
  ImgMTrains = true;
}
