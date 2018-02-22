


//Opcodes
var WSopc_AddNewTrain        = 0x40;
var WSopc_LinkTrain          = 0x41;
var WSopc_TrainSpeed         = 0x42;
var WSopc_TrainFunction      = 0x43;
var WSopc_TrainOperation     = 0x44;
var WSopc_Z21TrainData       = 0x45;
var WSopc_TrainAddRoute      = 0x46;
var WSopc_TrainClearRoute    = 0x47;

var WSopc_ToggleSwitch       = 0x20;
var WSopc_ToggleMSSwitchUp   = 0x21;
var WSopc_ToggleMSSwitchDown = 0x22;
var WSopc_SetSwitch          = 0x23;
var WSopc_SetSwitchReserved  = 0x24;
var WSopc_BroadTrack         = 0x26;
var WSopc_BroadSwitch        = 0x27;

var WSopc_EmergencyStop      = 0x10;
var WSopc_ShortCircuitStop   = 0x11;
var WSopc_ClearEmergency     = 0x12;
var WSopc_NewMessage         = 0x13;
var WSopc_ClearMessage       = 0x14;
var WSopc_ChangeBroadcast    = 0x15;




var active_trains = []; //Trains that were active on the layout.
var broadcastFlags = 0xFF;

/*Client to Server*/
  function ev_throw_switch(evt){ //Click event throw switch
    console.log("throw switch");
    console.log(evt);
    var target = $(evt.currentTarget);

    if(target.children().length == 2){ //Normale Switch
      SwNr = parseInt(target.attr("class").split(' ')[0].slice(2));
      Module = parseInt(target.parent().parent().parent().attr("class").split(' ')[0].slice(1))
      console.log(Module+":"+SwNr);

      ws.send(String.fromCharCode(WSopc_ToggleSwitch)+String.fromCharCode(Module)+String.fromCharCode(SwNr));
    }else{ //MSwitch
      SwNr = parseInt(target.attr("class").split(' ')[0].slice(2));
      Module = parseInt(target.parent().parent().parent().attr("class").split(' ')[0].slice(1))
      console.log(Module+":"+SwNr);

      if(evt.shiftKey){ // -
        ws.send(String.fromCharCode(WSopc_ToggleMSSwitchDown)+String.fromCharCode(Module)+String.fromCharCode(SwNr));
      }else{
        ws.send(String.fromCharCode(WSopc_ToggleMSSwitchUp)+String.fromCharCode(Module)+String.fromCharCode(SwNr));
      }
    }
  }

  function ev_release_Emergency(evt){
    ws.send(String.fromCharCode(WSopc_ClearEmergency));
  }

  function ev_Emergency(evt){
    ws.send(String.fromCharCode(WSopc_EmergencyStop));
  }

  function ev_LinkTrain(evt){
    //ws.send(String.fromCharCode(WSopc_LinkTrain));
    var mID = parseInt($(evt.currentTarget).attr('Mid'));
    var tID = undefined;

    var val;
    if(tablet == 0){
      tID = parseInt($('#W'+mID+' .cs-selected').attr('data-value'));
    }else{
      console.log("tablet == 1");
      console.log(mID);
      tID = $('#W'+mID+' .cs-select').val();
    }
    if(typeof tID != 'undefined'){
      console.log(tID);

      var fID = $('#W'+mID+' but').attr("fID");
      
      ws.send(String.fromCharCode(WSopc_LinkTrain)+String.fromCharCode(fID)+String.fromCharCode(tID)+String.fromCharCode(mID >> 8)+String.fromCharCode(mID & 0xFF));
      
      setTimeout(function () {
        if($('#W'+mID).length != 0){
          alert("Train is already in use!!\nSelect another train");
        }
      }, 1000);

    }else{
      alert('Please select a train');
    }
  }

  function ev_Train_speed(tID,speed){
    var data = [];

    console.log("change speed, dir: "+ train_list[tID].data.direction);

    data[0] = WSopc_TrainSpeed;
    data[1] = tID;
    data[2] = (speed & 0x7F) + train_list[tID].data.direction;

    train_list[tID].data.speed = speed;

    var data2 = new Int8Array(data);

    ws.send(data2);
  }

  function ev_Train_direction(tID,direction){
    var data = [];

    direction = direction << 7;

    if(direction != train_list[tID].data.direction){
      data[0] = WSopc_TrainSpeed;
      data[1] = tID;
      data[2] = direction + train_list[tID].data.speed;

      train_list[tID].data.direction = direction;

      var data2 = new Int8Array(data);

      ws.send(data2);
    }
  }

  function ev_Toggle_Broadcast_Flag(evt){
    console.log(evt);
    if($(evt.target).attr("nr") != undefined){
      console.log(parseInt($(evt.target).attr("nr")));
      broadcastFlags ^= 1 << parseInt($(evt.target).attr("nr"));
      console.log("New broadcast flag: "+broadcastFlags);
      ws.send(new Uint8Array([WSopc_ChangeBroadcast,broadcastFlags]));
    }
  }
