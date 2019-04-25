
var windows = {
	tabs: ["main", "rollingstock", "moduleconfig"],

	switch: function(tab){
		var i = this.tabs.indexOf(tab);
		if(i == -1){
			return;
		}

		for(var j = 0; j < this.tabs.length; j++){
			$("#"+this.tabs[j]+".window").hide();
		}

		$("#"+this.tabs[i]+".window").show();

		if(tab == "moduleconfig"){
			ModuleEditor.open();
		}
	}
}
