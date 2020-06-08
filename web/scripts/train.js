var Train = {
  trains: [], //Train compositions
  engines: [],
  cars: [],
  railtrain: [],
  cat: {},

  get_engine_from_dcc: function(dcc){
    for(var i = 0; i<this.engines.length; i++){
      if(this.engines[i].dcc == dcc){
        return this.engines[i];
      }
    }
  }
}

var Train_Control = {
  init: function(){
    this.resize();

    this.train = [undefined, undefined, undefined];
    this.timer = 0;

    var text = '';

    for(var i = 0; i < 29; i++){
      position = (Math.floor(i / 4) * -32) + "px " + (((i % 4) + 1) * -32) + "px";
      text += '<div class="btn btn-sm btn-outline-primary train-function" func="' + i + '" style="background-position: '+position+'"></div>';
    }
    $('.train-box.box1 .train-function-box').html(text);
    $('.train-box.box2 .train-function-box').html(text);

    $('.train-box .train-function-box .train-function').on("mousedown touchstart", function(evt){
      evt.preventDefault();

      function_nr = $(evt.currentTarget).attr("func");

      console.log("Function "+function_nr + " down");

      var box = Train_Control.get_box(evt.currentTarget);

      var U = Train_Control.train[box].t;

      if(U.railtrainid != undefined){
        websocket.cts_TrainFunction({"id": U.railtrainid, "function": function_nr, "type": 1});
      }
      else if(Train_Control.train[box].type == 'E'){
        websocket.cts_DCCEngineFunction({"id": U.id, "function": function_nr, "type": 1});
      }
    });
    $('.train-box .train-function-box .train-function').on("mouseup touchend", function(evt){
      evt.preventDefault();

      function_nr = $(evt.currentTarget).attr("func");

      console.log("Function "+function_nr + " up");

      var box = Train_Control.get_box(evt.currentTarget);

      var U = Train_Control.train[box].t;

      if(U.functions[function_nr].button == 0){
        if(U.railtrainid != undefined){
          websocket.cts_TrainFunction({"id": U.railtrainid, "function": function_nr, "type": 0});
        }
        else if(Train_Control.train[box].type == 'E'){
          websocket.cts_DCCEngineFunction({"id": U.id, "function": function_nr, "type": 0});
        }
      }
    });

    function handler(box, evt) {
      evt.preventDefault();
        if (evt.type === 'mouseup' || evt.type == "touchend") {
          console.log("Remove events");
            $('body').off('mouseup mousemove touchmove touchend');
            this.drag = false;
            this.set_speed(box);
            this.remove_train_timer(box);
        } else {
          this.drag = true;

          var slider_box = $('.train-box.box'+box+' .train-speed-slider');
          var pageY = slider_box.offset().top + slider_box.height();
          var ylim = slider_box.height() - $('.slider-handle', slider_box).height();
          var pos = 0;
          if(evt.type == "mousemove" || evt.type == "mousedown"){
            pos = (pageY - evt.pageY);
          }else if(evt.type == "touchmove" || evt.type == "touchstart"){
            pos = (pageY - evt.touches[0].pageY);
          }
          if(pos < 0){
            pos = 0;
          }
          else if(pos > ylim){
            pos = ylim;
          }

          this.set_speed(box, pos / ylim);

          // websocket.cts_TrainSpeed({type: this.train[box].type, train: this.train[box].t});

          this.set_handle(box, pos);
        }
    }

    $('.train-box .train-speed-slider, .train-box .train-speed-slider .slider-handle').on('mousedown touchstart', function (evt) {
      evt.preventDefault();
      evt.stopImmediatePropagation();

      var box = this.get_box(evt.currentTarget);

      this.start_train_timer(box);

      handler.bind(this)(box, evt);
      
      $('body').on('mouseup mousemove touchmove touchend', handler.bind(this, box));
    }.bind(this));


    $('.train-box .train-info-box .control .btn-toggle').on("click", function(evt){
      evt.preventDefault();
      $('.btn-toggle', $(evt.target).closest(".train-box, .btn-toggle-group")).removeClass('btn-secondary');
      $('.btn-toggle', $(evt.target).closest(".train-box, .btn-toggle-group")).addClass('btn-outline-secondary');
      var btn = $(evt.target).closest('.btn-toggle');
      btn.removeClass('btn-outline-secondary');
      btn.addClass('btn-secondary');

      var control = parseInt(btn.attr("value"));
      var box = this.get_box(evt.currentTarget);

      this.set_control(box, control);
    }.bind(this));


    $('.train-box .train-dir-box .btn-toggle').on("click", function(evt){
      evt.preventDefault();
      $('.btn-toggle', $(evt.target).closest(".train-box, .btn-toggle-group")).removeClass('btn-secondary');
      $('.btn-toggle', $(evt.target).closest(".train-box, .btn-toggle-group")).addClass('btn-outline-secondary');
      var btn = $(evt.target).closest('.btn-toggle');
      btn.removeClass('btn-outline-secondary');
      btn.addClass('btn-secondary');

      var dir = btn.attr("value");
      var box = this.get_box(evt.currentTarget);

      this.set_dir(box, dir);
    }.bind(this));

    $('.train-box .train-stop > button.btn').on("click", function(evt){
      var box = this.get_box(evt.currentTarget);
      this.set_stop(box);
    }.bind(this));
  },

  init_train_functions: function(box){
    // $('.train-box.box'+box+' .train-function-box').empty();
    var text = '';

    // if(this.train[box] == undefined){
    //   return;
    // }

    if(this.train[box].type == 'E'){ // Engine
      var E = this.train[box].t;
      for(var i = 0; i < 29; i++){
        if(E.functions[i].type == 1){
          position = (Math.floor(i / 4) * -32) + "px " + (((i % 4) + 1) * -32) + "px";
        }
        else if(E.functions[i].type == 2){  // Headlight
          position = "0px 0px";
        }
        else if(E.functions[i].type == 3){  // Cablight
          position = "-32px 0px";
        }
        else if(E.functions[i].type == 4){  // General Horn
          position = "-96px 0px";
        }
        else if(E.functions[i].type == 5){  // Low Horn
          position = "-128px 0px";
        }
        else if(E.functions[i].type == 6){  // High Horn
          position = "-64px 0px";
        }
        else{
          $('.train-box.box' + box + ' .train-function-box .train-function[func=' + i + ']').hide();
          continue;
        }
        $('.train-box.box' + box + ' .train-function-box .train-function[func=' + i + ']').show();
        $('.train-box.box' + box + ' .train-function-box .train-function[func=' + i + ']').css("background-position", position);
      }
    }
    else{
      // Type is Train
      $('.train-box.box' + box + ' .train-function-box .train-function').hide();
      $('.train-box.box' + box + ' .train-function-box .train-function[func=0]').show();
      $('.train-box.box' + box + ' .train-function-box .train-function[func=1]').show();
      $('.train-box.box' + box + ' .train-function-box .train-function[func=2]').show();
      $('.train-box.box' + box + ' .train-function-box .train-function[func=3]').show();
      $('.train-box.box' + box + ' .train-function-box .train-function[func=4]').show();
      $('.train-box.box' + box + ' .train-function-box .train-function[func=0]').css("background-position", "0px 0px");
      $('.train-box.box' + box + ' .train-function-box .train-function[func=1]').css("background-position", "-32px 0px");
      $('.train-box.box' + box + ' .train-function-box .train-function[func=2]').css("background-position", "-96px 0px");
      $('.train-box.box' + box + ' .train-function-box .train-function[func=3]').css("background-position", "-128px 0px");
      $('.train-box.box' + box + ' .train-function-box .train-function[func=4]').css("background-position", "-64px 0px");
    }
  },

  update_Functions: function(box){
    if(this.train[box].type == 'E'){ // Engine
      var E = this.train[box].t;
      for(var i = 0; i < 29; i++){
        if(E.functions[i].state){
          $('.train-box.box' + box + ' .train-function-box .train-function[func=' + i + ']').addClass("active");
        }
        else{
          $('.train-box.box' + box + ' .train-function-box .train-function[func=' + i + ']').removeClass("active");
        }
      }
    }
  },

  resize: function(){
    if(window.innerWidth < 576){
      $('.train-box.box2').hide();
    }
    else{
      $('.train-box.box2').show();
    }

    var height = $('.train-box.box1 .train-img').height() + $('.train-box.box1 .train-info-box').height() + 20;

    $('.train-box.box1 .train-function-box').css("height", "calc(100% - " + height + "px)");
    $('.train-box.box2 .train-function-box').css("height", "calc(100% - " + height + "px)");
  },

  set_handle: function(box, pos){
    var slider_box = $('.train-box.box'+box+' .train-speed-slider');
    $('.slider-handle', slider_box).css("bottom", pos+"px");
    $('.slider-bar', slider_box).css("height", pos+"px");
  },

  set_speed: function(box, speed_ratio){
    if(speed_ratio != undefined){
      $('.train-box.box'+box+' .train-speed > span').text(Math.round(this.train[box].t.max_speed * speed_ratio));

      this.train[box].t.speed = Math.round(this.train[box].t.max_speed * speed_ratio);
    }
    else{
      this.send_speed(box);
    }
  },

  send_speed: function(box, resend){
    if(this.train[box].t.railtrainid != undefined){
      websocket.cts_TrainSpeed({train: this.train[box].t});
    }
    else if(this.train[box].type == 'E'){
      websocket.cts_DCCEngineSpeed({engine: this.train[box].t});
    }
    
    if(resend != undefined && resend){
      this.timer = setTimeout(this.send_speed.bind(this, box, true), 100);
    }
  },
  start_train_timer: function(box){
    this.timer = setTimeout(this.send_speed.bind(this, box, true), 100);
  },
  remove_train_timer: function(box){
    clearTimeout(this.timer);
    this.send_speed(box);
  },

  set_control: function(box, control){
    this.train[box].t.control = control;
    websocket.cts_TrainControl({train: this.train[box].t});
  },
  apply_control: function(box){
    $('.train-box.box'+box+' .train-info-box .control .btn-toggle').removeClass('btn-secondary');
    $('.train-box.box'+box+' .train-info-box .control .btn-toggle').addClass('btn-outline-secondary');

    $('.train-box.box'+box+' .train-info-box .control .btn-toggle[value='+this.train[box].t.control+']').removeClass('btn-outline-secondary');
    $('.train-box.box'+box+' .train-info-box .control .btn-toggle[value='+this.train[box].t.control+']').addClass('btn-secondary');
  },

  set_dir: function(box, dir){
    if(dir == "F"){
      this.train[box].t.dir = 1;
    }else{
      this.train[box].t.dir = 0;
    }

    websocket.cts_TrainSpeed({type: this.train[box].type, train: this.train[box].t});
  },
  apply_dir: function(box){
    $('.train-box.box'+box+' .train-dir-box .btn-toggle').removeClass('btn-secondary');
    $('.train-box.box'+box+' .train-dir-box .btn-toggle').addClass('btn-outline-secondary');
    if(this.train[box].t.dir == 0){
      $('.train-box.box'+box+' .train-dir-box .btn-toggle[value=R]').removeClass('btn-outline-secondary');
      $('.train-box.box'+box+' .train-dir-box .btn-toggle[value=R]').addClass('btn-secondary');
    }else if(this.train[box].t.dir == 1){
      $('.train-box.box'+box+' .train-dir-box .btn-toggle[value=F]').removeClass('btn-outline-secondary');
      $('.train-box.box'+box+' .train-dir-box .btn-toggle[value=F]').addClass('btn-secondary');
    }
  },
  set_stop: function(box){
    this.set_handle(box, 0);
    this.set_speed(box, 0);

    websocket.cts_TrainSpeed({type: this.train[box].type, train: this.train[box].t});
  },
  get_box: function(target){
    var box = $(target).closest('.train-box');
    if(box.hasClass("box1")){
      return 1;
    }
    else{
      return 2;
    }
  },
  select_train: function(box, type, id){
    var parent = $('.train-box.box'+box);

    var list;
    if(type == "E"){
      list = Train.engines;
    }
    else{
      list = Train.trains;
    }

    this.train[box] = {type: type, id: id, t: list[id]};

    
    this.init_train_functions(box);

    websocket.cts_TrainSubscribe(this.train);

    $('.header > div', parent).text(list[id].dcc.toString(10)+" - "+list[id].name);
    $('.train-img', parent).css('background-image', "url('./trains_img/"+list[id].img+"')");
    $('.train-info-box .description', parent).text(list[id].name);
    $('.train-info-box .route > span', parent).text(list[id].route);

    var slider_box = $('.train-box.box'+box+' .train-speed-slider')
    var pageY = slider_box.offset().top + slider_box.height();
    var ylim = slider_box.height() - $('.slider-handle', slider_box).height();
    var pos = ylim * list[id].speed / list[id].max_speed;

    $('.slider-handle', slider_box).css("bottom", pos+"px");
    $('.slider-bar', slider_box).css("height", pos+"px");

    $('.train-box.box'+box+' .train-speed > span').text(list[id].speed);    

    var slider_box = $('.train-box.box'+box+' .train-speed-slider');
    var pageY = slider_box.offset().top + slider_box.height();
    var ylim = slider_box.height() - $('.slider-handle', slider_box).height();

    var pos = ylim * (this.train[box].t.speed / 128);

    this.set_handle(box, pos);

  },

  link_request: function(data){
    websocket.cts_LinkTrain({fid: data.fid, real_id: data.id, type: data.type, msg_id: data.msg_id});
  },
  link: function(data){
    if(Modals.frame != undefined && Modals.frame.title == "Link Train"){
      Modals.hide();
    }

    if(data.type == 0){
      Train.trains[data.tid].ontrack = 1;
      Train.trains[data.tid].railtrainid = data.fid;

      for(var i = 0; i < Train.trains[data.tid].link.length; i++){
        if(Train.trains[data.tid].link[i][0] == 0){
          Train.engines[Train.trains[data.tid].link[i][1]].ontrack = 2;
        }
      }

      Train.railtrain[data.fid] = Train.trains[data.tid];
    }
    else{
      Train.engines[data.tid].ontrack = 1;
      Train.engines[data.tid].railtrainid = data.fid;
      
      Train.railtrain[data.fid] = Train.engines[data.tid];
    }

    // Messages.remove(data.msg_id, 1);

  }
}

