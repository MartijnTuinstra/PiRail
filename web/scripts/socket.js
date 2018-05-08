


//Opcodes
var WSopc_Track_Scan         = 0x82;
var WSopc_Track_PUp_Layout   = 0x83;
var WSopc_Track_Info         = 0x84;


var WSopc_AddNewTrain        = 0x40;
var WSopc_LinkTrain          = 0x41;
var WSopc_TrainSpeed         = 0x42;
var WSopc_TrainFunction      = 0x43;
var WSopc_TrainOperation     = 0x44;
var WSopc_Z21TrainData       = 0x45;
var WSopc_TrainAddRoute      = 0x46;
var WSopc_TrainClearRoute    = 0x47;

var WSopc_SetSwitch       = 0x20;

var WSopc_SetSwitchReserved  = 0x24;
var WSopc_BroadTrack         = 0x26;
var WSopc_BroadSwitch        = 0x27;
var WSopc_Track_Layout       = 0x30;

var WSopc_EmergencyStop      = 0x10;
var WSopc_ShortCircuitStop   = 0x11;
var WSopc_ClearEmergency     = 0x12;
var WSopc_NewMessage         = 0x13;
var WSopc_ChangeMessage      = 0x14;
var WSopc_ClearMessage       = 0x15;
var WSopc_ChangeBroadcast    = 0x16;
var WSopc_Service_State      = 0x17;


//Service States

var STATE_Z21_FLAG        = 0x8000;
var STATE_WebSocket_FLAG  = 0x4000;
var STATE_COM_FLAG        = 0x2000;
var STATE_Client_Accept   = 0x1000;

var STATE_TRACK_DIGITAL   = 0x0200;
var STATE_RUN             = 0x0100;

var STATE_Modules_Loaded  = 0x0002;
var STATE_Modules_Coupled = 0x0004;

var admin;

var active_trains = []; //Trains that were active on the layout.
var broadcastFlags = 0xFF;

var Track_Layout_data;
var blocks_load = 0;

/*Client to Server*/
  function ev_set_switch(m,s,st){ //Module, Switch, NewState
    console.log("Set switch "+m+":"+s+"=>"+st);
    ws.send(String.fromCharCode(WSopc_SetSwitch)+String.fromCharCode(m)+String.fromCharCode(s)+String.fromCharCode(st));
  }

  function ev_release_Emergency(evt){
    ws.send(String.fromCharCode(WSopc_ClearEmergency));
  }

  function ev_Emergency(evt){
    ws.send(String.fromCharCode(WSopc_EmergencyStop));
  }

  function ev_Toggle_Broadcast_Flag(evt){
    console.log(evt);
    if($(evt.target).attr("nr") != undefined){
      console.log(parseInt($(evt.target).attr("nr")));
      broadcastFlags ^= 1 << parseInt($(evt.target).attr("nr"));

      if($(evt.target).attr("nr") == "4"){
        if(broadcastFlags & 0x10){
          //LOGIN
          var passphrase = $.md5(prompt("Please enter your password", "password"));

          $.post('./login.php', {'pass':passphrase}, function(response) {
              // Log the response to the console
              console.log("Response: "+response);
              if(response == "LOGIN SUCCESFULL"){
                //PHP succesfull
                //now websocket
                var data = [];
                data[0] = 0xFF; //Admin login opcode
                for(var i = 0;i<passphrase.length;i++){
                  data[i+1] = passphrase.charCodeAt(i);
                }

                var data = new Uint8Array(data);

                ws.send(data);
              }else{
                alert("WRONG PASSWORD\n"+response);
              }
          });
        }else{
          //Logout
          $.post('./login.php', {'pass':''}, function(response) {
              // Log the response to the console
              console.log("Response: "+response);
              if(response == "LOGIN SUCCESFULL"){
                alert("SUCCESFULL LOGIN");
              }else{
                alert("WRONG PASSWORD");
              }
          });
        }
      }


      console.log("New broadcast flag: "+broadcastFlags);
      ws.send(new Uint8Array([WSopc_ChangeBroadcast,broadcastFlags]));
    }
  }

  function ev_Stop_Scan(){
    if(admin){
      var data = [];

      data[0] = WSopc_Track_Scan;
      data[1] = 1;

      ws.send(new Int8Array(data));
    }else{
      console.log("No Admin");
    }
  }
