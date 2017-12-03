<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

function CreatePropertiesContent($data){
	$response_text = "'".$data["Name"]."\r\n'Create Module\r\nCU\t".$data["ModuleNr"]."\t".ceil($data["HOA"]/8)."\t".ceil($data["HOA"]/8)."\r\n";

	$response_text = $response_text . "'\r\n'Blocks\r\n'\r\n";
	foreach ($data["Blocks"] as $value) {
	  $string = "CB" . "\t" . $value["IOAddr"] . "\t" . $value["Addr"] . "\t" . $value["Type"] . "\t" . $value["Next"][0] . "\t" . $value["Next"][1] . "\t" . $value["Next"][2] . "\t";
	  $string = $string . $value["Prev"][0] . "\t" . $value["Prev"][1] . "\t" . $value["Prev"][2] . "\t" . $value["Speed"] . "\t" . $value["Dir"] . "\t" . $value["Length"];
	  if($value["OneWay"] == 1 || $value["OneWay"] == "1"){
		  $string = $string . "\tY\r\n";
	  }else{
		$string = $string . "\tN\r\n";
	  }
	  $response_text = $response_text . $string;
	  //fwrite($myfile,serialize($value));
	}
	$response_text = $response_text . "'\r\n'Switches\r\n'\r\n";

	foreach ($data["Switch"] as $value) {
	  if($value["Type"] == "S"){
		$string = "CSw" . "\t" . $value["Addr"] . "\t" . $value["Appr"][0][0] . "\t" . $value["Appr"][0][1] . "\t" . $value["Appr"][0][1] . "\t" . $value["Dest"][0][0] . "\t" . $value["Dest"][0][1] . "\t";
		$string = $string . $value["Dest"][0][2] . "\t" . $value["Dest"][1][0] . "\t" . $value["Dest"][1][1] . "\t" . $value["Dest"][1][2] . "\t" . $value["States"] . "\t";

		$i = 0;
		foreach($value["Speed"] as $value2){
		  if($i != 0){
			$string = $string . " ";
		  }else{$i++;}
		  $string = $string . $value2;
		}
		$string = $string . "\t";
		$i = 0;
		foreach($value["IOAddr"] as $value2){
		  if($i != 0){
			$string = $string . " ";
		  }else{$i++;}
		  $string = $string . $value2;
		}

		$string = $string . "\r\n";
	  }
	  $response_text = $response_text . $string;
	  //fwrite($myfile,serialize($value));
	}

	$response_text = $response_text . "'\r\n'Signals\r\n'\r\n";
	foreach ($data["Signals"] as $value) {
	  $string = "CSig" . "\t" . $value["Name"] . "\t" . $value["Type"] . "\t";
	  $string = $string . "IOA";
	  $string = $string . "\t" . $value["Type"] . "\t";
	  $string = $string . "LEN";
	  $string = $string . "\t" . dechex($value["EnStates"][1]) . "\t";
	  $string = $string . "STATES";
	  $string = $string . "\t" . $value["Block"] . "\t" . $value["Side"] . "\r\n";

	  $response_text = $response_text . $string;
	  //fwrite($myfile,serialize($value));
	}

	$response_text = $response_text . "'\r\n'Station\r\n'\r\n";
	foreach ($data["Stations"] as $value) {
	  $string = "CSt" . "\t" . $value["Name"] . "\t" . $value["Type"] . "\t" . $value["NrOf"] . "\t";
	  $string = $string . "Blocks" . "\r\n";

	  $response_text = $response_text . $string;
	  //fwrite($myfile,serialize($value));
	}
	return $response_text;
}