var Train_Configurator = {
  clear: function(){
    $("#rollingstock .info-box.trains .box-container").empty();
    $("#rollingstock .info-box.engines .box-container").empty();
    $("#rollingstock .info-box.cars .box-container").empty();
  },
  update: function(){
    this.clear();

    // Update trains
    $("#rollingstock .info-box.trains .box-container").append("<div style='width:100%;'><ul class='list-unstyled'></ul></div>");
    // $('.modal-body > div > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
    for(var i = 0; i<Train.trains.length; i++){
      var t = Train.trains[i];

      var text = '<li class="list-unstyled" name="'+i+'">'+
          '<div class="mr-3 ml-3 mt-3 bg-outline-dark" style="width:calc(100% - 2rem);height:36px; padding: 3px; overflow: hidden"><div style="width:100%; height:150%; overflow-x: overlay;white-space: nowrap">';

      for(var j = 0; j < t.link.length; j++){
        if(t.link[j][0] == 0 && Train.engines[t.link[j][1]] != undefined){
          text += '<img src="./trains_img/'+Train.engines[t.link[j][1]].icon+'" style="height:30px"/>';
        }else if(Train.cars[t.link[j][1]] != undefined){
          text += '<img src="./trains_img/'+Train.cars[t.link[j][1]].icon+'" style="height:30px"/>';
        }
      }

      text += '</div></div><div class="m-2" style="width:calc(100% - 2rem)">'+
            '<button name="train_id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1 mr-3"\
              value="'+i+'" style="display: inline-block;">Update</button>'+
            '<div class="mt-0 mb-0" style="display: inline-block"><b>'+t.name+'</b></div>'+
            '<div class="" style="display: inline-block; float:right; max-width: 210px;font-size:small; width:100%;">\
              <i>DCC: '+t.dcc.join(", ")+'</i>\
              <i style="float:right"><span class="d-none d-md-block">Length: </span>'+t.length+' mm</i>\
            </div>'+
          '</div></li>';
      $("#rollingstock .info-box.trains .box-container ul").append(text);
      // $('.modal-body .cont ul', ref).append("<button name='train_id' class='modal-form btn btn-toggle btn-outline-primary m-1' value='"+i+"' style='display: block;'>"+t.name+'</button><br/>PIZZA<br/>');
    }
    $("#rollingstock .info-box.trains .box-container button").on("click", function(evt){
      var id = parseInt($(evt.target).parent().parent().attr("name"));

      Modals.open("trains.edit", {id: id});
    });

    //Update engines
    $("#rollingstock .info-box.engines .box-container button").off("click");
    $("#rollingstock .info-box.engines .box-container").append("<div style='width:100%;'><ul class='list-unstyled'></ul></div>");
    for(var i = 0; i<Train.engines.length; i++){
      var e = Train.engines[i];

      var text = '<li class="message media" name="'+i+'">'+
              '<div class="message-mbox align-self-center mr-3 bg-primary" style="width:120px;height:60px;position:relative;">\
                <div class="train-img" style="background-image: url(\'./trains_img/'+e.img+'\')"></div>\
              </div>'+
              '<div class="message-body media-body">'+
                '<div class="mt-0 mb-0 message-header"><b>'+e.name+'</b></div>'+
                '<div class="message-content" style="max-width: 210px;font-size:small; margin-right:0.5em">\
                  <i>DCC: '+e.dcc+'</i>\
                  <i style="float:right"><span class="d-none d-md-block">Length: </span>'+e.length+' mm</i>\
                </div>'+
                '<button name="train_id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1"\
                  value="'+i+'" style="display: block;">Update</button>'+
              '</div></li>';

      $("#rollingstock .info-box.engines .box-container ul").append(text);
    }
    $("#rollingstock .info-box.engines .box-container button").on("click", function(evt){
      var id = parseInt($(evt.target).parent().parent().attr("name"));

      Modals.open("engines.edit", {id: id});
    });

    //Update cars
    $("#rollingstock .info-box.cars .box-container").append("<div style='width:100%;'><ul class='list-unstyled'></ul></div>");
    for(var i = 0; i<Train.cars.length; i++){
      var c = Train.cars[i];

      var text = '<li class="message media" name="'+i+'">'+
              '<div class="message-mbox align-self-center mr-3" style="width:120px;height:60px;position:relative;">\
                <div class="car-icon" style="background-image: url(\'./trains_img/'+c.icon+'\')"></div>\
              </div>'+
              '<div class="message-body media-body">'+
                '<div class="mt-0 mb-0 message-header"><b>'+c.name+'</b></div>'+
                '<div class="message-content" style="max-width: 210px;font-size:small; margin-right:0.5em">\
                  <i>Nr: '+c.nr+'</i>\
                  <i style="float:right"><span class="d-none d-md-block">Length: </span>'+c.length+' mm</i>\
                </div>'+
                '<button name="train_id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1"\
                  value="'+i+'" style="display: block;">Update</button>'+
              '</div></li>';

      $("#rollingstock .info-box.cars .box-container ul").append(text);
    }
    $("#rollingstock .info-box.cars .box-container button").on("click", function(evt){
      var id = parseInt($(evt.target).parent().parent().attr("name"));

      Modals.open("cars.edit", {id: id});
    });
  },
  show: function(){
    $("#rollingstock .info-box.trains .box-container").show();
    $("#rollingstock .info-box.engines .box-container").show();
    $("#rollingstock .info-box.cars .box-container").show();
  },
  hide: function(){
    $("#rollingstock .info-box.trains .box-container").hide();
    $("#rollingstock .info-box.engines .box-container").hide();
    $("#rollingstock .info-box.cars .box-container").hide();
  }
}

events.add_init(Train_Control.init.bind(Train_Control));
events.add_resize(Train_Control.resize.bind(Train_Control));