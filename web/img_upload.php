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

$dst_loc = __DIR__."/../trains/".$_POST['name'].".".end(explode(".",$_FILES["file1"]["name"]));

if(move_uploaded_file($fileTmpLoc, $dst_loc)){
    echo $dst_loc." upload is complete\n";
    echo "name: ".$_POST['name']."\n";
}else {
    //phpinfo();
    echo "Error nr $fileErrorMsg\n";
    echo "Failed to move a file from $fileTmpLoc to $dst_loc\n";
    http_response_code(404);
    exit();
}

$resize = false;
if($fileType == "image/jpeg" || $fileType == "image/png"){
  echo "resize\n";
  list($width,$height) = getimagesize($dst_loc);
  $ratio = $height / $width;
  if($height > 500){
    $resize = true;
    $rheight = 500;
    $rwidth = $rheight / $ratio;
  }
  if($resize){
    //Resize image
    $thumb = imagecreatetruecolor($rwidth, $rheight);
    $source = imagecreatefromjpeg($dst_loc);
    imagecopyresized($thumb, $source, 0, 0, 0, 0, $rwidth, $rheight, $width, $height);
    //Save image
    imagejpeg($thumb, $dst_loc);
  }
}
?>