/**/
/*Server to Client*/
  function ws_switch_update(data){
    for(var i = 1;i<data.length;){
      var M = data[i];

      if(SwNr & 0x80){ //MSSwitch
        var MSSwNr= data[i+1] & 0x7F;
        var state = data[i+2];
        var len   = data[i+3];

        for(var i = 1;i<=len;i++){
          $(".M"+M+" #MSw"+MSSwNr+"S"+i).css("opacity",0); //Straight
        }
        //console.log(M+":"+B+":"+S+" State "+state+":"+".M"+M+" #B"+B+" .M"+S+"S"+state);
        $(".M"+M+" #MSw"+MSSwNr+"S"+(state+1)).css("opacity",1);

        i += 4;
      }else{ //Switch
        var SwNr = data[i+1];
        var state = data[i+2];

        var SwitchGroup  = $('.M'+M+' g.Sw'+SwNr);
        var SwitchGroupB = $('.M'+M+' g.BSw'+SwNr); //Background

        $('.LSw,.LSwO',SwitchGroup).css("opacity",0);
        $('.LSw,.LSwO',SwitchGroupB).css("opacity",0);

        $('.SwS'+state    ,SwitchGroup).css("opacity",1);
        $('.SwS'+state+'o',SwitchGroupB).css("opacity",1);

        i += 3;
      }
    };
  }

  function ws_track_update(data){
    for(var i = 1;i<data.length;i+=4){
      var M = data[i];
      var B = data[i+1];

      var block_element = $(".M"+M+" .B"+B);

      var D = data[i+2] >> 7;
      var blocked = (data[i+2] & 0b00010000) >> 4;
      var state = (data[i+2] & 0xF);
      var tID = data[i+3];

      console.log("Block "+M+":"+B+"\tdir: "+D+"\tBlocked: "+blocked+"\tstate: "+state+"\tTrain: "+tID);

      var color;
      var color2;

      if(blocked == 1){
        color = "#900";
        color2= "#b66";
      }else{
        if(state == 0){ //No train / Grey
          color = "#555";
          color2 = "#bbb";
        }else if(state == 1){ //SLOW
          color = "#f70";
          color2 = "#fd6";
        }else if(state == 2){ //Red
          color = "#f00";
          color2 = "#f66";
        }else if(state == 3){ //UNKOWN
          color = "#ff0";
          color2 = "#ff6";
        }else if(state == 4){ //GHOST
          color = "#f5f";
          color2 = "#fbf";
        }else if(state == 5){ //RESERVED
          color = "#00f";
          color2 = "#66f";
        }
      }

      //Paint A Lines in the color
      $(".L:not(.LSw,.LSwO)",block_element).css('stroke',color);
      $(".L.LSw",block_element).css('stroke',color);
      $(".L.LSwO",block_element).css('stroke',color2);


      if(state == 5 || blocked){
        $("g[class^=Sw]",".M"+M+" .B"+B).css('cursor','not-allowed');
      }else{
        $("g[class^=Sw]",".M"+M+" .B"+B).css('cursor','pointer');
      }

      //console.log(".L",".M"+M+" #B"+B+".S"+S);
      //Write Train id to block
      $(block_element).attr("train",tID);

      //Display train ID in textbox (Not existing yet/anymore)
      var text = "";
      if(D == 0){
        text = "&lt;"+tID;
      }else if(D == 1){
        text = tID+">";
      }
      $(".T",".M"+M+"B"+B).html(text);
      if(tablet){
        $(".M"+M+"B"+B).attr('style','display:block');
      }
    }
  }

  function ws_emergency(type){
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
/**/

var ws;

function WebSocket_handler(adress){
    $('#status .img').attr('onClick','');
    $('#status .img').css('cursor','default');
    // Let us open a web socket
    ws = new WebSocket("ws://"+adress+"/",broadcastFlags);
    ws.binaryType = 'arraybuffer';

    ws.onopen = function(){
      //alert("Connected");
      $('.notConnected').toggleClass('Connected');
      $('.notConnected').toggleClass('notConnected');
      $('#warning_list').empty();
      socket_tries = 0;
      $('#status .img').attr('data','./img/connected.svg');
      $('#status .header').html('Connected');

      //Create train accept is now possible
      $('#Train_Add #upload').attr('src','./img/checked.png');
      $('#Train_Add #upload').css('cursor','pointer');
      $('#Train_Add #upload').attr('onClick','add_train()');
    };

    ws.onmessage = function (evt){
      var received_msg = evt.data;
      var data = new Uint8Array(received_msg);
      console.log("Package length: "+data.length);
      console.log(data);

      //New Opcodes
      if(data[0] == WSopc_Z21TrainData){
        console.log("Z21 Train data");
        ws_full_train_data(data);
      }
      else if(data[0] == WSopc_LinkTrain){
        console.log("Link train");
        train_follow[data[1]] = data[2]; // Link the follow ID to the train ID
      }

      else if(data[0] == WSopc_BroadTrack){
        ws_track_update(data);
        console.log("Update for the track");
      }else if(data[0] == WSopc_BroadSwitch){
        ws_switch_update(data);
        console.log("Update for the Switches");
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

      /* Old Opcodes *//*
       0 - Track style (Analog/Digital)
       1 - Message / Notification
       2 - Track-setup
       3 - Track occupancy
       4 - Switches states
       5 - MS Switches states
       6 - Stations list
       7 - Train data
       8 - Request New Train
       8 -
       9 -
      10 -
      */
      if(data[0] == 0){
        //Track style
        console.log("Track style");
        console.log(data);
        if(data[1] == 1){
          console.log("It is a digital track");
          $("#digital").attr("onClick","ws.send(\"tA\");");
          $("#digital").attr("src","./img/Digital_y.svg");
          $("#digital").attr("title","Switch to DC");
        }else if(data[1] == 0){
          console.log("It is a analog track");
          $("#digital").attr("onClick","ws.send(\"tD\");");
          $("#digital").attr("src","./img/Digital_n.svg");
          $("#digital").attr("title","Switch to DCC");
        }
      }
      else if(data[0] == 1){
        //Message
        console.log("Message update");
        console.log(data);
        message_update(data);
      }
      else if(data[0] == 2){
        //Track setup
        console.log("Setup update");
        create_track(data);
      }
      /*else if(data[0] == 3){
        //Track
        console.log("Track update");
        console.log(data);
        track_update(data);
      }
      else if(data[0] == 4){
        //Switches
        console.log("Switch update");
        console.log(data);
        switch_update(data);
      }
      else if(data[0] == 5){
        //Multi State Switches
        console.log("MS Switch update");
        console.log(data);
        ms_switch_update(data);
      }*/
      else if(data[0] == 6){
        //Station list
        console.log("Station List");
        console.log(data);
        station_list_update(data);
      }
      else if(data[0] == 7){
        //Train data
        console.log('Train data update');
        console.log(data);
        train_data_update(data);
      }
      else if(data[0] == 8){
        //Request New Train
        console.log("Request for new train");
        if(data[1] == 2){
          //Start upload file and reload list
          if(data.length == 3){
            succ_add_train(data[2]);
            train_option += "<option value=\""+data[2]+"\">#"+$('#train_dcc').val();+"&nbsp;&nbsp;&nbsp;"+$('#train_name').val();+"</option>";
          }else{
            succ_add_train(data[2],data[3]);
            train_option += "<option value=\""+(data[2]+(data[3] << 8))+"\">#"+$('#train_dcc').val();+"&nbsp;&nbsp;&nbsp;"+$('#train_name').val();+"</option>";
          }
        }
        else if(data[1] == 1){
          alert("DCC address allready in use");
          fail_add_train();
        }
        else{
          alert("Error code:"+data[1]+"\n");
          fail_add_train();
        }
      }
    };

    ws.onclose = function(event){
      // websocket is closed.
      console.log("Connection closed" + event.code);
      if(socket_tries == 0){
        $('.Connected').toggleClass('notConnected');
        $('.Connected').toggleClass('Connected');
        console.log("Connection Closed\nRetrying....");
        $('#status .img').attr('data','./img/reconnecting.svg');
        $('#status .header').html('Reconnecting...');
        $('#warning_list').css('display','none');
        $("#CTrain").empty();
        $('#warning_list').empty();
        //Resetting values
        train_follow = [];
      }
      if(socket_tries != 5){
        setTimeout(function(){
          console.log("Reconnecting...");
          WebSocket_handler(adress);
          socket_tries++;
        }, 5000);
      }else{
        console.log("No Connection posseble\nIs the server on?");
        $('#status .img').attr('data','./img/disconnected.svg');
        $('#status .img').contents().find('svg').css("cursor","pointer");
        $('#status .img').contents().find('svg').attr('onClick','socket_tries = 0;WebSocket_handler(window.location.host+":9000")');
        $('#status .header').html('Disconnected');
      }
    };

}

$(document).ready(function(){
  if ("WebSocket" in window){
  try{
      setTimeout(WebSocket_handler(window.location.host+':9000'),5000);
  }
  catch(e){
    console.error(e);
  }
  }else{
    // The browser doesn't support WebSocket
    alert("WebSocket NOT supported by your Browser!\nBIG PROBLEM!!\n\nPlease use Chrome/FireFox/Safari, or update your browser");
    $('#status').attr('src','./img/status_w.png');
  }
});
