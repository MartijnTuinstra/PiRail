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

	update_config: function(module){
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
	}
}

events.add_init(ModuleEditor.init.bind(ModuleEditor));