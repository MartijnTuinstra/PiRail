
class message {
	constructor(id, type, data){
		this.id = id;
		this.type = type;
		this.data = data;
		this.modal = undefined;
	}

	draw(){
		var header, content, color;
		if(this.type == 0){ // New train
			header = "New train";
			content = "at "+this.data.location;
			color = "warning";
			this.modal = "train.link";
		}
		else if(this.type == 0x20){
			header = "Train split";
			content = "at "+this.data.location;
			color = "warning";
		}
		else if(this.type == 0x40){
			header = "Train split";
			content = "at "+this.data.location;
			color = "danger";
		}
		else if(this.type == 0xff){
			header = "Disconnected";
			content = "reconnecting ...";
			color = "danger";
		}
		var text = '<li class="message media message-id-'+this.id+'">'+
				'<div class="message-mbox align-self-center mr-3 bg-'+color+'"></div>'+
				'<div class="message-body media-body">'+
				'<div class="mt-0 mb-0 message-header"><b>'+header+'</b></div>'+
				'<small class="message-content"><i>'+content+'</i></small>'+
				'</div></li>';
		$('.messages ul').append(text);

		$('.messages ul .message-id-'+this.id).on("click", this.hit.bind(this));
	}
	hit(){
		if(this.modal != undefined){
			Modals.open(this.modal, this.data);
		}
	}
	remove(){
		$('.messages .message.message-id-'+this.id).remove();
	}
}

var Messages = {
	messages: {},
	popup_messages: [],
	message_counter: 1,

	add: function(msg){

		var id = msg.id;

		var m;

		//Check type
		if(msg.type == 0){ //New train
			// msg.data[0] // Folow ID
			// msg.data[1] // Found on module
			// msg.data[2] // and Section
			m = new message(id, 0, {fid: msg.data[0], location: msg.data[1]+":"+msg.data[2]});
			// text = '<div class="message mID'+id+'">'+
			// 		'<div class="color-box warning"></div>'+
			// 		'<div class="header">A new Train</div>'+
			// 		'<div class="content">Near '+msg.data[1]+':'+msg.data[2]+'</div>'+
			// 	   '</div>'

			// $('#settings .messages').append(text);
			// this.show(text,id);

			// $('#settings .messages .mID'+id+', #message_popup .messages .mID'+id).css("cursor", "pointer");
			// $('#settings .messages .mID'+id+', #message_popup .messages .mID'+id).on("click", function(evt){
				// Train.linker.open(msg);
				// Messages.hide(id);
			// });

			// setTimeout(function(){Messages.hide(id);},5000);
		}
		else if(msg.type == 0x20){ //Split train in yard
			// msg.data[0] // Folow ID
			// msg.data[1] // Found A on module
			// msg.data[2] // and Section
			// msg.data[3] // Found B on module
			// msg.data[4] // and Section
			m = new message(id, 0x20, {fid: msg.data[0], locationA: msg.data[1]+":"+msg.data[2], locationB: msg.data[3]+":"+msg.data[4]});
			// text = '<div class="message mID'+id+'">'+
			// 		'<div class="color-box warning"></div>'+
			// 		'<div class="header">Train has split in a yard</div>'+
			// 		'<div class="content">Near '+msg.data[1]+':'+msg.data[2]+'</div>'+
			// 	   '</div>'

			// $('#settings .messages').append(text);
			// this.show(text,id);
			// setTimeout(function(){Messages.hide(id);},5000);
		}
		else if(msg.type == 0x40){ //Split train on mainline
			// msg.data[0] // Folow ID
			// msg.data[1] // Found A on module
			// msg.data[2] // and Section
			// msg.data[3] // Found B on module
			// msg.data[4] // and Section
			m = new message(id, 0x40, {fid: msg.data[0], locationA: msg.data[1]+":"+msg.data[2], locationB: msg.data[3]+":"+msg.data[4]});
			// text = '<div class="message mID'+id+'">'+
			// 		'<div class="color-box servere"></div>'+
			// 		'<div class="header">Train has split on main line</div>'+
			// 		'<div class="content">Near '+msg.data[1]+':'+msg.data[2]+'</div>'+
			// 	   '</div>'

			// $('#settings .messages').append(text);
			// this.show(text,id);
		}
		else if(msg.type == 0xFF){ // Disconnected
			m = new message(0, 0xFF);
		}
        else{
            console.warn("add Messages.add() type "+msg.type);
        }
		if(msg[0] == 0){
			setTimeout(function(){Messages.hide(id);},5000);
		}
		else{
			m.draw();
		}
		this.messages[id] = m;
	},

	update: function(msg){
		console.warn("add Messages.update()");
	},

	remove: function(msg_id, ret_code){
		msg_index = -1;
		for (var i = 0; i < this.messages.length; i++) {
			if(this.messages[i] != undefined && this.messages[i].id == msg_id){
				msg_index = i;
				break;
			}
		}

		if(msg_index == -1){
			console.warn("MSG_ID not found");
			return;
		}

		if(this.messages[msg_index].type == 0){ //NewTrain
			if(ret_code == 0){
				alert("Failed");
				return;
			}else if(ret_code == 2){
				alert("Engine allready in use");
				return;
			}
			else if(ret_code == 3){
				alert("Train_link allready in use");
				return;
			}
		}

		this.messages[msg_index].remove();
	},

	clear: function(){
		console.log("Clearing messages");
		$.each(this.messages, function(index, element){
			element.remove();
		});
		this.messages = [];
	}
}

