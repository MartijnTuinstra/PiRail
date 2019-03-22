var admin;

var active_trains = []; //Trains that were active on the layout.
var broadcastFlags = 0xFF;

var Track_Layout_data;
var blocks_load = 0;

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
  //Opcodes
  opc: {
    ClearTrack:           0x80,
    ReloadTrack:          0x81,
    Track_Scan_Progress:  0x82,
    Track_Layout_Update:  0x83,
    Track_Layout_Config:  0x84,
    Z21_Track_Info:       0x86,
    Z21_Settings:         0x87,
    Reset_Switches:       0x8C,
    TrainsToDepot:        0x8F,

    EnableSubModule:      0x90,
    DisableSubModule:     0x91,
    SubModuleState:       0x92,
    RestartApplication:   0x9F,

    AdminEmergency:       0xC0,
    AdminEmergencyRelease:0xC1,
    AdminLogout:          0xCE,
    AdminLogin:           0xCF,


    //Trains
    LinkTrain:            0x41,
    TrainSpeed:           0x42,
    TrainFunction:        0x43,
    TrainOperation:       0x44,
    Z21TrainData:         0x45,
    TrainAddRoute:        0x46,

    AddNewEnginetolib: 0x50,
    EditEnginelib:     0x51,
    EnginesLibrary:    0x52,

    AddNewCartolib:    0x53,
    EditCarlib:        0x54,
    CarsLibrary:       0x55,

    AddNewTraintolib:  0x56,
    EditTrainlib:      0x57,
    TrainsLibrary:     0x58,

    TrainCategories:   0x5A,


    //Track and switches
    SetSwitch:            0x20,
    SetMultiSwitch:       0x21,
    SetSwitchReserved:    0x22,
    ChangeReservedSwitch: 0x23,
    SetSwitchRoute:       0x25,

    BroadTrack:           0x26,
    BroadSwitch:          0x27,

    TrackLayoutOnlyRawData: 0x30,
    TrackLayoutRawData:     0x31,
    TrackLayoutUpdateRaw:   0x33,
    StationLibrary:         0x36,


    //Client / General
    EmergencyStop:        0x10,
    ShortCircuitStop:     0x11,
    ClearEmergency:       0x12,
    NewMessage:           0x13,
    UpdateMessage:        0x14,
    ClearMessage:         0x15,
    ChangeBroadcast:      0x16,
    Service_State:        0x17,
    Canvas_Data:          0x1F,
  },

  message_id: -1,

  states: {
    //Service States

    Z21_FLAG:         0x8000,
    WebSocket_FLAG:   0x4000,
    COM_FLAG:         0x2000,
    Client_Accept:    0x1000,

    TRACK_DIGITAL:    0x0200,
    RUN:              0x0100,

    Modules_Loaded:   0x0002,
    Modules_Coupled:  0x0004
  },

  send: function(data){
    console.log("Send", data);
    ws.send(new Int8Array(data));
  },

  ws_close_list: [],
  ws_close: function(){
    for(var i = 0; i < this.ws_close_list.length; i++){
      this.ws_close_list[i]();
    }
  },
  ws_close_add: function(f){
    this.ws_close_list.push(f);
  },

