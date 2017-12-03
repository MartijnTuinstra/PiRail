<?php
$fileName = $_FILES["file1"]["name"]; // The file name
$fileTmpLoc = $_FILES["file1"]["tmp_name"]; // File in the PHP tmp folder
$fileType = $_FILES["file1"]["type"]; // The type of file it is
$fileSize = $_FILES["file1"]["size"]; // File size in bytes
$fileErrorMsg = $_FILES["file1"]["error"]; // 0 for false... and 1 for true
if (!$fileTmpLoc) { // if file not chosen
    echo "ERROR: Please browse for a file before clicking the upload button.";
    exit();
}
$ext = pathinfo($filename, PATHINFO_EXTENSION);

if(move_uploaded_file($fileTmpLoc, "./../trains/".$_POST['name'])){
    echo "./../trains/".$_POST['name']." upload is complete";
    //echo $_POST['name'];
} else {
		//phpinfo();
		echo "Error nr $fileErrorMsg";
		echo "Failed to move a file from $fileTmpLoc to ./uploads/$fileName";
}
?>
