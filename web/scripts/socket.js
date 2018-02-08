


//Opcodes
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

/*Client to Server*/
  function ev_throw_switch(evt){ //Click event throw switch
    console.log("remote_Switch2");
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
/**/

var ws;

function WebSocket_handler(adress){
    $('#status').attr('onClick','');
    $('#status').css('cursor','default');
    // Let us open a web socket
    ws = new WebSocket("ws://"+adress+"/",0xFF);
    ws.binaryType = 'arraybuffer';

    ws.onopen = function(){
      //alert("Connected");
      $('.notConnected').toggleClass('Connected');
      $('.notConnected').toggleClass('notConnected');
      $('#warning_list').empty();
      socket_tries = 0;
      $('#status').attr('src','./img/status_g.png');

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
      if(data[0] == WSopc_BroadTrack){
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
        console.warn("Discard Message!!!");
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
      else if(data[0] == 3){
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
      }
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
            train_list_t += "<option value=\""+data[2]+"\">#"+$('#train_dcc').val();+"&nbsp;&nbsp;&nbsp;"+$('#train_name').val();+"</option>";
          }else{
            succ_add_train(data[2],data[3]);
            train_list_t += "<option value=\""+(data[2]+(data[3] << 8))+"\">#"+$('#train_dcc').val();+"&nbsp;&nbsp;&nbsp;"+$('#train_name').val();+"</option>";
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
        $('#status').attr('src','./img/status_ow.gif');
        $('#warning_list').css('display','none');
        //$("#CTrain").empty();
        $('#warning_list').empty();
        //Resetting values
        //active_trains = [];
      }
      if(socket_tries != 5){
        setTimeout(function(){
          console.log("Reconnecting...");
          WebSocket_handler(adress);
          socket_tries++;
        }, 5000);
      }else{
        console.log("No Connection posseble\nIs the server on?");
        $('#status').attr('src','./img/status_r.png');
        $('#status').css('cursor','pointer');
        $('#status').attr('onClick','socket_tries = 0;WebSocket_handler(window.location.host+":9000")');
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