/*Client to Server*/
  cts_reload_previous: function(entry){
    console.warn("WEBSOCKET: RELOAD_PREVIOUS, not implemented, entry: "+entry);
  },
  cts_enable_submodule: function(module){
    data = [];
    data[0] = this.opc.EnableSubModule;
    data[1] = 1 << module;

    this.send(data);
  },
  cts_disable_submodule: function(module){
    data = [];
    data[0] = this.opc.DisableSubModule;
    data[1] = 1 << module;

    this.send(data);
  },

  cts_z21_settings: function(ip){
    data = [];
    data[0] = this.opc.Z21_Settings;
    data[1] = parseInt(ip.ip1);
    data[2] = parseInt(ip.ip2);
    data[3] = parseInt(ip.ip3);
    data[4] = parseInt(ip.ip4);

    this.send(data);
  },

  cts_restart: function(){
    data = [];
    data[0] = this.opc.RestartApplication;

    this.send(data);
  },

  /*Trains*/
    cts_link_train: function(fid, rid, type, mid){
      data = [];
      data[0] = this.opc.LinkTrain;
      data[1] = fid;
      data[2] = rid;
      data[3] = ((type == "E")?0x80:0) + ((mid & 0x1F) >> 8)

      this.send(data);
    },

    cts_train_speed: async function(type, train){
      data = [];
      data[0] = this.opc.TrainSpeed;
      data[1] = train.id & 0xFF;
      data[2] = (train.id & 0x300) >> 2;
      if(type == "T"){
        data[2] |= 0x20;
      }
      data[2] |= (train.dir & 1) << 4;
      data[2] |= (train.speed & 0xF00) >> 8;
      data[3] = train.speed & 0xFF;

      this.send(data);
    },

    cts_add_car: function(data){
      msg = [];
      msg[0] = this.opc.AddNewCartolib;
      msg[1] = (data.nr & 0x00ff);
      msg[2] = (data.nr & 0xff00) >> 8;
      msg[3] = (data.max_speed & 0x00ff);
      msg[4] = (data.max_speed & 0xff00) >> 8;
      msg[5] = (data.length & 0x00ff);
      msg[6] = (data.length & 0xff00) >> 8;
      msg[7] = data.type;

      if(data.icon_name.endsWith('jpg')){
        msg[8] |= 0x1;
      }
      msg[9] = data.name.length;

      //Upload timestamps
      var time = data.icon_name.split(".");
      time = time[time.length - 2].split("");

      var time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

      msg[10] = time_icon & 0xFF;
      msg[11] = time_icon >> 8;

      for (var i = 0; i < data.name.length; i++) {
        msg.push(data.name.charCodeAt(i));
      }

      this.send(msg);
    },

    cts_edit_car: function(data, id, edit=true){
      msg = [];
      msg[0] = this.opc.EditCarlib;
      msg[1] = ((!edit) << 7) | (id & 0x7f00) >> 8;
      msg[2] = id & 0xff;

      if(!edit){ //Remove
        this.send(msg);
        return;
      }

      msg[3] = (data.nr & 0x00ff);
      msg[4] = (data.nr & 0xff00) >> 8;
      msg[5] = (data.max_speed & 0x00ff);
      msg[6] = (data.max_speed & 0xff00) >> 8;
      msg[7] = (data.length & 0x00ff);
      msg[8] = (data.length & 0xff00) >> 8;
      msg[9] = data.type;

      if(data.icon_name.endsWith('jpg')){
        msg[10] |= 0x1;
      }
      msg[11] = data.name.length;

      //Upload timestamps
      var time = data.icon_name.split(".");
      time = time[time.length - 2].split("");

      var time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

      msg[12] = time_icon & 0x0F;
      msg[13] = time_icon >> 4;

      for (var i = 0; i < data.name.length; i++) {
        msg.push(data.name.charCodeAt(i));
      }

      this.send(msg);
    },

    cts_add_engine: function(data){
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
        return;
      }

      msg = [];
      msg[0] = this.opc.AddNewEnginetolib;
      msg[1] = (data.dcc & 0x00ff);
      msg[2] = (data.dcc & 0xff00) >> 8;
      msg[3] = (data.length & 0x00ff);
      msg[4] = (data.length & 0xff00) >> 8;
      msg[5] = data.type;

      if(data.speedstep == 28){
        msg[6] |= 0b0100;
      }
      else if(data.speedstep == 128){
        msg[6] |= 0b1000;
      }

      if(data.image_name.endsWith('jpg')){
        msg[6] |= 0b10;
      }
      if(data.icon_name.endsWith('jpg')){
        msg[6] |= 0b01;
      }

      msg[7] = data.name.length;

      msg[8] = data.speedsteps.length;

      //Upload timestamps
      var time = data.image_name.split(".");
      time = time[time.length - 2].split("");

      var time_image = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

      var time = data.icon_name.split(".");
      time = time[time.length - 2].split("");

      var time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);

      msg[9] = time_image & 0xFF;
      msg[10] = ((time_image >> 4) & 0xF0) | (time_icon & 0x0F);
      msg[11] = (time_icon >> 4);

      for (var i = 0; i < data.name.length; i++) {
        msg.push(data.name.charCodeAt(i));
      }

      for (var i = 0; i < data.speedsteps.length; i++){
        console.log(data.speedsteps[i]);
        msg.push(data.speedsteps[i].speed);
        msg.push(data.speedsteps[i].speed >> 8);
        msg.push(data.speedsteps[i].step);
      }

      this.send(msg);
    },

    cts_edit_engine: function(data, id, edit=true){
      msg = [];
      msg[0] = this.opc.EditEnginelib;
      msg[1] = ((!edit) << 7) | (id & 0x7f00) >> 8;
      msg[2] = id & 0x7fff;

      if(!edit){ //Remove
        this.send(msg);
        return;
      }

      msg[3] = (data.dcc & 0x00ff);
      msg[4] = (data.dcc & 0xff00) >> 8;
      msg[5] = (data.length & 0x00ff);
      msg[6] = (data.length & 0xff00) >> 8;
      msg[7] = data.type;

      if(data.speedstep == 28){
        msg[8] |= 0b0100;
      }
      else if(data.speedstep == 128){
        msg[8] |= 0b1000;
      }
      if(data.image_name.endsWith('jpg')){
        msg[8] |= 0b10;
      }
      if(data.icon_name.endsWith('jpg')){
        msg[8] |= 0b1;
      }

      msg[9] = data.name.length;
      msg[10] = data.speedsteps.length;

      //Upload timestamps
      var time = data.image_name.split(".");
      var time_image = 0xFFF;
      var time_icon = 0xFFF;

      if(time.length == 3){
        time = time[time.length - 2].split("");

        var time_image = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);
      }

      var time = data.icon_name.split(".");
      if(time.length == 3){
        time = time[time.length - 2].split("");

        var time_icon = parseInt(time[0]) * 600 + parseInt(time[1]) * 60 + parseInt(time[2]) * 10 + parseInt(time[3]);
      }

      msg[11] = time_image & 0xFF;
      msg[12] = ((time_image >> 4) & 0xF0) | (time_icon & 0x0F);
      msg[13] = (time_icon >> 4) & 0xFF;

      for (var i = 0; i < data.name.length; i++) {
        msg.push(data.name.charCodeAt(i));
      }

      for (var i = 0; i < data.speedsteps.length; i++){
        console.log(data.speedsteps[i]);
        msg.push(data.speedsteps[i].speed);
        msg.push(data.speedsteps[i].speed >> 8);
        msg.push(data.speedsteps[i].step);
      }

      this.send(msg);
    },

    cts_add_train: function(data){
      msg = [];
      msg[0] = this.opc.AddNewTraintolib;
      msg[1] = data.name.length;
      msg[2] = 0x80 | (data.list.length & 0x7f);
      msg[3] = data.type;

      for (var i = 0; i < data.name.length; i++) {
        msg.push(data.name.charCodeAt(i));
      }

      for (var i = 0; i < data.list.length; i++){
        msg.push(data.list[i].type);
        msg.push((data.list[i].id & 0x00ff));
        msg.push((data.list[i].id & 0xff00) >> 8);
      }

      this.send(msg);
    },

    cts_edit_train: function(data, id, edit=true){
      msg = [];
      msg[0] = this.opc.EditTrainlib;
      msg[1] = ((!edit) << 7) | (id & 0x7f00) >> 8;
      msg[2] = id & 0x7fff;

      if(!edit){ //Remove
        this.send(msg);
        return;
      }

      msg[3] = data.name.length;
      msg[4] = data.list.length & 0x7f;
      msg[5] = data.type;

      for (var i = 0; i < data.name.length; i++) {
        msg.push(data.name.charCodeAt(i));
      }

      for (var i = 0; i < data.list.length; i++){
        msg.push(data.list[i].type);
        msg.push((data.list[i].id & 0x00ff));
        msg.push((data.list[i].id & 0xff00) >> 8);
      }
      this.send(msg);
    },


  /*Track / Switches*/
    cts_set_switch: function(d){ //Module, Switch, NewState
      // console.log("Set switch "+m+":"+s+"=>"+st);
      var data = [];

      if(d.length == 1){
        data[0] = this.opc.SetSwitch;
        data[1] = d[0][0];
        data[2] = d[0][1];
        data[3] = d[0][2];

        this.send(data);
      }
      else{
        var di = 2; // Dataindex
        data[0] = this.opc.SetMultiSwitch
        data[1] = d.length;
        for (var i = 0; i < d.length; i++) {
          data[di++] = d[i][0]
          data[di++] = d[i][1]
          data[di++] = d[i][2]
        }
        this.send(data);
      }
    },

    cts_layout_request_raw: function(module){
      var data = [];

      data[0] = this.opc.TrackLayoutRawData;
      data[1] = module;

      this.send(data);
    },

  cts_release_Emergency: function(evt){
    this.send([this.opc.ClearEmergency]);
  },

  cts_Emergency: function(evt){
    this.send([this.opc.EmergencyStop]);
  },

  cts_BroadcastFlag: function(evt){
    var name = evt.currentTarget.classList[1];
    console.warn(name);
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

    websocket.send([websocket.opc.ChangeBroadcast,broadcastFlags]);
  },

  cts_Stop_Scan: function(){
    if(admin){
      var data = [];

      data[0] = this.opc.Track_Scan_Progress;
      data[1] = 1;

      ws.send(new Int8Array(data));
    }
  },
  cts_login: function(passphrase){
    var data = [];
    data[0] = this.opc.AdminLogin;
    for(var i = 0;i<passphrase.length;i++){
      data[i+1] = passphrase.charCodeAt(i);
    }

    websocket.send(data);
  },

  cts_logout: function(){
    websocket.send([this.opc.AdminLogout]);
  },