function CreateConnectContent($data){
	$response_text = "XY"."\t".$data["ConW"]."\t".$data["ConH"]."\r\n";
	
	$response_text = $response_text . "LA"."\t".$data["LAnchor"]['x']."\t".$data["LAnchor"]['y']."\r\n";
	$response_text = $response_text . "RA"."\t".$data["RAnchor"]['x']."\t".$data["RAnchor"]['y']."\r\n";
	
	$response_text = $response_text . "'\r\n";
	
	foreach ($data["Blocks"] as $value) {
	  $response_text = $response_text . "B" . "\t" . $value["Addr"] . "\t" . $value["Connector"]['x'] . "\t" . $value["Connector"]['y'] . "\r\n";
	}
	
	$response_text = $response_text . "'\r\n";
	
	foreach ($data["Switch"] as $value) {
	  $response_text = $response_text . "Sw" . "\t" . $value["Addr"] . "\t" . $value["Connector"]['x'] . "\t" . $value["Connector"]['y'] . "\t" . $value["Connector"]['flip'] . "\r\n";
	}
	
	return $response_text;
}

function prettyPrint( $json )
{
    $result = '';
    $level = 0;
    $in_quotes = false;
    $in_escape = false;
    $ends_line_level = NULL;
    $json_length = strlen( $json );

    for( $i = 0; $i < $json_length; $i++ ) {
        $char = $json[$i];
        $new_line_level = NULL;
        $post = "";
        if( $ends_line_level !== NULL ) {
            $new_line_level = $ends_line_level;
            $ends_line_level = NULL;
        }
        if ( $in_escape ) {
            $in_escape = false;
        } else if( $char === '"' ) {
            $in_quotes = !$in_quotes;
        } else if( ! $in_quotes ) {
            switch( $char ) {
                case '}': case ']':
                    $level--;
                    $ends_line_level = NULL;
                    $new_line_level = $level;
                    break;

                case '{': case '[':
                    $level++;
                case ',':
                    $ends_line_level = $level;
                    break;

                case ':':
                    $post = " ";
                    break;

                case " ": case "\t": case "\n": case "\r":
                    $char = "";
                    $ends_line_level = $new_line_level;
                    $new_line_level = NULL;
                    break;
            }
        } else if ( $char === '\\' ) {
            $in_escape = true;
        }
        if( $new_line_level !== NULL ) {
            $result .= "\n".str_repeat( "\t", $new_line_level );
        }
        $result .= $char.$post;
    }

    return $result;
}

$timestamp = date('Ymd_His').".txt";

