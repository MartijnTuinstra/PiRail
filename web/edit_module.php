<?php
session_start();

if($_SESSION['login'] == FALSE){
	//Failed to login
	exit 1;
}


?>