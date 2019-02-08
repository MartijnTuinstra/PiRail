
var windows = {
	tabs: ["main", "rollingstock", "moduleeditor"],

	switch: function(tab){
		var i = this.tabs.indexOf(tab);
		if(i == -1){
			return;
		}

		for(var j = 0; j < this.tabs.length; j++){
			console.log("Hiding "+this.tabs[j]);
			$("#"+this.tabs[j]+".window").hide();
		}

		console.log("Showing "+this.tabs[i]);
		$("#"+this.tabs[i]+".window").show();
	}
}