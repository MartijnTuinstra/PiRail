
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

		for(const key of Object.keys(modules)){
			modules[key]


			content += '<div class="modulebox" style="width:'+modules[key].width+'px;"> \
											<canvas id="EditorModule'+key+'"></canvas> \
											<div class="modulename">'+key+' - '+modules[key].name+'</div> \
										</div>';

			width += modules[key].width + 5;
		}


		$("#moduleconfig .moduleContainer").append(content);
		$("#moduleconfig .moduleContainer").css("width", width);

	}
}

events.add_init(ModuleEditor.init.bind(ModuleEditor));