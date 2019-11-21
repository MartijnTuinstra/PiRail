

function change_admin(){
  if(admin){
    $('#Modules_scan .admin').css('display','block');
  }else{
    $('#Modules_scan .admin').css('display','none');
  }
}
function IntArrayToString(data){
  var i, str = '';

  for (i = 0; i < data.length; i++) {
      str += String.fromCharCode(data[i]);
  }

  return str;
}

function ToInt16(value){
  // If a positive value, return it
  if ((value & 0x8000) == 0)
  {
    return value;
  }
  else{
    return value | 0xffff0000;
  }
}

var websocket = {
  connected: false,
  ws: undefined,
  disconnect_counter: 1,
  recv_cb: {},
  connect: function(url, protocol=undefined){
    this.url = url;
    this.protocol = protocol;

    this.__connect__();
  },
  __connect__: function(){
    console.log("Connect ....");
    this.ws = new WebSocket(this.url, this.protocol);
    this.ws.binaryType = 'arraybuffer';

    this.ws.onopen = this.on_connect.bind(this);
    this.ws.onclose = this.on_disconnect.bind(this);
    this.ws.onerror = this.on_error.bind(this);
    this.ws.onmessage = this.__recv__.bind(this);
  },

  on_connect: function(evt){
    this.connected = true;

    this.ws_call_cb("ws_open_cb");

    console.log("Connected");
  },
  on_disconnect: function(evt){
    if(this.connected == true){
      this.disconnect_counter = 1;
      this.ws_call_cb("ws_close_cb");
    }

    this.ws.close();

    setTimeout(this.__connect__.bind(this), this.disconnect_counter * 1000);

    this.disconnect_counter += 1;
    if(this.disconnect_counter > 20){
      this.disconnect_counter = 20;
    }

    this.connected = false;
    console.log("Connection lost");
  },
  on_error: function(evt){
    console.warn("Websocket error", evt);
  },

  //Callbacks
  ws_close_cb: [],
  ws_open_cb: [],
  ws_add_cb: function(list, f){
    this[list].push(f);
  },
  ws_call_cb: function(cb){
    for(var i = 0; i < this[cb].length; i++){
      this[cb][i]();
    }
  },

  //Messages
  opc: {},
  add_opcodes: function(configuration){
    for(var i = 0; i < configuration.length; i++){
      this.add_opcode(configuration[i].opcode, configuration[i].name, configuration[i].send, configuration[i].recv);
    }
  },
  add_opcode: function(opcode, name, send, recv){
    this["cts_"+name] = function(data){
      var msg = [];
      msg[0] = opcode;
      var msg2 = send(data);
      if(msg2 == undefined || !(typeof msg2 === 'object' && msg2.constructor === Array)){
        console.log("opcode send function thrown error");
        return;
      }
      msg = msg.concat(msg2);

      console.log(msg);

      if(!this.__send__(msg)){
        console.log("Failed to send");
      }
    };
    this["stc_"+name] = recv;

    this.opc[opcode] = name;
    this.opc[name] = opcode;
  },

  __send__: function(data){
    if(this.connected){
      console.log(new Int8Array(data));
      websocket.ws.send(new Int8Array(data));
      return 1;
    }
    else{
      return 0;
    }
  },

  __recv__: function(evt){
    var rmsg = evt.data;
    var msg = new Uint8Array(rmsg);
    var opcode = msg[0];

    if(msg.length > 0){
      msg = msg.slice(1);
    }

    if(this.opc[opcode] == undefined){
      console.warn("Unknown opcode "+opcode);
    }

    console.log("0x"+opcode.toString(16)+" stc_"+this.opc[opcode]);

    this["stc_"+this.opc[opcode]](msg);

    if(this.recv_cb["stc_"+name] != undefined){
      this.recv_cb["stc_"+name](msg);
    }
  }
};

$(document).ready(function(){
  Messages.add({type:0xff,id:0});
});

websocket.ws_add_cb("ws_close_cb", function(){
  $('.info-box .connection_state .indicator').removeClass("bg-success");
  $('.info-box .connection_state .indicator').addClass("bg-danger");

  Messages.clear();
  Messages.add({type:0xff,id:0});
});
websocket.ws_add_cb("ws_open_cb", function(){
  $('.info-box .connection_state .indicator').removeClass("bg-danger");
  $('.info-box .connection_state .indicator').addClass("bg-success");

  Messages.clear();
});