/**/
/*Server to Client*/
  function ws_switch_update(data){
    for(var i = 1;i<data.length;){
      var M = data[i];

      if(data[i+1] & 0x80){ //MSSwitch
        var len   = data[i+3];

        modules[M].switches[data[i+1]&0x7F] = data[i+2] & 0x7F;

        i += 4;
      }else{ //Switch
        modules[M].switches[data[i+1]] = data[i+2] & 0x7F;

        i += 3;
      }
    }

    update_frame();
  }

  function ws_track_update(data){
    for(var i = 1;i<data.length;i+=4){
      var m = data[i];
      var B = data[i+1];

      var D = data[i+2] >> 7;
      var blocked = (data[i+2] & 0b00010000) >> 4;
      var state = (data[i+2] & 0xF);
      var tID = data[i+3];

      console.log("Block "+m+":"+B+"\tdir: "+D+"\tBlocked: "+blocked+"\tstate: "+state+"\tTrain: "+tID);

      if(blocked == 1){
        modules[m].blocks[B] = 0;
      }else{
        if(state == 0){ //No train / Grey
          modules[m].blocks[B] = 5;
        }else if(state == 1){ //SLOW
          modules[m].blocks[B] = 2;
        }else if(state == 2){ //Red
          modules[m].blocks[B] = 1;
        }else if(state == 3){ //UNKOWN
          modules[m].blocks[B] = 6;
        }else if(state == 4){ //GHOST
          modules[m].blocks[B] = 3;
        }else if(state == 5){ //RESERVED
          modules[m].blocks[B] = 4;
        }
      }
    }

    update_frame();
  }

  function ws_partial_layout(data){
    console.warn("TODO: implement");return;
    // for(var i = 1;i<data.length;i++){
    //   for(var j = 1;j<=Track_Layout_data[data[i]].anchor_len;j++){
    //     Track_Layout_data[data[i]].anchors[j-1] = data[i+j];
    //   }
    //   i += Track_Layout_data[data[i]].anchor_len;
    // }
  }

  function ws_layout(data){
    console.warn("TODO: implement");return;
    // for(var i = 1;i<data.length;i++){
    //   for(var j = 1;j<=Track_Layout_data[data[i]].anchor_len;j++){
    //     Track_Layout_data[data[i]].anchors[j-1] = data[i+j];
    //   }
    //   i += Track_Layout_data[data[i]].anchor_len;
    // }

    // console.log("Finding first module");
    // $.each(Track_Layout_data, function(i,v){
    //   console.log("Module "+i);
    //   if(Track_Layout_data[i] != [] && Track_Layout_data[i].anchors.length != 0 && i == 20){
    //     Track_Layout_data[i].x = 0;
    //     Track_Layout_data[i].y = 0;
    //     Track_Layout_data[i].r = 0;

    //     $('#Modules').append("<div class=\"M"+i+" Module M"+i+"b\"></div>");
    //     blocks_load++;
    //     console.log("Load: "+'./../modules/'+i+'/layout.svg');
    //     $('.M'+i+'b').load('./../modules/'+i+'/layout.svg',function (evt){
    //       blocks_load--;
    //       if(blocks_load == 0){ //Done loading all modules?
    //         $('#Modules').css('display','block');
    //         ws.send("Ready");
    //         $('g.SwGroup','.Module').on("click",ev_throw_switch);  //Attach click event to all switch and mswitches
    //       }
    //     });

    //     Track_Layout_data[i].done = true;

    //     for(var j = 0;j<Track_Layout_data[i].anchor_len;j++){
    //       if(Track_Layout_data[i].anchors[j] != 0){
    //         console.log("=============== "+j+" ===============");
    //         track_layout(Track_Layout_data[i].anchors[j],i);
    //       }
    //     }
    //     return false;
    //   }
    // });
  }

  function ws_emergency(type){
    console.warn("TODO: reimplement");return;
    if(type == 0){ //Emergency stop
      if($('#warning_list #W_EmS').length == 0){
        var text = "<div id=\"W_EmS\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/stop.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/stop.png\"/></div><center><h2>EMERGENCY STOP</h2><br/><but onClick=\"ev_release_Emergency()\">";
        text += "<b>Resume</b></but></center></div>";
        $('#warning_list').prepend(text);
        $('#warning_list').css("display","block");
      }

    }else if(type == 1){ //Short Circuit
      if($('#warning_list #W_EsS').length == 0){
        var text = "<div id=\"W_EsS\" class=\"warning\"><div class=\"photobox l\"><img src=\"./img/shortcircuit.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/shortcircuit.png\"/></div><center><h2>SHORT CICRUIT</h2><br/><but onClick=\"ev_release_Emergency()\">";
        text += "<b>Resume</b></but></center></div>";
        $('#warning_list').prepend(text);
        $('#warning_list').css("display","block");
      }

    }else if(type == 2){ //Release
      if($('#warning_list #W_EmS').length != 0){
        $('#warning_list #W_EmS').remove();
      }
      if($('#warning_list #W_EsS').length != 0){
        $('#warning_list #W_EsS').remove();
      }
    }
  }

  function ws_message(data){
    console.warn("TODO: reimplement");return;
    var msgID = data[2] + ((data[1] & 0x1F) << 8);

    //Check type
    if((data[1] & 0xE0) == 0){ //New train

      //Check if message is not used
      if($('#warning_list #W'+msgID+"_11").length == 0){
        var text = "<div id=\"W"+msgID+"\" class=\"warning\" style=\"background-color:grey;\"><div class=\"photobox l\"><img src=\"./img/train.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/train.png\"/></div><center><h2>New Train</h2> at "+data[4]+":"+data[5];
        text += "<div style=\"width:calc(100% - 100px);height:40px;\"><div style=\"float:left;color:black;width:300px\"><select class=\"cs-selectn cs-select cs-skin-rotate\"><option value=\"\" disabled selected>Select train</option>";
        text += train_option+"</select></div><but style=\"float:right;position:relative;top:6px\"";
        text += "Mid=\""+msgID+"\" fID=\""+data[3]+"\">";
        text += "<b>Link</b></but></div></center></div>";
        $('#warning_list').append(text);
        $('but','#warning_list #W'+msgID).on('click',ev_LinkTrain);
        if(tablet == 0){
          redraw_selects('#W'+msgID);
        }
      }

    }
    else if((data[1] & 0xE0) == 0x20){ //Split train

      //Check if message is not used
      if($('#warning_list #W'+msgID).length == 0){
        var text = "<div id=\"W"+msgID+"\" class=\"warning\" style=\"background-color:light-red;\"><div class=\"photobox l\"><img src=\"./img/train.png\"/></div>";
        text += "<div class=\"photobox r\"><img src=\"./img/train.png\"/></div><center><h2>A Train ("+data[3]+") has split</h2><br/>One part is at "+data[4]+":"+data[5]+", the other part is in "+data[6]+":"+data[7];
        text += "<br/><but onClick=\"remote_request('Ec')\">";
        text += "<b>Resume</b></but></center></div>";
        $('#warning_list').append(text);
      }
    }


    //Set bell icon
    if($("#warning_list").html() == ""){
      $('#warning_list').css("display","none");
      $('#notify').attr('src','./img/notification.png');
    }else{
      $('#notify').attr('src','./img/notification_y.png');
    } 
  }

  function ws_clearmessage(data){
    console.warn("TODO: reimplement");return;
    var msgID = data[2] + ((data[1] & 0x1F) << 8);
    $('#warning_list #W'+msgID).remove();
    //Set bell icon
    if($("#warning_list").html() == ""){
      $('#warning_list').css("display","none");
      $('#notify').attr('src','./img/notification.png');
    }else{
      $('#notify').attr('src','./img/notification_y.png');
    } 
  }

  function ws_full_train_data(data){
    console.warn("TODO: reimplement");return;
    console.log('Train_Data_Update');
    console.log(active_trains);
    if(data.length > 5){
      var train_ID = data[1];
      var DCC_ID = (data[2] << 8) + data[3];

      if($('#T'+DCC_ID).length != 0){
        console.log('Known');
        console.log(DCC_ID);

        var speed_step = (data[5] & 0x7F);
        var max_speed = parseInt(train_list[train_ID][4]);

        console.log('Speed: '+speed_step);

        $("#T"+DCC_ID+" .slider" ).slider('value',((speed_step / 127)*max_speed));
        $("#T"+DCC_ID+" .slider-handle" ).html(Math.round((speed_step / 127)*max_speed));
        train_list[train_ID].data.speed = speed_step;

        if((data[5] & 0x80) == 0){
          train_list[train_ID].data.direction = 0x0;
          //Reverse
          console.log("Reverse");
          $('#T'+DCC_ID+" .dir_right").addClass('selected');
          $('#T'+DCC_ID+" .dir_left").removeClass('selected');
        }else{
          //Forward
          train_list[train_ID].data.direction = 0x80;
          console.log("Forward");
          $('#T'+DCC_ID+" .dir_left").addClass('selected');
          $('#T'+DCC_ID+" .dir_right").removeClass('selected');
        }

      }
      else{
        //console.log(data[6]+'<<+'+data[7]+'=='+DCC_ID);
        console.log("New train");
        console.log(train_list[train_ID]);
        var train_info = train_list[train_ID];
        var text = "";
        try{
          text += '<div id="T'+DCC_ID+'" class="trainbox">';
          text += '<div style="width:calc(100% - 60px);height:250px;float:left;">';
          text += '<div class="image_box">';
          text += '<img src="./../trains/'+train_info[0]+'.jpg">';
          text += '</div>';
          text += '<div class="name_tag">';
          text += '<span class="title">'+train_info[1]+'</span>';
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
          train_list[train_ID].data.direction = (data[5] & 0x80);
          text += '<svg viewBox="0 0 830.48 233.25" class="dir_arrow">';
          text +=   '<path onClick="ev_Train_direction('+train_ID+',1)" class="dir_left';
          if((data[5] & 0x80) == 0){text += ' selected';}
          text +=            '" d="M385.87,60.27h-238V5.07a2.14,2.14,0,0,0-3.44-1.62L3.79,115a2.07,2.07,0,0,0,0,3.24L144.44,229.8a2.09,2.09,0,0,0,1.29.45,2.33,2.33,0,0,0,1-.2,2.13,2.13,0,0,0,1.23-1.87V173H385.8a2.05,2.05,0,0,0,2.14-2V62.52a2.35,2.35,0,0,0-2.2-2.26"/>';
          text +=   '<path onClick="ev_Train_direction('+train_ID+',0)" class="dir_right';
          if(data[5] & 0x80){text += ' selected';}
          text +=            '" d="M444.62,173h238v55.2A2.14,2.14,0,0,0,686,229.8L826.7,118.25a2.07,2.07,0,0,0,0-3.24L686,3.45A2.09,2.09,0,0,0,684.75,3a2.33,2.33,0,0,0-1,.2,2.13,2.13,0,0,0-1.23,1.87v55.2H444.69a2.05,2.05,0,0,0-2.14,2V170.73a2.35,2.35,0,0,0,2.2,2.26"/>'
          text += '</svg>';
          text += '</div>';

          if((data[4] & 0b1000) != 0){
            text += 'Andere X-BUS Handregler<br/>';
          }
          if((data[4] & 0b111) == 0){
            text += '14 steps<br/>';
          }
          else if((data[4] & 0b111) == 2){
            text += '28 steps<br/>';
          }
          else if((data[4] & 0b111) == 4){
            text += '128 steps<br/>';
          }

          text += 'Fahrstufen KKK: ' + (data[6] & 0x7F) + '<br/>'

          text += 'Options active:<br/>';

          if((data[6] & 0x40) == 1){
            text += 'Doppeltraktion<br/>';
          }

          if((data[6] & 0x20) == 1){
            text += 'Smartsearch<br/>';
          }

          if((data[6] & 0x10) == 1){
            text += 'F0<br/>';
          }

          if((data[6] & 0x8) == 1){
            text += 'F4<br/>';
          }

          if((data[6] & 0x4) == 1){
            text += 'F3<br/>';
          }

          if((data[6] & 0x2) == 1){
            text += 'F2<br/>';
          }

          if((data[6] & 0x1) == 1){
            text += 'F1<br/>';
          }

          text += '<br/></div>';

          text += '<div style="width:50px;margin-left:10px;height:250px;float:left;">';
          text += '<div class="slider" tid="'+train_ID+'" style="height:210px;margin:0px auto;margin-top:20px;">';
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
          train_list[train_ID].data.speed = speed_step;
          var max_speed = parseInt(train_info[4]);

        }catch(e){
          
        }
        create_train_slider(DCC_ID,max_speed,speed_step);

      }
    }
  }

  function ws_Set_Broadcast_Flags(data){
    console.warn("TODO: reimplement");return;
    $('#status .broad').each(function(i,v){
      v.style['background'] = 'red';
    })
    if(data[1] & 0x80){
      $('#status .broad')[0].style['background'] = 'green';
    }
    if(data[1] & 0x40){
      $('#status .broad')[1].style['background'] = 'green';
    }
    if(data[1] & 0x20){
      $('#status .broad')[2].style['background'] = 'green';
    }
    if(data[1] & 0x10){
      $('#status .broad')[3].style['background'] = 'green';
      admin = 1;
      change_admin();
    }else{
      //Admin unset: logout
      $.post('./login.php', {'pass':''});
      admin = 0;
      change_admin();
    }
    if(data[1] & 0x8){
      $('#status .broad')[4].style['background'] = 'green';
    }
    if(data[1] & 0x4){
      $('#status .broad')[5].style['background'] = 'green';
    }
    if(data[1] & 0x2){
      $('#status .broad')[6].style['background'] = 'green';
    }
    if(data[1] & 0x1){
      $('#status .broad')[7].style['background'] = 'green';
    }
    broadcastFlags = data[1];
  }

  function ws_scan_progress(data){
    console.warn("TODO: reimplement");return;
    $('#Modules_scan .progress').html(data[1]+'/'+data[2]+' connections');
    $('#Modules_scan .found_modules').html("");
    var i = 3;
    for(i;i<data.length;i++){
      text = data[i] + "<br/>"

      $('#Modules_scan .found_modules').append(text);
    }
  }

  function ws_service_state(data){
    console.warn("TODO: reimplement");return;
    state = (data[1] << 8) + data[2];
    if(state & STATE_Modules_Coupled){
      $('#Modules_scan').css('display','none');
    }else{
      $('#Modules_scan').css('display','block');
    }

    if(state & STATE_Modules_Loaded && state & STATE_Modules_Coupled){
      $('#Modules_wrapper').css('display','block');
    }else{
      $('#Modules_wrapper').css('display','none');
    }
    console.log("service State: "+state);
  }
