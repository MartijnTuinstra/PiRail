var Emergency = {
	enabled: false,
	type:"",
	timer:0,
	start: function(type,w=true){
		if(this.enabled == false){
			if(window.navigator.vibrate != undefined){
				window.navigator.vibrate([100,50,100,50,100]);
			}
			$('.Emergency_btn').off("click");
			$('.Emergency_btn').on("click",function(){Emergency.stop()});
			if(w == true && ws.connected)
				websocket.cts_Emergency();
			this.timer = setInterval(function(){
				if($('.Emergency_btn').css('background-color') == 'rgb(244, 67, 54)'){
					$('.Emergency_btn').css('background','#FFC107');
				}else{
					$('.Emergency_btn').css('background','#F44336');
				}
			},500);

			if(type == "Es"){
				$('.Emergency_btn').toggleClass("Emergency_Es");
				this.type = "Es";
			}else{
				$('.Emergency_btn').toggleClass("Emergency_Em");
				this.type = "Em";
			}

			this.enabled = true;
		}
	},
	stop: function(w=true){
		if(this.enabled){
			this.enabled = false;
			if(w == true && ws.connected){
				console.warn("RESEND");
				websocket.cts_release_Emergency();
			}
			$('.Emergency_btn').off("click");
			$('.Emergency_btn').on("click",function(){Emergency.start("Em")});
			clearInterval(this.timer);
			$('.Emergency_btn').css('background','#F44336');

			if($('.Emergency_btn').hasClass("Emergency_Es")){
				$('.Emergency_btn').toggleClass("Emergency_Es");
			}else{
				$('.Emergency_btn').toggleClass("Emergency_Em");
			}
		}
	}
}
