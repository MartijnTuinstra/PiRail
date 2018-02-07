<?php
  $data = file_get_contents("./../setup.json");
  $data = json_decode($data);
  date_default_timezone_set("Europe/Amsterdam");
?><html>
  <head>
    <title>TEST</title>
    <meta id="vp" name="viewport" content="width=device-width, initial-scale=0.7">
    <meta name="mobile-web-app-capable" content="yes">
    <link rel="manifest" href="./manifest.json">
    <script src="./jquery-3.1.1.min.js"></script>
    <script src="./jquery-ui.min.js"></script>
    <style>
      .B {
        fill:none;
        stroke:#000000;
        stroke-width:10px;
        stroke-linecap:butt;
        stroke-linejoin:round;
        stroke-dasharray:none
      }
      .L {
        fill:none;
        stroke-width:4px;
        stroke-linecap:butt;
        stroke-linejoin:round;
        stroke-dasharray:none
      }
      .W {
        fill:none;
        stroke:#FFFFFF;
        stroke-width:4px;
        stroke-linecap:butt;
        stroke-linejoin:round;
        stroke-dasharray:none
      }
      * {
        margin: 0px;
        padding: 0px;
      }
      body,html {
        width:100%;
        height:100%;
	      overflow: none;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        cursor: default;
      }

      body {
        min-width:550px; /* suppose you want minimun width of 1000px */
         width: auto !important;  /* Firefox will set width as auto */
         width:550px
      }

      ul {
	       margin-left:20px;
      }

      input[type=range][orient=vertical] {
        writing-mode: bt-lr; /* IE */
        -webkit-appearance: slider-vertical; /* WebKit */
        width: 8px;
        height: 175px;
        padding: 0 5px;
      }

      h1, h2 {
          display: inline-block;
      }

      #clock {
        position:absolute;
        left:calc(50% - 50px);
        width: 100px;
        text-align: center;
        color:white;
      }

      #menu_button {
        margin:6px;
        margin-right:20px;
        width:16px;
        height:16px;
        padding:6px;
        float:right;
        border-radius: 18px;
        cursor: pointer;
      }

      #menu {
        float: right;
        width: 300px;
        height: 259px;
        position: absolute;
        right: 10px;
        top: 50px;
        background-color: grey;
        z-index: 20;
        border-radius: 5px;
      }

      #menu ul{
        margin:0px;
      }

      #menu li {
        text-align: center;
      	font-size: large;
       	padding: 15px;
      	border-bottom: 1px solid black;
        list-style-type: none;
        margin: 0px;
        cursor: pointer;
      }

      #menu li:hover {
        background-color: #aaa;
      }

      #trains, #switches {
        float: right;
        position: absolute;
        right: calc(50% - 250px);
        top: 200px;
        width: 50%;
        height: 160px;
        max-height: 160px;
        background-color: #ccc;
        z-index: 10;
        padding-top: 20px;
      }

      #trains {
        top:400px;
      }

      #trains .box, #switches .box{
        overflow-x: scroll;
        width: 100%;
        height: calc(100% - 20px);
        position: absolute;
      }

      .switchbox {
        height: 100%;
        width: 70px;
        display: inline-block;
        padding: 0px 5px;
        border-right: 1px solid black;
        white-space: normal;
        font-family: sans-serif;
        float: left;
      }

      .switchbox svg{
        margin-bottom: 5px;
      }

      #switches .box .big {
        width: 140px;
      }

      #switches .box svg {
        margin-bottom: 0px;
      }

      .trainbox {
        height: 100%;
        width: 200px;
        display: inline-block;
        padding: 0px 5px;
        border-right: 1px solid black;
        white-space: normal;
        font-family: sans-serif;
        float: left;
      }

      .train_image_container {
        width: 120px;
        height:50px;
        text-align: center;
        float:left;
      }

      .train_speed_container {
        height: calc(100% - 16px);
        width: 40px;
        padding: 8px;
        float:left;
        text-align: center;
      }

      .train_idbox {
        height: calc(100% - 60px);
        padding-top: 5px;
        padding-bottom: 5px;
        width: 120px;
        float:left;
        position:absolute;
        top: 47px;
        text-align: center;
        font-size: small;
      }

      .drag {
        width: calc(100% - 20px);
        padding-left: 20px;
        height: 20px;
        top: 0px;
        position: absolute;
        background-color: #999;
        font-family: sans-serif;
        cursor: move;
      }

      #trains .content, #switches .content{
        height: 100%;
        white-space: nowrap;
      }

      .Module{
        position: absolute;
      }
      .Switch {
        cursor: pointer;
        opacity: 0;
      }

      #warning_list {
        position:absolute;
        top:40px;
        left:calc(50% - 275px);
        width:500px;
        max-height: calc(100% - 40px);
        z-index:40;
        font-family:sans-serif;
        color:white;
        overflow-y: overlay;
      }

      .warning_list_active {
        padding: 0px 25px 200px 25px;
      }

      .warning_list_nactive {
        padding: 0px 25px;
      }

      .warning {
        position:relative;
        width:470px;
        min-height:30px;
        border-radius:10px;
        border:2px solid black;
        background-color:red;
        margin: 20px auto;
        padding:15px
      }

      .warning .photobox {
        width:30px;height:30px
      }

      .warning .l {
        float: left;
      }

      .warning .r {
        float: right;
      }

      .warning img {
        max-height:100%;max-width:100%
      }

      .warning h2 {
        margin-bottom: 10px;
      }

      .warning but {
        padding:4px;
        border-radius:4px;
        background-color: #000;
        cursor: pointer;
      }

      .warning but:hover {
        background-color:#fff;
        color: black;
      }

      #Window {
      	height: calc(100% - 40px);
      	overflow: auto;
      	position: relative;
      }

      #Modules{
        position: relative;
      }
    </style>
		<!--<link rel="stylesheet" type="text/css" href="./SelectInspiration/css/normalize.css" />
		<link rel="stylesheet" type="text/css" href="./SelectInspiration/css/demo.css" />-->
		<link rel="stylesheet" type="text/css" href="./cs-select.css" />
		<link rel="stylesheet" type="text/css" href="./cs-skin-border.css" />
    <script>
      $.support.cors = true;
      var pause = 1;
      var menu = 0;
      var train = 1;
      var Switch = 1;
      var time_addr;
      var tablet = 1;
      var myWindow;

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

      function trains(){
        if(train == 1){
          $('#trains').css('display','none');
          train = 0;
        }else{
          $('#trains').css('display','block');
          if(tablet == 1 && Switch == 1){
            switches_w();
          }
          train = 1;
        }
      }

      function switches_w(){
        if(Switch == 1){
          $('#switches').css('display','none');
          Switch = 0;
        }else{
          $('#switches').css('display','block');
          if(tablet == 1 && train == 1){
            trains();
          }
          Switch = 1;
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

      $(function (){
        if($.urlParam('tablet') == 'y'){
          tablet = 1;
        }else{
          tablet = 0;
        }

        if (screen.width < 550) {
            var mvp = document.getElementById('vp');
            mvp.setAttribute('content','width=550');
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
          //toggleFullScreen();
          document.documentElement.requestFullscreen();
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

        //$("#play_pause").bind("tap",play_pause(this));
      });

    </script>
  </head>
  <body>
    <div style="height:40px;width:100%;background-color:#B00">
      <div style="width:130px;margin-left:3%;float:left;height:40px">
        <img src="./img/rotate_left_w.png" width="20px" style="margin:10px;cursor:pointer;"
            onClick="window.location.href = window.location.href.substr(0, window.location.href.indexOf('?'))+'?rot='+(parseInt($.urlParam('rot'))+90);"
            onmouseenter="this.src = './img/rotate_left.png'" onmouseleave="this.src = './img/rotate_left_w.png'"/>
        <img id="id_play_pause" src="./img/pause_w.png" width="20px" style="margin:10px;cursor:pointer;" onClick="play_pause(this)"
            onmouseenter="if(pause == 0){this.src = './img/pause.png'}else{this.src = './img/play.png'}" onmouseleave="if(pause == 0){this.src = './img/play_w.png'}else{this.src = './img/pause_w.png'}"/>
        <img src="./img/rotate_right_w.png" width="20px" style="margin:10px;cursor:pointer;"
            onClick="window.location.href = window.location.href.substr(0, window.location.href.indexOf('?'))+'?rot='+(parseInt($.urlParam('rot'))-90);"
            onmouseenter="this.src = './img/rotate_right.png'" onmouseleave="this.src = './img/rotate_right_w.png'"/>
      </div>
      <div style="width:100px;margin-left:3%;float:left;height:20px;padding:10px;color:white;font-family:sans-serif">
        <img id="tab" src="./img/checkbox_e.png" height="20px"
            onClick="if($.urlParam('tablet')=='y'){var t = 'n';}else{var t = 'y';};window.location.href = window.location.href.substr(0, window.location.href.indexOf('?'))+'?tablet='+t+'\&rot='+$.urlParam('rot');"
            onmouseenter="if($.urlParam('tablet')=='y'){this.src = './img/checkbox_g.png';}else{this.src = './img/checkbox_g.png';}"
            onmouseleave="if($.urlParam('tablet')=='y'){this.src = './img/checkbox_c.png';}else{this.src = './img/checkbox_e.png';}"/>
        <b>Mobile</b>
      </div>
      <div style="margin:1.5px;font-family:Sans-serif;float:left;">
        <h1 id="clock"><?php echo date('H:i');?></h1>
      </div>
      <div id="menu_button" style="background-color:white;"
            onMouseenter="$(this).css('background-color','black');$(this).children()[0].src = './img/menu_w.png';"
            onmouseleave="$(this).css('background-color','white');;$(this).children()[0].src = './img/menu.png';"
            onClick="menu_but()">
        <img src="./img/menu.png" width="16px"/>
      </div>
    </div>
    <div id="menu" style="display:none;">
    	<ul>
    	  <li style="border-top-left-radius:5px;border-top-right-radius:5px;" onClick="trains();menu_but()">Trains</li>
    	  <li onClick="switches_w();menu_but()">Switches</li>
    	  <li>Manage blocks</li>
    	  <li onClick="create_train_window()">Manage trains</li>
    	  <li style="border-bottom:none;border-bottom-left-radius:5px;border-bottom-right-radius:5px;">Settings</li>
    	</ul>
    </div>
    <div id="trains" style="display:block">
      <div class="drag"><b>Trains</b></div>
      <div class="box">
        <div class="content">
          <div class="trainbox">
            <div class="train_image_container">
              <img src="./../trains/1.jpg" style="max-width:100%;max-height:100%;"/><br/>
            </div>
            <div class="train_speed_container">
              <input type="range" orient="vertical" style="height:100%;"/><br/>
            </div>
            <div class="train_idbox">
              Koploper (#0103)<br/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onClick="  myWindow = window.open('', 'MsgWindow', 'width=200,height=100');myWindow.document.write('<p>This is MsgWindow. I am 200px wide and 100px tall!</p>');" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
            </div>
          </div>
          <div class="trainbox">
            <div class="train_image_container">
              <img src="./../trains/2.jpg" style="max-width:100%;max-height:100%;"/><br/>
            </div>
            <div class="train_speed_container">
              <input type="range" orient="vertical" style="height:100%;"/><br/>
            </div>
            <div class="train_idbox">
              Hondekop (#0053)<br/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
            </div>
          </div>
          <div class="trainbox">
            <div class="train_image_container">
              <img src="./../trains/3.jpg" style="max-width:100%;max-height:100%;"/><br/>
            </div>
            <div class="train_speed_container">
              <input type="range" orient="vertical" style="height:100%;"/><br/>
            </div>
            <div class="train_idbox">
              Eurostar (#0125)<br/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
              <img src="./img/train_function_light_off.png" width="24px" style="margin:3px;" onMouseenter="this.src='./img/train_function_light_on.png'" onMouseleave="this.src='./img/train_function_light_off.png'"/>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div id="switches" style="display:block">
      <div class="drag"><b>Switches</b></div>
      <div class="box">
        <div class="content"></div>
      </div>
    </div>
    <div id="warning_list" class="warning_list_nactive" style="display:none;">
      <div class="warning">
        <div class="photobox l">
          <img src="./img/stop.png"/>
        </div>
        <div class="photobox r">
          <img src="./img/stop.png"/>
        </div>
        <center>
          <h2>Emergency Stop</h2>
          <but onClick="alert('Clicked')">
            <b>Resume</b>
          </but>
        </center>
      </div>
      <div class="warning">
        <div class="photobox r">
          <img src="./img/shortcircuit.png"/>
        </div>
        <div class="photobox l">
          <img src="./img/shortcircuit.png"/>
        </div>
        <center>
          <h2>Short Circuit</h2>
          <but onClick="alert('Clicked')">
            <b>Resume</b>
          </but>
        </center>
      </div>

      Warning list
    </div>
    <div id="Window">
      <center>
        <div id="Modules_wrapper">
          <div id="Modules">
            <?php
              foreach($data[0] as $i){
                echo("<div class=\"M".$i." Module M".$i."b\">");
                include("./../modules/".$i.".svg");
                echo("</div>");
              }
              foreach($data[1] as $i){
                echo("<div class=\"M".$i." Module M".$i."b\">");
                include("./../modules/".$i.".svg");
                echo("</div>");
              }
             ?>
          </div>
        </div>
      </center>
    </div>
		<script src="./classie.js"></script>
		<script src="./selectFx.js"></script>
		<script>
			function redraw_selects() {
				[].slice.call( document.querySelectorAll( 'select.cs-selectn' ) ).forEach( function(el) {
          console.log(el);
					new SelectFx(el);
				} );
			};
		</script>
    <script src="./script_0.8.js">
    </script>
  </body>
</html>
