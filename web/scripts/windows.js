var menu = 0;
var ImgMTrains = false;

function menu_but(){
  if(menu == 1){
    $('#menu').css('display','none');
    menu = 0;
  }else{
    $('#menu').css('display','block');
    menu = 1;
  }
}

function switch_win(){
  $('#index').css('display','none');
  $('#MModules').css('display','none');
  $('#switches').css('display','block');
  $('#track').css('display','none');
  $('#MTrain').css('display','none');
  $('#CTrain').css('display','none');
}

function track_win(){
  $('#index').css('display','none');
  $('#MModules').css('display','none');
  $('#track').css('display','block');
  $('#switches').css('display','none');
  $('#MTrain').css('display','none');
  $('#CTrain').css('display','none');
}

function mTrain_win(){
  if(ImgMTrains == false){LoadImgManageTrains();}
    $('#index').css('display','none');
    $('#MTrain').css('display','block');
    $('#MModules').css('display','none');
    $('#switches').css('display','none');
    $('#track').css('display','none');
    $('#CTrain').css('display','none');
}

function cTrain_win(){
  $('#index').css('display','none');
  $('#CTrain').css('display','block');
  $('#MModules').css('display','none');
  $('#switches').css('display','none');
  $('#track').css('display','none');
  $('#MTrain').css('display','none');
}

function index_win(){
  $('#index').css('display','block');
  $('#MModules').css('display','none');
  $('#CTrain').css('display','none');
  $('#switches').css('display','none');
  $('#track').css('display','none');
  $('#MTrain').css('display','none');
}

function mModules_win(){
  $('#MModules').css('display','block');
  $('#index').css('display','block');
  $('#CTrain').css('display','none');
  $('#switches').css('display','none');
  $('#track').css('display','none');
  $('#MTrain').css('display','none');
}


function settings(){
  if($('#Settings_box').css('display') == 'none'){
    $('#Settings_box').css('display','block');
  }else{
    $('#Settings_box').css('display','none');
  }
}

function warning_toggle(){
  if($('#warning_list').css('display') == 'none' && $("#warning_list").html() != ""){
    $('#warning_list').css('display','block');
  }else{
    $('#warning_list').css('display','none');
  }
}
