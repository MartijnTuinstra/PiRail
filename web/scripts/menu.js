var Menu = {
	min: -185,
	open: {track:true,trains:false,stations:false,settings:false},
	window_sizes: {"track":{min_h:500},"trains":{min_h:300},"route":{min_h:300},"settings":{min_h:700}},
	window_height: window.innerHeight - 90,
	init: function(){
		this.resize();
		if (this.window_type == 3){
			$('.menu .menu_btn.trains').css('display','none');
			this.min = -115;
		}else{
			$('.menu .menu_btn.trains').css('display','block');
			this.min = -185;
		}
		$('.menu').css("bottom",this.min+"px");
	},
	resize: function(){
		this.window_height = window.innerHeight - 90;
		this.window_width  = window.innerWidth;

		this.window_type = -1;

		if(this.window_width < 600 && this.window_height < 800){
			this.window_type = 0; //Mobile
		}
		else if(this.window_width < 1030 && this.window_height < 800){
			this.window_type = 1; //Tablet landscape
		}
		else if(this.window_width < 800 && this.window_height < 1010){
			this.window_type = 2; //Portrait Tablet
		}
		else{
			this.window_type = 3; //Large desktop
			if($('#route').hasClass("vertical")){
				$('#route').toggleClass("vertical");
				$('#route').toggleClass("horizontal");
			}
		}

		if(!$('#route').hasClass("vertical") && this.window_type != 3){
			$('#route').toggleClass("vertical");
			$('#route').toggleClass("horizontal");
		}

		$('#track, #trains, #route, #settings').css("height",this.window_height+"px");

		this.open_window();
	},
	toggle: function(){
		if($('.menu').hasClass("active")){
			$('.menu').css("bottom",this.min+"px");
		}else{
			$('.menu').css("bottom","25px");
		}
		$('.menu').toggleClass("active");
		$('.menu_toggle_btn img').toggleClass("active");
	},
	open_window: function(w){
		if($('#trains').hasClass("half")){
			$('#trains').toggleClass("half");
		}

		if(this.window_type == 0){
			//Mobile
			$('#track, #trains, #route, #settings').css("display","none");
			if(w == "trains"){
				this.open.trains = !this.open.trains;
				this.open.stations = false;
				this.open.settings = false;
			}
			if(w == "route"){
				this.open.stations = !this.open.stations;
				this.open.trains = false;
				this.open.settings = false;
			}
			if(w == "settings"){
				this.open.settings = !this.open.settings;
				this.open.trains = false;
				this.open.stations = false;
			}
			if(!this.open.trains && !this.open.stations && !this.open.settings){
				$('#track').css("display","block");
			}
			if(this.open.trains){
				$('#trains').css("display","block");
			}
			if(this.open.stations){
				$('#route').css("display","flex");
			}
			if(this.open.settings){
				$('#settings').css("display","block");
			}

			$('#track, #trains, #route').css("height",this.window_height+"px");

			return;
		}
		else if(this.window_type == 1){
			//Tablet landscape
			$('#track, #trains, #route, #settings').css("display","none");
			if(w == "trains"){
				this.open.trains = !this.open.trains;
			}
			if(w == "route"){
				this.open.stations = !this.open.stations;
			}
			if(w == "settings"){
				this.open.settings = !this.open.settings;
				this.open.stations = false;
				this.open.trains = false;
			}

			if(!this.open.stations && !this.open.trains && !this.open.settings){
				$('#track').css("height",this.window_height+"px");
				$('#track').css("display","block");
			}
			else if(this.open.stations){
				if(!this.open.trains){
					$('#route').css("display","flex");
					$('#route').css("width","100%");
				}
				else{
					if(!$('#trains').hasClass("half")){
						$('#trains').toggleClass("half");
					}

					$('#route').css("width","50%");
					$('#route').css("display","flex");
					$('#trains').css("display","block");
				}
			}
			else if(this.open.trains){
				if(!this.open.stations){
					$('#trains').css("display","block");
				}
				else{
					if(!$('#trains').hasClass("half")){
						$('#trains').toggleClass("half");
					}

					$('#route').css("width","50%");
					$('#route').css("display","flex");
					$('#trains').css("display","block");
				}
			}
			else if(this.open.settings){
				$('#settings').css("display","block");
			}

			$('#track, #trains, #route').css("height",this.window_height+"px");

			Canvas.resize()

			return;
		}
		else if(this.window_type == 2){
			//Portrait Tablet
			$('#trains, #route, #route, #settings').css("display","none");
			$('#track').css("display","block");
			if(w == "trains"){
				this.open.trains = !this.open.trains;
			}
			if(w == "route"){
				this.open.stations = !this.open.stations;
			}
			if(w == "settings"){
				this.open.settings = !this.open.settings;
				this.open.stations = false;
				this.open.trains = false;
			}

			if(!this.open.stations && !this.open.trains && !this.open.settings){
				$('#track').css("height",this.window_height+"px");
			}
			else if(this.open.stations){
				if(!this.open.trains){
					$('#route').css("display","flex");
					$('#route').css("height","500px");
					$('#route').css("width","100%");
					$('#track').css("height",(this.window_height-500)+"px");
				}
				else{
					if(!$('#trains').hasClass("half")){
						$('#trains').toggleClass("half");
					}

					$('#route').css("width","50%");
					$('#route').css("display","flex");
					$('#route').css("height","500px");
					$('#trains').css("display","block");
					$('#trains').css("height","500px");
					$('#track').css("height",(this.window_height-500)+"px");
				}
			}
			else if(this.open.trains){
				if(!this.open.stations){
					$('#trains').css("display","block");
					$('#trains').css("height","500px");
					$('#track').css("height",(this.window_height-500)+"px");
				}
				else{
					if(!$('#trains').hasClass("half")){
						$('#trains').toggleClass("half");
					}

					$('#route').css("width","50%");
					$('#route').css("display","flex");
					$('#route').css("height","500px");
					$('#trains').css("display","block");
					$('#trains').css("height","500px");
					$('#track').css("height",(this.window_height-500)+"px");
				}
			}
			else if(this.open.settings){
				$('#track').css("display","none");
				$('#settings').css("display","block");
			}

			Canvas.resize()

			return;
		}
		else{
			$('#trains, #route, #settings').css("display","none");
			$('#track').css("height",this.window_height+"px");
			if(w == "route"){
				this.open.stations = !this.open.stations;
			}
			if(w == "settings"){
				this.open.settings = !this.open.settings;
				this.open.stations = false;
			}

			if(this.open.stations){
				$('#route').css("display","flex");
				$('#route').css("height",355+"px");
				$('#track').css("height",(this.window_height-355)+"px");
				
			}
			else if(this.open.settings){
				$('#track').css("display","none");
				$('#settings').css("display","block");
			}
			else{
				$("#track").css("display","block");
			}
			

			Canvas.resize();

			return;
		}
	}
}

