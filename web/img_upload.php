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
if($fileType == "image/jpeg" || $fileType == "image/png"){
  list($width,$height) = getimagesize($fileTmpLoc);
  $ratio = $height / $width;
  if($height > 500){
    $resize = true;
    $rheight = 500;
    $rwidth = $rheight / $ratio;
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
if(move_uploaded_file($fileTmpLoc, "./../trains/tmp/".$_POST['name'])){
    echo "./../trains/".$_POST['name']." upload is complete";
    //echo $_POST['name'];
} else {
		//phpinfo();
		echo "Error nr $fileErrorMsg";
		echo "Failed to move a file from $fileTmpLoc to ./../trains/tmp/$fileName";
}
?>