class submodule {
	constructor(offset, size, enable_bit, title, description){
		this.enable_bit = enable_bit;
		this.offset = offset;
		this.mask = (0xFF >> (8-size));
		this.title = title;
		this.state = 0;
		this.description = description;
		this.colors = ["danger", "warning", "success", "danger", "warning", "success"];
		this.create();
	}

	check_flag(flags){
		if((flags & (this.mask << this.offset)) != (this.state << this.offset)){
			return true;
		}
		return false;
	}

	create(){
		var text = '<li class="media submodule-'+this.title.toLowerCase()+'" style="border-bottom:1px solid #ddd; padding: 0px 30px">'+
					'<div class="indicator align-self-center mr-3 bg-'+this.colors[this.state]+'" style="width:16px;height:16px;border-radius: 3px;"></div>'+
					'<div class="media-body">'+
					'<div class="mt-0 mb-0" style="position: relative; top: 0.2em; font-weight: bold;">'+this.title+'</div>'+
					'<small style="position:relative; top:-0.3em; font-style: italic;">'+this.description[this.state]+'</small>'+
					'</div></li>';
		$('.sidebar #subsystems ul').append(text);
		$('.sidebar #subsystems li.submodule-'+this.title.toLowerCase()).on("click", this.hit.bind(this));
	}

	hit(evt){
		if(this.state == 0 || this.state == 1){
			websocket.cts_EnableSubModule(this.enable_bit);
		}
		else{
			websocket.cts_DisableSubModule(this.enable_bit);
		}
	}

	update(flags){
		flags = (flags[0] << 8) + flags[1];
		if(!this.check_flag(flags)){
			return;
		}

		var box = $('.sidebar #subsystems li.submodule-'+this.title.toLowerCase())
		$('.indicator', box).removeClass("bg-"+this.colors[this.state]);
		$('.indicator', box).removeClass("blink");
		this.state = (flags >> this.offset) & this.mask;
		$('.indicator', box).addClass("bg-"+this.colors[this.state]);
		if(this.state == 3 || this.state == 5){
			$('.indicator', box).addClass("blink");
		}
		$('small', box).html(this.description[this.state]);
	}
}

var Submodules = {
	modules: [],
	init: function(){
		this.modules.push(new submodule(14, 2, 7, "Websocket", ["Stopped", "Admin only", "Running", "Failure"]));
		this.modules.push(new submodule(12, 2, 6, "Z21", ["Stopped", "Initializing", "Running", "Failure"]));
		this.modules.push(new submodule(10, 2, 5, "UART", ["Stopped", "Initializing", "Running", "Failure"]));
		this.modules.push(new submodule( 5, 3, 4, "LayoutControl", ["Stopped", "Initializing", "Running", "Failure", "Finding rails", "Connecting rails"]));
		this.modules.push(new submodule( 8, 2, 3, "TrainControl", ["Stopped", "Initializing", "Running", "Failure"]));
		this.modules.push(new submodule( 3, 2, 2, "SimA", ["Stopped", "Initializing", "Running", "Failure"]));
		this.modules.push(new submodule( 1, 2, 1, "SimB", ["Stopped", "Initializing", "Running", "Failure"]));
	},
	update: function(flags){
		for(var i = 0; i < this.modules.length; i++){
			this.modules[i].update(flags)
		}
	}
}

Submodules.init();