/**/

function track_layout(module,prev){
  console.log("At "+module+" from "+prev);

  //Check if the recursive function hasn't allready passed this node.
  if(Track_Layout_data[module].done){
    console.log("Allready done");
    return;
  }else{
    Track_Layout_data[module].done = true;
  }




  //Add Module to the HTML page
  $('#Modules').append("<div class=\"M"+module+" Module M"+module+"b\"></div>");
  blocks_load++;
  console.log("Load: "+'./../modules/'+module+'/layout.svg');
  $('.M'+module+'b').load('./../modules/'+module+'/layout.svg',function (evt){
    blocks_load--;
    if(blocks_load == 0){ //Done loading all modules?
      $('#Modules').css('display','block');
      ws.send("Ready");
      $('g.SwGroup','.Module').on("click",ev_throw_switch);  //Attach click event to all switch and mswitches
      $('.L:not(.LSwO)','.Module').attr("stroke","#555555");
      $('.L.LSwO','.Module').attr("stroke","lightgrey");
      $('.L.LSwS1','.Module').css("display","none");
    }
  });

  var anchor_id;
  $.each(Track_Layout_data[prev].anchors,function(i,v){
    if(v == module){
      anchor_id = i;
      return false;
    }
  });

  var id;
  $.each(Track_Layout_data[module].anchors,function(i,v){
    if(v == prev){
      id = i;
      return false;
    }
  });

  var x = Track_Layout_data[prev].x;
  var y = Track_Layout_data[prev].y;
  var r = Track_Layout_data[prev].r;

  //                                                                               dX                                                                             dY
  x = x + Math.cos(Math.PI * r/180) * Track_Layout_data[prev].anchor_pos[anchor_id][0] + Math.sin(Math.PI * r/180) * Track_Layout_data[prev].anchor_pos[anchor_id][1];
  y = y + Math.sin(Math.PI * r/180) * Track_Layout_data[prev].anchor_pos[anchor_id][0] + Math.cos(Math.PI * r/180) * Track_Layout_data[prev].anchor_pos[anchor_id][1];
  
  r = r + Track_Layout_data[prev].anchor_pos[anchor_id][2] + Track_Layout_data[module].anchor_pos[id][2] + 180; //dR
  r = r % 360;

  //                                                                          dX                                                                        dY
  x = x - Math.cos(Math.PI * r/180) * Track_Layout_data[module].anchor_pos[id][0] - Math.sin(Math.PI * r/180) * Track_Layout_data[module].anchor_pos[id][1];
  y = y - Math.sin(Math.PI * r/180) * Track_Layout_data[module].anchor_pos[id][0] - Math.cos(Math.PI * r/180) * Track_Layout_data[module].anchor_pos[id][1];

  var ma_x = Math.max(parseInt($("#Modules").css("width").slice(0,-2)),  x + Math.cos(Math.PI * r/180) * Track_Layout_data[module].width + Math.sin(Math.PI * r/180) * Track_Layout_data[module].height);
  var ma_y = Math.max(parseInt($("#Modules").css("height").slice(0,-2)), y + Math.sin(Math.PI * r/180) * Track_Layout_data[module].width + Math.cos(Math.PI * r/180) * Track_Layout_data[module].height);

  var mi_x = Math.min(-parseInt($("#Modules").css("margin-left").slice(0,-2)), x + Math.cos(Math.PI * r/180) * Track_Layout_data[module].width + Math.sin(Math.PI * r/180) * Track_Layout_data[module].height);
  var mi_y = Math.min(-parseInt($("#Modules").css("margin-top").slice(0,-2)),  y + Math.sin(Math.PI * r/180) * Track_Layout_data[module].width + Math.cos(Math.PI * r/180) * Track_Layout_data[module].height);
  mi_x = Math.min(mi_x,x);
  mi_y = Math.min(mi_y,y);

  $("#Modules").css("height",(ma_y)+"px");
  $("#Modules").css("width",ma_x+"px");
  $("#Modules").css("margin-top",(-mi_y)+"px");
  $("#Modules").css("margin-left",(-mi_x)+"px");

  /*if(rotate == 0){
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
  }*/
  //console.log("M"+setup[i]+" x:"+x+", y:"+y+", R:"+r);
  //console.log("MAX_X: "+max_x+"MAX_Y: "+max_y+":MIN_X "+min_x+"MIN_Y "+min_y);
  $(".M"+module+"b","#Modules").css("left",x+"px");
  $(".M"+module+"b","#Modules").css("top",y+"px");
  $(".M"+module+"b","#Modules").css("-ms-transform","rotate("+r+"deg)");
  $(".M"+module+"b","#Modules").css("-webkit-transform","rotate("+r+"deg)");
  $(".M"+module+"b","#Modules").css("transform","rotate("+r+"deg)");
  $(".M"+module+"b","#Modules").css("transform-origin","0px 0px");


  Track_Layout_data[module].x = x;
  Track_Layout_data[module].y = y;
  Track_Layout_data[module].r = r;

  //Go recursivly to the next
  $.each(Track_Layout_data[module].anchors, function(i,v){
    if(v != prev && v != 0){
      track_layout(v,module);
    }
  });
}

