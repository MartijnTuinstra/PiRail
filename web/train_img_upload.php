<?php
$fileName = $_FILES["file1"]["name"]; // The file name
$fileTmpLoc = $_FILES["file1"]["tmp_name"]; // File in the PHP tmp folder
$fileType = $_FILES["file1"]["type"]; // The type of file it is
$fileSize = $_FILES["file1"]["size"]; // File size in bytes
$fileErrorMsg = $_FILES["file1"]["error"]; // 0 for false... and 1 for true
if (!$fileTmpLoc) { // if file not chosen
    echo "ERROR: Please browse for a file before clicking the upload button.";
    http_response_code(404);
    exit();
}
$ext = pathinfo($fileName, PATHINFO_EXTENSION);

echo $fileType;
$resize = false;
if($fileType == "image/jpeg"){
  list($width,$height) = getimagesize($fileTmpLoc);
  $ratio = $height / $width;
  if($width > 500){
    $resize = true;
    $rwidth = 500;
    $rheight = $rwidth * $ratio;

    if($rheight < 200){
      $rheight = 200;
      $rwidth = $rheigth / $ratio;
    }
  }

  if($resize){
    //Resize image
    $thumb = imagecreatetruecolor($rwidth, $rheight);
    $source = imagecreatefromjpeg($fileTmpLoc);
    imagecopyresized($thumb, $source, 0, 0, 0, 0, $rwidth, $rheight, $width, $height);

    //Save image
    imagejpeg($thumb, $fileTmpLoc);
  }
}

if(move_uploaded_file($fileTmpLoc, "./../trains/".$_POST['name'])){
    echo "./../trains/".$_POST['name']." upload is complete";
    //echo $_POST['name'];
} else {
		//phpinfo();
		echo "Error nr $fileErrorMsg";
		echo "Failed to move a file from $fileTmpLoc to ./uploads/$fileName";
}

if($_POST['file_edit'] == 1){
  $string = "php\t".$_POST['number'] . "\t" . $_POST['train_name'] . "\t" . $_POST['dcc'] . "\t";
  if($_POST['type'] == "Passenger"){
    $string = $string . "P\t";
  }elseif($_POST['type'] == "Cargo"){
    $string = $string . "C\t";
  }else{
    $string = $string . "_\t";
  }
  $string = $string . $_POST['max'] . "\t000000000000000000000000000\r\n";

  file_put_contents("./../trains/trainlist_raw.txt",$string,FILE_APPEND);
}
?>
