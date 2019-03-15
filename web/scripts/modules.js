
function round(number, decimals){
	return Math.round(number * Math.pow(10, decimals))/Math.pow(10, decimals);
}

var ModuleEditor = {
	init: function(){
		console.log("ModuleEditor Init");
		if(loading_modules != 0){
			setTimeout(ModuleEditor.init.bind(ModuleEditor), 100);
			return;
		}

		console.error("ModuleEditor Init");

		content = "";

		width = 10;

		function lineColour(b, s=0, cnvs){
			(s == 0)?cnvs.strokeStyle = "#aaa":cnvs.strokeStyle = "#ddd";
		}

		for(const key of Object.keys(modules)){
			modules[key]


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

		this.loadModule(20);

	},

	loadModule: function(module){
		$("#moduleconfig #ModuleConfigurator tbody").empty();

		$("#moduleconfig #ModuleConfigurator tbody").append(modules[module].configdata());
	}
}

events.add_init(ModuleEditor.init.bind(ModuleEditor));