websocket.add_opcodes([
  //Settings / Admin
    // {
    //   opcode: 0x80,
    //   name: "ClearTrack",
    //   send: function(data){ return [data.name, data.nr];},
    //   recv: function(data){ console.log("Recv Name");}
    // },
    // {
    //   opcode: 0x81,
    //   name: "ReloadTrack",
    //   send: function(data){ return [data.name, data.nr];},
    //   recv: function(data){ console.log("Recv Name");}
    // },
    {
      opcode: 0x82,
      name: "Track_Scan_Progress",
      // send: function(data){ return [data.name, data.nr];},
      recv: function(data){ console.warn("Track_Scan_Progress", data);}
    },
    {
      opcode: 0x83,
      name: "Track_Layout_Update", // Partial layout
      recv: function(data){
        for(var i = 0;i<data.length;i++){
          var module = modules[data[i]]
          for(var j = 1;j<= module.connections.length; j++){
            module.connection_link[j-1] = data[i+j];
          }
          console.log("Module "+data[i]+" connect ", module.connection_link, {x: module.OffsetX, y:module.OffsetY, r:module.r});
          i += module.connections.length;

          module.connect();
        }

        Canvas.fit();
        Canvas.fitOptimize();

        Canvas.rescale(1);
      }
    },
    {
      opcode: 0x84,
      name: "Track_Layout_Config", // Full layout
      recv: function(data){
        for(var i = 0;i<data.length;i++){
          var module = modules[data[i]]
          for(var j = 1;j<= module.connections.length; j++){
            module.connection_link[j-1] = data[i+j];
          }
          console.log("Module "+data[i]+" connect ", module.connection_link, {x: module.OffsetX, y:module.OffsetY, r:module.r});
          i += module.connections.length;

          module.connect();
        }

        Canvas.fit();
        Canvas.fitOptimize();

        Canvas.rescale(1);
      }
    },
    {
      opcode: 0x86,
      name: "Z21_Track_Info",
      send: function(data){ return []; },
      recv: function(data){
        var i=0;
        var mcur = ToInt16(data[i++] + (data[i++] << 8));
        var mfcur = ToInt16(data[i++] + (data[i++] << 8));
        var pcur = ToInt16(data[i++] + (data[i++] << 8));
        var vcc = data[i++] + (data[i++] << 8);
        var sup = data[i++] + (data[i++] << 8);
        var temp = ToInt16(data[i++] + (data[i++] << 8));
        var flags = data[i++] + (data[i++] << 8);
        Z21.update({"tcur": mcur, "tfcur": mfcur, "pcur": pcur, "svol": sup, "tvol": vcc, "temp": temp, "flags": flags});
      }
    },
    {
      opcode: 0x87,
      name: "Z21_Settings",
      send: function(ip){
        return [parseInt(ip.ip1),
                parseInt(ip.ip2),
                parseInt(ip.ip3),
                parseInt(ip.ip4)];
      },
      recv: function(data){
        var i = 0;
        var ip = data[i++] + "." + data[i++] + "." + data[i++] + "." + data[i++];
        var fw = data[i++] + "." + data[i++];
        Z21.update({"ip-addr": ip, "fw-version": fw});
      }
    },
    {
      opcode: 0x8C,
      name: "Reset_Switches",
      send: function(data){ return [];},
    },
    {
      opcode: 0x8F,
      name: "TrainsToDepot",
      send: function(data){ return [];},
      // recv: function(data){ console.log("Recv Name");}
    },

    {
      opcode: 0x90,
      name: "EnableSubModule",
      send: function(module){ return [1 << module] },
    },
    {
      opcode: 0x91,
      name: "DisableSubModule",
      send: function(module){ return [1 << module] },
    },
    {
      opcode: 0x92,
      name: "SubModuleState",
      recv: function(data){
        Submodules.update([data[0], data[1], data[2], data[3]])
      }
    },
    {
      opcode: 0x9F,
      name: "RestartApplication",
      send: function(data){ return []; },
    },

    {
      opcode: 0xC0,
      name: "AdminEmergency",
      send: function(data){ return [data[0], data[1]];},
      // recv: function(data){ console.log("Recv Name");}
    },
    {
      opcode: 0xC1,
      name: "AdminEmergencyRelease",
      send: function(data){ return [data[0], data[1]];},
      // recv: function(data){ console.log("Recv Name");}
    },
    {
      opcode: 0xCE,
      name: "AdminLogout",
      send: function(data){ return [];},
      // recv: function(data){ console.log("Recv Name");}
    },
    {
      opcode: 0xCF,
      name: "AdminLogin",
      send: function(passphrase){
        var data = [];
        for(var i = 0;i<passphrase.length;i++){
          data[i] = passphrase.charCodeAt(i);
        }
        return data;
      },
      recv: function(data){
        // var passphrase = $.md5(prompt("Please enter your password", "password"));
        var passphrase = $.md5("password");
        websocket.cts_AdminLogin(passphrase);
      }
    },

  //Trains
    {
      opcode: 0x41,
      name: "LinkTrain",
      send: function(data){
        return [data.fid, data.real_id, ((data.type == "E")?1:0) + ((data.msg_id >> 8) & 0x7F), data.msg_id & 0xFF];
      },
      recv: function(data){
        var follow_id = data[0];
        var train_id = data[1];
        var type = (data[2] & 0x1);

        var msg_id = ((data[2] & 0xFE) << 7) + data[3];

        Train_Control.link({fid: follow_id, tid: train_id, type:type, msg_id: msg_id});
      }
    },
    {
      opcode: 0x42,
      name: "TrainSpeed",
      send: function(data){
        return [data.train.railtrainid, (data.train.dir & 1) << 4 | (data.train.speed & 0xF00) >> 8,
                data.train.speed & 0xFF];
      },
    },
    {
      opcode: 0x43,
      name: "TrainFunction",
      send: function(data){ console.warn("TrainFunction", data); },
      recv: function(data){ console.warn("TrainFunction", data); }
    },
    {
      opcode: 0x44,
      name: "TrainControl",
      send: function(data){
        if(data.train.railtrainid != undefined && data.train.control != undefined){
          return [data.train.railtrainid, data.train.control];
        }
      },
      recv: function(data){ console.warn("TrainControl", data); }
    },
    {
      opcode: 0x45,
      name: "TrainUpdate",
      send: function(data){ console.warn("TrainUpdate", data); },
      recv: function(data){

        var id = data[0];

        var box = 0;

        if(Train_Control.train[1] != undefined && Train_Control.train[1].t.railtrainid == id){
          box = 1;
        }
        else if(Train_Control.train[2] != undefined && Train_Control.train[2].t.railtrainid == id){
          box = 2;
        }

        if(box == 0){
          console.warn("No box");
          return;
        }

        Train_Control.train[box].t.control = (data[1] & 0x70) >> 4;
        Train_Control.train[box].t.dir = (data[1] & 0x80) >> 7;
        Train_Control.train[box].t.speed = ((data[1] & 0x0F) << 8) + data[2];
        var ratio = Train_Control.train[box].t.speed / Train_Control.train[box].t.max_speed;

        var slider_box = $('.train-box.box'+box+' .train-speed-slider');
        var pageY = slider_box.offset().top + slider_box.height();
        var ylim = slider_box.height() - $('.slider-handle', slider_box).height(); 

        var pos = ylim * ratio;

        $('.train-box.box'+box+' .train-speed > span').text(Train_Control.train[box].t.speed);
        Train_Control.set_handle(box, pos);

        Train_Control.apply_dir(box);

        Train_Control.apply_control(box);
      }
    },
    {
      opcode: 0x46,
      name: "TrainAddRoute",
      send: function(data){ 
        var msg = [];

        msg[0] = data.train_id;
        msg[1] = data.station_id;
        msg[2] = data.station_module;

        return msg;
      },
      recv: function(data){ console.warn("TrainAddRoute", data); }
    },
    {
      opcode: 0x4F,
      name: "TrainSubscribe",
      send: function(data){ 
        var msg = [];
        if(data[1] == undefined){
          msg[0] = 0xFF;
        }
        else{
          msg[0] = data[1].t.railtrainid & 0xFF;
        }
        if(data[2] == undefined){
          msg[1] = 0xFF;
        }
        else{
          msg[1] = data[2].t.railtrainid & 0xFF;
        }

        return msg;
      }
    },

    {
      opcode: 0x50,
      name: "AddNewEnginetolib",
      send: function(data){
        var req_keys = ["icon_name", "image_name", "name", "dcc", "length", "type", "speedstep"];
        var missing_data = [];
        for(var i = 0; i < req_keys.length; i++){
          if(data[ req_keys[i] ] == undefined){
            missing_data.push( req_keys[i] );
          }
          else if(typeof data[ req_keys[i] ] == "number" && isNaN(data[ req_keys[i] ])){
            missing_data.push( req_keys[i] );
          }
          else if(typeof data[ req_keys[i] ] == "string" && data[ req_keys[i] ] == ""){
            missing_data.push( req_keys[i] );
          }
        }
        if(missing_data.length > 0){
          Modals.call_cb("create_cb", {return_code: -2, data: missing_data});
          return undefined;
        }

        msg = [];
        msg[0] = (data.dcc & 0x00ff);
        msg[1] = (data.dcc & 0xff00) >> 8;
        msg[2] = (data.length & 0x00ff);
        msg[3] = (data.length & 0xff00) >> 8;
        msg[4] = data.type;

        if(data.speedstep == 28){
          msg[5] |= 0b0100;
        }
        else if(data.speedstep == 128){
          msg[5] |= 0b1000;
        }

        if(data.image_name.endsWith('jpg')){
          msg[5] |= 0b10;
        }
        if(data.icon_name.endsWith('jpg')){
          msg[5] |= 0b01;
        }

        msg[6] = data.name.length;

        msg[7] = data.speedsteps.length;

        //Upload timestamps
        var time = data.image_name.split(".");
        time = time[time.length - 2].split("");

        var time_image = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

        var time = data.icon_name.split(".");
        time = time[time.length - 2].split("");

        var time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

        msg[8] = time_image & 0xFF;
        msg[9] = ((time_image >> 4) & 0xF0) | (time_icon & 0x0F);
        msg[10] = (time_icon >> 4);

        for (var i = 0; i < data.name.length; i++) {
          msg.push(data.name.charCodeAt(i));
        }

        for (var i = 0; i < data.speedsteps.length; i++){
          console.log(data.speedsteps[i]);
          msg.push(data.speedsteps[i].speed);
          msg.push(data.speedsteps[i].speed >> 8);
          msg.push(data.speedsteps[i].step);
        }

        return msg;
      },
      recv: function(data){
        if(data[0] == 1){
          Modals.hide();
        }
        else if(data[0] == 0){
          Modals.call_cb("create_cb", {return_code: 0});
        }
        else if(data[0] == 255){
          Modals.call_cb("create_cb", {return_code: -1});
        }
      }
    },
    {
      opcode: 0x51,
      name: "EditEnginelib",
      send: function(data){
        msg = [];
        msg[0] = ((!data.edit) << 7) | (data.id & 0x7f00) >> 8;
        msg[1] = data.id & 0x7fff;

        if(!data.edit){ //Remove
          return msg;
        }

        msg[2] = (data.dcc & 0x00ff);
        msg[3] = (data.dcc & 0xff00) >> 8;
        msg[4] = (data.length & 0x00ff);
        msg[5] = (data.length & 0xff00) >> 8;
        msg[6] = data.type;

        if(data.speedstep == 28){
          msg[7] |= 0b0100;
        }
        else if(data.speedstep == 128){
          msg[7] |= 0b1000;
        }
        if(data.image_name.endsWith('jpg')){
          msg[7] |= 0b10;
        }
        if(data.icon_name.endsWith('jpg')){
          msg[7] |= 0b1;
        }

        msg[8] = data.name.length;
        msg[9] = data.speedsteps.length;

        //Upload timestamps
        var time = data.image_name.split(".");
        var time_image = 0xFFF;
        var time_icon = 0xFFF;

        if(time.length == 3){
          time = time[time.length - 2].split("");

          time_image = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);
        }

        var time = data.icon_name.split(".");
        if(time.length == 3){
          time = time[time.length - 2].split("");

          time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);
        }

        msg[10] = time_image & 0xFF;
        msg[11] = ((time_image >> 4) & 0xF0) | (time_icon & 0x0F);
        msg[12] = (time_icon >> 4) & 0xFF;

        for (var i = 0; i < data.name.length; i++) {
          msg.push(data.name.charCodeAt(i));
        }

        for (var i = 0; i < data.speedsteps.length; i++){
          console.log(data.speedsteps[i]);
          msg.push(data.speedsteps[i].speed);
          msg.push(data.speedsteps[i].speed >> 8);
          msg.push(data.speedsteps[i].step);
        }

        return msg;
      }
    },
    {
      opcode: 0x52,
      name: "EnginesLibrary",
      recv: function(data){
        Train.engines = [];
        for(var i = 0;i<data.length;){
          var dcc_id = (data[i] + (data[i+1] << 8));
          i += 2;
          var max_spd = (data[i] + (data[i+1] << 8));
          i += 2;
          var length = (data[i] + (data[i+1] << 8));
          i += 2;
          type = data[i++];
          speedsteps = data[i++];
          steps_len = data[i++];

          var name = IntArrayToString(data.slice(i+3, i+3+data[i]));
          var text_length = data[i++];

          var img = IntArrayToString(data.slice(i+2+text_length, i+2+text_length+data[i]));
          text_length += data[i++];

          var icon = IntArrayToString(data.slice(i+1+text_length, i+1+text_length+data[i]));
          text_length += data[i];

          i += text_length + 1;

          steps = [];
          for(var j = 0; j < steps_len; j++){
            steps[j] = {}
            steps[j].speed = (data[i] + (data[i+1] << 8));
            i+=2;
            steps[j].step = data[i++];
          }

          if(speedsteps == 0){
            speedsteps = 14;
          }
          else if(speedsteps == 1){
            speedsteps = 28;
          }
          else{
            speedsteps = 128;
          }
          
          Train.engines.push({ontrack: 0, id: Train.engines.length, name: name, dcc: dcc_id, img: img, icon: icon, max_speed: max_spd, length: length, type: type, speedstep: speedsteps, steps: steps});
        }

        Train_Configurator.update();
      }
    },

    {
      opcode: 0x53,
      name: "AddNewCartolib",
      send: function(data){
        var msg = [];
        msg[0] = (data.nr & 0x00ff);
        msg[1] = (data.nr & 0xff00) >> 8;
        msg[2] = (data.max_speed & 0x00ff);
        msg[3] = (data.max_speed & 0xff00) >> 8;
        msg[4] = (data.length & 0x00ff);
        msg[5] = (data.length & 0xff00) >> 8;
        msg[6] = data.type;

        if(data.icon_name.endsWith('jpg')){
          msg[7] |= 0x1;
        }
        msg[8] = data.name.length;

        //Upload timestamps
        var time = data.icon_name.split(".");
        time = time[time.length - 2].split("");

        var time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

        msg[9] = time_icon & 0xFF;
        msg[10] = time_icon >> 8;

        for (var i = 0; i < data.name.length; i++) {
          msg.push(data.name.charCodeAt(i));
        }
        return msg;
      },
      recv: function(data){
        if(data[0] == 1){
          Modals.hide();
        }
        else if(data[0] == 0){
          Modals.call_cb("create_cb", {return_code: 0});
        }
        else if(data[0] == 255){
          Modals.call_cb("create_cb", {return_code: -1});
        }
      }
    },
    {
      opcode: 0x54,
      name: "EditCarlib",
      send: function(data){
        var msg = [];
        msg[0] = ((!data.edit) << 7) | (data.id & 0x7f00) >> 8;
        msg[1] = data.id & 0xff;

        if(!data.edit){ //Remove
          return msg;
        }

        msg[2] = (data.nr & 0x00ff);
        msg[3] = (data.nr & 0xff00) >> 8;
        msg[4] = (data.max_speed & 0x00ff);
        msg[5] = (data.max_speed & 0xff00) >> 8;
        msg[6] = (data.length & 0x00ff);
        msg[7] = (data.length & 0xff00) >> 8;
        msg[8] = data.type;

        if(data.icon_name.endsWith('jpg')){
          msg[9] |= 0x1;
        }
        msg[10] = data.name.length;

        //Upload timestamps
        var time = data.icon_name.split(".");
        var time_icon = 0xFFF;

        if(time.length == 3){
          time = time[time.length - 2].split("");

          time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);
        }

        msg[11] = time_icon & 0x0F;
        msg[12] = time_icon >> 4;

        for (var i = 0; i < data.name.length; i++) {
          msg.push(data.name.charCodeAt(i));
        }

        return msg;
      }
    },
    {
      opcode: 0x55,
      name: "CarsLibrary",
      recv: function(data){
        Train.cars = [];
        for(var i = 0;i<data.length;){
          var nr_id = (data[i] + (data[i+1] << 8));
          i += 2;
          var max_spd = (data[i] + (data[i+1] << 8));
          i += 2;
          var length = (data[i] + (data[i+1] << 8));
          i += 2;
          type = data[i++];

          var name = IntArrayToString(data.slice(i+2, i+2+data[i]));
          var text_length = data[i++];

          var icon = IntArrayToString(data.slice(i+1+text_length, i+1+text_length+data[i]));
          text_length += data[i++];

          Train.cars.push({name: name, nr: nr_id, icon: icon, max_speed: max_spd, length: length, type: type});
          i += text_length;
        }

        Train_Configurator.update();
      }
    },

    {
      opcode: 0x56,
      name: "AddNewTraintolib",
      send: function(data){
        msg = [];
        msg[0] = data.name.length;
        msg[1] = 0x80 | (data.list.length & 0x7f);
        msg[2] = data.type;

        for (var i = 0; i < data.name.length; i++) {
          msg.push(data.name.charCodeAt(i));
        }

        for (var i = 0; i < data.list.length; i++){
          msg.push(data.list[i].type);
          msg.push((data.list[i].id & 0x00ff));
          msg.push((data.list[i].id & 0xff00) >> 8);
        }

        return msg;
      },
      recv: function(data){
        console.warn("AddNewTraintolib", data);
      }
    },
    {
      opcode: 0x57,
      name: "EditTrainlib",
      send: function(data){
        msg = [];

        msg[0] = ((!data.edit) << 7) | (data.id & 0x7f00) >> 8;
        msg[1] = data.id & 0xff;

        if(!data.edit){ //Remove
          return msg;
        }

        msg[2] = data.name.length;
        msg[3] = data.list.length;
        msg[4] = data.type;

        for (var i = 0; i < data.name.length; i++) {
          msg.push(data.name.charCodeAt(i));
        }

        for (var i = 0; i < data.list.length; i++){
          msg.push(data.list[i].type);
          msg.push((data.list[i].id & 0x00ff));
          msg.push((data.list[i].id & 0xff00) >> 8);
        }

        return msg;
      }
    },
    {
      opcode: 0x58,
      name: "TrainsLibrary",
      recv: function(data){
        Train.trains = [];
        for(var i = 0;i<data.length;){
          var max_spd = (data[i] + (data[i+1] << 8));
          var length = (data[i+2] + (data[i+3] << 8));
          i += 4;
          var type = data[i] >> 1;

          var use = data[i++] & 0b1;

          var name = IntArrayToString(data.slice(i+2, i+2+data[i]));
          var data_len = data[i++];

          var links = data.slice(i+1+data_len, i+1+data_len+3*data[i]);
          data_len += data[i++]*3;

          var link_list = [];
          var dcc = [];
          
          for(var j = 0; j<links.length; j+=3){
            link_list.push([links[j], links[j+1]+ (links[j+2] << 8)]);

            if(links[j] == 0 && Train.engines[links[j+1]+ (links[j+2] << 8)] != undefined){
              dcc.push(Train.engines[links[j+1]+ (links[j+2] << 8)].dcc);
            }
          }

          Train.trains.push({ontrack: 0, id: Train.trains.length, name: name, dcc: dcc, max_speed: max_spd, length: length, type: type, link: link_list, use: use});
          i += data_len;
        }

        Train_Configurator.update();
      }
    },

    {
      opcode: 0x5A,
      name: "TrainCategories",
      recv: function(data){
        Train.cat = {};
        for(var i = 0;i<data.length;){
          var id = data[i++];

          var name = IntArrayToString(data.slice(i+1, i+1+data[i]));

          Train.cat[id] = name;
          i += data[i]+1;
        }
      }
    },

  // Track and switches
    {
      opcode: 0x20,
      name: "SetSwitch",
      send: function(data){
        return data;
      }
    },
    {
      opcode: 0x21,
      name: "SetMultiSwitch",
      send: function(data){
        var msg = [];
        var di = 1;

        msg[0] = data.length;
        for (var i = 0; i < data.length; i++) {
          msg[di++] = data[i][0]
          msg[di++] = data[i][1]
          msg[di++] = data[i][2]
        }

        return msg;
      }
    },
    {
      opcode: 0x22,
      name: "SetSwitchReserved",
      send: function(data){}
    },
    {
      opcode: 0x23,
      name: "ChangeReservedSwitch",
      send: function(data){}
    },
    {
      opcode: 0x25,
      name: "SetSwitchRoute",
      send: function(data){}
    },

    {
      opcode: 0x26,
      name: "BroadTrack",
      recv: function(data){
        for(var i = 0;i<data.length;i+=4){
          var m = data[i];
          var B = data[i+1];

          var D = data[i+2] >> 7;
          var state = (data[i+2] & 0x7F);
          var tID = data[i+3];

          modules[m].blocks[B] = state;
        }

        Canvas.update_frame();
      }
    },
    {
      opcode: 0x27,
      name: "BroadSwitch",
      recv: function(data){
        for(var i = 0;i<data.length;){
          var M = data[i];

          if(data[i+1] & 0x80){ //MSSwitch
            var len   = data[i+3];

            modules[M].msswitches[data[i+1]&0x7F] = data[i+2] & 0x7F;

            i += 4;
          }else{ //Switch
            try{
              modules[M].switches[data[i+1]] = data[i+2] & 0x7F;
            }
            catch(e){
              console.error(e);
            }

            i += 3;
          }
        }

        Canvas.update_frame();
      }
    },

    {
      opcode: 0x30,
      name: "TrackLayoutOnlyRawData",
      recv: function(data){
        console.log(data);
        function JsonParse(obj){
          return Function('"use strict";return (' + obj + ')')();
        };

        var newdata = JsonParse(String.fromCharCode.apply(null, data.slice(1)));
        
        modules[newdata.id] = new canvas_module(newdata);

        modules[newdata.id].init({visible: true, OffsetX: 0, OffsetY: newdata.id*300, r: 0});

        ModuleEditor.update()

        Canvas.resize();
      }
    },
    {
      opcode: 0x31,
      name: "TrackLayoutRawData",
      send: function(data){
        return [data.module];
      },
      recv: function(data){
        console.log("TrackLayoutRawData", data);

        var moduleid = data[0];
        var connections = data[1];
        var nodes = data[2];
        var nr_blocks = data[3] + (data[4] << 8);
        var nr_switches = data[5] + (data[6] << 8);
        var nr_msswitches = data[7] + (data[8] << 8);
        var nr_signals = data[9] + (data[10] << 8);
        var nr_stations = data[11];

        modules[moduleid].config = {};
        var conf = modules[moduleid].config;
        conf.nodes = [];
        conf.blocks = [];
        conf.switches = [];
        conf.msswitches = [];
        conf.signals = [];
        conf.stations = [];

        console.log(moduleid, connections, nodes, nr_blocks, nr_switches, nr_msswitches, nr_signals, nr_stations);

        var i = 13;

        for(var j = 0; j < nodes; j++){
          conf.nodes.push({id: data[i++], size: data[i++]});

          i++;
        }

        for(var j = 0; j < nr_blocks; j++){
          var b = {id: data[i++],
                  type: data[i++],
                  next: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                  prev: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                  io_in: {n: data[i++], p: data[i++] + (data[i++]<<8)},
                  io_out: {n: data[i++], p: data[i++] + (data[i++]<<8)},
                  speed: data[i++],
                  length: data[i++] + (data[i++] << 8),
                  flags: data[i++]};

          conf.blocks.push(b);

          i++;
        }

        for(var j = 0; j < nr_switches; j++){
          var sw =  {id: data[i++],
                     det_block: data[i++],
                     app: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                     str: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                     div: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                     iolen: data[i++],
                     speed: {str: data[i++], div: data[i++]},
                     ports: []};

          i++;

          for(var k = 0; k < sw.iolen; k++){
            sw.ports.push({m: data[i++], id: data[i++] + (data[i++]<<8)})
          }

          i++;

          conf.switches.push(sw);
        }

        for(var j = 0; j < nr_msswitches; j++){
          var mssw =  {id: data[i++],
                     det_block: data[i++],
                     stateslen: data[i++],
                     iolen: data[i++],
                     states: [],
                     ports: []};

          i++;

          for(var k = 0; k < mssw.stateslen; k++){
            mssw.states.push({A: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                              B: {m: data[i++], id: data[i++] + (data[i++]<<8), type: data[i++]},
                              speed: data[i++],
                              io_out: data[i++] + (data[i++] << 8)});
          }

          i++;

          for(var k = 0; k < mssw.iolen; k++){
            mssw.ports.push({m: data[i++], id: data[i++] + (data[i++]<<8)})
          }

          i++;

          conf.msswitches.push(mssw);
        }

        for(var j = 0; j < nr_signals; j++){
          var sig =  {side: data[i] & 0x01,
                     id: (data[i++] >> 1) + (data[i++] << 7),
                     block: data[i++] + (data[i++] << 8),
                     outlen: data[i++],
                     ports: [],
                     stating: []};

          i++;

          for(var k = 0; k < sig.outlen; k++){
            sig.ports.push({m: data[i++], id: data[i++] + (data[i++]<<8)})
          }

          i++;

          for(var k = 0; k < sig.outlen; k++){
            sig.stating.push([data[i++],data[i++],data[i++],data[i++],data[i++],data[i++],data[i++],data[i++]]);
          }

          i++;

          conf.signals.push(sig);
        }

        for(var j = 0; j < nr_stations; j++){
          var st =  {type: data[i++],
                     blocklen: data[i++],
                     blocks: [],
                     namelen: data[i++],
                     name: ""};

          i++;

          for(var k = 0; k < st.blocklen; k++){
            st.blocks.push(data[i++]);
          }

          i++;

          st.name =  String.fromCharCode.apply(null, data.slice(i, i+st.namelen));
          i += st.namelen;

          i++;

          conf.stations.push(st);
        }

        ModuleEditor.update_config(moduleid);
      }
    },
    {
      opcode: 0x33,
      name: "TrackLayoutUpdateRaw",
      send: function(data){
        console.warn("TrackLayoutUpdateRaw", data);
      },
      recv: function(data){
        console.warn("TrackLayoutUpdateRaw", data);
      }
    },
    {
      opcode: 0x36,
      name: "StationLibrary",
      send: function(data){},
      recv: function(data){
        nr = data[0];

        stations = {};

        $.each(modules, function(){
          this.stations = {};
        });
        
        var i = 1;
        for(var j = 0; j < nr; j++){
          st = {module: data[i++],
                id: data[i++],
                type: data[i++],
                name_len: data[i++],
                name: ""};
          st.name = String.fromCharCode.apply(null, data.slice(i, i+st.name_len));

          i += st.name_len;

          stations[j] = st;
          modules[st.module].stations[st.id] = st;
        }
      }
    },

  // Client / General
    {
      opcode: 0x10,
      name: "EmergencyStop",
      send: function(data){ return [] },
      recv: function(data){
        Emergency.set("Em", false);
      }
    },
    {
      opcode: 0x11,
      name: "ShortCircuitStop",
      recv: function(data){
        Emergency.set("Es", false);
      }
    },
    {
      opcode: 0x12,
      name: "ClearEmergency",
      send: function(data){ return [] },
      recv: function(data){
        Emergency.unset(false);
      }
    },
    {
      opcode: 0x13,
      name: "NewMessage",
      recv: function(data){
        var msg = {};
        msg.id = data[1] + ((data[0] & 0x1F) << 8);
        msg.type = (data[1] & 0xE0);
        msg.data = data.slice(2);

        Messages.add(msg);
      }
    },
    {
      opcode: 0x14,
      name: "UpdateMessage",
      recv: function(data){
        console.log("UpdateMessage", data);
      }
    },
    {
      opcode: 0x15,
      name: "ClearMessage",
      recv: function(data){
        var msgID = data[1] + ((data[0] & 0x1F) << 8);
        Messages.remove(msgID, data[0] >> 5);
      }
    },
    {
      opcode: 0x16,
      name: "ChangeBroadcast",
      send: function(data){
        console.warn("ChangeBroadcast", data);
        var name = evt.currentTarget.classList[1];
        broadcastFlags ^= parseInt(name.slice(2),16);

        if($(evt.target).attr("nr") == "4"){
          if(broadcastFlags & 0x10){
            var passphrase = $.md5(prompt("Please enter your password", "password"));

            //LOGIN
            websocket.cts_login(passphrase);
          }else{
            websocket.cts_logout();
          }
        }

        return [broadcastFlags];
      },
      recv: function(data){
        $('#Broadcastflags .checkbox input[type="checkbox"]:checked').prop("checked",false);

        if(data[0] & 0x80){
          $('#Broadcastflags .checkbox.bf80 input[type="checkbox"]').prop("checked",true);
        }
        if(data[0] & 0x40){
          $('#Broadcastflags .checkbox.bf40 input[type="checkbox"]').prop("checked",true);
        }
        if(data[0] & 0x20){
          $('#Broadcastflags .checkbox.bf20 input[type="checkbox"]').prop("checked",true);
        }
        if(data[0] & 0x10){
          $('#Broadcastflags .checkbox.bf10 input[type="checkbox"]').prop("checked",true);
          admin = 1;
          change_admin();
        }else{
          //Admin unset: logout
          admin = 0;
          change_admin();
        }

        if(data[0] & 0x8){
          $('#Broadcastflags .checkbox.bf08 input[type="checkbox"]').prop("checked",true);
        }
        if(data[0] & 0x4){
          $('#Broadcastflags .checkbox.bf04 input[type="checkbox"]').prop("checked",true);
        }
        if(data[0] & 0x2){
          $('#Broadcastflags .checkbox.bf02 input[type="checkbox"]').prop("checked",true);
        }
        if(data[0] & 0x1){
          $('#Broadcastflags .checkbox.bf01 input[type="checkbox"]').prop("checked",true);
        }
        broadcastFlags = data[0];
      }
    },
    {
      opcode: 0x17,
      name: "Service_State",
      send: function(data){},
      recv: function(data){
        state = (data[0] << 8) + data[1];
      
        $('#status_flags .green').toggleClass("red");
        $('#status_flags .green').toggleClass("green");

        if(state & 0x8000){
            $('.f8000','#status_flags').toggleClass("green");
            $('.f8000','#status_flags').toggleClass("red");
        }
        if(state & 0x4000){
            $('.f4000','#status_flags').toggleClass("green");
            $('.f4000','#status_flags').toggleClass("red");
        }
        if(state & 0x2000){
            $('.f2000','#status_flags').toggleClass("green");
            $('.f2000','#status_flags').toggleClass("red");
        }
        if(state & 0x1000){
            $('.f1000','#status_flags').toggleClass("green");
            $('.f1000','#status_flags').toggleClass("red");
        }

        if(state & 0x0800){
            $('.f0800','#status_flags').toggleClass("green");
            $('.f0800','#status_flags').toggleClass("red");
        }
        if(state & 0x0400){
            $('.f0400','#status_flags').toggleClass("green");
            $('.f0400','#status_flags').toggleClass("red");
        }
        if(state & 0x0200){
            $('.f0200','#status_flags').toggleClass("green");
            $('.f0200','#status_flags').toggleClass("red");
        }
        if(state & 0x0100){
            $('.f0100','#status_flags').toggleClass("green");
            $('.f0100','#status_flags').toggleClass("red");
        }

        if(state & 0x0080){
            $('.f0080','#status_flags').toggleClass("green");
            $('.f0080','#status_flags').toggleClass("red");
        }
        if(state & 0x0040){
            $('.f0040','#status_flags').toggleClass("green");
            $('.f0040','#status_flags').toggleClass("red");
        }
        if(state & 0x0020){
            $('.f0020','#status_flags').toggleClass("green");
            $('.f0020','#status_flags').toggleClass("red");
        }
        if(state & 0x0010){
            $('.f0010','#status_flags').toggleClass("green");
            $('.f0010','#status_flags').toggleClass("red");
        }

        if(state & 0x0008){
            $('.f0008','#status_flags').toggleClass("green");
            $('.f0008','#status_flags').toggleClass("red");
        }
        if(state & 0x0004){
            $('.f0004','#status_flags').toggleClass("green");
            $('.f0004','#status_flags').toggleClass("red");
        }
        if(state & 0x0002){
            $('.f0002','#status_flags').toggleClass("green");
            $('.f0002','#status_flags').toggleClass("red");
        }
        if(state & 0x0001){
            $('.f0001','#status_flags').toggleClass("green");
            $('.f0001','#status_flags').toggleClass("red");
        }
      }
    }
]);

$(document).ready(function(){
  websocket.connect("ws://192.168.2.92:9000/", 0xFF);
});
