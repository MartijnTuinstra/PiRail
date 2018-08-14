var Train = {
  trains: [], //Train compositions
  engines: [],
  cars: [],

  selected: [],
  
  select: function(id){
    var n = false;
    if(this.selected[0] == id || this.selected[1] == id){
      n = true;
    }
    console.log("Select train "+id);
    $('#train_list .train_list_box.active').toggleClass("active");
    if(Menu.window_type == 0 || Menu.window_type == 3){
      this.selected = [id];
    }
    else{
      if(! this.selected.includes(id)){
        if(this.selected.length == 2){
          this.selected = [this.selected.pop()];
        }
        this.selected.push(id);
      }
      else{
        var tmp = this.selected[0];
        this.selected[0] = this.selected[1];
        this.selected[1] = tmp;
      }
      console.log(this.selected);
    }
    $.each(this.selected,function(i,v){
      $('#train_list #Tr'+v+'.train_list_box').toggleClass("active");
    });

    if(Menu.window_type != 3){
      if(this.selected[0] != undefined){
        var train = this.trains[this.selected[0]];
        $('#trains .controlA .name').text(train.name);
        $('#trains .controlA .specs .img').css('background-image',"url('./trains_img/"+train.img+"'");
        $('#trains .controlA .control .type').text(((train.control & 0xC0) == 0)?"Manual":(((train.control & 0xC0) == 0x40)?"Semi-auto":"Full-auto"));
      }

      if(this.selected[1] != undefined){
        var train = this.trains[this.selected[1]];
        $('#trains .controlB .name').text(train.name);
        $('#trains .controlB .specs .img').css('background-image',"url('./trains_img/"+train.img+"'");
        $('#trains .controlB .control .type').text(((train.control & 0xC0) == 0)?"Manual":(((train.control & 0xC0) == 0x40)?"Semi-auto":"Full-auto"));
      }

      this.update();
    }
    else{
      if($('#train_hover').css("display") == "none"){
        $('body').on("mousedown", Train.hide);
        $('#train_hover').css("display","block")
      }
      else if(n){
        $('#train_hover').css("display","none");
        $('body').off("mousedown", Train.hide);
        return;
      }

      pos = $('#train_list #Tr'+this.selected[0]+'.train_list_box').position()

      $('#train_hover').css("left",(pos.left-20)+"px"); //272 - 252
      $('#train_hover').css("bottom","120px")

      var train = this.trains[this.selected[0]];

      $('#train_hover .specs .img').css('background-image',"url('./../trains/"+train.img+"'");
      $('#train_hover .specs .name').text(train.name);
      $('#train_hover .control .type').text(((train.control & 0xC0) == 0)?"M":(((train.control & 0xC0) == 0x40)?"SA":"FA"));
      $('#train_hover .control .route').text((train.route.S >= 0)?(train.route.S + ":" + train.route.P):"No Route");

      this.update();
    }
  },
  init: function(){
    $('.train_list_box').off("click");
    $('#train_list .scroller').empty();

    this.comp.init();
    this.linker.init();
    this.engine_car_creator.init();

    this.update_list();
  },
  resize: function(){
    $('#train_compositor .selector').css("max-height",(window.innerHeight - 80 - 38 - 106 - 38 - 40)+"px");
  },
  setRoute: function(dcc,from,to){
    console.warn("Implement: Train set route:",dcc,from,to);
  },
  route: function(tid,DestM,DestS){
    console.warn("Implement: Train "+tid+" has a route to "+DestM+"-"+DestS);
  },
  import: function(data){
    console.warn("Implement")
  },

  comp:{
    index: 0,
    list: [],
    init: function(){
      $('#train_compositor .selector').hide();

      $('#train_compositor .btn.add_cars').on("click",this.toggleCar);
      $('#train_compositor .btn.add_engines').on("click",this.toggleEngines);
      $('#train_compositor .btn.save').on("click",this.save);
      $('#train_compositor .btn.close').on("click",this.close);

      this.update_list();

      $('#train_compositor .compositor .scroller > div').empty();
    },

    update_list: function(){
      $('#train_compositor .engines.selector, #train_compositor .cars.selector').empty();
      $.each(Train.engines, function(i,v){
        text =  '<div index="'+i+'" class="list_box">';
        text += '<div  class="img" style="background-image: url(\'./../trains/'+v.img+'\');"></div>'
        text += '<div class="name">'+v.name+'</div>';
        text += '<div class="id">'+v.dcc+'</div>';
        text += '</div>';

        $('#train_compositor .engines.selector').append(text);
      });

      $.each(Train.cars, function(i,v){
        text =  '<div index="'+i+'" class="list_box">';
        text += '<div  class="img" style="background-image: url(\'./../trains/'+v.img+'\');"></div>'
        text += '<div class="name">'+v.name+'</div>';
        text += '<div class="id">'+v.nr+'</div>';
        text += '</div>';

        $('#train_compositor .cars.selector').append(text);
      });

      $('#train_compositor .cars .list_box, #train_compositor .engines .list_box').on("click",this.add);
    },

    toggleCar: function(){
      if($('#train_compositor .cars.selector').css("display") == "none"){
        $('#train_compositor .cars.selector').show();
        $('#train_compositor .engines.selector').hide();
      }
      else{
        $('#train_compositor .cars.selector').hide();
      }
    },
    toggleEngines: function(){
      if($('#train_compositor .engines.selector').css("display") == "none"){
        $('#train_compositor .engines.selector').show();
        $('#train_compositor .cars.selector').hide();
      }
      else{
        $('#train_compositor .engines.selector').hide();
      }
    },
    add: function(evt){
      var index = parseInt($(evt.currentTarget).attr("index"));
      var text = "";
      if($(evt.currentTarget).parent().hasClass("engines")){
        console.log("Add engine");
        text = '<div class="box new b'+Train.comp.index+'">' +
             '<img src="./../trains/'+Train.engines[index].icon+'"/>' +
             '<div class="name">'+Train.engines[index].name+' - '+Train.engines[index].dcc+'</div>' +
             '<div class="control">' +
             '<div class="cbtn l">&lt;</div>' +
             '<div class="cbtn b">b</div>' +
             '<div class="cbtn r">&gt;</div>' +
             '</div></div>';
        Train.comp.list.push({t:"E",i:Train.comp.index});
      }
      else{
        console.log("Add car");
        text = '<div class="box new b'+Train.comp.index+'">' +
             '<img src="./../trains/'+Train.cars[index].icon+'"/>' +
             '<div class="name">'+Train.cars[index].name+'</div>' +
             '<div class="control">' +
             '<div class="cbtn l">&lt;</div>' +
             '<div class="cbtn b">b</div>' +
             '<div class="cbtn r">&gt;</div>' +
             '</div></div>';
        Train.comp.list.push({t:"C",i:Train.comp.index});
      }
      Train.comp.index++;
      $('#train_compositor .compositor .scroller > div').append(text);

      var width_sum = 0;
      $.each($('#train_compositor .compositor img'),function(){
        if($(this).width() < 20){
          width_sum += 200
        }
        width_sum += $(this).width()
        console.log($(this).width())
        width_sum += 10
      });

      $('#train_compositor .compositor .scroller > div').css("width",width_sum + "px");


      $('#train_compositor .compositor .box.new .cbtn.b').on("click",Train.comp.del);


      $('#train_compositor .compositor .box.new').toggleClass("new");
    },
    del: function(evt){
      Train.comp.list[parseInt($(evt.currentTarget).parent().parent()[0].classList[1].slice(1))] = undefined;
      $('.cbtn',$(evt.currentTarget).parent().parent()).off();
      $(evt.currentTarget).parent().parent().remove();
    },
    open: function(){
      $('#train_compositor').show();
      Train.comp.list = [];
      $('#train_compositor .compositor .scroller > div').empty();
    },
    save: function(evt){
      console.warn("Implement Train compositor save");
      $('#train_compositor').hide();
    },
    close: function(evt){
      $('#train_compositor').hide();
    }
  },

  linker:{
    message: undefined,

    init: function(){
      $('#train_linker .selected .container').on("click",function(){
        $('#train_linker .selected .selector').toggleClass("active");
      });

      this.update_list();

      $('#train_linker .btn.select').on("click", this.done);
      $('#train_linker .btn.close').on("click", this.close);
      $('#train_linker .btn.add_cars').on("click", Train.comp.open);
    },

    update_list: function(){

      $('#train_linker .selected .selector').empty();

      Train.trains.forEach(function(e,i){
        text = '<div train="'+i+'">' +
             '<div class="imgbox"><div class="scroller"><div style="white-space: nowrap;">';

        e.link.forEach(function(e2,i2){
          if((String.fromCharCode(e2[0]) == "E" || String.fromCharCode(e2[0]) == "e") && Train.engines[e2[1]] != undefined){
            text += '<img src="./../trains/'+Train.engines[e2[1]].icon+'"/>';
          }else if(Train.cars[e2[1]] != undefined){
            text += '<img src="./../trains/'+Train.cars[e2[1]].icon+'"/>';
          }
        })
            
        text += '</div></div></div>'+
            '<div class="textbox"><span class="name">'+e.name+'</span><span class="dcc">'+e.dcc+'</span></div>'+
            '</div>'

        $('#train_linker .selected .selector').append(text);
      });

      Train.engines.forEach(function(e,i){
        text = '<div engine="'+i+'">' +
          '<div class="imgbox"><div class="scroller"><div style="white-space: nowrap;">'+
            '<img src="./../trains/'+e.icon+'"/>'+
          '</div></div></div>'+
          '<div class="textbox"><span class="name">&nbsp;&nbsp;&nbsp;'+e.name+'</span><span class="dcc">'+e.dcc+'</span></div>'+
        '</div>'

        $('#train_linker .selected .selector').append(text);
      });

      $('#train_linker .selected .selector > div').on("click", this.select);

    },

    select: function(evt){
      train = $(evt.currentTarget).clone();
      $('#train_linker .selected .container').off();
      $('#train_linker .selected .container').remove();
      $('#train_linker .selected').prepend(train);
      $('#train_linker .selected > div:first-child').toggleClass("container");
      $('#train_linker .selected .selector').toggleClass("active");

      $('#train_linker .selected .container').on("click",function(){
        $('#train_linker .selected .selector').toggleClass("active");
      });
    },

    done: function(){
      var trainID = parseInt($('#train_linker .selected .container').attr("train"));
      var engineID = parseInt($('#train_linker .selected .container').attr("engine"));
      console.log("Train_link "+Train.linker.followID);
      if(isNaN(trainID) && !isNaN(engineID)){
        websocket.cts_link_train(Train.linker.followID, trainID, "E", Train.linker.message);
      }
      else if(!isNaN(trainID) && isNaN(engineID)){
        websocket.cts_link_train(Train.linker.followID, engineID, "T", Train.linker.message);
      }
      Train.linker.close();
    },

    open: function(msg){
      this.message = msg.id;
      this.followID = msg.data[0];
      $('#train_linker .header').html("Train "+msg.data[0]+" on "+msg.data[1] + ":" + msg.data[2]);

      $('#train_linker .selected .container').empty();
      $('#train_linker').show();
    },

    close: function(){
      console.log("CLOSE linker")
      $('#train_linker').hide();
    }
  },

  engine_car_creator:{
    init: function(){
      $('#engine_car_creator .btn.save').on("click", this.done);
      $('#engine_car_creator .btn.save_new').on("click", this.done_n);
      $('#engine_car_creator .btn.close').on("click", this.close);
      $('#engine_car_creator .speed_steps_box .add_box').on("click", this.create_speed_step);
    },

    upload: function(field){
      var file = $('input'+field, '#engine_car_creator').get(0).files[0];
      var formdata = new FormData();
      formdata.append("file1", file);

      if(file.name.split(".")[file.name.split(".").length-1] != "jpg" && 
        file.name.split(".")[file.name.split(".").length-1] != "jpeg" && 
        file.name.split(".")[file.name.split(".").length-1] != "png"){
          alert("Select a png or jpeg/jpg file");
          var C = $('#engine_car_creator');
          $('.field_container input'+field, C).val("");
          return;
      }

      if (field == ".icon"){
        formdata.append("name", "tmp_icon");
      }
      else{
        formdata.append("name", "tmp_img");
      }
      var ajax = new XMLHttpRequest();
      ajax.addEventListener("load", function(){
        console.log(this.responseText);
        var time = (new Date).getTime()
        var extention = file.name.split(".")[file.name.split(".").length-1]
        if (field == ".icon"){
          $('#engine_car_creator .img_icon').css("background-image", ('url(./tmp_icon.'+extention+'?q='+time+')'));
        }
        else{
          $('#engine_car_creator .img_train').attr("src","./tmp_img."+extention+"?q="+time);
        }
      }, false);
      ajax.addEventListener("error", function(){alert("error");console.warn(this.responseText);}, false);
      ajax.open("POST", "./img_upload.php");
      ajax.send(formdata);
    },

    create_speed_step: function(evt){
      $('#engine_car_creator .speed_steps_box .add_box').before('<div class="box"><input type="text" name="speed" placeholder="speed" style="text-align:center;"/><input type="text" name="step"  placeholder="step" style="text-align:center;"/></div>');
      $('#engine_car_creator .speed_steps_box .scroller > div').width($('#engine_car_creator .speed_steps_box .scroller > div').width() + 95);
    },

    add_speed_step: function(speed, step){
      $('#engine_car_creator .speed_steps_box .add_box').before('<div class="box"><input type="text" name="speed" placeholder="speed" style="text-align:center;"/><input type="text" name="step"  placeholder="step" style="text-align:center;"/></div>');
      $('#engine_car_creator .speed_steps_box .scroller > div').width($('#engine_car_creator .speed_steps_box .scroller > div').width() + 95);
      $('input[name="speed"]', '#engine_car_creator .speed_steps_box .box:last').val(speed);
      $('input[name="step"]', '#engine_car_creator .speed_steps_box .box:last').val(step);
    },

    done: function(close = true){
      data = {}

      data['name'] = $('#engine_car_creator .name input').val();
      data['dcc'] = $('#engine_car_creator .dcc input').val();
      data['nr'] = $('#engine_car_creator .nr input').val();
      data['length'] = $('#engine_car_creator .length input').val();
      data['type'] = $('#engine_car_creator .type input:checked').val();

      listA = $('#engine_car_creator .speed_steps_box input[name="speed"]');
      listB = $('#engine_car_creator .speed_steps_box input[name="step"]');

      data['speed_steps'] = {};

      for (var i = listA.length - 1; i >= 0; i--) {
        data['speed_steps'][listB[i].value] = listA[i].value;
      }

      console.log(data);
      if(close && close.target == undefined){
        this.close();
      }
    },

    done_n: function(){ // Done and new
      this.done(false);
      this.clear();
    },

    close: function(){
      $('#engine_car_creator').hide();
    },

    clear: function(){
      var C = $('#engine_car_creator');
      $('.field_container input', C).val("");
      $('.field_container input:checked', C).prop("checked", false);

      $('.img_icon', C).css("background-image", "");
      $('.img_train', C).attr("src", "");

      $('#engine_car_creator .speed_steps_box .box:not(:first)').remove();
      $('#engine_car_creator .speed_steps_box .box input').val("");
    },

    open: function(type){
      this.clear();

      $('#engine_car_creator').show();

      if(type == "E"){
        $('#engine_car_creator .field_container.nr').hide();
        $('#engine_car_creator .field_container.dcc').show();

        $('#engine_car_creator .field_container.speed_steps').show();
        $('#engine_car_creator .field_container.functions').css("width", "calc(25% - 20px)");

        $('#engine_car_creator .field_container.speed input[name="speed"]').css("width","calc(50% - 20px)");
        $('#engine_car_creator .field_container.speed input[name="steps"]').show();
      }
      else{
        $('#engine_car_creator .field_container.nr').show();
        $('#engine_car_creator .field_container.dcc').hide();

        $('#engine_car_creator .field_container.speed_steps').hide();
        $('#engine_car_creator .field_container.functions').css("width", "calc(50% - 20px)");

        $('#engine_car_creator .field_container.speed input[name="speed"]').css("width","calc(100% - 10px)");
        $('#engine_car_creator .field_container.speed input[name="steps"]').hide();
      }
    }
  },

  // System Events
  update_list: function(){
    $('#train_list .train_list_box').off("click");
    $('#train_list .slider_containter').off("mousedown touchstart");

    var notFirst;

    Train.trains.forEach(function(e,i){
      if(Train.trains[i].dcc == "" || Train.trains[i].dcc == undefined){
        Train.trains[i].dcc = "";
        notFirst = false;
        e.link.forEach(function(e2,i2){
          if(String.fromCharCode(e2[0]) == "E" || String.fromCharCode(e2[0]) == 'e'){
            if(notFirst == true){
              Train.trains[i].dcc += ", "
            }
            if(e.img == "" || e.img == undefined){
              Train.trains[i].img = Train.engines[ e2[1] ].img;
            }
            notFirst = true;
            Train.trains[i].dcc += Train.engines[ e2[1] ].dcc;
          }
        });
      }
    });

    var content = "";
    var width = 0;

    $.each(this.trains,function(i,v){
      if(v.use == false){
        return true;
      }
      content += '<div id="Tr'+i+'" class="train_list_box">' +
            '<div  class="img" style="background-image: url(\'./../trains/'+v.img+'\');"></div>' +
            '<span class="id">'+v.dcc+'</span>' +
            '<span class="name">'+v.name+'</span>' +
             '</div>';
      width += 252;
    });

    content = '<div style="width:'+width+'px;height: 10px">' + content + '</div>';

    $('#train_list .scroller').html(content);

    $('.train_list_box').on("click",function(evt){
      Train.select(parseInt(evt.currentTarget.id.slice(2)));
    });

    $('.slider_containter').on("mousedown touchstart",function(evt){
      Train.slider(evt);
      $('.slider_containter').on("mousemove touchmove",Train.slider);
      $('.slider_containter').on("mouseup touchend",function(evt){
        Train.slider(evt);
        $('.slider_containter').off("mousemove mouseup touchmove touchend");
      });
    });
  },

  update: function(data){
    if(Menu.window_type != 3){
      var train_data = this.trains[this.selected[0]];
      if(train_data != undefined){
        $('#trains .controlA .slider').css('height',(train_data.speed/train_data.max_speed)*100+"%");
        $('#trains .controlA .slider_value').text(Math.round(train_data.speed));

        if(((train_data.speed/train_data.max_speed) < 0.1 && !$('#trains .controlA .slider_value').hasClass("inverse")) || (train_data.speed/train_data.max_speed) >= 0.1 && $('#trains .controlA .slider_value').hasClass("inverse")){
          $('#trains .controlA .slider_value').toggleClass("inverse")
        }
      }


      var train_data = this.trains[this.selected[1]];
      if(train_data != undefined){
        $('#trains .controlB .slider').css('height',(train_data.speed/train_data.max_speed)*100+"%");
        $('#trains .controlB .slider_value').text(Math.round(train_data.speed));

        if(((train_data.speed/train_data.max_speed) < 0.1 && !$('#trains .controlB .slider_value').hasClass("inverse")) || (train_data.speed/train_data.max_speed) >= 0.1 && $('#trains .controlB .slider_value').hasClass("inverse")){
          $('#trains .controlB .slider_value').toggleClass("inverse")
        }
      }
    }
    else{
      var train_data = this.trains[this.selected[0]]
      $('#train_hover .slider').css('height',(train_data.speed/train_data.max_speed)*100+"%");
      $('#train_hover .slider_value').text(Math.round(train_data.speed));

      if(((train_data.speed/train_data.max_speed) < 0.1 && !$('#train_hover .slider_value').hasClass("inverse")) || (train_data.speed/train_data.max_speed) >= 0.1 && $('#train_hover .slider_value').hasClass("inverse")){
        $('#train_hover .slider_value').toggleClass("inverse")
      }
    }
  },

  // User Events
  slider: function(evt){
    var clickY;
    console.log(evt)
    if(evt.type == "touchmove" || evt.type == "mousemove" || evt.type == "touchstart" || evt.type == "mousedown"){
      if(evt.clientY == undefined){
        clickY = $(evt.currentTarget).offset().top - evt.touches[0].clientY
        evt.preventDefault();
        evt.stopPropagation();
      }
      else{
        clickY = $(evt.currentTarget).offset().top - evt.clientY
        evt.preventDefault();
        evt.stopPropagation();
      }
      var procent = ((clickY + $(evt.currentTarget).height())/$(evt.currentTarget).height())
      procent = (procent < 0)?0:((procent > 1)?1:procent);
      if($(evt.currentTarget).hasClass("sliderB")){
        Train.trains[Train.selected[1]].speed = procent*Train.trains[Train.selected[1]].max_speed;
      }
      else{
        Train.trains[Train.selected[0]].speed = procent*Train.trains[Train.selected[0]].max_speed;
      }
    }
    if($(evt.currentTarget).hasClass("sliderB")){
      if(evt.type != "touchmove" && evt.type != "mousemove"){
        websocket.cts_train_speed(Train.trains[Train.selected[1]].id, 0, Math.round((Train.trains[Train.selected[1]].speed/Train.trains[Train.selected[1]].max_speed)*127));
      }
    }
    else{
      if(evt.type != "touchmove" && evt.type != "mousemove"){
        websocket.cts_train_speed(Train.trains[Train.selected[0]].id, 0, Math.round((Train.trains[Train.selected[0]].speed/Train.trains[Train.selected[0]].max_speed)*127));
      }
    }
    Train.update();

    return false;
  },
  hide: function(event){
    if(!$(event.target).closest('#train_hover').length && !$(event.target).hasClass("train_list_box") && !$(event.target.parentNode).hasClass("train_list_box")) {
      if($('#train_hover').is(":visible")) {
        $('#train_hover').hide();
        $('body').off("mousedown", Train.hide);
      }
    }
  },
}

