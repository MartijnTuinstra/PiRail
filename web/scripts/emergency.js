var Emergency = {
	enabled: false,
	type:"",
	timer:0,
	init: function(){
		$(".emergencybutton").on("click", this.toggle.bind(this));
	},
	toggle: function(){
		console.log("Emergency button toggle");
		if(this.enabled == false){
			this.set();
		}
		else{
			this.unset();
		}
	},
	set: function(ws=true){
		if(this.enabled)
			return
		console.warn("Emergency set");
		if(window.navigator.vibrate != undefined){
			window.navigator.vibrate([100,50,100,50,100]);
		}
		this.enabled = true;

		if(ws)
			websocket.cts_EmergencyStop();

		$('.emergencybutton').css('color','white');
		$('.emergencybutton').css('background','red');

		this.timer = setInterval(function(){
			if($('.emergencybutton').css('background-color') == 'rgb(255, 0, 0)'){
				$('.emergencybutton').css('background','orange');
			}else{
				$('.emergencybutton').css('background','red');
			}
		},500);
	},
	unset: function(ws=true){
		if(!this.enabled)
			return
		console.warn("Emergency unset");
		this.enabled = false;

		clearInterval(this.timer);
		$('.emergencybutton').css('background','white');
		$('.emergencybutton').css('color','red');

		if(ws)
			websocket.cts_ClearEmergency();
	}
}

Emergency.init();
