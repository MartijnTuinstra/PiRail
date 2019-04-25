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

$time = date("is"); //Minute and second
$filename = $_POST['name'];
$file_extension = end(explode(".",$_FILES["file1"]["name"]));

$new_filename = $filename . "." . $time . "." . $file_extension;

$dst_loc = __DIR__."/".$new_filename;

echo $new_filename;

if(!move_uploaded_file($fileTmpLoc, $dst_loc)){
    echo "Failed to move a file from $fileTmpLoc to $dst_loc";
    http_response_code(404);
    exit();
}

// $resize = false;
// if($fileType == "image/jpeg" || $fileType == "image/png"){
//   list($width,$height) = getimagesize($dst_loc);
//   $ratio = $height / $width;
//   if($height > 500){
//     $resize = true;
//     $rheight = 500;
//     $rwidth = $rheight / $ratio;
//   }
//   if($resize){
//     //Resize image
//     $thumb = imagecreatetruecolor($rwidth, $rheight);
//     $source = imagecreatefromjpeg($dst_loc);
//     imagecopyresized($thumb, $source, 0, 0, 0, 0, $rwidth, $rheight, $width, $height);
//     //Save image
//     imagejpeg($thumb, $dst_loc);
//   }
// }
?>
