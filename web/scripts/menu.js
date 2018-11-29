var Menu = {
	init: function(){
		this.buttons = {
			"Settings": {name: "Settings", page: "settings-group"},
			"Train": {name: "Train", page: "train-group"},
			"Layout": {name: "Layout", page: "main"},
		};
		this.current = undefined;
		this.page_size = window.innerWidth;
		this.draw();
		if(this.page_size < 1140){
			this.toggle({currentTarget: $('#menu-box button[target=Layout]')[0]})
		}else{
			this.toggle({currentTarget: $('#menu-box button[target=Train]')[0]})
		}
	},
	resize: function(){
		this.page_size = window.innerWidth;
		this.draw();
	},

	button: function(name){
		var text = '<button type="button" class="btn btn-xs btn-outline-primary" target="'+name+'">'+name+
					 '</button>';
		return text;
	},
	draw: function(){
		$('#menu-box button').off();
		$('#menu-box').empty();
		if(this.page_size < 1140){
			$('#menu-box').css({"right": "10px", "text-align": "right"});
			$('#menu-box').append(this.button(this.buttons["Settings"].name));
			$('#menu-box').append(this.button(this.buttons["Train"].name));
			$('#menu-box').append(this.button(this.buttons["Layout"].name));

			if(this.current != undefined && this.current.name != "Settings" && this.current.name != "Train"){
				$('body .sidebar').hide();
			}
			else if(this.current != undefined && this.current.name != "Layout"){
				$('body .main').hide();
			}
		}
		else{
			$('body .sidebar').show();
			$('body .main').show();
			$('#menu-box').css({"left": "10px", "text-align": "left"});
			$('#menu-box').append(this.button(this.buttons["Train"].name));
			$('#menu-box').append(this.button(this.buttons["Settings"].name));
		}

		if(this.current != undefined){
			$('#menu-box button[target='+this.current.name+']').removeClass("btn-outline-primary");
			$('#menu-box button[target='+this.current.name+']').addClass("btn-primary");
		}

		$('#menu-box button').on("click", this.toggle.bind(this));
	},
	toggle: function(evt){
		var page = evt.currentTarget.getAttribute("target");

		if(this.current != undefined){
			$('#menu-box button[target='+this.current.name+']').addClass("btn-outline-primary");
			$('#menu-box button[target='+this.current.name+']').removeClass("btn-primary");
		}

		$('body .train-group').hide();
		$('body .settings-group').hide();
		if(this.page_size < 1140){
			$('body .main').hide();
			$('body .sidebar').hide();
		}

		this.current = this.buttons[page];

		$('body .'+this.current.page).show();

		if(this.current.name == "Settings" || this.current.name == "Train"){
			$('body .sidebar').show();
		}

		$('#menu-box button[target='+page+']').removeClass("btn-outline-primary");
		$('#menu-box button[target='+page+']').addClass("btn-primary");
	}
}

init_list.push(Menu.init.bind(Menu));
resize_list.push(Menu.resize.bind(Menu));