/**/
/*Server to Client*/
  /*  ADMIN MESSAGES  */
    stc_login: function(){
      // var passphrase = $.md5(prompt("Please enter your password", "password"));
      var passphrase = $.md5("password");

      websocket.cts_login(passphrase);
    },
    // Track scan progress
    stc_scan_progress: function(data){
      console.warn("TODO: reimplement");return;
      $('#Modules_scan .progress').html(data[1]+'/'+data[2]+' connections');
      $('#Modules_scan .found_modules').html("");
      var i = 3;
      for(i;i<data.length;i++){
        text = data[i] + "<br/>"

        $('#Modules_scan .found_modules').append(text);
      }
    },

    // Track Setup Partial Update
    stc_track_setup_partial: function(data){
      for(var i = 1;i<data.length;i++){
        var module = modules[data[i]]
        for(var j = 1;j<= module.connections.length; j++){
          module.connection_link[j-1] = data[i+j];
        }
        console.log("Module "+data[i]+" connect ", module.connection_link, {x: module.OffsetX, y:module.OffsetY, r:module.r});
        i += module.connections.length;

        module.connect();
      }

      Canvas.rescale(1);
    },

    // Track Layout Setup
    stc_track_setup: function(data){
      var first = true;
      for(var i = 1;i<data.length;i++){
        var module = modules[data[i]]
        for(var j = 1;j<= module.connections.length; j++){
          module.connection_link[j-1] = data[i+j];
        }
        console.log("Module "+data[i]+" connect ", module.connection_link, {x: module.OffsetX, y:module.OffsetY, r:module.r});
        i += module.connections.length;

        module.connect();
      }

      Canvas.rescale(1);
    },


    // Track info, voltage,current
    stc_track_info: function(data){
      var i=0;
      var mcur = ToInt16(data[i++] + (data[i++] << 8));
      var mfcur = ToInt16(data[i++] + (data[i++] << 8));
      var pcur = ToInt16(data[i++] + (data[i++] << 8));
      var vcc = data[i++] + (data[i++] << 8);
      var sup = data[i++] + (data[i++] << 8);
      var temp = ToInt16(data[i++] + (data[i++] << 8));
      var flags = data[i++] + (data[i++] << 8);
      Z21.update({"tcur": mcur, "tfcur": mfcur, "pcur": pcur, "svol": sup, "tvol": vcc, "temp": temp, "flags": flags});
    },

    stc_z21_settings: function(data){
      var i = 0;
      var ip = data[i++] + "." + data[i++] + "." + data[i++] + "." + data[i++];
      var fw = data[i++] + "." + data[i++];
      Z21.update({"ip-addr": ip, "fw-version": fw});
    },


  /*  TRAIN MESSAGES  */

    // Train data
    stc_train_data: function(data){
      console.warn("TODO: Train_Data_Update reimplement");
      if(data.length > 5){
        var train_ID = data[1];
        var DCC_ID = (data[2] << 8) + data[3];

        var speed_step = (data[5] & 0x7F); // divide by 127

        Train.data[train_ID-2].speed = (speed_step/127)*Train.data[train_ID-2].max_speed;

        // (data[5] & 0x80) == 0 // direction reverse

        // (data[4] & 0b1000) != 0 //'Andere X-BUS Handregler<br/>';
          
        // (data[4] & 0b111) == 0 // '14 steps<br/>';
        // (data[4] & 0b111) == 2 // 28 steps
        // (data[4] & 0b111) == 4 // 128 steps

        // (data[6] & 0x7F) // Fahrstufen KKK: 

        // (data[6] & 0x40) == 1 // 'Doppeltraktion<br/>';
        
        // (data[6] & 0x20) == 1 // 'Smartsearch<br/>';
        
        // (data[6] & 0x10) == 1 // F0
        // (data[6] & 0x8) == 1  // F4
        // (data[6] & 0x4) == 1  // F3
        // (data[6] & 0x2) == 1  // F2
        // (data[6] & 0x1) == 1  // F1
      }
    },

    // Train Route
    stc_set_route: function(data){
      Train.route(data[1],data[2],data[3])
    },

    // Engines Lib 0x51
    stc_engines_lib: function(data){
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
    },

    // Add new train to library
    stc_newengine_tolib: function(data){
      if(data[3] == 1){
        Modals.hide();
      }
      else if(data[3] == 0){
        Modals.call_cb("create_cb", {return_code: 0});
      }
      else if(data[3] == 255){
        Modals.call_cb("create_cb", {return_code: -1});
      }
    },

    // Cars Lib 0x53
    stc_cars_lib: function(data){
      Train.cars = [];
      for(var i = 0;i<data.length;){
        var nr_id = (data[i] + (data[i+1] << 8));
        i += 2;
        var max_spd = (data[i] + (data[i+1] << 8));
        i += 2;
        var length = (data[i] + (data[i+1] << 8));
        i += 2;
        type = data[i++];

        var name = IntArrayToString(data.slice(i+3, i+3+data[i]));
        var text_length = data[i++];

        var img = IntArrayToString(data.slice(i+2+text_length, i+2+text_length+data[i]));
        text_length += data[i++];

        var icon = IntArrayToString(data.slice(i+1+text_length, i+1+text_length+data[i]));
        text_length += data[i++];

        Train.cars.push({name: name, nr: nr_id, img: img, icon: icon, max_speed: max_spd, length: length, type: type});
        i += text_length;
      }

      Train_Configurator.update();
    },

    // Add new train to library
    stc_newcar_tolib: function(data){
      if(data[3] == 1){
        if(Train.engine_car_creator.save_cb != undefined){
          Train.engine_car_creator.save_cb();
        }
      }
      else if(data[3] == 0){
        alert("Failed with no reason");
      }
      else if(data[3] == -1){
        alert("DCC address is allready in use");
      }
    },

    //Trains 0x55
    stc_trains_lib: function(data){
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
    },

    stc_newtrain_tolib: function(data){
      console.warn("implement");
    },

    stc_traincategories: function(data){
      Train.cat = {};
      for(var i = 0;i<data.length;){
        var id = data[i++];

        var name = IntArrayToString(data.slice(i+1, i+1+data[i]));

        Train.cat[id] = name;
        i += data[i]+1;
      }
    },

  /*  TRACK MESSAGES  */
    // Broadcast track occupation
    stc_track_broadcast: function(data){
      for(var i = 1;i<data.length;i+=4){
        var m = data[i];
        var B = data[i+1];

        var D = data[i+2] >> 7;
        var state = (data[i+2] & 0x7F);
        var tID = data[i+3];

        modules[m].blocks[B] = state;
      }

      Canvas.update_frame();
    },

    // Broadcast states of switches
    stc_switch_broadcast: function(data){
      for(var i = 1;i<data.length;){
        var M = data[i];

        if(data[i+1] & 0x80){ //MSSwitch
          var len   = data[i+3];

          modules[M].switches[data[i+1]&0x7F] = data[i+2] & 0x7F;

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
    },

    stc_track_layout_only_data: function(data){
      function JsonParse(obj){
          return Function('"use strict";return (' + obj + ')')();
      };

      var newdata = JsonParse(String.fromCharCode.apply(null, data.slice(2)));
      
      modules[newdata.id] = new canvas_module(newdata);

      modules[newdata.id].init({visible: true, OffsetX: 0, OffsetY: newdata.id*500, r: 0});

      ModuleEditor.update()

      Canvas.resize();
    },

    // Station Library
    stc_station_lib: function(data){
      console.warn("implement");
      Stations.import(data.slice(1,data.length));
    },


  /* GENERAL MESSAGES */
    // Emergency Stops
    stc_emergency: function(type){
      if(type == 0){ //Emergency stop
        Emergency.start("Em",false);

      }else if(type == 1){ //Short Circuit
        Emergency.start("Es",false);

      }else if(type == 2){ //Release
        Emergency.stop(false);
      }
    },

    // Message Add
    stc_new_message: function(data){
      var msgID = data[2] + ((data[1] & 0x1F) << 8);

      Messages.add({id: msgID,type: (data[1] & 0xE0),data: data.slice(3,data.length)});
    },
    // Message Update
    stc_update_message: function(data){
      console.warn("TODO: implement");return;
      var msgID = data[2] + ((data[1] & 0x1F) << 8);

      Messages.update({id: msgID,type: (data[1] & 0xE0),data: data.slice(3,data.length)});
    },
    // Message Clear
    stc_clear_message: function(data){
      var msgID = data[2] + ((data[1] & 0x1F) << 8);
      Messages.remove(msgID, data[1] >> 5);
    },

    // Broadcast flags
    stc_Broadcast_Flags: function(data){
      
      $('#Broadcastflags .checkbox input[type="checkbox"]:checked').prop("checked",false);

      if(data[1] & 0x80){
        $('#Broadcastflags .checkbox.bf80 input[type="checkbox"]').prop("checked",true);
      }
      if(data[1] & 0x40){
        $('#Broadcastflags .checkbox.bf40 input[type="checkbox"]').prop("checked",true);
      }
      if(data[1] & 0x20){
        $('#Broadcastflags .checkbox.bf20 input[type="checkbox"]').prop("checked",true);
      }
      if(data[1] & 0x10){
        $('#Broadcastflags .checkbox.bf10 input[type="checkbox"]').prop("checked",true);
        admin = 1;
        change_admin();
      }else{
        //Admin unset: logout
        admin = 0;
        change_admin();
      }

      if(data[1] & 0x8){
        $('#Broadcastflags .checkbox.bf08 input[type="checkbox"]').prop("checked",true);
      }
      if(data[1] & 0x4){
        $('#Broadcastflags .checkbox.bf04 input[type="checkbox"]').prop("checked",true);
      }
      if(data[1] & 0x2){
        $('#Broadcastflags .checkbox.bf02 input[type="checkbox"]').prop("checked",true);
      }
      if(data[1] & 0x1){
        $('#Broadcastflags .checkbox.bf01 input[type="checkbox"]').prop("checked",true);
      }
      broadcastFlags = data[1];
    },

    // Server state
    stc_service_state: function(data){
      state = (data[1] << 8) + data[2];
      
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
    },

    // Canvas Data
    stc_canvas_data: function(data){
      moduleID = data[1];
      for (var i = 2; i < data.length; i++) {
        console.warn("Add canvas_data() processing");
      }
    },   
/**/
}

function track_layout(module,prev){
  console.warn("Track_Layout depricated");
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

function WebSocket_handler(adress){
    // Let us open a web socket
    ws = new WebSocket("ws://"+adress+"/",broadcastFlags);
    ws.binaryType = 'arraybuffer';

    ws.connected = false;

    ws.onopen = function(){
      ws.connected = true;

      events.ws_onopen();

      Messages.clear();
    };

    ws.onmessage = function (evt){
      var received_msg = evt.data;
      var data = new Uint8Array(received_msg);
      // console.log(data);





      /* SYSTEM MESSAGES  */
        if(data[0] == websocket.opc.SubModuleState){
          console.log("SubModule States", data);
          Submodules.update([data[1], data[2]]);
        }
      /*  ADMIN MESSAGES  */

        else if(data[0] == websocket.opc.Track_Scan_Progress){
          console.log("Scan Progress", data);
          websocket.stc_scan_progress(data);
        }
        else if(data[0] == websocket.opc.Track_Layout_Update){
          console.log("Partial Track Layout", data);
          websocket.stc_track_setup_partial(data);
        }
        else if(data[0] == websocket.opc.Track_Layout_Config){
          console.log("Track Layout", data);
          websocket.stc_track_setup(data);
        }
        else if(data[0] == websocket.opc.Z21_Track_Info){
          console.log("Z21_Track_Info", data);
          websocket.stc_track_info(data.slice(1));
        }
        else if(data[0] == websocket.opc.Z21_Settings){
          console.log("Z21_Settings", data);
          websocket.stc_z21_settings(data.slice(1));
        }
        else if(data[0] == websocket.opc.AdminLogin){
          console.log("AdminLogin", data);
          websocket.stc_login();
        }

      /*  TRAIN MESSAGES  */

        else if(data[0] == websocket.opc.Z21TrainData){
          console.log("Z21 Train data", data);
          websocket.stc_train_data(data);
        }
        else if(data[0] == websocket.opc.LinkTrain){
          console.log("Link train", data);
          train_follow[data[1]] = data[2]; // Link the follow ID to the train ID
        }

        else if(data[0] == websocket.opc.EnginesLibrary){
          console.log("Engine Lib", data);
          websocket.stc_engines_lib(data.slice(1));
        }

        else if(data[0] == websocket.opc.AddNewEnginetolib){
          console.log("AddNewEnginetoLibrary", data);
          websocket.stc_newengine_tolib(data);
        }

        else if(data[0] == websocket.opc.CarsLibrary){
          console.log("Engine Lib", data);
          websocket.stc_cars_lib(data.slice(1));
        }

        else if(data[0] == websocket.opc.AddNewCartolib){
          console.log("AddNewCartoLibrary");
          websocket.stc_newcar_tolib(data);
        }

        else if(data[0] == websocket.opc.TrainsLibrary){
          console.log("Trains Lib", data);
          websocket.stc_trains_lib(data.slice(1));
        }

        else if(data[0] == websocket.opc.AddNewTraintolib){
          console.log("AddNewTraintoLibrary");
          websocket.stc_newtrain_tolib(data);
        }

        else if(data[0] == websocket.opc.TrainCategories){
          console.log("Train Catagories", data);
          websocket.stc_traincategories(data.slice(1));
        }

      /*  TRACK MESSAGES  */

        else if(data[0] == websocket.opc.BroadTrack){
          console.log("Update for the track", data);
          websocket.stc_track_broadcast(data);
        }else if(data[0] == websocket.opc.BroadSwitch){
          console.log("Update for the Switches", data);
          websocket.stc_switch_broadcast(data);
        }
        else if(data[0] == websocket.opc.TrackLayoutSetup){
          console.log("Track Layout", data);
          websocket.stc_track_load(data);
        }
        else if(data[0] == websocket.opc.TrackLayoutOnlyRawData){
          console.log("Track Layout Only Data", data);
          websocket.stc_track_layout_only_data(data);
        }
        else if(data[0] == websocket.opc.TrackLayoutRawData){
          console.log("Track Layout Raw Data", data);
          // websocket.stc_track_layout_only_data(data);
        }

      /* GENERAL MESSAGES */
        else if(data[0] == websocket.opc.EmergencyStop){
          console.warn("Emergency Stop enabled", data);
          websocket.stc_emergency(0);
        }
        else if(data[0] == websocket.opc.ShortCircuitStop){
          console.warn("Short Circuit enabled", data);
          websocket.stc_emergency(1);
        }
        else if(data[0] == websocket.opc.ClearEmergency){
          console.warn("Emergency released", data);
          websocket.stc_emergency(2);
        }
        else if(data[0] == websocket.opc.NewMessage){
          console.log("New Message", data);
          websocket.stc_new_message(data);
        }
        else if(data[0] == websocket.opc.UpdateMessage){
          console.log("Clear Message", data);
          websocket.stc_update_message(data);
        }
        else if(data[0] == websocket.opc.ClearMessage){
          console.log("Clear Message", data);
          websocket.stc_clear_message(data);
        }
        else if(data[0] == websocket.opc.ChangeBroadcast){
          console.log("New Broadcast Flags", data);
          websocket.stc_Broadcast_Flags(data);
        }
        else if(data[0] == websocket.opc.Service_State){
          websocket.stc_service_state(data, data);
        }
        else if(data[0] == websocket.opc.Canvas_Data){
          console.log("Canvas Data", data);
          websocket.stc_canvas_data(data);
        }


        else{
          console.warn("Unkown data packet", data);
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
      if(ws.connected == true){
        websocket.ws_close();
        Canvas.update_frame();
        Messages.clear();
        Messages.add({type:0xff,id:0});
      }

      events.ws_onclose();

      ws.connected = false;
      setTimeout(function(){
        WebSocket_handler(adress);
      }, 1000);
    };

}

events.add_ws_onclose(function(){
  $('.info-box .connection_state .indicator').removeClass("bg-success");
  $('.info-box .connection_state .indicator').addClass("bg-danger");
});
events.add_ws_onopen(function(){
  $('.info-box .connection_state .indicator').removeClass("bg-danger");
  $('.info-box .connection_state .indicator').addClass("bg-success");
});

$(document).ready(function(){
  Messages.add({type:0xff,id:0});
  //Open websocket or fail on unsupported browsers
  if ("WebSocket" in window){
  try{
      WebSocket_handler(window.location.host+':9000');
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

