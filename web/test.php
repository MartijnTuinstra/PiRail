<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

$list = [];

$directories = glob('./../modules/*' , GLOB_ONLYDIR);
foreach($directories as $value){
  array_push($list,explode("modules/",$value)[1]);
}
print("<br/><br/>");
print_r($list);

 ?>
