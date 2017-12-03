<html>
  <head>
    <title>TEST</title>
    <script src="./jquery-3.1.1.min.js"></script>
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
      }
      .Module{
        float:left;
      }
      #Modules{
        width: 2000px;
      }
      </style>
  </head>
  <body>
    <div style="height:20px">
      <?php $tablet = $_GET['T'];

      if($tablet == 0){
        echo("Desktop");
      }elseif($tablet == 1){
        echo("Tablet");
      }
      ?>
    </div>
    <div id="Modules">
      <div id="M1" class="Module">
        <?php include("./modules/1.svg"); ?>
      </div>
      <div id="M5" class="Module">
        <?php include("./modules/5.svg"); ?>
      </div>
      <div id="M4" class="Module">
        <?php include("./modules/4.svg"); ?>
      </div>
      <div id="M10" class="Module">
        <?php include("./modules/10.svg"); ?>
      </div>
      <div id="M8" class="Module" style="position:relative;left:970px;top:350px;-ms-transform: rotate(90deg);-webkit-transform: rotate(90deg);transform: rotate(90deg);">
        <?php include("./modules/8.svg"); ?>
      </div>
      <div id="M2" class="Module" style="position:relative;left:480px;top:800px;-ms-transform: rotate(90deg);-webkit-transform: rotate(90deg);transform: rotate(90deg);">
        <?php include("./modules/2.svg"); ?>
      </div>
      <div id="M6" class="Module" style="position:relative;left:-798px;top:-162px;-ms-transform: rotate(90deg);-webkit-transform: rotate(90deg);transform: rotate(90deg);">
        <?php include("./modules/6.svg"); ?>
      </div>
      <div id="M7" class="Module" style="position:relative;left:-1169px;top:232px;-ms-transform: rotate(90deg);-webkit-transform: rotate(90deg);transform: rotate(90deg);">
        <?php include("./modules/7.svg"); ?>
      </div>
    </div>
    <script src="./script_0.8.js">
    </script>
    <button onClick="stop()">Stop</button>
    <button onClick="Start()">Start</button>
    <button onClick="update()">Once</button>
  </body>
</html>
