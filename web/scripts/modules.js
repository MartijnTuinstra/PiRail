

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
		ModuleEditor.open = module;

		websocket.cts_layout_request_raw(module);

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
	}
}

events.add_init(ModuleEditor.init.bind(ModuleEditor));