function change_admin(){
  if(admin){
    $('#Modules_scan .admin').css('display','block');
  }else{
    $('#Modules_scan .admin').css('display','none');
  }
}

var ws;
var socket_tries = 0;

function WebSocket_handler(adress){
    // Let us open a web socket
    ws = new WebSocket("ws://"+adress+"/",broadcastFlags);
    ws.binaryType = 'arraybuffer';

    ws.onopen = function(){
      socket_tries = 0;
    };

    ws.onmessage = function (evt){
      var received_msg = evt.data;
      var data = new Uint8Array(received_msg);
      console.log("Package length: "+data.length);
      console.log(data);




      //New Opcodes
      if(data[0] == WSopc_Track_Scan){
        console.log("Scan Progress");
        ws_scan_progress(data);
      }
      else if(data[0] == WSopc_Track_PUp_Layout){
        console.log("Partial Track Layout");
        ws_partial_layout(data);
      }


      else if(data[0] == WSopc_Z21TrainData){
        console.log("Z21 Train data");
        ws_full_train_data(data);
      }
      else if(data[0] == WSopc_LinkTrain){
        console.log("Link train");
        train_follow[data[1]] = data[2]; // Link the follow ID to the train ID
      }

      else if(data[0] == WSopc_BroadTrack){
        console.log("Update for the track");
        ws_track_update(data);
      }else if(data[0] == WSopc_BroadSwitch){
        console.log("Update for the Switches");
        ws_switch_update(data);
      }
      else if(data[0] == WSopc_Track_Layout){
        console.log("Track Layout");
        ws_layout(data);
      }

      else if(data[0] == WSopc_EmergencyStop){
        console.warn("Emergency Stop enabled");
        ws_emergency(0);
      }
      else if(data[0] == WSopc_ShortCircuitStop){
        console.warn("Short Circuit enabled");
        ws_emergency(1);
      }
      else if(data[0] == WSopc_ClearEmergency){
        console.warn("Emergency released");
        ws_emergency(2);
      }
      else if(data[0] == WSopc_NewMessage){
        console.log("New Message");
        ws_message(data);
      }
      else if(data[0] == WSopc_ClearMessage){
        console.log("Clear Message");
        ws_clearmessage(data);
      }

      else if(data[0] == WSopc_ChangeBroadcast){
        console.log("New Broadcast Flags");
        ws_Set_Broadcast_Flags(data);
      }
      else if(data[0] == WSopc_Service_State){
        console.log("Service State");
        ws_service_state(data);
      }

      /* Old Opcodes */
      /* Deprecated  */
      if(data[0] == 0){
        //Track style
        console.error("OLD OPCODE: Track style");
        // console.log(data);
        // if(data[1] == 1){
        //   console.log("It is a digital track");
        //   $("#digital").attr("onClick","ws.send(\"tA\");");
        //   $("#digital").attr("src","./img/Digital_y.svg");
        //   $("#digital").attr("title","Switch to DC");
        // }else if(data[1] == 0){
        //   console.log("It is a analog track");
        //   $("#digital").attr("onClick","ws.send(\"tD\");");
        //   $("#digital").attr("src","./img/Digital_n.svg");
        //   $("#digital").attr("title","Switch to DCC");
        // }
      }
      else if(data[0] == 2){
        //Track setup
        console.error("OLD OPCODE: Setup update");
        // console.warn("create_track(data) is disable. It is going to be depricated")
        //create_track(data);
      }
      else if(data[0] == 6){
        //Station list
        console.error("OLD OPCODE: Station List");
        // console.log(data);
        // station_list_update(data);
      }
      else if(data[0] == 8){
        //Request New Train
        console.error("OLD OPCODE: Request for new train");
        // if(data[1] == 2){
        //   //Start upload file and reload list
        //   if(data.length == 3){
        //     succ_add_train(data[2]);
        //     train_option += "<option value=\""+data[2]+"\">#"+$('#train_dcc').val();+"&nbsp;&nbsp;&nbsp;"+$('#train_name').val();+"</option>";
        //   }else{
        //     succ_add_train(data[2],data[3]);
        //     train_option += "<option value=\""+(data[2]+(data[3] << 8))+"\">#"+$('#train_dcc').val();+"&nbsp;&nbsp;&nbsp;"+$('#train_name').val();+"</option>";
        //   }
        // }
        // else if(data[1] == 1){
        //   alert("DCC address allready in use");
        //   fail_add_train();
        // }
        // else{
        //   alert("Error code:"+data[1]+"\n");
        //   fail_add_train();
        // }
      }
    };

    ws.onclose = function(event){
      // websocket is closed.
      console.log("Connection closed" + event.code);
      if(socket_tries == 0){
        console.log("Connection Closed\nRetrying....");
      }
      if(socket_tries != 5){
        setTimeout(function(){
          console.log("Reconnecting...");
          WebSocket_handler(adress);
          socket_tries++;
        }, 5000);
      }else{
        console.log("No Connection posseble\nIs the server on?");
      }
    };

}

$(document).ready(function(){
  //Open websocket or fail on unsupported browsers
  if ("WebSocket" in window){
  try{
      setTimeout(WebSocket_handler(window.location.host+':9000'),5000);
  }
  catch(e){
    console.error(e);
  }
  }else{
    // The browser doesn't support WebSocket
    alert("WebSocket NOT supported by your Browser!\nBIG PROBLEM!!\n\nPlease use the latest version of Chrome/FireFox/Safari, IE/Edge not supported");
    $('body').html("<h2>No Websocket support</h2><i>Use latest version of Chrome, FireFox or Safari</i>");
  }
});

