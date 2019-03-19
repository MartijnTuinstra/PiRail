var Modals = {
  setups: {
    "hard.reset":{
      link: "button.btn-hard-reset",
      title: "Are You Sure?",
      content: "The system will be down for a couple minutes",
      buttons: {
        success: {visible: true, content: "Cancel", cb: undefined, wait: false},
        warning: {visible: false, content: ""},
        danger: {visible: true, content: "Yes, Restart", cb: function(){websocket.cts_restart()}, wait: false},
      }
    },
    "Z21.settings":{
      link: "button.btn-Z21-settings",
      title: "Z21 Settings",
      content: '<div> \
          IP-address<br/> \
          <input type="text" size="3" maxlength="3" class="ip-input modal-form" name="ip1" placeholder="127">.\
          <input type="text" size="3" maxlength="3" class="ip-input modal-form" name="ip2" placeholder="0">.\
          <input type="text" size="3" maxlength="3" class="ip-input modal-form" name="ip3" placeholder="0">.\
          <input type="text" size="3" maxlength="3" class="ip-input modal-form" name="ip4" placeholder="1">\
        </div><br/>\
        Updating will result in subsystem restarting',
      buttons: {
        success: {visible: false, content: ""},
        warning: {visible: true, content: "Update", cb: function(ip){websocket.cts_z21_settings(ip)}, wait: false},
        danger: {visible: true, content: "Cancel", cb: undefined, wait: false},
      }
    },
    "train.link":{
      open_cb: function(data, ref_obj){
        $('.imgbox', ref_obj).append(data.fid);
      },
      title: "Link Train",
      content: '<div> \
          <div class="selected">\
            <div class="container">\
              <div data="10">\
                <div class="imgbox"></div>\
              </div>\
            </div>\
            <div class="selector"></div>\
          </div>\
        </div>',
      buttons: {
        success: {visible: true, content: "Link", cb: function(data){/*websocket.cts_link_train(fid, rid, type, mid)*/}, wait: false},
        warning: {visible: false, content: ""},
        danger: {visible: true, content: "Cancel", cb: undefined, wait: false},
      }
    },

    "trains.edit":{
      link: "button.btn-trains-new",
      open_cb: function(data, ref){

        if(data.id == undefined){
          $('button.btn-warning', ref).hide();
        }
        else{
          $('button.btn-success', ref).hide();
        }

        if(Train.trains[data.id] != undefined){
          var t = Train.trains[data.id];

          for(var j = 0; j < t.link.length; j++){
            if(t.link[j][0] == 0 && Train.engines[t.link[j][1]] != undefined){
              $('.modal-body .comp_box', ref).append('<img src="./trains_img/'+Train.engines[t.link[j][1]].icon+
                                   '" type="E" id="'+t.link[j][1]+'" style="height:30px"/>');
            }else if(Train.cars[t.link[j][1]] != undefined){
              $('.modal-body .comp_box', ref).append('<img src="./trains_img/'+Train.cars[t.link[j][1]].icon+
                                   '" type="C" id="'+t.link[j][1]+'" style="height:30px"/>');
            }
          }

          $('input[name=name]', ref).val(t.name);
        }

        Sortable.create($('.comp_box', ref)[0], {dragable: "img"});

        $('button', $('.btn-toggle-group', ref).first()).on("click", function(evt){
          $('.train-settings', ref).hide();
          $('.engine', ref).hide();
          $('.car', ref).hide();

          if(evt.currentTarget.innerText == "Add Engine"){
            $('.engine', ref).show();
          }
          else if(evt.currentTarget.innerText == "Settings"){
            $('.train-settings', ref).show();
          }
          else if(evt.currentTarget.innerText == "Add Car"){
            $('.car', ref).show();
          }
        });

        for(var i = 0; i < Train.engines.length; i++){
          var t = Train.engines[i];

          var text = "<div id='"+i+"' type='E' class='engine-add col-6 col-md-4 mb-1'>\
                <div style='border: 1px solid #ddd;'>\
                <img style='width:40px' src='./trains_img/"+t.icon+"'/>\
                <div style='display: inline-block'><b>"+t.name+"</b><br/>"+
                "<i>"+t.dcc+"</i></div></div></div>"

          $('.engine', ref).append(text);
        }

        for(var i = 0; i < Train.cars.length; i++){
          var t = Train.cars[i];

          var text = "<div id='"+i+"' type='C' class='car-add col-6 col-md-4 mb-1'>\
                <div style='border: 1px solid #ddd;'>\
                <img style='width:40px' src='./trains_img/"+t.icon+"'/>\
                <div style='display: inline-block'><b>"+t.name+"</b><br/>"+
                "<i>"+t.nr+"</i></div></div></div>"

          $('.car', ref).append(text);
        }

        $('.engine-add, .car-add', ref).on("click", function(evt){
          var i = evt.currentTarget.getAttribute('id');
          var t = evt.currentTarget.getAttribute('type');
          var s = $('img', evt.currentTarget).attr("src");
          $('.modal-body .comp_box', ref).append('<img src="'+s+'" id="'+i+'" type="'+t+'" style="height:30px"/>')
        });

        var keys = Object.keys(Train.cat);
        var enter = false;
        for(var i = 0; i < keys.length; i++){
          if(keys[i] >= 128 && !enter){
            enter = true;
            $('.train-settings .btn-toggle-group', ref).append("<br/>");
          }
          $('.train-settings .btn-toggle-group', ref).append('<button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary" value="'+keys[i]+'">'+Train.cat[ keys[i] ]+'</button>');
        }
        
      },
      close_cb: function(data, ref){
        $('button', $('.btn-toggle-group', ref).first()).off();
      },
      title: "Train",
      content: '<div class="m-1" style="border: 1px solid #eee; width:calc(100% - 0.5rem);height:56px;\
                padding: 3px; overflow: hidden">\
                <div class="comp_box" style="width:100%; height:150%; overflow-x: scroll;white-space: nowrap"></div></div>\
              <div class="row mb-2 btn-toggle-group" style="text-align: center; border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
              <div class="col"><button class="btn btn-toggle btn-outline-primary btn-xs">Add Engine</button></div>\
              <div class="col"><button class="btn btn-toggle btn-primary btn-xs">Settings</button></div>\
              <div class="col"><button class="btn btn-toggle btn-outline-primary btn-xs">Add Car</button></div>\
              </div>\
              <div class="train-settings" style="display: block">\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
              <div class="col-4 control-label"><span style="line-height: 38px;vertical-align:middle">Name</span></div>\
              <div class="col-8"><input name="name" class="modal-form form-control input-sm"></div>\
              </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
              <div class="col-4 control-label"><span style="line-height: 48px;vertical-align:middle">Type</span></div>\
                <div class="col-8" style="height: 3.2rem; overflow: hidden">\
                  <div class="btn-toggle-group" style="overflow-x:scroll;height:150%;white-space: nowrap;min-width:100%"></div>\
                </div>\
                </div>\
              </div>\
            </div>\
            <div class="row engine" style="display:none; max-height: 300px; overflow-y: scroll; font-size: 0.8em">\
            </div>\
            <div class="row car" style="display:none; max-height: 300px; overflow-y: scroll; font-size: 0.8em">\
            </div>',
      buttons: {
        success: {visible: true, content: "Create", cb: function(data){/*websocket.cts_link_train(fid, rid, type, mid)*/}, wait: false},
        warning: {visible: true, content: "Update", cb: function(){alert("Update");}, wait: false},
        danger: {visible: true, content: "Cancel", cb: undefined, wait: false},
      }
    },

    "engines.edit":{
      link: "button.btn-engines-new",
      chart: undefined,
      open_cb: function(data, ref){
        console.log("engines.edit open_cb: ", data);
        if(data.id == undefined){
          $('button.btn-warning', ref).hide();
        }
        else{
          $('button.btn-success', ref).hide();
        }

        Modals.setups["engines.edit"].speedsteps = [];

        //Init catagories buttons
        var keys = Object.keys(Train.cat);
        var enter = false;
        for(var i = 0; i < keys.length; i++){
          if(keys[i] >= 128 && !enter){
            enter = true;
            $('.train-categories.btn-toggle-group', ref).append("<br/>");
          }
          $('.train-categories.btn-toggle-group', ref).append('<button name="type" type="number" class="modal-form btn-toggle btn btn-xs btn-outline-primary" value="'+keys[i]+'">'+Train.cat[ keys[i] ]+'</button>');
        }

        if(data.id != undefined){
          $('input[name=name]', ref).val(Train.engines[parseInt(data.id)].name);
          $('input[name=dcc]', ref).val(Train.engines[parseInt(data.id)].dcc);
          $('input[name=length]', ref).val(Train.engines[parseInt(data.id)].length);
          
          $('input[name=icon_name]', ref).val(Train.engines[parseInt(data.id)].icon);
          $('#modal .train-icon').css("background-image", ('url("./trains_img/'+Train.engines[parseInt(data.id)].icon+'")'));
          $('input[name=image_name]', ref).val(Train.engines[parseInt(data.id)].img);
          $('#modal .train-img').css("background-image", ('url("./trains_img/'+Train.engines[parseInt(data.id)].img+'")'));

          Modals.setups["engines.edit"].speedsteps = Train.engines[parseInt(data.id)].steps;

          $('button[name=speedstep][value='+Train.engines[parseInt(data.id)].speedstep+']').removeClass("btn-outline-primary").addClass("btn-primary");
          $('button[name=type][value='+Train.engines[parseInt(data.id)].type+']').removeClass("btn-outline-primary").addClass("btn-primary");
        }

        $('input[type=file]', ref).on("change", function(evt){
          var input = $(evt.currentTarget);
          var type = input.attr("name");
          var file = input.get(0).files[0];
          var extention = file.name.split(".")[file.name.split(".").length-1];

          //Get all data and put in a form
          var formdata = new FormData();
          formdata.append("file1", file);
          if(extention != "jpg" && extention != "jpeg" && extention != "png"){
              alert("Select a png or jpeg/jpg file");
              input.val("");
              return;
          }

          //Rename to temporary name
          if (type == "icon"){
            formdata.append("name", "tmp_icon");
            $('#modal .train-icon').css("background-image", ('url("./img/loading.svg")'));
          }
          else{
            formdata.append("name", "tmp_img");
            $('#modal .train-img').css("background-image", ('url("./img/loading.svg")'));
          }

          function success(location){
            
            if (type == "icon"){
              $('#modal .train-icon').css("background-image", ('url(./'+location+')'));
              var file = $('#modal .train-icon-name').val(location);

            }
            else{
              $('#modal .train-img').css("background-image", ('url(./'+location+')'));
              var file = $('#modal .train-image-name').val(location);
            }
          }

          //Create ajax request
          var ajax = new XMLHttpRequest();
          ajax.addEventListener("load", function(message){
            setTimeout(success.bind(null, ajax.responseText), 1000);
          }, false);
          ajax.addEventListener("error", function(){alert("error");console.warn(this.responseText);}, false);
          ajax.open("POST", "./img_upload.php");
          ajax.send(formdata);
        });

        var ctx = document.getElementById('engine_Speedsteps').getContext('2d');
        Modals.setups["engines.edit"].chart = new Chart(ctx, {
          // The type of chart we want to create
          type: 'line',

          // The data for our dataset
          data: {
            datasets: [{
              borderColor: 'rgb(255, 99, 132)',
              lineTension: 0,
              data: [{x:0, y:0}, {x:127, y:160}],
            }]
          },

          // Configuration options go here
          options: {
            legend: false,
            scales: {
              xAxes: [{
                type: 'linear',
                display: true,
                ticks: {
                  suggestedMin: 0,    // minimum will be 0, unless there is a lower value.
                  suggestedMax: 128,
                  stepSize: 16,
                }
              }],
              yAxes: [{
                type: 'linear',
                display: true,
                ticks:{
                  suggestedMin: 0,
                  suggestedMax: 100
                }
              }]
            },
            animation: false,
          }
        });

        Modals.setups["engines.edit"].update_graph();

        $('#engine_update_speedstep').on("click", function(){
          var speed = parseInt($('#engine_speedstep_config input[name=speed]').val());
          var step = parseInt($('#engine_speedstep_config input[name=step]').val());
          var list = Modals.setups["engines.edit"].speedsteps;

          if(speed >= 0){
            for(var i = 0; i < list.length; i++){
              if(list[i].step == step){
                list[i].speed = speed;
              }
              if(i == list.length - 1){
                list.push({step: step, speed: speed});
                break;
              }
            }
            if(list.length == 0){
              list.push({step: step, speed: speed});
            }
          }
          else{
            var i = -1;
            for(i; i < list.length; i++){
              if(list[i].step == step){
                break;
              }
            }
            if(i >= 0){
              list.splice(i, 1);
            }
          }
          list.sort(function(a, b){return a.step-b.step});
          Modals.setups["engines.edit"].speedsteps = list;
          Modals.setups["engines.edit"].update_graph();
        });
      },
      close_cb: function(data, ref){
        $('input[type=file]', ref).off();

        console.log(data);
      },
      update_graph: function(){
        var i, datetime, used, sun;
        var chart = Modals.setups["engines.edit"].chart;

        chart.data.datasets[0].data = [{x:0, y:0}];
        for (i = 0; i < Modals.setups["engines.edit"].speedsteps.length; i++) {
          step = Modals.setups["engines.edit"].speedsteps[i];
          chart.data.datasets[0].data.push({'x': step.step, 'y': step.speed});
        }
        chart.update();
      },
      create_cb: function(data){
        if(data.return_code == undefined){
          return;
        }

        if(data.return_code == 0){
          alert("Failed with no error code");
        }else if(data.return_code == -1){
          alert("DCC allready in use");
          $('#modal .modal-body input[name=dcc]').addClass("is-invalid");
          $('#modal .modal-body input[name=dcc]').focus();
        }
        else if(data.return_code == -2){
          $('#modal .modal-body input[name='+data.data[0]+']').focus();
          console.warn(data);
          for(var i = 0; i < data.data.length; i++){
            $('#modal .modal-body input[name='+data.data[i]+']').addClass("is-invalid");
            $('#modal .modal-body button[name='+data.data[i]+']').addClass("btn-outline-danger").addClass("isinvalid").removeClass("btn-outline-primary");
            $('#modal .modal-body button[name='+data.data[i]+'-btn]').addClass("btn-danger").addClass("isinvalid").removeClass("btn-primary");
          }
        }
      },
      update_cb: function(return_code){

      },
      title: "Engine",
      content: '<div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-6">\
                    <div class="bg-light" style="width: 100%; padding-top: 50%; position: relative;">\
                      <div class="train-icon" style="background-image: url(\'./trains_img/200_ice3_im.jpg\');"></div>\
                    </div>\
                    <input class="modal-form train-icon-name" name="icon_name" type="text" style="display: none;">\
                    <input name="icon" type="file" style="display: none;">\
                    <button name="train-icon-name-btn" onclick="$(\'#modal .modal-body input[type=file][name=icon]\').click();" class="btn mt-2 btn-sm btn-primary" style="margin:auto;display:block;">Browse icon</button>\
                  </div>\
                  <div class="col-6">\
                    <div class="bg-light" style="width: 100%; padding-top: 50%; position: relative;">\
                      <div class="train-img" style="background-image: url(\'./trains_img/200_ice3_im.jpg\');"></div>\
                    </div>\
                    <input class="modal-form train-image-name" name="image_name" type="text" style="display: none;">\
                    <input name="image" type="file" style="display: none;">\
                    <button name="train-image-name-btn" onclick="$(\'#modal .modal-body input[type=file][name=image]\').click();" class="btn mt-2 btn-sm btn-primary" style="margin:auto;display:block;">Browse image</button>\
                  </div>\
                </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-4 control-label"><span style="line-height: 38px;vertical-align:middle">Name</span></div>\
                  <div class="col-8"><input name="name" type="text" class="modal-form form-control input-sm"></div>\
                </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-4 control-label"><span style="line-height: 38px;vertical-align:middle">DCC address</span></div>\
                  <div class="col-8"><input type="number" name="dcc" class="modal-form form-control input-sm"></div>\
                </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-4 control-label"><span style="line-height: 38px;vertical-align:middle">Length</span></div>\
                  <div class="col-8"><input type="number" name="length" class="modal-form form-control input-sm"></div>\
                </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-4 control-label"><span style="line-height: 28px;vertical-align:middle">Speedsteps</span></div>\
                  <div class="col-8 btn-toggle-group">\
                    <button type="number" name="speedstep" class="modal-form btn-toggle btn btn-xs btn-outline-primary" value="14">14</button>\
                    <button type="number" name="speedstep" class="modal-form btn-toggle btn btn-xs btn-outline-primary" value="28">28</button>\
                    <button type="number" name="speedstep" class="modal-form btn-toggle btn btn-xs btn-outline-primary" value="128">128</button>\
                  </div>\
                </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-4 control-label"><span style="line-height: 48px;vertical-align:middle">Type</span></div>\
                  <div class="col-8" style="height: 3.2rem; overflow: hidden">\
                      <div class="train-categories btn-toggle-group" style="overflow:overlay;height:150%;white-space: nowrap;min-width:100%"></div>\
                    </div>\
                  </div>\
                </div>\
                <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-4 control-label"><span class="modal-form" type="speedsteps" name="speedsteps" style="line-height: 38px;vertical-align:middle">Speedsteps</span></div>\
                  <div id="engine_speedstep_config" class="col-8">\
                    <input type="number" name="step" placeholder="step" class="form-control input-sm" style="width:40%;display:inline-block">\
                    <input type="number" name="speed" placeholder="speed" class="form-control input-sm" style="width:40%;display:inline-block">\
                    <svg id="engine_update_speedstep" x="0px" y="0px" width="24px" height="24px" viewBox="0 0 530 530" style="float:right;margin:7px;cursor:pointer">\
                      <path d="M328.883,89.125l107.59,107.589l-272.34,272.34L56.604,361.465L328.883,89.125z M518.113,63.177l-47.981-47.981\
                        c-18.543-18.543-48.653-18.543-67.259,0l-45.961,45.961l107.59,107.59l53.611-53.611\
                        C532.495,100.753,532.495,77.559,518.113,63.177z M0.3,512.69c-1.958,8.812,5.998,16.708,14.811,14.565l119.891-29.069\
                        L27.473,390.597L0.3,512.69z" fill="#dddddd"/>\
                    </svg>\
                  </div>\
                </div>\
                <div class="row">\
                  <div class="col-12">\
                      <canvas id="engine_Speedsteps"></canvas>\
                    </div>\
                  </div>\
                </div>',
      buttons: {
        success: {visible: true, content: "Create",
              cb: function(data){
                  console.log("Engine Create");
                  console.log(data);
                  websocket.cts_add_engine(data);
              },
              wait: true},
        warning: {visible: true, content: "Update",
                  cb: function(data){
                      console.log("Engine Update");
                      console.log(data);
                      websocket.cts_edit_engine(data);
                  },
                  wait: true},
        danger: {visible: true, content: "Delete",
                 cb: function(){
                   console.warn("Delete Engine");
                   websocket.cts_edit_engine({id:data.id}, false)
                 },
                 wait: false},
      }
    },

    "cars.choose":{
      link: "button.btn-cars-edit",
      open_cb: function(data, ref){
        $('.modal-body', ref).append("<div style='width:100%; overflow:hidden; max-height: 600px'><div style='width:125%;position: relative; overflow: overlay;'><div class='cont' style='width:80%;'><ul class='list-unstyled'></ul></div></div></div>");
        $('.modal-body > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
        $('.modal-body > div > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
        for(var i = 0; i<Train.cars.length; i++){
          var t = Train.cars[i];

          var text = '<li class="message media">'+
              '<div class="message-mbox align-self-center mr-3 bg-primary" style="width:120px;height:60px"></div>'+
              '<div class="message-body media-body">'+
                '<div class="mt-0 mb-0 message-header"><b>'+t.name+'</b></div>'+
                '<small class="message-content">\
                  <i>nr: '+t.nr+'</i>\
                  <span style="width:2em; display:inline-block;"></span>\
                  <i>Length: '+t.length+' mm</i>\
                </small>'+
                '<button name="train_id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1"\
                  value="'+i+'" style="display: block;">Select</button>'+
              '</div></li>';
          $('.modal-body .cont ul', ref).append(text);
          // $('.modal-body .cont ul', ref).append("<button name='train_id' class='modal-form btn btn-toggle btn-outline-primary m-1' value='"+i+"' style='display: block;'>"+t.name+'</button><br/>PIZZA<br/>');
        }
      },
      title: "Choose a car",
      content: '',
      buttons: {
        success: {visible: false, content: "", cb: undefined, wait: false},
        warning: {visible: true, content: "Edit", cb: function(){Modals.hide(); Modals.open('cars.edit', {id: Modals.data.train_id})}, wait:true},
        danger: {visible: true, content: "Remove", cb: undefined, wait: false},
      }
    },
    "cars.edit":{
      link: "button.btn-cars-new",
      open_cb: function(data, ref){
        console.log("cars.edit open_cb: ", data);
        $('.datafield', ref).html(parseInt(data.id));
        if(data.id == undefined){
          $('button.btn-warning', ref).hide();
          $('button.btn-danger', ref).hide();
        }
        else{
          $('button.btn-success', ref).hide();
        }

        if(data.id != undefined){
          $('input[name=name]', ref).val(Train.cars[parseInt(data.id)].name);
          $('input[name=nr]', ref).val(Train.cars[parseInt(data.id)].nr);
          $('input[name=length]', ref).val(Train.cars[parseInt(data.id)].length);

          // TODO select type buttons
        }

        $('input[type=file]', ref).on("update", function(){});
      },
      title: "Car",
      content: '<div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                  <div class="col-12">\
                    <div class="bg-light" style="width: 100%; padding-top: 50%; position: relative;">\
                      <div class="train-icon" style="background-image: url(\'./trains_img/200_ice3_im.jpg\');"></div>\
                    </div>\
                    <input class="modal-form train-icon-name" name="icon_name" type="text" style="display: none;">\
                    <input name="icon" type="file" style="display: none;">\
                    <button name="train-icon-name-btn" onclick="$(\'#modal .modal-body input[type=file][name=icon]\').click();" class="btn mt-2 btn-sm btn-primary" style="margin:auto;display:block;">Browse icon</button>\
                  </div>\
                  <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                    <div class="col-4 control-label">\
                      <span style="line-height: 38px;vertical-align:middle">Name</span>\
                    </div>\
                    <div class="col-8"><input name="name" class="modal-form form-control input-sm"></div>\
                  </div>\
                  <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                    <div class="col-4 control-label"><span style="line-height: 38px;vertical-align:middle">ID number</span></div>\
                    <div class="col-8"><input type="number" name="nr" class="modal-form form-control input-sm"></div>\
                  </div>\
                  <div class="row mb-2" style="border-bottom: 1px solid #ddd; padding-bottom: 0.5rem;">\
                    <div class="col-4 control-label"><span style="line-height: 38px;vertical-align:middle">Length</span></div>\
                    <div class="col-8"><input type="number" name="length" class="modal-form form-control input-sm"></div>\
                  </div>\
                  <div class="row">\
                    <div class="col-4 control-label"><span style="line-height: 48px;vertical-align:middle">Type</span></div>\
                    <div class="col-8">\
                      <button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary">Highspeed</button>\
                      <button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary">Intercity</button>\
                      <button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary">Regional</button><br/>\
                      <button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary">Cargo</button>\
                      <button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary">Coal</button>\
                      <button name="type" class="modal-form btn-toggle btn btn-xs btn-outline-primary">Boxes</button>\
                    </div>\
                  </div>',
      buttons: {
        success: {visible: true, content: "Create", cb: function(data){/*websocket.cts_link_train(fid, rid, type, mid)*/}, wait: false},
        warning: {visible: true, content: "Update", cb: function(){alert("Update");}, wait: false},
        danger: {visible: true, content: "Delete", cb: function(){alert("Delete")}, wait: false},
      }
    },

    "train_control.select":{
      link: [{link: ".train-box.box1 button.select-train", args: {box: 1}},
           {link: ".train-box.box2 button.select-train", args: {box: 2}}],
      open_cb: function(data, ref){
        $('.modal-body', ref).append('<input name="box" type="number" class="modal-form" style="display: none;" value="'+data.box+'"/>');
        $('.modal-body', ref).append("<div style='width:100%; overflow:hidden; max-height: 600px'><div style='width:125%;position: relative; overflow: overlay;'><div class='cont' style='width:80%;'><ul class='list-unstyled btn-toggle-group train-list'><li class='empty'><i>No trains available</i></li></ul></div></div></div>");
        $('.modal-body ul.train-list', ref).after("<ul class='list-unstyled btn-toggle-group engine-list'><li class='empty'><i>No engines available</i></li></ul>");

        $('.modal-body > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
        $('.modal-body > div > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
        for(var i = 0; i<Train.trains.length; i++){
          var t = Train.trains[i];

          var text = '<li ontrack="'+t.ontrack+'" class="list-unstyled">'+
              '<div class="mr-3 ml-3 mt-3 bg-outline-dark" style="width:calc(100% - 2rem);height:36px; padding: 3px; overflow: hidden"><div style="width:100%; height:150%; overflow-x: overlay;white-space: nowrap">';

          for(var j = 0; j < t.link.length; j++){
            if(t.link[j][0] == 0 && Train.engines[t.link[j][1]] != undefined){
              text += '<img src="./trains_img/'+Train.engines[t.link[j][1]].icon+'" style="height:30px"/>';
            }else if(Train.cars[t.link[j][1]] != undefined){
              text += '<img src="./trains_img/'+Train.cars[t.link[j][1]].icon+'" style="height:30px"/>';
            }
          }

          text += '</div></div><div class="m-2" style="width:calc(100% - 2rem)">'+
                '<button type="number" name="id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1 mr-3"\
                  value="'+i+'" style="display: inline-block;">Select</button>'+
                '<div class="mt-0 mb-0" style="display: inline-block"><b>'+t.name+'</b></div>'+
                '<div class="" style="display: inline-block; float:right; max-width: 210px;font-size:small; width:100%;">\
                  <i>DCC: '+t.dcc.join(", ")+'</i>\
                  <i style="float:right">Length: '+t.length+' mm</i>\
                </div>'+
              '</div></li>';
          $('.modal-body .cont ul.train-list li:last-child', ref).before(text);
        }

        for(var i = 0; i<Train.engines.length; i++){
          var t = Train.engines[i];

          var text = '<li ontrack="'+t.ontrack+'" class="message media">'+
              '<div class="message-mbox align-self-center mr-3 bg-primary" style="width:120px;height:60px"></div>'+
              '<div class="message-body media-body">'+
                '<div class="mt-0 mb-0 message-header"><b>'+t.name+'</b></div>'+
                '<div class="message-content" style="max-width: 210px;font-size:small; width:100%;">\
                  <i>DCC: '+t.dcc+'</i>\
                  <i style="float:right">Length: '+t.length+' mm</i>\
                </div>'+
                '<button type="number" name="id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1"\
                  value="'+i+'" style="display: block;">Select</button>'+
              '</div></li>';
          $('.modal-body .cont ul.engine-list li:last-child', ref).before(text);
        }


        $('button', $('.btn-toggle-group', ref)[0]).on("click", function(evt){
          $('ul.train-list', ref).hide();
          $('ul.engine-list', ref).hide();

          if(evt.currentTarget.innerText == "Engines"){
            $('ul.engine-list', ref).show();
          }
          else if(evt.currentTarget.innerText == "Trains"){
            $('ul.train-list', ref).show();
          }
        });

        function show_empty_list(){
          if($('#modal ul.train-list li:not(.hide-train)').first().hasClass("empty")){
            $('#modal ul.train-list li.empty').show();
          }
          else{
            $('#modal ul.train-list li.empty').hide();
          }

          if($('#modal ul.engine-list li:not(.hide-train)').first().hasClass("empty")){
            $('#modal ul.engine-list li.empty').show();
          }
          else{
            $('#modal ul.engine-list li.empty').hide();
          }
        }

        $('button', $('.btn-toggle-group', ref)[1]).on("click", function(evt){
          if(evt.currentTarget.innerText == "All"){
            $('li:not(.empty)', ref).show()
            $('li:not(.empty)', ref).removeClass("hide-train");
          }
          else if(evt.currentTarget.innerText == "On layout"){
            $('li[ontrack=0]:not(.empty)', ref).hide();
            $('li[ontrack=0]:not(.empty)', ref).addClass("hide-train");
          }

          show_empty_list();
        });

        $('#modal ul.train-list').hide();
        $('li[ontrack=0]:not(.empty)', ref).hide();
        $('li[ontrack=0]:not(.empty)', ref).addClass("hide-train");
        show_empty_list();
      },
      title: "Select a train",
      content: '<div class="row mb-3">\
        <div class="col-6">\
          <div class="row btn-toggle-group">\
            <div class="col-6" style="text-align: right;">\
              <button name="type" value="E" class="modal-form btn btn-toggle btn-xs btn-primary">Engines</button>\
            </div>\
            <div class="col-6" style="text-align:left;">\
              <button name="type" value="T" class="modal-form btn btn-toggle btn-xs btn-outline-primary">Trains</button>\
            </div>\
          </div>\
        </div>\
        <div class="col-6">\
          <div class="row btn-toggle-group">\
            <div class="col-6" style="text-align: right;">\
              <button class="btn btn-toggle btn-xs btn-outline-primary">All</button>\
            </div>\
            <div class="col-6" style="text-align:left;">\
              <button class="btn btn-toggle btn-xs btn-primary">On layout</button>\
            </div>\
          </div>\
        </div>',
      buttons: {
        success: {visible: true, content: "Select", cb: function(data){console.log(data);Train_Control.select_train(data.box, data.type, data.id)}, wait: false},
        warning: {visible: false, content: "", cb: undefined, wait: false},
        danger: {visible: true, content: "Cancel", cb: undefined, wait: false},
      }
    },
    "train_control.set_route":{
      link: [{link: ".train-box.box1 button.set-route", args: {box: 1}},
           {link: ".train-box.box2 button.set-route", args: {box: 2}}],
      open_cb: function(data, ref){
        // $('.modal-body', ref).append("<div style='width:100%; overflow:hidden; max-height: 600px'><div style='width:125%;position: relative; overflow: overlay;'><div class='cont' style='width:80%;'><ul class='list-unstyled'></ul></div></div></div>");
        $('.modal-body', ref).append(data.box);
        // $('.modal-body > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
        // $('.modal-body > div > div').css("max-height", "calc("+(window.innerHeight*0.8-71)+"px - 3em)");
        // for(var i = 0; i<Train.cars.length; i++){
        //  var t = Train.cars[i];

        //  var text = '<li class="message media">'+
        //      '<div class="message-mbox align-self-center mr-3 bg-primary" style="width:120px;height:60px"></div>'+
        //      '<div class="message-body media-body">'+
        //        '<div class="mt-0 mb-0 message-header"><b>'+t.name+'</b></div>'+
        //        '<small class="message-content">\
        //          <i>nr: '+t.nr+'</i>\
        //          <span style="width:2em; display:inline-block;"></span>\
        //          <i>Length: '+t.length+' mm</i>\
        //        </small>'+
        //        '<button name="train_id" class="modal-form btn btn-toggle btn-xs btn-outline-primary m-1"\
        //          value="'+i+'" style="display: block;">Select</button>'+
        //      '</div></li>';
        //  $('.modal-body .cont ul', ref).append(text);
        //  // $('.modal-body .cont ul', ref).append("<button name='train_id' class='modal-form btn btn-toggle btn-outline-primary m-1' value='"+i+"' style='display: block;'>"+t.name+'</button><br/>PIZZA<br/>');
        // }
      },
      title: "Set route for train",
      content: '',
      buttons: {
        success: {visible: true, content: "Select", cb: undefined, wait: false},
        warning: {visible: false, content: "", cb: undefined, wait: false},
        danger: {visible: true, content: "Cancel", cb: undefined, wait: false},
      }
    },

    "module.line":{
      link: "",
      open_cb: function(data, ref){

      },
      title: "Edit Line",
      content: 'Test Line',
      buttons: {
        success: {visible: true, content: "Update", cb: undefined, wait: false},
        warning: {visible: true, content: "Discard", cb: undefined, wait: false},
        danger: {visible: false, content: "", cb: undefined, wait: false}
      }
    },
    "module.arc":{
      link: "",
      open_cb: function(data, ref){

      },
      title: "Edit Arc",
      content: 'Test Arc',
      buttons: {
        success: {visible: true, content: "Update", cb: undefined, wait: false},
        warning: {visible: true, content: "Discard", cb: undefined, wait: false},
        danger: {visible: false, content: "", cb: undefined, wait: false}
      }
    },
    "module.sw":{
      link: "",
      open_cb: function(data, ref){

      },
      title: "Edit Switch",
      content: 'Test Switch',
      buttons: {
        success: {visible: true, content: "Update", cb: undefined, wait: false},
        warning: {visible: true, content: "Discard", cb: undefined, wait: false},
        danger: {visible: false, content: "", cb: undefined, wait: false}
      }
    },
    "module.ds":{
      link: "",
      open_cb: function(data, ref){

      },
      title: "Edit Double Slip Switch",
      content: 'Test Double Slip Switch',
      buttons: {
        success: {visible: true, content: "Update", cb: undefined, wait: false},
        warning: {visible: true, content: "Discard", cb: undefined, wait: false},
        danger: {visible: false, content: "", cb: undefined, wait: false}
      }
    }
  },

  init: function(){
    var keys = Object.keys(this.setups);
    for(var i = 0; i < keys.length; i++){
      var s = this.setups[ keys[i] ].link;
      if(s != undefined){
        if((typeof s) == "string" ){
          $(s).on("click", this.open.bind(this, keys[i]));
        }
        else if((typeof s) == "object"){
          var keys2 = Object.keys(s);
          for(var j = 0; j < keys2.length; j++){
            $(s[ keys2[j] ].link).on("click", this.open.bind(this, keys[i], s[ keys2[j] ].args));
          }
        }
      }
    }
  },

  call_cb: function(func_name, arg){
    if(this.frame == undefined){
      return;
    }

    if(this.frame[func_name] && {}.toString.call(this.frame[func_name]) === '[object Function]'){
      this.frame[func_name](arg);
    }
  },

  frame: undefined,

  open: function(modal, args=undefined){
    // Remove previous events
    console.log(modal);
    console.log(args);
    $('#modal .modal-footer button').off();

    this.frame = this.setups[modal];
    $('#modal .modal-title').html(this.frame.title);
    $('#modal .modal-body').html(this.frame.content);

    buttons = Object.keys(this.frame.buttons);
    for(var i = 0; i < buttons.length; i++){
      var btn = this.frame.buttons[ buttons[i] ];
      if(btn.visible){
        $('#modal .modal-footer .btn-'+buttons[i]).show();
        $('#modal .modal-footer .btn-'+buttons[i]).html(btn.content);
        $('#modal .modal-footer .btn-'+buttons[i]).on("click", this.close.bind(this, buttons[i]));
      }
      else{
        $('#modal .modal-footer .btn-'+buttons[i]).hide();
        $('#modal .modal-footer .btn-'+buttons[i]).html("");
      }
    }

    if(this.frame.open_cb != undefined && args != undefined){
      this.frame.open_cb(args, $('#modal'));
    }

    $('#modal .modal-body button.btn-toggle').on("click", function(evt){
      if($(evt.target).hasClass("isinvalid")){
        $('button.btn-toggle', $(evt.target).closest("div.modal-body, .btn-toggle-group")).removeClass('btn-outline-danger').removeClass("isinvalid");
      }
      $('button.btn-toggle', $(evt.target).closest("div.modal-body, .btn-toggle-group")).removeClass('btn-primary');
      $('button.btn-toggle', $(evt.target).closest("div.modal-body, .btn-toggle-group")).addClass('btn-outline-primary');
      $(evt.target).removeClass('btn-outline-primary');
      $(evt.target).addClass('btn-primary');
    });

    $('#modal').modal('show');
  },

  close: function(evt){
    this.data = {};
    var frame = this.frame;

    var that = this;
    $.each($('#modal .modal-form'), function(){
      var name = this.getAttribute("name");
      var type = this.getAttribute("type")
      if(this.tagName == "BUTTON" && this.classList.contains("btn-primary")){
        that.data[name] = this.getAttribute("value");
        if(type == "number"){
          that.data[name] = parseInt(that.data[name]);
        }
      }
      else if(type == "file"){
        that.data[name] = this.files[0];
      }
      else if(this.tagName != "BUTTON" && (type == "text" || type == "number")){
        that.data[name] = this.value;
        if(type == "number"){
          that.data[name] = parseInt(that.data[name]);
        }
      }
      else if(this.tagName != "BUTTON" && type == "speedsteps"){
        that.data[name] = frame.speedsteps;
      }
    });

    console.log(this.data);

    if(frame.buttons[evt].cb != undefined){
      frame.buttons[evt].cb(this.data);
    }

    if(!frame.buttons[evt].wait){
      this.hide();
    }
  },

  hide: function(){
    if(this.frame.close_cb != undefined){
      this.frame.close_cb(this.data, $('#modal'));
    }

    this.frame == undefined;

    $('#modal .modal-footer button').off();
    $('#modal').modal('hide');
  }
};

events.add_init(Modals.init.bind(Modals));
