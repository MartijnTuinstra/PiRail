<?php

$fn = (isset($_SERVER['HTTP_X_FILENAME']) ? $_SERVER['HTTP_X_FILENAME'] : false);

if ($fn) {

	// AJAX call
	file_put_contents(
		'./uploads/' . $fn,
		file_get_contents('php://input')
	);
	echo "$fn uploaded";
	exit();

}else{
  echo "problem";
}
/*
if(file_exists($_FILES['fileToUpload']['tmp_name']) || is_uploaded_file($_FILES['fileToUpload']['tmp_name'])) {
  if (0 < $_FILES['file']['error']) {
      echo 'Error: ' . $_FILES['file']['error'] . '<br>';
  } else {
      move_uploaded_file($_FILES['file']['tmp_name'], './uploads/' . $_FILES['file']['name']);
      echo $_FILES['file']['name'];
      echo "success";
  }
}else{
  echo "Does not exists";
}*/
?>
