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

init_list.push(Train_Control.init.bind(Train_Control));
resize_list.push(Train_Control.resize.bind(Train_Control));