$request_body = file_get_contents('php://input');
if($request_body == ""){
  http_response_code(417);
  $request_body = '{"Blocks":[{"IOAddr":0,"Addr":0,"Type":"R","Next":{"0":"I1","1":"I","2":"I","Connect":true},"Prev":{"0":"X","1":0,"2":"S","Connect":true},"Speed":180,"Dir":0,"Length":100,"Connector":{"x":"100","y":"100"},"OneWay":true},{"IOAddr":1,"Addr":1,"Type":"T","Next":{"0":"E","1":"E","2":"E","Connect":-1},"Prev":{"0":"E","1":"E","2":"E","Connect":-1},"Speed":180,"Dir":0,"Length":100,"Connector":{"x":"180","y":"100"},"OneWay":false},{"IOAddr":8,"Addr":2,"Type":"S","Next":{"0":"X","1":1,"2":"s","Connect":true},"Prev":{"0":"X","1":3,"2":"R","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"500","y":"60"},"OneWay":false},{"IOAddr":9,"Addr":3,"Type":"S","Next":{"0":"X","1":2,"2":"R","Connect":true},"Prev":{"0":"X","1":4,"2":"R","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"580","y":"60"},"OneWay":false},{"IOAddr":24,"Addr":4,"Type":"S","Next":{"0":"X","1":3,"2":"R","Connect":true},"Prev":{"0":"X","1":5,"2":"R","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"660","y":"60"},"OneWay":false},{"IOAddr":25,"Addr":5,"Type":"S","Next":{"0":"X","1":4,"2":"R","Connect":true},"Prev":{"0":"X","1":5,"2":"s","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"740","y":"60"},"OneWay":false},{"IOAddr":10,"Addr":6,"Type":"S","Next":{"0":"X","1":1,"2":"s","Connect":true},"Prev":{"0":"X","1":7,"2":"R","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"500","y":"100"},"OneWay":false},{"IOAddr":11,"Addr":7,"Type":"S","Next":{"0":"X","1":6,"2":"R","Connect":true},"Prev":{"0":"X","1":8,"2":"R","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"580","y":"100"},"OneWay":false},{"IOAddr":26,"Addr":8,"Type":"S","Next":{"0":"X","1":7,"2":"R","Connect":true},"Prev":{"0":"X","1":9,"2":"R","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"660","y":"100"},"OneWay":false},{"IOAddr":27,"Addr":9,"Type":"S","Next":{"0":"X","1":8,"2":"R","Connect":true},"Prev":{"0":"X","1":5,"2":"s","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"740","y":"100"},"OneWay":false},{"IOAddr":50,"Addr":10,"Type":"T","Next":{"0":"E","1":"E","2":"E","Connect":-1},"Prev":{"0":"E","1":"E","2":"E","Connect":-1},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"820","y":"100"},"OneWay":false},{"IOAddr":51,"Addr":11,"Type":"R","Next":{"0":"X","1":6,"2":"S","Connect":true},"Prev":{"0":"E1","1":"E","2":"E","Connect":true},"Speed":90,"Dir":0,"Length":100,"Connector":{"x":"1140","y":"100"},"OneWay":true},{"IOAddr":2,"Addr":12,"Type":"R","Next":{"0":"I2","1":"I","2":"I","Connect":true},"Prev":{"0":"X","1":2,"2":"s","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"100","y":"140"},"OneWay":true},{"IOAddr":3,"Addr":13,"Type":"T","Next":{"0":"E","1":"E","2":"E","Connect":-1},"Prev":{"0":"E","1":"E","2":"E","Connect":-1},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"180","y":"140"},"OneWay":false},{"IOAddr":12,"Addr":14,"Type":"S","Next":{"0":"X","1":3,"2":"s","Connect":true},"Prev":{"0":"X","1":15,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"500","y":"140"},"OneWay":false},{"IOAddr":13,"Addr":15,"Type":"S","Next":{"0":"X","1":14,"2":"R","Connect":true},"Prev":{"0":"X","1":16,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"580","y":"140"},"OneWay":false},{"IOAddr":28,"Addr":16,"Type":"S","Next":{"0":"X","1":15,"2":"R","Connect":true},"Prev":{"0":"X","1":17,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"660","y":"140"},"OneWay":false},{"IOAddr":29,"Addr":17,"Type":"S","Next":{"0":"X","1":16,"2":"R","Connect":true},"Prev":{"0":"X","1":7,"2":"s","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"740","y":"140"},"OneWay":false},{"IOAddr":14,"Addr":18,"Type":"S","Next":{"0":"X","1":4,"2":"s","Connect":true},"Prev":{"0":"X","1":19,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"500","y":"180"},"OneWay":false},{"IOAddr":15,"Addr":19,"Type":"S","Next":{"0":"X","1":18,"2":"R","Connect":true},"Prev":{"0":"X","1":20,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"580","y":"180"},"OneWay":false},{"IOAddr":30,"Addr":20,"Type":"S","Next":{"0":"X","1":19,"2":"R","Connect":true},"Prev":{"0":"X","1":21,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"660","y":"180"},"OneWay":false},{"IOAddr":31,"Addr":21,"Type":"S","Next":{"0":"X","1":20,"2":"R","Connect":true},"Prev":{"0":"X","1":9,"2":"s","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"740","y":"180"},"OneWay":false},{"IOAddr":16,"Addr":22,"Type":"S","Next":{"0":"X","1":4,"2":"s","Connect":true},"Prev":{"0":"X","1":23,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"500","y":"220"},"OneWay":false},{"IOAddr":17,"Addr":23,"Type":"S","Next":{"0":"X","1":22,"2":"R","Connect":true},"Prev":{"0":"X","1":24,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"580","y":"220"},"OneWay":false},{"IOAddr":32,"Addr":24,"Type":"S","Next":{"0":"X","1":23,"2":"R","Connect":true},"Prev":{"0":"X","1":25,"2":"R","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"660","y":"220"},"OneWay":false},{"IOAddr":33,"Addr":25,"Type":"S","Next":{"0":"X","1":24,"2":"R","Connect":true},"Prev":{"0":"X","1":9,"2":"s","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"740","y":"220"},"OneWay":false},{"IOAddr":52,"Addr":26,"Type":"T","Next":{"0":"E","1":"E","2":"E","Connect":-1},"Prev":{"0":"E","1":"E","2":"E","Connect":-1},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"820","y":"140"},"OneWay":false},{"IOAddr":53,"Addr":27,"Type":"R","Next":{"0":"X","1":8,"2":"s","Connect":true},"Prev":{"0":"E2","1":"E","2":"E","Connect":true},"Speed":90,"Dir":1,"Length":100,"Connector":{"x":"1140","y":"140"},"OneWay":true}],"Switch":[{"Addr":0,"Type":"S","Appr":[{"0":"X","1":0,"2":"R","Connect":0}],"Dest":[{"0":"X","1":2,"2":"s","Connect":0},{"0":"X","1":1,"2":"S","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[0,1],"Connector":{"x":"180","y":"100","flip":"0"}},{"Addr":1,"Type":"S","Appr":[{"0":"X","1":0,"2":"s","Connect":0}],"Dest":[{"0":"X","1":2,"2":"R","Connect":0},{"0":"X","1":6,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[2,3],"Connector":{"x":"340","y":"100","flip":"h"}},{"Addr":2,"Type":"S","Appr":[{"0":"X","1":3,"2":"S","Connect":0}],"Dest":[{"0":"X","1":0,"2":"s","Connect":0},{"0":"X","1":12,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[4,5],"Connector":{"x":"260","y":"140","flip":"vh"}},{"Addr":3,"Type":"S","Appr":[{"0":"X","1":2,"2":"S","Connect":0}],"Dest":[{"0":"X","1":4,"2":"S","Connect":0},{"0":"X","1":14,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[6,7],"Connector":{"x":"340","y":"140","flip":"0"}},{"Addr":4,"Type":"S","Appr":[{"0":"X","1":3,"2":"s","Connect":0}],"Dest":[{"0":"X","1":18,"2":"R","Connect":0},{"0":"X","1":22,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[8,9],"Connector":{"x":"420","y":"180","flip":"0"}},{"Addr":5,"Type":"S","Appr":[{"0":"X","1":6,"2":"s","Connect":0}],"Dest":[{"0":"X","1":5,"2":"R","Connect":0},{"0":"X","1":9,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[10,11],"Connector":{"x":"900","y":"100","flip":"vh"}},{"Addr":6,"Type":"S","Appr":[{"0":"X","1":11,"2":"R","Connect":0}],"Dest":[{"0":"X","1":8,"2":"s","Connect":0},{"0":"X","1":5,"2":"S","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[12,13],"Connector":{"x":"1060","y":"100","flip":"v"}},{"Addr":7,"Type":"S","Appr":[{"0":"X","1":8,"2":"S","Connect":0}],"Dest":[{"0":"X","1":9,"2":"S","Connect":0},{"0":"X","1":17,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[14,15],"Connector":{"x":"900","y":"140","flip":"v"}},{"Addr":8,"Type":"S","Appr":[{"0":"X","1":7,"2":"S","Connect":0}],"Dest":[{"0":"X","1":6,"2":"s","Connect":0},{"0":"X","1":27,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[16,17],"Connector":{"x":"980","y":"140","flip":"h"}},{"Addr":9,"Type":"S","Appr":[{"0":"X","1":7,"2":"s","Connect":0}],"Dest":[{"0":"X","1":21,"2":"R","Connect":0},{"0":"X","1":25,"2":"R","Connect":0}],"States":2,"Speed":[180,90],"IOAddr":[18,19],"Connector":{"x":"820","y":"180","flip":"v"}}],"Signals":[{"Name":"Spoor 1 ","Type":1,"Block":5,"Side":"N","EnStates":224,"IOAddress":[0,1,2],"States":[128,64,32],"Output":{"FREE":0,"Restricted":1,"Stop":2}},{"Name":"Spoor 2 ","Type":2,"Block":9,"Side":"N","EnStates":224,"IOAddress":[3,4,5],"States":[128,64,32],"Output":{"FREE":3,"Restricted":4,"Stop":5}},{"Name":"Spoor 3E","Type":2,"Block":14,"Side":"N","EnStates":224,"IOAddress":[6,7,8],"States":[128,64,32],"Output":{"FREE":6,"Restricted":7,"Stop":8}},{"Name":"Spoor 4E","Type":1,"Block":18,"Side":"N","EnStates":224,"IOAddress":[9,10,11],"States":[128,64,32],"Output":{"FREE":9,"Restricted":10,"Stop":11}},{"Name":"Spoor 5E","Type":1,"Block":22,"Side":"N","EnStates":224,"IOAddress":[12,13,14],"States":[128,64,32],"Output":{"FREE":12,"Restricted":13,"Stop":14}},{"Name":"Spoor 3W","Type":2,"Block":17,"Side":"P","EnStates":224,"IOAddress":[15,16,17],"States":[128,64,32],"Output":{"FREE":15,"Restricted":16,"Stop":17}},{"Name":"Spoor 4W","Type":1,"Block":21,"Side":"P","EnStates":224,"IOAddress":[18,19,20],"States":[128,64,32],"Output":{"FREE":18,"Restricted":19,"Stop":20}},{"Name":"Spoor 5W","Type":1,"Block":25,"Side":"P","EnStates":224,"IOAddress":[21,22,23],"States":[128,64,32],"Output":{"FREE":21,"Restricted":22,"Stop":23}}],"Stations":[{"Name":"Spoor 1","Type":1,"NrOf":4,"Blocks":"2 3 4 5"},{"Name":"Spoor 2","Type":1,"NrOf":4,"Blocks":"6 7 8 9"},{"Name":"Spoor 3","Type":1,"NrOf":4,"Blocks":"14 15 16 17"},{"Name":"Spoor 4","Type":1,"NrOf":4,"Blocks":"18 19 20 21"},{"Name":"Spoor 5","Type":1,"NrOf":4,"Blocks":"22 23 24 25"}],"LineCount":33,"NrSignals":8,"NrStation":5,"HOA":23,"HIA":53,"Name":"Station 4 blocks","ModuleNr":4,"ConW":1290,"ConH":290,"LAnchor":{"x":"20","y":"100"},"RAnchor":{"x":"1210","y":"100"}}';
  echo("No request_body, using old data");
}
$myfile = fopen($timestamp, "w");

