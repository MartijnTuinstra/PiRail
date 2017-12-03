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
    <div style="height:300px">
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
      <div id="M3" class="Module">
        <?php include("./modules/3.svg"); ?>
      </div>
      <div id="M8" class="Module">
        <?php include("./modules/8.svg"); ?>
      </div>
      <div id="M2" class="Module">
        <?php include("./modules/2.svg"); ?>
      </div>
    </div>
    <script src="./script_0.8.js">
    </script>
    <button onClick="stop()">Stop</button>
    <button onClick="Start()">Start</button>
    <button onClick="update()">Once</button>
  </body>
</html>
