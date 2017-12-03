<?php
  $data = file_get_contents("./../setup.json");
  $data = json_decode($data);
  date_default_timezone_set("Europe/Amsterdam");
?><html>
  <head>
    <title>TEST</title>
    <meta id="vp" name="viewport" content="width=device-width, initial-scale=1">
    <script src="./jquery-3.1.1.min.js"></script>
    <script src="./jquery-ui.min.js"></script>
    <style>
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
        min-width:380px; /* suppose you want minimun width of 1000px */
         width: auto !important;  /* Firefox will set width as auto */
         width:380px
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

      #Window {
      	height: calc(100% - 40px);
      	overflow: auto;
      	position: relative;
      }

      .button {
        font-family:sans-serif;
        color:white;
        width:300px;
        height:150px;
        padding:20px;
        border-radius:10px;
        background-color:#B00;
        margin:20px;
        text-align:center;
        float:left;
        cursor: pointer;
      }

      .button:hover {
        background-color: #0BB;
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
      <div style="margin:1.5px;font-family:Sans-serif;float:left;">
        <h1 id="clock"><?php echo date('H:i');?></h1>
      </div>
    </div>
    <div id="Window">
      <center>
        <div class="button" onClick="window.location.href = './manage_trains.php';">
          <h1>Manage trains</h1><br/><br/><br/>
          <img src="./img/train.png" height="64px"/>
        </div>
        <div class="button" onClick="window.location.href = './baan.php';">
          <h1>Track</h1><br/><br/><br/>
          <img src="./img/rail.png" height="64px"/>
        </div>
      </center>
    </div>
    </script>
  </body>
</html>
