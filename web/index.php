<?php
  $data = file_get_contents("./../setup.json");
  $data = json_decode($data);
  date_default_timezone_set("Europe/Amsterdam");
?><html>
  <head>
    <title>TEST</title>
    <script src="./jquery-3.1.1.min.js"></script>
    <script src="./jquery-ui.min.js"></script>
    <style>
      svg {
      //  shape-rendering:crispEdges;
      }
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
      .s {
        stroke:#000;
        fill:none;
        stroke-width:4px;
        stroke-linecap:round;
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

      ul {
	       margin-left:20px;
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
      }

      #menu {
        float: right;
        width: 300px;
        height: 207px;
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
        height: 150px;
        background-color: #ccc;
        z-index: 10;
        padding-top: 20px;
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
      }

      .switchbox svg{
        margin-bottom: 5px;
      }

      .drag {
        width: calc(100% - 20px);
        padding-left: 20px;
        height: 20px;
        top: 0px;
        position: absolute;
        background-color: #999;
        font-family: sans-serif;
      }

      #trains .content, #switches .content{
        height: 100%;
        white-space: nowrap;
      }

      .Module{
        position: absolute;
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
    <script>
      var pause = 1;
      var menu = 0;
      var train = 0;
      var Switch = 1;
      var time_addr;
      var tablet = 0;

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
          train = 1;
        }
      }

      function switches(){
        if(Switch == 1){
          $('#switches').css('display','none');
          Switch = 0;
        }else{
          $('#switches').css('display','block');
          Switch = 1;
        }
      }

      $(function (){
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
        }else{
          $( "#trains" ).draggable({ handle: "div.drag",containment: "html", });
          $("#switches").draggable({ handle: "div.drag",containment: "html", });
        }
        if($.urlParam('tablet') == 'y'){
          $("#tab").attr("src","./img/checkbox_c.png");
        }else{
          $("#tab").attr("src","./img/checkbox_e.png");
        }

        //$("#play_pause").bind("tap",play_pause(this));
      });

    </script>
  </head>
  <body>
    <div style="height:40px;width:100%;background-color:#B00">
      <div style="width:130px;margin-left:3%;float:left;height:40px">
        <img src="./img/rotate_left_w.png" width="20px" style="margin:10px"
            onClick="window.location.href = window.location.href.substr(0, window.location.href.indexOf('?'))+'?rot='+(parseInt($.urlParam('rot'))+90);"
            onmouseenter="this.src = './img/rotate_left.png'" onmouseleave="this.src = './img/rotate_left_w.png'"/>
        <img id="id_play_pause" src="./img/pause_w.png" width="20px" style="margin:10px" onClick="play_pause(this)"
            onmouseenter="if(pause == 0){this.src = './img/pause.png'}else{this.src = './img/play.png'}" onmouseleave="if(pause == 0){this.src = './img/play_w.png'}else{this.src = './img/pause_w.png'}"/>
        <img src="./img/rotate_right_w.png" width="20px" style="margin:10px"
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
    	  <li onClick="switches();menu_but()">Switches</li>
    	  <li>Visible blocks</li>
    	  <li style="border-bottom:none;border-bottom-left-radius:5px;border-bottom-right-radius:5px;">Settings</li>
    	</ul>
    </div>
    <div id="trains" style="display:none">
      <div class="drag"><b>Trains</b></div>
      <div class="box">
        <div class="content"></div>
      </div>
    </div>
    <div id="switches" style="display:block">
      <div class="drag"><b>Switches</b></div>
      <div class="box">
        <div class="content"></div>
      </div>
    </div>
    <div id="Window">
      <center>
        <div id="Modules_wrapper">
          <div id="Modules">
            <?php
              foreach($data as $i){
                echo("<div class=\"M".$i." Module\">");
                include("./../modules/".$i.".svg");
                echo("</div>");
              }
             ?>
          </div>
        </div>
      </center>
    </div>
    <script src="./script_0.8.js">
    </script>
  </body>
</html>
