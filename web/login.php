<?php
//Login page
session_start();

$passphrase = file_get_contents('./../password.txt');

if(isset($_POST['pass']) && $_POST['pass'] != ""){
	//echo "LOGIN SUCCESFULL";
	if($_POST['pass'] == $passphrase){
		echo "LOGIN SUCCESFULL";
		$_SESSION['login'] = TRUE;
	}else{
		$_SESSION['login'] = FALSE;
		echo "LOGIN FAILED";
	}
}else{
	$_SESSION['login'] = FALSE;
	echo "LOGOUT SUCCESFULL";
}

?>