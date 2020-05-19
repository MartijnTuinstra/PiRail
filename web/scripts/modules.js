function print_rail_link(link){
	switch(link.type){
		case 0:
			return link.m+":"+link.id+" R";
		case 1:
			return link.m+":"+link.id+" S";
		case 2:
			return link.m+":"+link.id+" s";
		case 3:
			return link.m+":"+link.id+" M";
		case 4:
			return link.m+":"+link.id+" m";
		case 0xfe:
			return "C "+link.m+":"+link.id;
		case 0xff:
			return "E";
	}
}

var ModuleEditor = {
	open: 0,

	list328: [['R', 240,86.5],['R', 240,110.5],['R', 240,134.4],['R', 240,158.3],['R', 240,182.2],['R', 240,206.2],['R', 240,230.1],['R', 240,254.0],['R', 240,278.0],['R', 240,301.9],['R', 240,325.8],['R', 240,349.8]],
	list64: [['L',86.6,408.3],['L',86.6,384],['L',86.6,360.1],['L',86.6,336.1],['L',86.6,312.1],['L',86.6,288.1],['L',86.6,264.2],['L',86.6,240.2],['L',86.6,216.2],['L',86.6,192.3],['L',86.6,168.3],['L',86.6,144.3],['L',86.6,120.3],['L',86.6,96.4],['L',86.6,72.4],['L',86.6,48.4],['T',269.7,-13],['T',293.7,-13],['T',317.7,-13],['T',341.7,-13],['T',365.6,-13],['T',389.6,-13],['T',413.6,-13],['T',437.5,-13],['R',608.2,48.8],['R',608.2,72.8],['R',608.2,96.7],['R',608.2,120.7],['R',608.2,144.7],['R',608.2,168.6],['R',608.2,192.6],['R',608.2,216.6],['R',608.2,240.5],['R',608.2,264.5],['R',608.2,288.5],['R',608.2,312.5],['R',608.2,336.4],['R',608.2,360.4],['R',608.2,384.4],['R',608.2,408.3]],
	list2560: [['R',50,397.5],['L',190,397.6],['R',50,373.5],['L',190,373.6],['R',50,349.6],['L',190,349.6],['R',50,325.6],['L',190,325.7],['R',50,301.6],['L',190,301.7],['R',50,277.7],['L',190,277.7],['R',50,253.7],['L',190,253.7],['R',50,229.7],['L',190,229.8],['R',50,205.8],['L',190,205.8],['R',50,181.8],['L',190,181.8],['R',50,157.8],['L',190,157.9],['R',50,133.9],['L',190,133.9],['B',365.4,-157],['T',365.4,-17],['B',389.4,-157],['T',389.3,-17],['B',413.4,-157],['T',413.3,-17],['B',437.3,-157],['T',437.3,-17],['B',461.3,-157],['T',461.2,-17],['B',485.3,-157],['T',485.2,-17],['B',509.2,-157],['T',509.2,-17],['B',533.2,-157],['T',533.1,-17],['B',557.1,-157],['T',557.1,-17],['B',581.1,-157],['T',581.1,-17],['B',605.1,-157],['T',605.1,-17],['B',629,-157],['T',629,-17],['L',940,133.9],['R',800,133.9],['L',940,157.9],['R',800,157.9],['L',940,181.9],['R',800,181.8],['L',940,205.8],['R',800,205.8],['L',940,229.8],['R',800,229.8],['L',940,253.8],['R',800,253.7],['L',940,277.8],['R',800,277.7],['L',940,301.7],['R',800,301.7],['L',940,325.7],['R',800,325.6],['L',940,349.7],['R',800,349.6],['L',940,373.6],['R',800,373.6],['L',940,397.6],['R',800,397.5]],

	init: function(){
		if(loading_modules != 0){
			setTimeout(ModuleEditor.init.bind(ModuleEditor), 100);
			return;
		}

		$("#moduleconfig #ModuleSettings td[name='cog']").html(settings("#ccc", 23));
		$("#moduleconfig #ModuleSettings td[name='cog'] svg").on("click", function(){
			Modals.open("module.settings", ModuleEditor.open);
		});

		this.update();
	},

	open: function(module){

	},

	update: function(module){
		$("#moduleconfig div > canvas").off("click");
		$("#moduleconfig .moduleContainer").empty();
		content = "";

		width = 10;

		function lineColour(b, s=0, cnvs){
			(s == 0)?cnvs.strokeStyle = "#aaa":cnvs.strokeStyle = "#ddd";
		}

		for(const key of Object.keys(modules)){
			var scaling = 1;
			if(modules[key].height > 200){
				scaling = 200 / modules[key].height;
			}

			content = '<div class="modulebox" style="width:'+(modules[key].width*scaling)+'px;"> \
											<canvas id="EditorModule'+key+'"></canvas> \
											<div class="modulename">'+key+' - '+modules[key].name+'</div> \
										</div>';

			width += (modules[key].width*scaling) + 5;


			$("#moduleconfig .moduleContainer").append(content);			

			$("#moduleconfig div > canvas#EditorModule"+key).on("click", function(){ModuleEditor.loadModule(key)});

			query = $('#moduleconfig #EditorModule'+key);
			cnvs = query[0].getContext('2d');

			query[0].width = (modules[key].width*scaling);
			query[0].height = 200;

			cnvs.setTransform(scaling, 0, 0, scaling, 0, 0);
			cnvs.lineWidth = 6;

			modules[key].draw_background(cnvs, lineColour, {X:0,Y:0,R:0});
			modules[key].draw_foreground(cnvs, lineColour, {X:0,Y:0,R:0});
		}

		$("#moduleconfig .moduleContainer").css("width", width);
	},

	loadModule: function(module){
		ModuleEditor.open = parseInt(module);

		websocket.cts_TrackLayoutRawData({module: module});

		$("#moduleconfig .moduleContainer .modulebox.selected").removeClass("selected");
		$("#moduleconfig .moduleContainer .modulebox canvas#EditorModule"+module).parent().addClass("selected");

		$("#moduleconfig #ModuleLayoutConfigurator tbody").empty();

		$("#moduleconfig #ModuleLayoutConfigurator tbody").append(modules[module].configdata());

		$("#moduleconfig #ModuleLayoutConfigurator svg").on("click", function(evt){
			console.log($("th", $(evt.target).closest("tr")));
			var id = parseInt($("th", $(evt.target).closest("tr")).text());

			module = modules[ModuleEditor.open].data[id];

			if(module == undefined){
				console.warn("No Module data obj found ("+ModuleEditor.open+", "+id+")");
			}

			Modals.open("module."+module.edit_type, {m: ModuleEditor.open, id:id});
		});

		$("#moduleconfig #ModuleSettings td[name='name']").text(modules[ModuleEditor.open].name);
		$("#moduleconfig #ModuleSettings td[name='dim']").text(modules[ModuleEditor.open].width + " / " + modules[ModuleEditor.open].height);
		$("#moduleconfig #ModuleSettings td[name='blocks']").text(modules[ModuleEditor.open].blocks.length);
		$("#moduleconfig #ModuleSettings td[name='switches']").text(modules[ModuleEditor.open].switches.length);

		var text = "";
		for(var i = 0; i < modules[ModuleEditor.open].connections.length; i++){
			text += "X: " + modules[ModuleEditor.open].connections[i].x + ", Y: " + modules[ModuleEditor.open].connections[i].y + ", R: " + modules[ModuleEditor.open].connections[i].r + "<br/>";
		}
		$("#moduleconfig #ModuleSettings td[name='anchor']").html(text);
	},

	selectNode: function(evt){
		var id = 0;
		if(evt != undefined){
			id = parseInt($(evt.currentTarget).attr("node"));
		}

		// $('#moduleNodes .dropdown-menu[node='+id+']').addClass("active");
		$('#moduleNodes .dropdown .dropdown-toggle').innerHTML = "Node "+id;

		$('#moduleNodes #board328').hide();
		$('#moduleNodes #board64').hide();
		$('#moduleNodes #board2560').hide();


		options = ["", "Output", "Blink", "Servo", "PWM", "Input", "InB", "InSw", "InMSSw"];

		$("#board328 .node328pins, #board64 .node328pins, #board2560 .node2560pins").empty();

		if(modules[ModuleEditor.open].config.nodes[id].size == 12){
			$('#moduleNodes #board328').show();
			for(var i = 0; i < ModuleEditor.list328.length; i++){
			    document.querySelector("#board328 .node328pins").appendChild(ModuleEditor.svg_balloon(ModuleEditor.list328[i], i, options[modules[ModuleEditor.open].config.nodes[id].data[i]]));
			}
		}
		else if(modules[ModuleEditor.open].config.nodes[id].size == 40){
			$('#moduleNodes #board64').show();
			for(var i = 0; i < ModuleEditor.list64.length; i++){
			    document.querySelector("#board64 .node64pins").appendChild(ModuleEditor.svg_balloon(ModuleEditor.list64[i], i, options[modules[ModuleEditor.open].config.nodes[id].data[i]]));
			}
		}
		else if(modules[ModuleEditor.open].config.nodes[id].size == 72){
			$('#moduleNodes #board2560').show();
			for(var i = 0; i < ModuleEditor.list2560.length; i++){
			    document.querySelector("#board2560 .node2560pins").appendChild(ModuleEditor.svg_balloon(ModuleEditor.list2560[i], i, options[modules[ModuleEditor.open].config.nodes[id].data[i]]));
			}
		}


		function clickNodeIOBalloon(evt){
			var pin = $(evt.currentTarget).attr("nodepin");
			var node = $(evt.target).closest("svg").parent().attr("id")
			Modals.open("module.io", {"pin": pin, "node": node});
		}

		$("#board64 .node64pins .iotextballoon").on("click",     clickNodeIOBalloon);
		$("#board328 .node328pins .iotextballoon").on("click",   clickNodeIOBalloon);
		$("#board2560 .node2560pins .iotextballoon").on("click", clickNodeIOBalloon);
	},

	update_config: function(module){
		ModuleEditor.open = module;
		$('#moduleNodes .dropdown-menu').empty();
		for(var i = 0; i < modules[module].config.nodes.length; i++){
			$('#moduleNodes .dropdown-menu').append('<a class="dropdown-item" href="#" node="'+i+'">Node '+i+' ('+modules[module].config.nodes[i].size+'IO)</a>');
		}
		$('#moduleNodes .dropdown-menu:first-child').addClass("active");
		$('#moduleNodes .dropdown .dropdown-toggle').innerHTML = "Node 0";
		$('#moduleNodes .dropdown-menu .dropdown-item').on("click", ModuleEditor.selectNode);

		// Initialize Node 0
		ModuleEditor.selectNode(undefined);


		if(modules[module].config.blocks.length == 0){
			$("#ModuleConfigurator table.blocks").closest(".info-box").hide();
		}
		else{
			$("#ModuleConfigurator table.blocks").closest(".info-box").show();
			$("#ModuleConfigurator table.blocks tbody").empty();
		}
		for(var i = 0; i < modules[module].config.blocks.length; i++){
			var b = modules[module].config.blocks[i];
			var text = "<tr><td>"+i+"</td><td>"+b.type+"</td><td>"+print_rail_link(b.next)+"</td><td>"+print_rail_link(b.prev)+"</td><td>"+
				                                          b.io_in.n+"-"+b.io_in.p+"</td><td>";
			if(b.flags & 0x08){
				text += b.io_out.n+"-"+b.io_out.p;
			}
			text += "</td><td>"+b.speed+"</td><td>"+b.flags+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
			$("#ModuleConfigurator table.blocks tbody").append(text);
		}

		if(modules[module].config.switches.length == 0){
			$("#ModuleConfigurator table.switches").closest(".info-box").hide();
		}
		else{
			$("#ModuleConfigurator table.switches").closest(".info-box").show();
			$("#ModuleConfigurator table.switches tbody").empty();
		}
		for(var i = 0; i < modules[module].config.switches.length; i++){
			var sw = modules[module].config.switches[i];
			var text = "<tr><td>"+i+"</td><td>"+sw.det_block+"</td><td>"+print_rail_link(sw.app)+
				                                          "</td><td>"+print_rail_link(sw.str)+"</td><td>"+print_rail_link(sw.div)+"</td><td>"+
				                                          sw.speed.str+", "+sw.speed.div+"</td><td>";
			var temp = [];
			for(var j = 0; j<sw.ports.length; j++){
				temp.push(sw.ports[j].m+"-"+sw.ports[j].id);
			}
			text += temp.join(", ")+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
			$("#ModuleConfigurator table.switches tbody").append(text);
		}

		if(modules[module].config.msswitches.length == 0){
			$("#ModuleConfigurator table.msswitches").closest(".info-box").hide();
		}
		else{
			$("#ModuleConfigurator table.msswitches").closest(".info-box").show();
			$("#ModuleConfigurator table.msswitches tbody").empty();
		}
		for(var i = 0; i < modules[module].config.msswitches.length; i++){
			var mssw = modules[module].config.msswitches[i];
			var text = "<tr><td>"+i+"</td><td>"+mssw.det_block+"</td><td>"+mssw.stateslen+"</td><td>"+mssw.iolen+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
			$("#ModuleConfigurator table.msswitches tbody").append(text);
		}

		if(modules[module].config.signals.length == 0){
			$("#ModuleConfigurator table.signals").closest(".info-box").hide();
		}
		else{
			$("#ModuleConfigurator table.signals").closest(".info-box").show();
			$("#ModuleConfigurator table.signals tbody").empty();
		}
		for(var i = 0; i < modules[module].config.signals.length; i++){
			var sig = modules[module].config.signals[i];
			var text = "<tr><td>"+i+"</td><td>"+sig.block+"</td><td>"+sig.side+"</td><td>";
			var temp = [];
			for(var j = 0; j<sig.ports.length; j++){
				temp.push(sig.ports[j].m+"-"+sig.ports[j].id);
			}
			text += temp.join(", ")+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
			$("#ModuleConfigurator table.signals tbody").append(text);
		}

		if(modules[module].config.stations.length == 0){
			$("#ModuleConfigurator table.stations").closest(".info-box").hide();
		}
		else{
			$("#ModuleConfigurator table.stations").closest(".info-box").show();
			$("#ModuleConfigurator table.stations tbody").empty();
		}
		for(var i = 0; i < modules[module].config.stations.length; i++){
			var st = modules[module].config.stations[i];
			var text = "<tr><td>"+i+"</td><td>"+st.name+"</td><td>"+st.type+"</td><td>"+st.blocks.join(", ")+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
			$("#ModuleConfigurator table.stations tbody").append(text);
		}

		$("#ModuleConfigurator svg").on("click", function(evt){
			var id = parseInt($("td:first-child", $(evt.target).closest("tr")).text());

			var data = {id: id, module: ModuleEditor.open};

			if($(evt.target).closest("table").hasClass("blocks")){
				Modals.open("module.block", data);
			}
			else if($(evt.target).closest("table").hasClass("switches")){
				Modals.open("module.switch", data);
			}
			else if($(evt.target).closest("table").hasClass("msswitches")){
				Modals.open("module.msswitch", data);
			}
			else if($(evt.target).closest("table").hasClass("signals")){
				Modals.open("module.signal", data);
			}
			else if($(evt.target).closest("table").hasClass("stations")){
				Modals.open("module.station", data);
			}
		});
	},


	svg_balloon: function(l, pin, option){
		const group = document. createElementNS("http://www.w3.org/2000/svg", "g");
		const circle = document. createElementNS("http://www.w3.org/2000/svg", "circle");
		const path = document. createElementNS("http://www.w3.org/2000/svg", "path");
		const text = document. createElementNS("http://www.w3.org/2000/svg", "text");

		var side = l[0];
		var x = l[1];
		var y = l[2];

		group.setAttribute("class", "iotextballoon");
		group.setAttribute("nodepin", pin);

		if(side == 'B'){
			group.setAttribute("style", "transform: translate("+x+"px, "+y+"px) rotate(90deg)");
		}
		else if(side == 'T'){
			group.setAttribute("style", "transform: translate("+x+"px, "+y+"px) rotate(-90deg)");
		}
		else{
			group.setAttribute("style", "transform: translate("+x+"px, "+y+"px)");
		}

	    circle.setAttribute("cx", "0");
	    circle.setAttribute("cy", "0");
	    circle.setAttribute("r", "5.2");

	    if(side == 'R' || side == 'T'){
		    text.setAttribute("x", "5");
		    text.setAttribute("y", "4");

	    	path.setAttribute("d", "M0,0 v-2 a10,10 0 0 1 10,-10 h50 a10,10 0 0 1 10,10 v4 a10,10 0 0 1 -10,10 h-50 a10,10 0 0 1 -10,-10 z");
	    }
	    else if(side == 'L'){
		    text.setAttribute("x", "-65");
		    text.setAttribute("y", "4");

	    	path.setAttribute("d", "M0,0 v-2 a10,10 0 0 0 -10,-10 h-50 a10,10 0 0 0 -10,10 v4 a10,10 0 0 0 10,10 h50 a10,10 0 0 0 10,-10 z");
	    }
	    else if(side == 'B'){
		    text.setAttribute("x", "-65");
		    text.setAttribute("y", "4");
		    text.setAttribute("style", "transform: rotate(180deg)");
	    	path.setAttribute("d", "M0,0 v-2 a10,10 0 0 1 10,-10 h50 a10,10 0 0 1 10,10 v4 a10,10 0 0 1 -10,10 h-50 a10,10 0 0 1 -10,-10 z");
	    }

	    text.innerHTML = option;

	    group.appendChild(circle);
	    group.appendChild(path);
	    group.appendChild(text);

	    return group;
	}

}

events.add_init(ModuleEditor.init.bind(ModuleEditor));
