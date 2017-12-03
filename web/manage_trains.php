<?php
error_reporting(E_ERROR | E_WARNING | E_PARSE);
ini_set('display_errors', 1);

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

      function create(){
        //if(view == 0){
          view = 1;
          var text = "<div id='test'><form action=\"manage_trains.php\" method=\"post\" enctype=\"multipart/form-data\"><h3 style=\"text-align:center;\">New train</h3>";
          text += "<table><tr><td style=\"border-top:1px solid black;\" colspan=\"2\">Select image (jpg only) to upload:<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\"></td></tr>";
          text += "<tr><td>Name</td><td><input type=\"text\" value=\"Name\" name=\"Name\"/></td></tr>";
          text += "<tr><td>DCC Address</td><td><input type=\"number\" value=\"0\" name=\"DCC\"/></td></tr>";
          text += "<tr><td>Type</td><td><select name=\"type\"><option value=\"P\">Passenger</option><option value=\"C\">Cargo</option></select></td></tr>";
          text += "<tr><td>Max speed (km/h)</td><td><input type=\"number\" value=\"160\" name=\"speed\"/></td></tr>";
          text += "<tr><td colspan=\"2\"><input type=\"submit\" value=\"Add\" name=\"submit\"></td></tr></table></form></div>";
          $('body').append(text);
        //}
      }
    </script>
  </head>
  <body>
    <table border="0">
    <?php
      $i = 1;
      $var = json_decode(file_get_contents("./../trains/trainlist.txt"),true);
      echo("<tr><th></th><th>".$var[0][0]."</th><th>".$var[0][1]."</th><th>".$var[0][2]."</th><th>".$var[0][3]."</th></tr>");
      for($i;$i<count($var);$i++){
        echo("<tr><td><img src=\"./../trains/".$i.".jpg\" style=\"max-height:50px;max-width:100px;float:right;\"/></td><td>".$var[$i][0]."</td><td>".$var[$i][1]."</td><td>".$var[$i][2]."</td><td>".$var[$i][3]."</td></tr>");
      }

      if(file_exists($_FILES['fileToUpload']['tmp_name']) || is_uploaded_file($_FILES['fileToUpload']['tmp_name'])) {
        $target_dir = "./../trains/";
        $target_file = $target_dir . $i . ".jpg";
        $uploadOk = 1;
        $imageFileType = pathinfo(basename($_FILES["fileToUpload"]["name"]),PATHINFO_EXTENSION);
        // Check if image file is a actual image or fake image
        if(isset($_POST["submit"])) {
            $check = getimagesize($_FILES["fileToUpload"]["tmp_name"]);
            if($check !== false) {
                echo "File is an image - " . $check["mime"] . "\"".basename($_FILES["fileToUpload"]["name"])."\"=".$imageFileType.".";
                $uploadOk = 1;
            } else {
                echo "File is not an image.";
                $uploadOk = 0;
            }
        }
        // Check if file already exists
        if (file_exists($target_file)) {
            echo "Sorry, file already exists.";
            $uploadOk = 0;
        }
        // Check file size
        if ($_FILES["fileToUpload"]["size"] > 2000000) {
            echo "Sorry, your file is too large. ".(($_FILES["fileToUpload"]["size"]-2000000)/1000)."KB too large";
            $uploadOk = 0;
        }
        // Allow certain file formats
        if($imageFileType != "jpg" && $imageFileType != "jpeg") {
            echo "Sorry, only JPG files are allowed.";
            $uploadOk = 0;
        }
        // Check if $uploadOk is set to 0 by an error
        if ($uploadOk == 0) {
            echo "Sorry, your file was not uploaded.";
        // if everything is ok, try to upload file
        } else {
            if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
                echo "The file ". basename( $_FILES["fileToUpload"]["name"]). " has been uploaded.";

                $data[0] = $_POST['Name'];
                $data[1] = $_POST['DCC'];
                $data[2] = $_POST['type'];
                $data[3] = $_POST['speed'];

                $inp = file_get_contents('./../trains/trainlist.txt');
                $tempArray = json_decode($inp);
                array_push($tempArray, $data);
                $jsonData = json_encode($tempArray);
                file_put_contents('./../trains/trainlist.txt', $jsonData);
                header("Refresh:0");
            } else {
                echo "Sorry, there was an error uploading your file.";
            }
        }
      }


    ?>
    </table>
    <div style="width:150px;margin:auto;height:40px">
      <div id="Add_button" style="float:left" onClick="create();">
        <b>Add</b>
      </div>
      <div id="Quit_button" style="float:right" onClick="<?php if(!isset($_GET['tablet'])){echo "self.close();";}else{echo "window.history.back();";} ?>">
        <b>Close</b>
      </div>
    </div>
  </body>
</html>
