var Train = {
  trains: [], //Train compositions
  engines: [],
  cars: [],
  cat: {},
}

var Train_Control = {
  init: function(){
    this.resize();

    this.train = [undefined, undefined, undefined];
    this.timer = 0;

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

          // websocket.cts_train_speed(this.train[box].type, this.train[box].t);

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

    $('.train-box .btn-toggle').on("click", function(evt){
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
  resize: function(){
    if(window.innerWidth < 576){
      $('.train-box.box2').hide();
    }
    else{
      $('.train-box.box2').show();
    }
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
    websocket.cts_train_speed(this.train[box].type, this.train[box].t);
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

  set_dir: function(box, dir){
    if(dir == "F"){
      this.train[box].t.dir = 1;
    }else{
      this.train[box].t.dir = 0;
    }

    websocket.cts_train_speed(this.train[box].type, this.train[box].t);
  },
  set_stop: function(box){
    this.set_handle(box, 0);
    this.set_speed(box, 0);

    websocket.cts_train_speed(this.train[box].type, this.train[box].t);
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

    this.train[box] = {type: type, t: list[id]};

    $('.header > div', parent).text(list[id].name);
    $('.train-img', parent).css('background-image', "url('./trains_img/"+list[id].img+"')");
    $('.train-info-box .description', parent).text(list[id].description);
    $('.train-info-box .route > span', parent).text(list[id].route);

    var slider_box = $('.train-box.box'+box+' .train-speed-slider')
    var pageY = slider_box.offset().top + slider_box.height();
    var ylim = slider_box.height() - $('.slider-handle', slider_box).height();
    var pos = ylim * list[id].speed / list[id].max_speed;

    $('.slider-handle', slider_box).css("bottom", pos+"px");
    $('.slider-bar', slider_box).css("height", pos+"px");

    $('.train-box.box'+box+' .train-speed > span').text(list[id].speed);    

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