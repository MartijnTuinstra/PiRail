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
    <script src="./jquery-3.1.1.min.js"></script>
    <script src="./jquery-ui.min.js"></script>
    <script src="./jquery.ui.touch-punch.min.js"></script>
    <link rel="stylesheet" type="text/css" href="./jquery-ui.min.css">
    <link rel="stylesheet" type="text/css" href="./main.css">
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

      echo("/*");
      var_dump($var4);
      echo("*/");
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

		<link rel="stylesheet" type="text/css" href="./cs-select.css" />
		<link rel="stylesheet" type="text/css" href="./cs-skin-border.css" />
    <script>
      $.support.cors = true;
      var pause = 1;
      var menu = 0;
      var time_addr;
      var tablet = 0;
      var myWindow;
      var index_w = 1;
      var switch_w = 0;
      var mTrain_w = 0;
      var cTrain_w = 0;
      var track_w = 0;

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

      function menu_but(){
        if(menu == 1){
          $('#menu').css('display','none');
          menu = 0;
        }else{
          $('#menu').css('display','block');
          menu = 1;
        }
      }

      function switch_win(){
        $('#index').css('display','none');
        $('#MModules').css('display','none');
        $('#switches').css('display','block');
        $('#track').css('display','none');
        $('#MTrain').css('display','none');
        $('#CTrain').css('display','none');
      }

      function track_win(){
        $('#index').css('display','none');
        $('#MModules').css('display','none');
        $('#track').css('display','block');
        $('#switches').css('display','none');
        $('#MTrain').css('display','none');
        $('#CTrain').css('display','none');
      }

      function mTrain_win(){
          $('#index').css('display','none');
          $('#MTrain').css('display','block');
          $('#MModules').css('display','none');
          $('#switches').css('display','none');
          $('#track').css('display','none');
          $('#CTrain').css('display','none');
      }

      function cTrain_win(){
        $('#index').css('display','none');
        $('#CTrain').css('display','block');
        $('#MModules').css('display','none');
        $('#switches').css('display','none');
        $('#track').css('display','none');
        $('#MTrain').css('display','none');
      }

      function index_win(){
        $('#index').css('display','block');
        $('#MModules').css('display','none');
        $('#CTrain').css('display','none');
        $('#switches').css('display','none');
        $('#track').css('display','none');
        $('#MTrain').css('display','none');
      }

      function mModules_win(){
        $('#MModules').css('display','block');
        $('#index').css('display','block');
        $('#CTrain').css('display','none');
        $('#switches').css('display','none');
        $('#track').css('display','none');
        $('#MTrain').css('display','none');
      }


      function settings(){
        if($('#Settings_box').css('display') == 'none'){
          $('#Settings_box').css('display','block');
        }else{
          $('#Settings_box').css('display','none');
        }
      }

      function warning_toggle(){
        if($('#warning_list').css('display') == 'none' && $("#warning_list").html() != ""){
          $('#warning_list').css('display','block');
        }else{
          $('#warning_list').css('display','none');
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
        if(ws.readyState == 3){
          $('#Train_Add #upload').attr('src','./img/checked_n.png');
          $('#Train_Add #upload').css('cursor','not-allowed');
          $('#Train_Add #upload').attr('onClick','');
        }else{
          $('#Train_Add #upload').attr('src','./img/checked.png');
          $('#Train_Add #upload').css('cursor','pointer');
          $('#Train_Add #upload').attr('onClick','add_train()');
        }
      }

      function add_train(){
        if ($('#Train_Add #train_name').val().indexOf(':') > -1) {
          alert("Name cannot contain a ':'");
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
          ws.send('RNt'+$('#train_type').val()+$('#train_name').val()+':'+$('#train_dcc').val()+':'+$('#train_speed').val());
          addingTrain = 1;
          console.log("Requesting");
        }
      }

      function succ_add_train(number){
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
        setTimeout(function () {
      	  $("#Train_Add").css('display','none');
      	  $("#Train_Add  #upload").attr('onClick','add_train()');
          var text = "<tr><td><img src=\"./../trains/"+data[0]+".jpg\" style=\"max-height:50px;max-width:100px;float:right;\"/></td><td>";
          text    += data[1]+"</td><td>"+data[2]+"</td><td>"+data[3]+"</td><td>"+data[4]+"</td><td><img src=\"./img/bin_w.png\" style=\"width:20px;\"/></td></tr>"
          $("#list").append(text);
        }, 500);
      }

      function errorHandler(event){
      	alert("Upload Failed");
      }

      function abortHandler(event){
      	alert("Upload Aborted");
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
        <li onClick="cTrain_win();menu_but()">Control trains</li>
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
      <div class="MBox">
        <?php include("./../modules/3.svg"); ?>
        <div style="width:500px;float:left;">
          <h1>Module 1</h1><br/>
          15 Blocks <br/>
          4 Signals<br/>
          20 Switches<br/>
          5  Stations<br/>
        </div>
      </div>
      Test Manage Modules
    </div>
    <div id="index"    class="Window" style="display:block">
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
		<select></select></br></br>
		<div id="Route_Button" style="margin:auto;position:relative;top:6px" onClick="train_set_route(this);"><b>Start</b></div>
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
    <script src="./classie.js"></script>
		<script src="./selectFx.js"></script>
		<script>
			function redraw_selects(data) {
        var el = document.querySelector(data + ' select.cs-selectn');
        console.log(el);
				new SelectFx(el);
			};
		</script>
    <script src="./script.js">
    </script>
  </body>
</html>
