var Messages = {
	open_messages: [],
	popup_messages: [],
	message_counter: 0,

	add: function(msg){

		var id = msg.id;
		this.open_messages[id] = msg;

		//Check type
		if(msg.type == 0){ //New train
			msg.data[0] // Folow ID
			msg.data[1] // Found on module
			msg.data[2] // and Section
			text = '<div class="message mID'+id+'">'+
					'<div class="color-box warning"></div>'+
					'<div class="header">A new Train</div>'+
					'<div class="content">Near '+msg.data[1]+':'+msg.data[2]+'</div>'+
				   '</div>'

			$('#settings .messages').append(text);
			this.show(text,id);

			$('#settings .messages .mID'+id+', #message_popup .messages .mID'+id).css("cursor", "pointer");
			$('#settings .messages .mID'+id+', #message_popup .messages .mID'+id).on("click", function(evt){
				Train.linker.open(msg);
				Messages.hide(id);
			});

			setTimeout(function(){Messages.hide(id);},5000);
		}
		else if(msg.type == 0x20){ //Split train in yard
			msg.data[0] // Folow ID
			msg.data[1] // Found A on module
			msg.data[2] // and Section
			msg.data[3] // Found B on module
			msg.data[4] // and Section
			text = '<div class="message mID'+id+'">'+
					'<div class="color-box warning"></div>'+
					'<div class="header">Train has split in a yard</div>'+
					'<div class="content">Near '+msg.data[1]+':'+msg.data[2]+'</div>'+
				   '</div>'

			$('#settings .messages').append(text);
			this.show(text,id);
			setTimeout(function(){Messages.hide(id);},5000);
		}
		else if(msg.type == 0x40){ //Split train on mainline
			msg.data[0] // Folow ID
			msg.data[1] // Found A on module
			msg.data[2] // and Section
			msg.data[3] // Found B on module
			msg.data[4] // and Section
			text = '<div class="message mID'+id+'">'+
					'<div class="color-box servere"></div>'+
					'<div class="header">Train has split on main line</div>'+
					'<div class="content">Near '+msg.data[1]+':'+msg.data[2]+'</div>'+
				   '</div>'

			$('#settings .messages').append(text);
			this.show(text,id);
		}
		else if(msg.type == 0xFF){ // Disconnected
			text = '<div class="message mID'+id+'">'+
					'<div class="color-box servere"></div>'+
					'<div class="header">Reconnecting</div>'+
					'<div class="content"></div>'+
				   '</div>'

			$('#settings .messages').append(text);
			this.show(text,id);
		}
        else{
            console.warn("add Messages.add() type "+msg.type);
        }
		if(msg[0] == 0){
			setTimeout(function(){Messages.hide(id);},5000);
		}

		return id;
	},

	show: function(text,id){
		$('#message_popup').show();
		this.popup_messages.push(id);
		$('#message_popup .messages').append(text);
	},

	hide: function(id){
		if(this.popup_messages.indexOf(id) >= 0){
			console.warn("HIDE");
			$('#message_popup .messages .mID'+id).remove();
			this.popup_messages.pop(this.popup_messages.indexOf(id));
			if(this.popup_messages.length == 0)
				$('#message_popup').hide();
		}
	},

	update: function(msg){
		console.warn("add Messages.update()");
	},

	remove: function(msg_id, ret_code){
		msg_index = -1;
		for (var i = 0; i < this.open_messages.length; i++) {
			if(this.open_messages[i] != undefined && this.open_messages[i].id == msg_id){
				msg_index = i;
				break;
			}
		}

		if(msg_index == -1){
			console.warn("MSG_ID not found");
			return;
		}

		if(this.open_messages[msg_index].type == 0){ //NewTrain
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

		this.open_messages.pop(msg_index);
		$('#message_popup .messages .mID'+msg_id).off();

		$('#settings .messages .message.mID'+msg_id).remove();
		this.hide(msg_id);
	},

	clear: function(){
		console.log("Clearing messages");
		this.open_messages.forEach(function(element){
			console.log('Msg', element.id);
			Messages.remove(element.id);
		});
		this.open_messages = [];
	}
}