$listFile = fopen("./../modules/list.txt","r");
$OptionList = json_decode(fread($listFile,filesize("./../modules/list.txt")),true);
fclose($listFile);

$data = json_decode($request_body,true);

if($data == null){
  http_response_code(415);
}
//echo( prettyPrint($request_body));
echo("<br/><br/>");

$response_text  = CreatePropertiesContent($data);
$response2_text = CreateConnectContent($data);

print("<pre>");

$OptionList[$data['ModuleNr']]["name"] = $data["Name"];
$OptionList[$data['ModuleNr']]["blocks"]   = count($data['Blocks']);
$OptionList[$data['ModuleNr']]["switch"]   = count($data['Switch']);
$OptionList[$data['ModuleNr']]["signals"]  = count($data['Signals']);
$OptionList[$data['ModuleNr']]["stations"] = count($data['Stations']);
$writeOptionList = json_encode($OptionList,JSON_PRETTY_PRINT);
$listFile = fopen("./../modules/list.txt","w");
fwrite($listFile,$writeOptionList);
fclose($listFile);

//print_r($writeOptionList);
//print_r($data);

echo("Name:".$data["Name"]."<br/>");
echo("Blocks: ".count($data['Blocks'])."<br/>");
echo("Switches: ".count($data['Switch'])."<br/>");
echo("Signals: ".count($data['Signals'])."<br/>");
echo("Station: ".count($data['Stations'])."<br/>");


var_dump($response_text);
var_dump($response2_text);
fwrite($myfile,$response_text);
print("</pre>");


  //$data = serialize($data);

  //fwrite($myfile,$request_body);

fclose($myfile);
 ?>
