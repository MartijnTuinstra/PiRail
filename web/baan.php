<?php
  header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
  header("Cache-Control: post-check=0, pre-check=0", false);
  header("Pragma: no-cache");

  $data = file_get_contents("./../setup.json");
  $data = json_decode($data);
  date_default_timezone_set("Europe/Amsterdam");
  error_reporting(E_ERROR | E_WARNING | E_PARSE);
  ini_set('display_errors', 1);

  $var2 = file_get_contents("./../trains/trainlist_raw.txt");
  $var3 = explode("\r\n",$var2);
  for($i = 0;$i<(count($var3)-1);$i++){
    $var4[$i] = explode("\t",$var3[$i]);
  }
?><html>
  <head>
    <title>TEST</title>
    <meta id="vp" name="viewport" content="width=device-width, initial-scale=0.7">
    <meta name="mobile-web-app-capable" content="yes">
    <link rel="manifest" href="./manifest.json">
    <script src="./scripts/framework/jquery-3.1.1.min.js"></script>
    <script src="./scripts/framework/jquery-ui.min.js"></script>
    <script src="./scripts/framework/jquery.ui.touch-punch.min.js"></script>
    <link rel="stylesheet" type="text/css" href="./styles/jquery-ui.min.css">
    <link rel="stylesheet" type="text/css" href="./styles/main.css">
		<!--<link rel="stylesheet" type="text/css" href="./SelectInspiration/css/normalize.css" />
		<link rel="stylesheet" type="text/css" href="./SelectInspiration/css/demo.css" />-->
    <script>
      <?php

      array_shift($var4);

      foreach ($var4 as $key => $row) {
        $a[$key] = $row[0];
        $b[$key] = $row[1];
        $c[$key] = $row[2];
      }

      // Sort the data with volume descending, edition ascending
      // Add $data as the last parameter, to sort by the common key
      if(!isset($_GET['sort']) || $_GET['sort'] == 'Name'){
        array_multisort($b, $var4);
      }else if($_GET['sort'] == 'DCC'){
        array_multisort($c, $var4);
      }else if($_GET['sort'] == 'List'){
        array_multisort($a, $var4);
      }
      $js_array = json_encode($var4);
      echo "var train_list = ". $js_array . ";";
      ?>

      var train_list_c = [];
      train_list.forEach( function(item, index){
        console.log(item);
        console.log(parseInt(item[0])+':'+index);
        train_list_c[parseInt(item[0])] = index;
      });
    </script>

		<link rel="stylesheet" type="text/css" href="./styles/cs-select.css" />
		<link rel="stylesheet" type="text/css" href="./styles/cs-skin-border.css" />
    <script src="./scripts/windows.js"></script>
    <script>
      $.support.cors = true;
      var pause = 1;
      var time_addr;
      var tablet = 0;
      var myWindow;

      for(var i = 0;i<train_list.length;i++){
        var chars = train_list[i][5].split('');
        train_list[i][5] = chars;
      }

      var train_list_t = "";
      var Station_list_t = "";
      var Station_list = [];

      $.urlParam = function(name){
          var results = new RegExp('[\?&]' + name + '=([^&#]*)').exec(window.location.href);
          if (results==null){
             return null;
          }
          else{
             return results[1] || 0;
          }
      }

      setInterval(function(){
        today = new Date();
        var H = today.getHours();
        var M = today.getMinutes();
        if(H < 10){
          H = "0"+H;
        }
        if(M < 10){
          M = "0"+M;
        }
        $("#clock").html(H+":"+M);
      },10000);

      function play_pause(obj){
        if(pause == 0){
          obj.src='./img/pause_w.png';
          clearInterval(time_addr);
          pause = 1;
        }else{
          obj.src='./img/play_w.png';
          time_addr = setInterval(update,500);
          pause = 0;
        }
      }

      function toggleFullScreen() {
        var doc = window.document;
        var docEl = doc.documentElement;

        var requestFullScreen = docEl.requestFullscreen || docEl.mozRequestFullScreen || docEl.webkitRequestFullScreen || docEl.msRequestFullscreen;
        var cancelFullScreen = doc.exitFullscreen || doc.mozCancelFullScreen || doc.webkitExitFullscreen || doc.msExitFullscreen;

        if(!doc.fullscreenElement && !doc.mozFullScreenElement && !doc.webkitFullscreenElement && !doc.msFullscreenElement) {
          requestFullScreen.call(docEl);
        }
        else {
          cancelFullScreen.call(doc);
        }
      }

      function zoom(scale){
        console.log("Zoom: "+parseInt($('#track').css('zoom'))+scale)
        if(scale == '+' && parseInt($('#track').css('zoom')) != 5){
          $('#track').css('zoom',parseInt($('#track').css('zoom'))+1);
          $('#track').css('height','calc(100% - '+(40/(parseInt($('#track').css('zoom'))))+'px)');
        }else if(scale == '-' && parseInt($('#track').css('zoom')) != 1){
          $('#track').css('zoom',parseInt($('#track').css('zoom'))-1);
          $('#track').css('height','calc(100% - '+(40/(parseInt($('#track').css('zoom'))))+'px)');
        }
      }

      $(function (){
        // device detection
        if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|ipad|iris|kindle|Android|Silk|lge |maemo|midp|mmp|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows (ce|phone)|xda|xiino/i.test(navigator.userAgent)
            || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(navigator.userAgent.substr(0,4))) tablet = 1;

        if($.urlParam('tablet') == 'y'){
          tablet = 1;
        }

        //alert(tablet);

        $.each(train_list, function(i){
          if(i != 0){
            train_list_t += "<option value=\""+train_list[i][0]+"\">#"+train_list[i][2]+"&nbsp;&nbsp;&nbsp;"+train_list[i][1]+"</option>";
          }
        });

        if (screen.width < 800) {
            var mvp = document.getElementById('vp');
            mvp.setAttribute('content','width=800');
        }
        if(tablet == 1){
          $("#trains").css("width","100%");
          $("#trains").css("position","absolute");
          $("#trains").css("left","0px");
          $("#trains").css("top","auto");
          $("#trains").css("bottom","0px");
          $("#switches").css("width","100%");
          $("#switches").css("position","absolute");
          $("#switches").css("left","0px");
          $("#switches").css("top","auto");
          $("#switches").css("bottom","0px");
          $("#tab").attr("src","./img/checkbox_c.png");

          $("#track").css("height","calc(100% - 20px)");
          $("#track").css("zoom","200%");
          //toggleFullScreen();
          //document.documentElement.requestFullscreen();
        }else{
          $( "#trains" ).draggable({ handle: "div.drag",containment: "html", });
          $("#switches").draggable({ handle: "div.drag",containment: "html", });
          $("#tab").attr("src","./img/checkbox_e.png");
          /*
          $( "#slider-vertical" ).slider({
            orientation: "vertical",
            range: "min",
            min: 0,
            max: 100,
            value: 60,
            slide: function( event, ui ) {
              console.log( ui.value );
            }
          });*/
        }

        var handle = $("#T1200 .slider-handle" );

        $( "#T1200 .slider" ).slider({
          orientation: "vertical",
          range: "min",
          min: 0,
          max: 160,
          value: 60,
          create: function() {
            handle.text( $( this ).slider( "value" ) );
          },
          slide: function( event, ui ) {
            handle.text( ui.value );
          },
          change: function(event, ui){
            handle.text( ui.value );
          }/*,
          change: function(event, ui){
            var location = handle.css('bottom');
            location = location.substring(0,location.length - 2);
            location = location / $(this).height() * 100;
            location = location * 0.95;
            console.log(location);
            handle.css('bottom',location+'%');
          }*/
          //slide: function( event, ui ) {
            //$( "#amount" ).val( ui.value );
          //}
        });
        //$( "#amount" ).val( $( "#slider-vertical" ).slider( "value" ) );

        //$("#play_pause").bind("tap",play_pause(this));
      });

      /*MTrains*/

      var view = 0;
      var addingTrain = 0;
      function getExtension(filename) {
          var parts = filename.split('.');
          return parts[parts.length - 1];
      }

      function create(){
        $('#Train_Add').css("display",'block');
        $('#Train_Add #upload').attr('src','./img/checked.png');
        $('#Train_Add #upload').css('cursor','pointer');
        $('#Train_Add #upload').attr('onClick','add_train()');
      }

      function add_train(){
        if ($('#Train_Add #train_name').val().indexOf(':') > -1) {
          alert("Name cannot contain a ':'");
          return;
        }

        if(parseInt($('#train_dcc').val()) > 9999){
          alert("Invalid DCC address\nMust be smaller than 9999");
          return;
        }

        //Check if a file is selected
        if($("#Train_Add #file1").get(0).value == "") {
          alert("Select a file");
          return;
        }

        $("#Train_Add #upload").attr('src','./img/loading.svg');
        $("#Train_Add #upload").attr('onClick','');

        if(ws.readyState==1){
          addingTrain = 1;
          var dcc = parseInt($('#train_dcc').val());
          var speed = parseInt($('#train_speed').val());
          var name = $('#train_name').val().split("");
          $.each(name,function(index){
            name[index] = name[index].charCodeAt();
          })
          var data = [];
          data[0] = 3;
          data = data.concat(name);
          data.push(0);
          data.push($('#train_type').val().charCodeAt(0));
          data.push(dcc >> 8,dcc & 0xFF);
          data.push(speed >> 8,speed & 0xFF);
          ws.send(new Uint8Array(data));
          //ws.send('RNt'+$('#train_type').val()+$('#train_name').val()+':'+$('#train_dcc').val()+':'+$('#train_speed').val());
          console.log("Requesting");
        }else{
        addingTrain = 1;
          succ_add_train($("#train_dcc").val(),1);
          console.log("Uploading Picture without server");
        }
      }

      function succ_add_train(number, file_edit = 0){
        console.log("Request succes");

        var data = [];
        data[0] = number;
        data[3] = $('#train_type').val();
        data[1] = $('#train_name').val();
        data[2] = $('#train_dcc').val();
        data[4] = $('#train_speed').val();

        if(addingTrain == 1){
          var file = $("#file1").get(0).files[0];
          // alert(file.name+" | "+file.size+" | "+file.type);
          var name = number + "." + getExtension(file.name).toLowerCase();
          var formdata = new FormData();
          formdata.append("file1", file);
          formdata.append("name", name);
          formdata.append("number", number);
          formdata.append("file_edit", file_edit);
          if(file_edit == 1){
            console.log("Service not on.")
            formdata.append("dcc",$('#train_dcc').val());
            formdata.append("type",$('#train_type').val());
            formdata.append("train_name",$('#train_name').val());
            formdata.append("max",$('#train_speed').val());
          }
          var ajax = new XMLHttpRequest();
          ajax.addEventListener("load", function(event){completeHandler(event,data)}, false);
          ajax.addEventListener("error", errorHandler, false);
          ajax.addEventListener("abort", abortHandler, false);
          ajax.open("POST", "./train_img_upload.php");
          ajax.send(formdata);
        }else{
          setTimeout(function () {
            var text = "<tr><td><img src=\"./../trains/"+data[0]+".jpg\" style=\"max-height:50px;max-width:100px;float:right;\"/></td><td>";
            text    += data[1]+"</td><td>"+data[2]+"</td><td>"+data[3]+"</td><td>"+data[4]+"</td><td><img src=\"./img/bin_w.png\" style=\"width:20px;\"/></td></tr>"
            $("#list").append(text);
          }, 4000);
        }
      }

      function completeHandler(event,data){
        console.log("Complete");
        console.log(event);
        setTimeout(function () {
          console.log(data);
      	  $("#Train_Add").css('display','none');
      	  $("#Train_Add  #upload").attr('onClick','add_train()');
          var text = "<tr><td><img src=\"./../trains/"+data[0]+".jpg\" style=\"max-height:50px;max-width:100px;float:right;\"/></td><td>";
          text    += data[1]+"</td><td>"+data[2]+"</td><td>"+data[3]+"</td><td>"+data[4]+"</td><td><img src=\"./img/bin_w.png\" style=\"width:20px;\"/></td></tr>"
          $("#list").append(text);
        }, 5000);
      }

      function errorHandler(event,data){
      	alert("Upload Failed");
        console.log(event);
        console.log(data);
      }

      function abortHandler(event,data){
      	alert("Upload Aborted");
        console.log(data);
      }

      function fail_add_train(){
        $("#Train_Add #upload").attr('src','./img/checked.png');
        $("#Train_Add #upload").attr('onClick','add_train()');
        console.log("Request fail");
      }

      function ManageTrain(object){
        var s = $("img",$(object).parent()).attr("src");
        var classes = $(object).parent().attr('class').split(/\s+/);
        n = parseInt(classes[1].substring(4,classes[1].length));
        console.log(n);
        console.log(train_list[n]);
        $('#Train_Settings_box').css('display','block');
        $('h1','#Train_Settings_box').html(train_list[n][1]);
        $('h2','#Train_Settings_box').html('DCC adr: '+train_list[n][2]);
        $('#train_set_box_img','#Train_Settings_box').attr('src','./../trains/'+train_list[n][0]+'.jpg');
      }

  	  function train_set_route(obj){
  		var TrainNr = $($(obj).parent().children()[1]).children()[0].value;
  		var choice  = $(obj).parent().children()[2].value;

  		var str = [34,(TrainNr >> 8),(TrainNr & 0xFF),choice];

  		var data2 = new Int8Array(str);

  		ws.send(data2);

  		$(obj).parent().css('display','none');
  	  }

  	  function open_Route(obj){
  		  var ID = parseInt($(obj).parent().parent().parent().attr("id").slice(1));

  		  $('#Station_List').css('display','block');
  		  $('#Station_List input').val(ID);
  	  }
	  </script>
  </head>
  <body>
    <div id="header" style="height:40px;width:100%;background-color:#B00">
      <div style="width:130px;margin-left:3%;float:left;height:40px">
        <?php include("./img/back.svg"); ?>
        <img src="./img/rotate_left_w.png" width="20px" style="margin:10px;cursor:pointer;"
            onClick="rotate('-');" onmouseenter="this.src = './img/rotate_left.png'" onmouseleave="this.src = './img/rotate_left_w.png'"/>
        <img src="./img/rotate_right_w.png" width="20px" style="margin:10px;cursor:pointer;"
            onClick="rotate('+');" onmouseenter="this.src = './img/rotate_right.png'" onmouseleave="this.src = './img/rotate_right_w.png'"/>
      </div>
      <div style="margin:1.5px;font-family:Sans-serif;float:left;">
        <h1 id="clock"><?php echo date('H:i');?></h1>
        <img id="notify" src="./img/notification.png" style="float:left;margin:8px;position: absolute;left: calc(50% + 50px);cursor:pointer"
                         height="24px" onClick="warning_toggle();"/>
      </div>
      <div id="menu_button" style="background-color:white;"
            onMouseenter="$(this).css('background-color','black');$(this).children()[0].src = './img/menu_w.png';"
            onmouseleave="$(this).css('background-color','white');$(this).children()[0].src = './img/menu.png';"
            onClick="menu_but()">
        <img src="./img/menu.png" width="16px"/>
      </div>
      <img id="status" src="./img/status_o.png" width="24px" style="float:right;margin:8px;margin-right:20px;"/>
    </div>
    <div id="menu" style="display:none;">
    	<ul>
    	  <li style="border-top-left-radius:5px;border-top-right-radius:5px;"
            onClick="cTrain_win();menu_but()">Trains</li>
    	  <li onClick="switch_win();menu_but()">Switches</li>
    	  <li onClick="track_win(); menu_but()">Track</li>
    	  <li onClick="mTrain_win();menu_but()">Manage trains</li>
        <li onClick="mModules_win();menu_but()">Manage modules</li>
    	  <li style="border-bottom:none;border-bottom-left-radius:5px;border-bottom-right-radius:5px;"
            onClick="settings();  menu_but()">Settings</li>
    	</ul>
    </div>
    <div id="warning_list" class="warning_list_nactive" style="display:none;">
    </div>
    <div id="track"    class="Window" style="display:none">
      <center>
        <div id="Modules_wrapper">
          <div id="Modules">
          </div>
        </div>
      </center>
    </div>
    <div id="switches" class="Window" style="display:none">
      <div style="height:40px;width:100%;float:left;padding: 5px 0px;text-align:center;background-color:grey"><h1>Switches</h1></div>
      <div class="content"></div>
    </div>
    <div id="MTrain"   class="Window" style="display:none;">
      <?php
        $i = 0;

        $cog = "<svg version=\"1.1\" x=\"0px\" y=\"0px\" width=\"438.529px\" height=\"438.529px\" viewBox=\"0 0 438.529 438.529\" style=\"enable-background:new 0 0 438.529 438.529;\" xml:space=\"preserve\"><path d=\"M436.25,181.438c-1.529-2.002-3.524-3.193-5.995-3.571l-52.249-7.992c-2.854-9.137-6.756-18.461-11.704-27.98c3.422-4.758,8.559-11.466,15.41-20.129c6.851-8.661,11.703-14.987,14.561-18.986c1.523-2.094,2.279-4.281,2.279-6.567c0-2.663-0.66-4.755-1.998-6.28c-6.848-9.708-22.552-25.885-47.106-48.536c-2.275-1.903-4.661-2.854-7.132-2.854c-2.857,0-5.14,0.855-6.854,2.567l-40.539,30.549c-7.806-3.999-16.371-7.52-25.693-10.565l-7.994-52.529c-0.191-2.474-1.287-4.521-3.285-6.139C255.95,0.806,253.623,0,250.954,0h-63.38c-5.52,0-8.947,2.663-10.278,7.993c-2.475,9.513-5.236,27.214-8.28,53.1c-8.947,2.86-17.607,6.476-25.981,10.853l-39.399-30.549c-2.474-1.903-4.948-2.854-7.422-2.854c-4.187,0-13.179,6.804-26.979,20.413c-13.8,13.612-23.169,23.841-28.122,30.69c-1.714,2.474-2.568,4.664-2.568,6.567c0,2.286,0.95,4.57,2.853,6.851c12.751,15.42,22.936,28.549,30.55,39.403c-4.759,8.754-8.47,17.511-11.132,26.265l-53.105,7.992c-2.093,0.382-3.9,1.621-5.424,3.715C0.76,182.531,0,184.722,0,187.002v63.383c0,2.478,0.76,4.709,2.284,6.708c1.524,1.998,3.521,3.195,5.996,3.572l52.25,7.71c2.663,9.325,6.564,18.743,11.704,28.257c-3.424,4.761-8.563,11.468-15.415,20.129c-6.851,8.665-11.709,14.989-14.561,18.986c-1.525,2.102-2.285,4.285-2.285,6.57c0,2.471,0.666,4.658,1.997,6.561c7.423,10.284,23.125,26.272,47.109,47.969c2.095,2.094,4.475,3.138,7.137,3.138c2.857,0,5.236-0.852,7.138-2.563l40.259-30.553c7.808,3.997,16.371,7.519,25.697,10.568l7.993,52.529c0.193,2.471,1.287,4.518,3.283,6.14c1.997,1.622,4.331,2.423,6.995,2.423h63.38c5.53,0,8.952-2.662,10.287-7.994c2.471-9.514,5.229-27.213,8.274-53.098c8.946-2.858,17.607-6.476,25.981-10.855l39.402,30.84c2.663,1.712,5.141,2.563,7.42,2.563c4.186,0,13.131-6.752,26.833-20.27c13.709-13.511,23.13-23.79,28.264-30.837c1.711-1.902,2.569-4.09,2.569-6.561c0-2.478-0.947-4.862-2.857-7.139c-13.698-16.754-23.883-29.882-30.546-39.402c3.806-7.043,7.519-15.701,11.136-25.98l52.817-7.988c2.279-0.383,4.189-1.622,5.708-3.716c1.523-2.098,2.279-4.288,2.279-6.571v-63.376C438.533,185.671,437.777,183.438,436.25,181.438z M270.946,270.939c-14.271,14.277-31.497,21.416-51.676,21.416c-20.177,0-37.401-7.139-51.678-21.416c-14.272-14.271-21.411-31.498-21.411-51.673c0-20.177,7.135-37.401,21.411-51.678c14.277-14.272,31.504-21.411,51.678-21.411c20.179,0,37.406,7.139,51.676,21.411c14.274,14.277,21.413,31.501,21.413,51.678C292.359,239.441,285.221,256.669,270.946,270.939z\"/></svg>";

        for($i;$i<count($var4);$i++){
          if($var4[$i][2] != "0"){
            echo("<div class=\"MTrainbox MT_T".$i."\"><div class=\"img\"></div><div style=\"width: 120px;position: absolute;right: 5px;color:white\"><b>".$var4[$i][1]."</b><br/><i>#".$var4[$i][2]."</i></div>");
            echo("<div class=\"setting_cog\" onclick=\"ManageTrain(this);\">".$cog."</div></div>");
          }
        }
      ?>
      <div style="width: calc(100% - 20px);padding:10px;margin:auto;height:40px;float:left;">
        <div id="Add_button" onClick="create();">
          <b>Add</b>
        </div>
      </div>
    </div>
    <div id="CTrain"   class="Window" style="display:none">
      <!--Dinamical generated-->
    </div>
    <div id="MModules" class="Window" style="display:none;">
      <?php

      $list = [];

      $listFile = fopen("./../modules/list.txt","r");
      $OptionList = json_decode(fread($listFile,filesize("./../modules/list.txt")),true);
      fclose($listFile);

      $directories = glob('./../modules/*' , GLOB_ONLYDIR);
      foreach($directories as $value){
        $nr = explode("modules/",$value)[1];

        echo('<div class="MBox"><div class="svgContainer">');
        $file = "./../modules/".$nr."/layout.svg";
        echo($file);
        include($file);
        echo('</div><div class="title"><h1>'.$OptionList[$nr]['name'].'</h1></div>');
        echo('<div class="info_table"><table style="width:100px"');
        echo('<tr><td>'.$OptionList[$nr]['blocks'].'</td><td>Blocks</td></tr>');
        echo('<tr><td>'.$OptionList[$nr]['signals'].'</td><td>Signals</td></tr>');
        echo('<tr><td>'.$OptionList[$nr]['switch'].'</td><td>Switches</td></tr>');
        echo('<tr><td>'.$OptionList[$nr]['stations'].'</td><td>Stations</td></table><img src="./img/setting_cog.svg" style="width: 40px;margin: 15px auto;cursor:pointer;" onClick="LoadModule(event,'.$nr.');"/></div></div>');
      }
      ?>
      <script>
        $('.MBox .L').css("stroke","#0f0");
        $('.MBox .Sw').css("stroke","#999");
        $('.MBox .L').css("opacity","1");
      </script>
    </div>
    <div id="EModules" class="Window" style="display:block">
      <div class="tHeader">
        <div id="EMtitle">
          <span>Header</span>
          <img id="EMEditTitle_img" src="./img/pencil.png" onClick="$('#EMEditTitle').css('display','block');$('#EMEditTitle input').val($('#EMtitle span').html());"/>
          <div id="EMEditTitle" style="display:none">
            <input type="text" value="Header"/>
            <img style="right:35px" src="./img/checked.png" onClick="$('#EMtitle span').html($('#EMEditTitle input').val());;$(event.target.parentNode).css('display','none')"/>
            <img style="right:5px" src="./img/deny.png" onClick="$(event.target.parentNode).css('display','none')"/>
          </div>
        </div>
        <div style="position:absolute;left:50px;bottom:0px;height:40px;">
          <div class="tabbutton" onClick="$('#IO_Edit').css('display','block');$('#BoxConnector').css('display','none');$('#Layout').css('display','none')">
            Input & Output
          </div>
          <div class="tabbutton" onClick="if(!(tablet && !window.confirm('Not supported on touch devices!\nDo you want to continue?'))){$('#IO_Edit').css('display','none');$('#BoxConnector').css('display','block');$('#Layout').css('display','none')}">
            Connector
          </div>
          <div class="tabbutton" onClick="if(!(tablet && !window.confirm('Not supported on touch devices!\nDo you want to continue?'))){$('#IO_Edit').css('display','none');$('#BoxConnector').css('display','none');$('#Layout').css('display','block')}">
            Layout
          </div>
        </div>
        <img style="right:20px;" class="buttons" src="./img/checked.png" onClick="SaveModule();$('#EModules').css('display','none');$('#MModules').css('display','block');">
        <img style="right:90px;" class="buttons" src="./img/deny.png" onClick="$('#EModules').css('display','none');$('#MModules').css('display','block');">
      </div>
      <div id="IO_Edit" class="editBox" style="display:none">
        <script src="./scripts/EditModule_IO.js"></script>

        <div class="IO_table_wrapper blocks"><!--Blocks-->
          <div class="tableHeader red" style="position:relative">
            Blocks
            <img src="./img/plus.svg" style="width:20px;position:absolute;right:0px;margin:10px;cursor:pointer"
                    onClick="$('#EModulesBox .Block .Addr')[0].disabled = false;$('#EModulesBox .Block').css('display','block');$('#EModulesBox').css('display','block');" />
          </div>
          <div id="TableBlock" class="table">
            <div class="row header red">
              <div class="cell">IO Address</div>
              <div class="cell">ID nr</div>
              <div class="cell">Type</div>
              <div class="cell">Max speed</div>
              <div class="cell">Direction</div>
              <div class="cell">Length</div>
              <div class="cell last"></div>
            </div>
            <div class="row">
              <div class="cell">0</div>
              <div class="cell">0</div>
              <div class="cell">Rail</div>
              <div class="cell">180</div>
              <div class="cell">0</div>
              <div class="cell">100</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editBlock(event)"/></div>
            </div>

            <div class="row">
              <div class="cell">1</div>
              <div class="cell">1</div>
              <div class="cell">Turnout block</div>
              <div class="cell">180</div>
              <div class="cell">0</div>
              <div class="cell">100</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editBlock(event)"/></div>
            </div>

            <div class="row">
              <div class="cell">8</div>
              <div class="cell">2</div>
              <div class="cell">Station</div>
              <div class="cell">90</div>
              <div class="cell">0</div>
              <div class="cell">100</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editBlock(event)"/></div>
            </div>
            <div class="row">
              <div class="cell">9</div>
              <div class="cell">3</div>
              <div class="cell">Station</div>
              <div class="cell">90</div>
              <div class="cell">0</div>
              <div class="cell">100</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editBlock(event)"/></div>
            </div>

          </div>
        </div>
        <div class="IO_table_wrapper switches"><!--Switches-->
          <div class="tableHeader green" style="position:relative">
            Switches
            <object type="image/svg+xml" data="./img/plus.svg" style="width:20px;position:absolute;right:0px;margin:10px"></object>
          </div>
          <div id="TableSwitch" class="table">

            <div class="row header green">
              <div class="cell first"></div>
              <div class="cell">ID nr</div>
              <div class="cell">States</div>
              <div class="cell">IO Addresses</div>
              <div class="cell">Max speed</div>
              <div class="cell last"></div>
            </div>

            <div class="row">
              <div class="cell first">S</div>
              <div class="cell">0</div>
              <div class="cell">2</div>
              <div class="cell">[0, 1]</div>
              <div class="cell">[180, 90]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSwitch(event)"/></div>
            </div>

            <div class="row">
              <div class="cell first">S</div>
              <div class="cell">1</div>
              <div class="cell">2</div>
              <div class="cell">[2, 3]</div>
              <div class="cell">[180, 90]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSwitch(event)"/></div>
            </div>

            <div class="row">
              <div class="cell first"></div>
              <div class="cell">ID nr</div>
              <div class="cell">States</div>
              <div class="cell">IO Addresses</div>
              <div class="cell">Max speed</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSwitch(event)"/></div>
            </div>

            <div class="row">
              <div class="cell first"></div>
              <div class="cell">ID nr</div>
              <div class="cell">States</div>
              <div class="cell">IO Addresses</div>
              <div class="cell">Max speed</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSwitch(event)"/></div>
            </div>

          </div>
        </div>
        <div class="IO_table_wrapper signals"><!--Signals-->
          <div class="tableHeader Blue" style="position:relative">
            Signals
            <img src="./img/plus.svg" style="width:20px;position:absolute;right:0px;margin:10px;cursor:pointer"
                    onClick="$('#EModulesBox .Signal .Addr')[0].disabled = false;$('#EModulesBox .Signal .Addr').val(EditObj.Signals.length);$('#EModulesBox .Signal').css('display','block');$('#EModulesBox').css('display','block');" />
          </div>
          <div id="TableSignal" class="table">

            <div class="row header blue">
              <div class="cell" style="width:20px"></div>
              <div class="cell">Nr - Name</div>
              <div class="cell">Block</div>
              <div class="cell">States</div>
              <div class="cell">IO Addresses</div>
              <div class="cell last"></div>
            </div>

            <div class="row">
              <div class="cell" style="position:relative;"><img src="./../signals/1.svg" width="20px"/></div>
              <div class="cell">0 - Spoor 1</div>
              <div class="cell">5N</div>
              <div class="cell">3</div>
              <div class="cell">0 1 2</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSwitch(event)"/></div>
            </div>

          </div>
        </div>
        <div class="IO_table_wrapper stations"><!--Stations-->
          <div class="tableHeader yellow" style="position:relative">
            Stations
            <object type="image/svg+xml" data="./img/plus.svg" style="width:20px;position:absolute;right:0px;margin:10px"></object>
          </div>
          <div id="TableStation" class="table">

            <div class="row header yellow">
              <div class="cell">Name</div>
              <div class="cell">Type</div>
              <div class="cell">Nr of Blocks</div>
              <div class="cell">Block IDs</div>
              <div class="cell last"></div>
            </div>

            <div class="row">
              <div class="cell">Spoor 1</div>
              <div class="cell">Passenger</div>
              <div class="cell">4</div>
              <div class="cell">[2, 3, 4, 5]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editStation(event)"/></div>
            </div>

            <div class="row">
              <div class="cell">Spoor 2</div>
              <div class="cell">Cargo</div>
              <div class="cell">4</div>
              <div class="cell">[6, 7, 8, 9]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editStation(event)"/></div>
            </div>

            <div class="row">
              <div class="cell">Spoor 3</div>
              <div class="cell">Storage</div>
              <div class="cell">4</div>
              <div class="cell">[10, 11, 12, 13]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editStation(event)"/></div>
            </div>

            <div class="row">
              <div class="cell">Spoor 4</div>
              <div class="cell">Passenger</div>
              <div class="cell">4</div>
              <div class="cell">[14, 15, 16, 17]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editStation(event)"/></div>
            </div>

            <div class="row">
              <div class="cell">Spoor 5</div>
              <div class="cell">Passenger</div>
              <div class="cell">4</div>
              <div class="cell">[18, 19, 20, 21]</div>
              <div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editStation(event)"/></div>
            </div>

          </div>
        </div>
      </div>
      <div id="BoxConnector" class="editBox" style="display:none">
        <script src="./scripts/EditModule_Connect.js"></script>

        <div class="svgContainer" style="max-width:100%;max-height:100%">
          <svg id="ConnectBox" viewBox="0 0 1200 600" style="float:left;border:1px solid lightgrey;">
            <defs>
              <marker id="arrow" markerWidth="10" markerHeight="10" refX="0" refY="3" orient="auto" markerUnits="strokeWidth">
                <path d="M0,0 L0,6 L9,3 z" />
              </marker>
            </defs>
            <g id="BAnchorL" transform="translate(100,100)" contact="25,22.5 25,47.5">
              <path class="Container" d="M0,0 h25 a10,10 0 0 1 10,10 v50 a10,10 0 0 1 -10,10 h-25 a10,10 0 0 1 -10,-10 v-50 a10,10 0 0 1 10,-10 z" style="fill:#aaa;"/>
              <path class="A APoint" d="M25,15 h10 v15 h-10 a 5,5 0 0 1 -5,-5 v-5 a 5,5 0 0 1 5,-5 z" style="cursor:crosshair" connected="L0_1" onMouseDown="startDraw(event)"/>
              <path class="B APoint" d="M25,40 h10 v15 h-10 a 5,5 0 0 1 -5,-5 v-5 a 5,5 0 0 1 5,-5 z" style="cursor:crosshair" connected="" onMouseDown="startDraw(event)"/>
              <circle cx="5" cy="35" r="5" style="cursor:move" onmousedown="StartMove(event);"/>
            </g>
            <g id="BAnchorR" transform="translate(300,100)" contact="-10,22.5 -10,47.5">
              <path class="Container" d="M0,0 h25 a10,10 0 0 1 10,10 v50 a10,10 0 0 1 -10,10 h-25 a10,10 0 0 1 -10,-10 v-50 a10,10 0 0 1 10,-10 z" style="fill:#aaa;"/>
              <path class="A APoint" d="M-10,15 h10 a5,5 0 0 1 5,5 v5 a5,5 0 0 1 -5,5 h-10 v-15 z" style="cursor:crosshair" connected="" onMouseDown="startDraw(event)"/>
              <path class="B APoint" d="M-10,40 h10 a5,5 0 0 1 5,5 v5 a5,5 0 0 1 -5,5 h-10 v-15 z" style="cursor:crosshair" connected="" onMouseDown="startDraw(event)"/>
              <circle cx="20" cy="35" r="5" style="cursor:move" onmousedown="StartMove(event);"/>
            </g>
            <g id="BBlock1" transform="translate(200,100)" contact="0,17.5 35,17.5">
              <path class="BAnchor" d="M0,0 h35 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-35 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z" style="fill:#aaa;"/>
              <path class="P APoint" d="M0,0 h5 v35 h-5 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z" style="fill:#000;cursor:crosshair;" onMouseDown="startDraw(event)" connected="L0_2"/>
              <path class="N APoint" d="M30,0 h5 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-5 z" style="fill:#000;cursor:crosshair;" onMouseDown="startDraw(event)" connected=""/>
              <text text-anchor="middle" x="17.5" y=22.5 style="line-height:5px;font-weight:bold">1</text>
              <path d="M5,0 h25 v35 h-25 v-35 z" style="fill:rgba(0,0,0,0);cursor:move" onmousedown="StartMove(event);"/>
              <line x1="10" y1="30" x2="20" y2="30" stroke="#000" stroke-width="1" marker-end="url(#arrow)" />
            </g>
            <g id="Switch1" transform="translate(200,300)" contact="0,17.5 35,17.5">
              <path class="BAnchor" d="M0,0 h35 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-35 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z" style="fill:#aaa;"/>
              <path class="P APoint" d="M0,0 h5 v35 h-5 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z" style="fill:#000;cursor:crosshair;" onMouseDown="startDraw(event)" connected="L0_2"/>
              <path class="N APoint" d="M30,0 h5 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-5 z" style="fill:#000;cursor:crosshair;" onMouseDown="startDraw(event)" connected=""/>
              <text text-anchor="middle" x="17.5" y=22.5 style="line-height:5px;font-weight:bold">1</text>
              <path d="M5,0 h25 v35 h-25 v-35 z" style="fill:rgba(0,0,0,0);cursor:move" onmousedown="StartMove(event);"/>
              <line x1="10" y1="30" x2="20" y2="30" stroke="#000" stroke-width="1" marker-end="url(#arrow)" />
            </g>
            <line id="L0" x1="125" y1="122.5" x2="202.5" y2="117.5" style="stroke:black;stroke-width:2px"/>
          </svg>
        </div>
        Connector
      </div>
      <div id="Layout" class="editBox" style="display:block;">
        <script src="./scripts/EditModule_Layout.js"></script>
        <div id="LayoutContainer">
         <svg width="2101" height="841" viewBox="0 0 2101 841" xmlns="http://www.w3.org/2000/svg">
           <defs>
              <pattern id="smallGrid" width="21" height="21" patternUnits="userSpaceOnUse">
                <path class="grid_line" d="M 50 0 L 0 0 0 50" fill="none" stroke="gray" stroke-width="0.25"/>
              </pattern>
              <pattern id="mediumGrid" width="42" height="42" patternUnits="userSpaceOnUse">
                <rect id="small_grid" width="42" height="42" fill="url(#smallGrid)" style="display:none"/>
                <path class="grid_line" d="M 50 0 L 0 0 0 50" fill="none" stroke="gray" stroke-width="0.5"/>
              </pattern>
              <pattern id="grid" width="1050" height="420" patternUnits="userSpaceOnUse">
                <rect width="1050" height="420" fill="url(#mediumGrid)"/>
                <path class="grid_line" d="M 1050 0 L 0 0 0 420" fill="none" stroke="gray" stroke-width="1"/>
              </pattern>
            </defs>
            <rect class="grid" width="100%" style="height:100%;" fill="url(#grid)" onClick=""/>
            <g id="Rail"></g>
            <g id="Signals"></g>
            <g id="Nodes"></g>
            <g transform="translate(100,100)" id="toolbar">
              <path d="M 0,15 a 15,15 1,0,1 15,-15 h 700 a 15,15 1,0,1 15,15 v 30 a 15,15 1,0,1 -15,15 h -700 a 15,15 1,0,1 -15,-15 Z" style="stroke:lightgrey;fill:lightgrey;"/>
              <path class="drag_handle" d="M 10,15 a 5,5 1,0,1 5,-5 h 15 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -15 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;cursor:move;"/>

              <g class="tool layout_warning_list" transform="translate(40,5)">
                <path d="M0,5A5,5,0,0,1,5,0H45a5,5,0,0,1,5,5h0V45a5,5,0,0,1-5,5H5a5,5,0,0,1-5-5H0Z" style="fill:#ddd;stroke-width:0"/>
                <path d="M45,38.53,27.14,8.89a2.5,2.5,0,0,0-4.27,0L5,38.53a2.5,2.5,0,0,0,2.14,3.78H42.87A2.5,2.5,0,0,0,45,38.53ZM25,17.94
                          a1.69,1.69,0,0,1,1.89,1.61c0,3.13-.37,7.63-.37,10.76,0,.82-.89,1.16-1.53,1.16-.84,0-1.55-.34-1.55-1.16,0-3.13-.37-7.63-.37-10.76
                          C23.09,18.52,23.93,17.94,25,17.94Zm0,19.39a2,2,0,1,1,2-2A2.06,2.06,0,0,1,25,37.33Z" style="fill:#d80027;stroke-width:0"/>
                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 45 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#afafaf;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Warning List</text>
                  <text x="25" y="95" text-anchor="middle" style="font-family:sans-serif;fill:white;">Missing <tspan style="font-weight:bold;fill:red">3</tspan> Switches</text>
                </g>
              </g>

              <g class="tool" part="BEC" transform="translate(100,5)" style="cursor:move;"><!-- Block End Connector -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <circle cx="15" cy="25" r="5" style="fill:black;stroke-width:0"/>
                <circle cx="30" cy="15" r="3" style="fill:blue;stroke-width:0"/>
                <circle cx="30" cy="35" r="3" style="fill:blue;stroke-width:0"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Block End Connector</text>
                </g>
              </g>
              <g class="tool" part="BI" transform="translate(160,5)" style="cursor:move;"><!-- Block Isolation -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <circle cx="25" cy="25" r="5" style="fill:orange;stroke-width:0"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Block Isolation</text>
                </g>
              </g>
              <g class="tool" part="SwN" transform="translate(220,5)" style="cursor:move;"><!-- Switch Node -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <path d="M 25,17.5 v 15 l -7.5,-7.5 Z" style="fill:#41b7dd;stroke-width:0"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Switch Node</text>
                </g>
              </g>
              <g class="tool" part="MAN" transform="translate(280,5)" style="cursor:move;"><!-- Module Attach Node -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <path d="M 20,20 h 5 a 5,5 0,0,1 0,10 h -5 Z" style="fill:#ec00ff;stroke-width:0"/>
                <path d="M 25,5 v 40" style="fill:black;stroke-width:2px;"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Module Attach Node</text>
                </g>
              </g>
              <g class="tool" part="RS" transform="translate(340,5)" style="cursor:move;"><!-- Straight Rail -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <path d="M 5,25 h 40" style="stroke-width:6px;stroke:black;"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Straight Rail</text>
                </g>
              </g>
              <g class="tool" part="RC" transform="translate(400,5)" style="cursor:move;"><!-- Curve Rail -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <path d="M 5,25 a 56.33,56.33 0,0,0 40,-15.5" style="stroke-width:6px;stroke:black;"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Curve Rail</text>
                </g>
              </g>
              <g class="tool" part="S3"transform="translate(460,5)" style="cursor:move;"><!-- 3 Light Signal -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <path d="M 40,20 v 10 a 5,5 0,0,1 -10,0 v -10 a 5,5 0,0,1 10,0" style="stroke-width:0px;stroke:black;fill:black;"/>
                <circle cx="35" cy="20" r="2" style="stroke-width:0px;fill:red;"/>
                <circle cx="35" cy="25" r="2" style="stroke-width:0px;fill:orange;"/>
                <circle cx="35" cy="30" r="2" style="stroke-width:0px;fill:lime;"/>

                <circle cx="25" cy="10" r="2" style="stroke-width:0px;fill:purple;"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Generic 3 Light Signal</text>
                </g>
              </g>
              <g class="tool" part="S2" transform="translate(520,5)" style="cursor:move;"><!-- 2 Light Signal -->
                <path d="M 0,5 a 5,5 1,0,1 5,-5 h 40 a 5,5 1,0,1 5,5 v 40 a 5,5 1,0,1 -5,5 h -40 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:#ddd;"/>
                <path d="M 40,25 v 5 a 5,5 0,0,1 -10,0 v -5 a 5,5 0,0,1 10,0" style="stroke-width:0px;stroke:black;fill:black;"/>
                <circle cx="35" cy="25" r="2" style="stroke-width:0px;fill:red;"/>
                <circle cx="35" cy="30" r="2" style="stroke-width:0px;fill:lime;"/>

                <circle cx="25" cy="10" r="2" style="stroke-width:0px;fill:purple;"/>

                <g class="tooltip" style="cursor:default;">
                  <path d="M -80,56 a 5,5 1,0,1 5,-5 h 96 l 4,-4 l 4,4 h 96 a 5,5 1,0,1 5,5 v 30 a 5,5 1,0,1 -5,5 h -200 a 5,5 1,0,1 -5,-5 Z" style="stroke-width:0;fill:grey;"/>
                  <text x="25" y="75" text-anchor="middle" style="font-family:sans-serif;font-weight:bold;fill:white;">Generic 2 Light Signal</text>
                </g>
              </g>
            </g>
            <g id="Drawing"></g>
          </svg>
          <script>
            $("#LayoutContainer #toolbar path.drag_handle").on("mousedown",Layout_dragToolbar);
            $("#LayoutContainer #toolbar g.tool:not(:first)").on("mousedown",Layout_createPart);
            $('#LayoutContainer svg').on("contextmenu",Layout_ContextMenu);
          </script>
          <div id="LayoutContextMenu" style="display:none;left:0px;top:0px">
            <div class="header">Block</div>
            <div class="content"></div>
          </div>
        </div>
      </div>
    </div>
    <div id="EModulesBox" style="display:none;z-index:5;">
      <div class="Block" style="width:100%;height:100%;display:none">
        <div id="TableEditBlock" class="table">
          <div class="row">
            <div class="cell" style="width:50%">Address</div>
            <div class="cell" style="width:50%"><input class="Addr" type="number" style="width:100%" /></div>
          </div>
          <div class="row">
            <div class="cell">Input pin Address</div>
            <div class="cell"><input class="IOAddr" type="number" style="width:100%" /></div>
          </div>
          <div class="row">
            <div class="cell">Type</div>
            <div class="cell"><select class="Type" style="width:100%"><option>Free Track</option><option>Station</option><option>Shunt</option><option>Siding</option><option>Special Detection</option></select></div>
          </div>
          <div class="row">
            <div class="cell">Speed options</div>
            <div class="cell">
              <span>Maximum <input class="MaxSPD" type="number" style="width:100%" /></span>
              <span>Expected Stop <input class="MaxSPD" type="number" style="width:100%" /></span>
              <span>Restricted <input class="MaxSPD" type="number" style="width:100%" /></span>
              <span>Shunting <input class="MaxSPD" type="number" style="width:100%" /></span>
            </div>
          </div>
          <div class="row">
            <div class="cell">Direction</div>
            <div class="cell"><select class="Dir" style="width:100%"><option>Clockwise (-&gt;)</option><option>CounterClockwise (&lt;-)</option><option>Switching</option></select></div>
          </div>
          <div class="row">
            <div class="cell">Length</div>
            <div class="cell"><input class="Len" type="number" style="width:100%" /></div>
          </div>
          <div class="row">
            <div class="cell">OneWay</div>
            <div class="cell"><input class="OneWay" type="checkbox" /></div>
          </div>
        </div>
        <img class="button" style="left:calc(50% - 75px);cursor:pointer;" src="./img/checked.png" onClick="if(SaveEditBlock(event)){$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none')};"/>
        <img class="button" style="right:calc(50% - 75px);cursor:pointer;" src="./img/deny.png" onClick="$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none');"/>
        <img class="button" style="right:calc(50% - 25px);cursor:pointer;" src="./img/bin_g.png" onClick="if(DeleteBlock(event)){$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none')};"/>
      </div>
      <div class="Switch" style="width:100%;height:100%;display:none">
        Switch
        <img class="button" style="left:calc(50% - 50px)" src="./img/checked.png" />
        <img class="button" style="right:calc(50% - 50px)" src="./img/deny.png" onClick="$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none');"/>
      </div>
      <div class="Signal" style="width:100%;height:100%;display:none">
        <center><h3>Add/Edit Signal</h3></center>
        <div id="TableEditSignal" class="table">
          <div class="row">
            <div class="cell" style="width:50%">Name</div>
            <div class="cell" style="width:50%"><input class="Addr" style="display:none" type="text"/><input class="Name" type="text" style="width:100%" /></div>
          </div>
          <div class="row">
            <div class="cell" style="vertical-align:middle">Type</div>
            <div class="cell type">
              <div style="float:left;width:20px;height:40px;margin:5px;text_align:center;position:relative;">
                <input style="margin: 0px 3.5px" type="radio" name="type" value="1"/><br />
                <img src="./img/Signals/1.svg" style="width:20px;height:20px;position:absolute;top:20px;left:0px"/>
              </div>
              <div style="float:left;width:20px;height:40px;margin:5px;text_align:center;position:relative;">
                <input style="margin: 0px 3.5px" type="radio" name="type" value="2"/><br />
                <img src="./img/Signals/2.svg" style="width:20px;height:20px;position:absolute;top:20px;left:0px"/>
              </div>
              <div style="float:left;width:20px;height:40px;margin:5px;text_align:center;position:relative;">
                <input style="margin: 0px 3.5px" type="radio" name="type" value="3"/><br />
                <img src="./img/Signals/3.svg" style="width:20px;height:20px;position:absolute;top:20px;left:0px"/>
              </div>
              <div style="float:left;width:20px;height:40px;margin:5px;text_align:center;position:relative;">
                <input style="margin: 0px 3.5px" type="radio" name="type" value="4"/><br />
                <img src="./img/Signals/4.svg" style="width:20px;height:20px;position:absolute;top:20px;left:0px"/>
              </div>
              <div style="float:left;width:20px;height:40px;margin:5px;text_align:center;position:relative;">
                <input style="margin: 0px 3.5px" type="radio" name="type" value="x"/><br />
                <span style="width: 30px;height: 20px;margin-left: -5px;margin-right: -5px;position: absolute;text-align: center;top: 20px;left: 0px;font-size: 10px;">Other</span>
              </div>
            </div>
          </div>
          <div class="row">
            <div class="cell" style="vertical-align:middle">Block ID + Side</div>
            <div class="cell">
              <div style="width:55px;float:left;"><input style="margin:5px" type="radio" name="side" value="P"/>Prev</div>
              <div style="width:55px;float:right;">Next<input style="margin:5px" type="radio" name="side" value="N"/></div>
              <div style="margin:auto;margin-top:20px;width:50%"><input type="number" class="S_block" style="width:100%;text-align:center"/></div>
            </div>
          </div>
        </div>
        <div class="IO_Wrapper">
          <div class="states">
            <div class="header">States</div>
            <ul>
              <li class="green">Free</li>
              <li class="orange">Ready to stop</li>
              <li class="red">Stop</li>
              <li class="yellow">Slow-free</li>
              <li class="dred">Yard</li>
              <!--<li class="red">C</li>-->
            </ul>
          </div>
          <div class="IOEn">
            <div class="header">Enable</div>
            <span><input type="checkbox"/></span>
            <span><input type="checkbox"/></span>
            <span><input type="checkbox"/></span>
            <span><input type="checkbox"/></span>
            <span><input type="checkbox"/></span>
          </div>
          <div class="IOACon" style="width:120px;">
            <div class="header">IO</div>
            <div class="IOAddr first">
              <div><input type="text"/></div>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
            </div>
            <div class="IOAddr">
              <div><input type="text"/></div>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
              <span><input type="checkbox"/></span>
            </div>
            <div class="IOAddr Add">
              <div><img src="./img/plus.png" style="margin-top:2px;height:16px;cursor:pointer"/></div>
            </div>
          </div>
        </div>
        <img class="button" style="left:calc(50% - 75px);cursor:pointer;" src="./img/checked.png" onClick="if(SaveEditSignal(event)){$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none')};"/>
        <img class="button" style="right:calc(50% - 75px);cursor:pointer;" src="./img/deny.png" onClick="$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none');"/>
        <img class="button" style="right:calc(50% - 25px);cursor:pointer;" src="./img/bin_g.png" onClick="if(DeleteSignal(event)){$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none')};"/>
      </div>
      <div class="Station" style="width:100%;height:100%;display:block">
        <center><h3>Add/Edit Station</h3></center>
        <div id="TableEditStation" class="table">
          <div class="row">
            <div class="cell" style="width:50%">Name</div>
            <div class="cell" style="width:50%"><input class="Addr" style="display:none" type="text"/><input class="Name" type="text" style="width:100%" /></div>
          </div>
          <div class="row">
            <div class="cell" style="width:50%">Type</div>
            <div class="cell" style="width:50%"><select class="Type"><option>None</option><option>Passengers</option><option>Cargo</option><option>Both</option></select></div>
          </div>
          <div class="row">
            <div class="cell" style="width:50%;vertical-align: middle;">Blocks</div>
            <div class="cell StationBlocks" style="width:50%"></div>
          </div>
        </div>
        <img class="button" style="left:calc(50% - 75px);cursor:pointer;" src="./img/checked.png" onClick="if(SaveEditStation(event)){$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none')};"/>
        <img class="button" style="right:calc(50% - 75px);cursor:pointer;" src="./img/deny.png" onClick="$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none');"/>
        <img class="button" style="right:calc(50% - 25px);cursor:pointer;" src="./img/bin_g.png" onClick="if(DeleteStation(event)){$('#EModulesBox > div').css('display','none');$('#EModulesBox').css('display','none')};"/>
      </div>
    </div>
    <div id="index"    class="Window" style="display:none">
      <div class="button" onClick="mTrain_win()">
        <h1>Manage trains</h1><br/>
        <img src="./img/list.png" width="200px" style="position:absolute;left:calc(50% - 100px);bottom:20px;"/>
      </div>
      <div class="button" onClick="mModules_win()">
        <h1>Manage Modules</h1><br/>
        <?php include("./img/Module.svg"); ?>
      </div>
      <div class="button" onClick="cTrain_win()">
        <h1>Remote control</h1>
        <img src="./img/remote-control.png" width="240px" style="position:absolute;left:calc(50% - 120px);bottom:20px;"/>
      </div>
      <div class="button" onClick="track_win()">
        <h1>Track</h1>
        <img src="./img/rail.png" height="64px" style="position:absolute;left:calc(50% - 150px);bottom:20px;"/>
      </div>
      <div class="button" onClick="switch_win()">
        <h1>Switches</h1>
        <img src="./img/switch.png" height="106px" style="position:absolute;left:calc(50% - 150px);bottom:20px;"/>
      </div>
      <br style="clear:both; display:block; height:1px; margin:-1px 0 0 0;"/>
    </div>

	  <div id="Station_List" style="display:none">
  		<h1 style="display:block;margin-bottom:10px;">Route</h1>
  		<span style="margin-bottom: 5px;display: block;">Route train <input type="text" style="width:40px"/> to</span>
  		<select></select><br/><br/>
  		<div id="Route_Button" style="margin:auto;position:relative;top:6px" onClick="train_set_route(this);">
        <b>Start</b>
      </div>
  	</div>
    <div id='Train_Add' style="display:none">
      <h3 style="text-align:center;">New train</h3>
      <form id="upload_form" enctype="multipart/form-data" method="post">
        <table>
          <tr>
            <td style="border-top:1px solid black;" colspan="2">
              Select image (jpg only) to upload:
              <input type="file" name="file1" id="file1">
            </td>
          </tr>
          <tr>
            <td>Name</td>
            <td>
              <input type="text" id="train_name" value="Name" name="Name" maxlength="20"/>
            </td>
          </tr>
          <tr>
            <td>DCC Address</td>
            <td>
              <input type="number" id="train_dcc" value="<?php echo count($var4); ?>" name="DCC" min="1" max="9999" maxlength="4"/>
            </td>
          </tr>
          <tr>
            <td>Type</td>
            <td>
              <select name="type" id="train_type">
                <option value="P">Passenger</option>
                <option value="C">Cargo</option>
              </select>
            </td>
          </tr>
          <tr>
            <td>Max speed (km/h)</td>
            <td>
              <input type="number" id="train_speed" value="160" name="speed"/>
            </td>
          </tr>
          <tr>
            <td colspan="2">
              <center><b>Functions</b></center>
              <script>
                function checkbox_handler(object){
                  if(object.checked){
                    console.log("Checked");
                    console.log($(object).next());
                    $(object).next().removeAttr("disabled");
                  }else{
                    console.log("UnChecked");
                    $(object).next().attr("disabled","");
                  }
                }
              </script>
              <?php
                function dropdown($i){
                  if($i <= 28){
                    return '<input type="checkbox" name="C'.$i.'" onchange="checkbox_handler(this)"><select name="T'.$i.'" disabled="">
                              <option value="volvo" selected>Default</option>
                              <option value="L">Light</option>
                              <option value="C">Cabine Light</option>
                              <option value="S">Sound</option></select>';
                  }
                }
              ?>
              <table style="width:100%">
                <tr>
                  <td style="width:10%"></td>
                  <td style="width:30%">0</td>
                  <td style="width:30%">10</td>
                  <td style="width:30%">20</td>
                </tr>
                <?php
                  for($i = 0;$i<10;$i++){
                    echo '<tr>
                      <td>'.$i.'</td>
                      <td>'.dropdown($i+00).'</td>
                      <td>'.dropdown($i+10).'</td>
                      <td>'.dropdown($i+20).'</td>
                    </tr>';
                  }
                ?>
              </table>
            </td>
          </tr>
          <tr>
            <td colspan="2" style="border-bottom:none;">
              <img src="./img/checked.png" id="upload" style="margin-top:20px" width="32px" onClick="/*uploadFile();*/add_train();"/>
              <img src="./img/deny.png" style="margin-top:20px;margin-left:20px;cursor:pointer" width="32px" onClick="$('#Train_Add').css('display','none')"/>
            </td>
          </tr>
        </table>
      </form>
    </div>
    <div id='Settings_box' style="display:none;z-index:5;">
      <h3 style="text-align:center;margin-bottom:20px">Settings</h3>
      <form id="upload_form" enctype="multipart/form-data" method="post" style="text-align:left">
        <table>
          <tr style="border-top:1px solid black;height:50px;">
            <td style="width:75%">
              <b>Track-control</b><br/>
              Analog (DC) or Digital (DCC)
            </td>
            <td>
              <img id="digital" class="test notConnected" src="./img/Digital_n.svg" height="24px" style="float:right;margin:8px;margin-right:20px;" title="Switch to DCC"/>
            </td>
          </tr>
          <tr style="height:50px">
            <td>
              <b>Track zoom</b>
            </td>
            <td>
              <img src="./img/plus.png" onMouseenter="$(this).attr('src','./img/plus_h.png');" onMouseleave="$(this).attr('src','./img/plus.png');" height="25px" style="cursor:pointer" onClick="zoom('+');"/>
              &nbsp;&nbsp;&nbsp;&nbsp;
              <img src="./img/minus.png" onMouseenter="$(this).attr('src','./img/minus_h.png');" onMouseleave="$(this).attr('src','./img/minus.png');" height="25px" style="cursor:pointer" onClick="zoom('-');"/>
              <!--
                        $("#track").css("height","calc(100% - 20px)");
                        $("#track").css("zoom","200%");
              -->
            </td>
          </tr>
              <?php
              $side = 1;
              $handle = fopen("./../settings.h", "r");
              if ($handle) {
                while (($line = fgets($handle)) !== false) {
                  if($line[0] == '/'){
                    if($line[1] == '*'){
                      echo("</table><table style=\"margin-top:10px;");
                      if($side == 0){
                        echo("float:left\"");
                        $side = 1;
                      }else{
                        echo("float:right\"");
                        $side = 0;
                      }
                      echo("><tr><td colspan=\"2\"><h2>".substr($line,6)."</h2></td></tr>");
                    }else{
                      //if($line[])
                      echo("<tr><td style=\"width:75%;text-align:right\"><b>".substr($line,2)."</b></td>");
                    }
                  }else if($line[0] == '#'){
                    $p = substr($line,strpos($line,' ',strpos($line,' ')+1));
                    echo("<td>".$p."</td></tr>");
                  }
                }
                echo("</table><table>");

                fclose($handle);
              } else {
                // error opening the file.
              }
              ?>
            <td colspan="2" style="border-bottom:none;">
              <img src="./img/checked.png" style="margin-top:20px;cursor:pointer" width="32px" onClick="settings();"/>
            </td>
          </tr>
        </table>
      </form>
    </div>
    <div id='Train_Settings_box' style="display:none;z-index:5;">
      <h1>Train Setting</h1>
      <h2>Train Setting</h2>
      <img id="train_set_box_img" src="./../trains/empty.jpg" style="max-width:100%"/>
      <img src="./img/checked.png" id="Tedit" style="margin-top:20px" width="32px" onClick="/*uploadFile();add_train();*/"/>
      <img src="./img/deny.png" style="margin-top:20px;margin-left:20px;cursor:pointer" width="32px" onClick="$('#Train_Settings_box').css('display','none')"/>
    </div>
    <script src="./scripts/framework/classie.js"></script>
		<script src="./scripts/framework/selectFx.js"></script>
		<script>
			function redraw_selects(data) {
        var el = document.querySelector(data + ' select.cs-selectn');
        console.log(el);
				new SelectFx(el);
			};
		</script>
    <script src="./scripts/script.js">
    </script>
  </body>
</html>
