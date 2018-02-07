<?php
error_reporting(E_ERROR | E_WARNING | E_PARSE);
ini_set('display_errors', 1);
$var = json_decode(file_get_contents("./../trains/trainlist.txt"),true);

$var2 = file_get_contents("./../trains/trainlist_raw.txt");
$var3 = explode("\r\n",$var2);
for($i = 0;$i<(count($var3)-1);$i++){
  $var4[$i] = explode("\t",$var3[$i]);
}
?>

<html>
  <head>
    <style>
      html,body{
        margin: 0px auto;
        padding: 0px;
        max-width: 1000px;
      }

      table {
        border-collapse: collapse;
        width: 100%
      }

      td,th {
        width: 100px;
        text-align: center;
        border-bottom: 1px solid black;
      }
      th {
        background-color: #aaa;
        height: 40px;
      }

      #Add_button, #Quit_button {
        padding:5px;
        max-width: 50px;
        border-radius: 5px;
        border: none;
        background-color: black;
        color: white;
        font-family:sans-serif;
        margin: 10px;
        vertical-align: center;
        text-align: center;
        position:relative;
        cursor: pointer;
      }

      #Add_button:hover, #Quit_button:hover {
        top:-1px;
        border: 1px solid black;
        background-color: #777;
        color: black;
      }

      #Add_button:hover {
        left:-1px;
      }

      #Quit_button:hover {
        right:-1px;
      }

      #test {
        background-color: white;
        width: 380px;
        padding: 10px;
        height:280px;
        left:calc(50% - 200px);
        position:absolute;
        top: 50px;
        margin: auto;
        border-radius: 10px;
        border: 3px solid black;
      }
    </style>
    <script src="./jquery-3.1.1.min.js"></script>
    <script>
      var view = 0;
      var addingTrain = 0;
      function getExtension(filename) {
          var parts = filename.split('.');
          return parts[parts.length - 1];
      }

      function create(){
        if(view == 0){
          view = 1;

          $('#test').css("display",'block');
        }
        if(window.opener.ws.readyState == 3){
          $('#upload').attr('src','./img/checked_n.png');
          $('#upload').css('cursor','not-allowed');
          $('#upload').attr('onClick','');
        }
      }

      function add_train(){
        if ($('#train_name').val().indexOf(':') > -1) {
          alert("Name cannot contain a ':'");
          return;
        }

        $("#upload").attr('src','./img/loading.svg');
        $("#upload").attr('onClick','');
        if(window.opener.ws.readyState==1){
          window.opener.ws.send('RNt'+$('#train_type').val()+$('#train_name').val()+':'+$('#train_dcc').val()+':'+$('#train_speed').val());
          addingTrain = 1;
        }
      }

      function succ_add_train(data){
        if(addingTrain == 1){
          var file = $("#file1").get(0).files[0];
          // alert(file.name+" | "+file.size+" | "+file.type);
          var name = data[0] + "." + getExtension(file.name);
          var formdata = new FormData();
          formdata.append("file1", file);
          formdata.append("name", name);
          var ajax = new XMLHttpRequest();
          ajax.addEventListener("load", completeHandler, false);
          ajax.addEventListener("error", errorHandler, false);
          ajax.addEventListener("abort", abortHandler, false);
          ajax.open("POST", "./train_img_upload.php");
          ajax.send(formdata);
        }
        setTimeout(function () {
          $("#list").append("<tr><td><img src=\"./../trains/"+data[0]+".jpg\" style=\"max-height:50px;max-width:100px;float:right;\"/></td><td>"+data[1]+"</td><td>"+data[2]+"</td><td>"+data[3]+"</td><td>"+data[4]+"</td></tr>");
        }, 1000);
      }

      function completeHandler(event){
        setTimeout(function () {
      	  $("#test").css('display','none');
      	  $("#upload").attr('onClick','add_train()');
        }, 500);
      }

      function errorHandler(event){
      	alert("Upload Failed");
      }

      function abortHandler(event){
      	alert("Upload Aborted");
      }

      function fail_add_train(){
        $("#upload").attr('src','./img/checked.png');
      }
    </script>
  </head>
  <body>
    <table id="list">
    <?php
      $i = 1;
      echo("<tr><th></th><th>".$var4[0][0]."</th><th>".$var4[0][1]."</th><th>".$var4[0][2]."</th><th>".$var4[0][3]."</th></tr>");
      for($i;$i<count($var4);$i++){
        echo("<tr><td><img src=\"./../trains/".($i-1).".jpg\" style=\"max-height:50px;max-width:100px;float:right;\"/></td><td>".$var4[$i][0]."</td><td>".$var4[$i][1]."</td><td>".$var4[$i][2]."</td><td>".$var4[$i][3]."</td></tr>");
      }
    ?>
    </table>
    <div style="width:150px;margin:auto;height:40px">
      <div id="Add_button" style="float:left" onClick="create();">
        <b>Add</b>
      </div>
      <div id="Quit_button" style="float:right" onClick="<?php if(!isset($_GET['tablet'])){echo "window.opener.train_list_window = 0;self.close();";}else{echo "window.history.back();";} ?>">
        <b>Close</b>
      </div>
    </div>
    <div id='test' style="display:none">
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
              <input type="number" id="train_dcc" value="<?php echo count($var); ?>" name="DCC" min="1" max="9999"/>
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
            <td colspan="2" style="border-bottom:none;">
              <img src="./img/checked.png" id="upload" style="margin-top:20px" width="32px" onClick="/*uploadFile();*/add_train();"/>
            </td>
          </tr>
        </table>
      </form>
    </div>
  </body>
</html>
