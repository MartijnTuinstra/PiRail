
var c;

var Canvas = {

	radia: [68.2843],
	ds: -8.28,
	ro: [],
	c: undefined,
	canvas: undefined,
	dimensions: {scale:1,width:0,height:0,ofX:0,ofY:0,content_width:0,content_height:0,box_width:0,box_height:0,x_left:0,x_right:0,y_top:0,y_bottom:0},
	init_calc: function(){
		this.ro = [{x:this.radia[0]*Math.cos(Math.PI*0.25),y:this.radia[0] - this.radia[0]*Math.sin(Math.PI*0.25),yr:this.radia[0]*Math.sin(Math.PI*0.25)}];
		this.ro[0].l = Math.sqrt(Math.pow(this.ro[0].x-this.ds,2)+Math.pow(this.ro[0].y,2));
	},
	init: function(){
		this.canvas = document.querySelector('canvas');
		this.c = this.canvas.getContext('2d');
		c = this.c;

	    this.canvas.height = $('#track').height();
		this.canvas.width = $('#track').width();

		$('canvas').on('mousedown',this.evt_mousedown);
		$('canvas').on('mouseup',this.evt_mouseup);

		$('canvas').on("mousewheel",this.evt_scrolled)

		$('canvas').on("touchstart",this.evt_drag_start);
		$('canvas').on("touchmove",this.evt_drag_move);
		$('canvas').on("touchend",this.evt_drag_end);
	},
	resize: function(){
		console.log("resize");

		tmp_scale = this.dimensions.scale;
		this.rescale(1);

	    this.canvas.height = $('#track').height();
		this.canvas.width  = $('#track').width();

		this.dimensions.width = this.canvas.width;
		this.dimensions.height = this.canvas.height;
		
		this.dimensions.ofX = 0.5*this.dimensions.width-0.5*this.dimensions.content_width;
		this.dimensions.ofY = 0;

		this.calc_limits();

		this.rescale(tmp_scale);

		this.update_frame();
	},
	rescale: function(new_scale){
		c.scale(1/this.dimensions.scale,1/this.dimensions.scale);

		this.dimensions.scale = new_scale;
		c.scale(this.dimensions.scale,this.dimensions.scale);

		this.dimensions.ofX = 0.5*this.dimensions.width/this.dimensions.scale-0.5*this.dimensions.content_width;
		this.dimensions.ofY = 0;

		this.calc_limits();

		this.moved();

		this.update_frame();
	},

	calc_limits: function(){
		if(this.dimensions.content_width == 0 || this.dimensions.content_height == 0){
			return;
		}
		var margin = 20/this.dimensions.scale;
		this.dimensions.x_left   = -margin;
		this.dimensions.x_right  = this.dimensions.content_width+margin;
		this.dimensions.y_top    = 0;
		this.dimensions.y_bottom = this.dimensions.content_height+margin;
	},

	calc_dotmatrix: function(){
		$.each(modules,function(module,module_v){
			$.each(module_v.data,function(i,v){
				var coords = [];
				var block_list = [];
				var switch_list = [];
				if(v.type == "line"){
					coords.push({x:v.x,y:v.y});
					coords.push({x:v.x2,y:v.y2});
					block_list.push(v.block);
				}
				else if(v.type == "arc"){
					coords.push({x:v.cx+Math.cos(v.start*Math.PI)*v.r,y:v.cy+Math.sin(v.start*Math.PI)*v.r});
					coords.push({x:v.cx+Math.cos(v.end*Math.PI)*v.r,y:v.cy+Math.sin(v.end*Math.PI)*v.r});
					block_list.push(v.block);
				}
				else if(v.type == "swl"){
					coords.push({x:v.x,y:v.y});
					if(v.r == 0.25){
						coords.push({x:v.x+ro[0].x+ds,y:v.y+2*ro[0].y});
						coords.push({x:v.x+ro[0].x,y:v.y+ro[0].y});
					}else if(v.r == -0.25){
						coords.push({x:v.x+ro[0].x+ds,y:v.y-2*ro[0].y});
						coords.push({x:v.x+ro[0].y,y:v.y-ro[0].x});
					}
					else if(v.r == 0){
						coords.push({x:v.x+ro[0].x,y:v.y});
						coords.push({x:v.x+ro[0].x,y:v.y-ro[0].y});
					}
					else if(v.r == 1 || v.r == -1){
						coords.push({x:v.x-ro[0].x,y:v.y});
						coords.push({x:v.x-ro[0].x,y:v.y+ro[0].y});
					}
					block_list.push(v.block);
					switch_list.push(v.switch);
				}
				else if(v.type == "swr"){
					coords.push({x:v.x,y:v.y});
					if(v.r == 0.25){
						coords.push({x:v.x+ro[0].x+ds,y:v.y+2*ro[0].y});
						coords.push({x:v.x+ro[0].y,y:v.y+ro[0].x});
					}else if(v.r == -0.25){
						coords.push({x:v.x+ro[0].x+ds,y:v.y-2*ro[0].y});
						coords.push({x:v.x+ro[0].y,y:v.y-ro[0].x});
					}
					else if(v.r == 0){
						coords.push({x:v.x+ro[0].x,y:v.y});
						coords.push({x:v.x+ro[0].x,y:v.y+ro[0].y});
					}
					else if(v.r == -1 || v.r == 1){
						coords.push({x:v.x-ro[0].x,y:v.y});
						coords.push({x:v.x-ro[0].x,y:v.y-ro[0].y});
					}
					block_list.push(v.block);
					switch_list.push(v.switch);
				}
				else if(v.type == "ds"){
					if(v.xtl != undefined && v.ytl != undefined){
						coords.push({x:v.xtl,y:v.ytl});
						coords.push({x:v.xtl+ds,y:v.ytl+ro[0].y});
						coords.push({x:v.xtl+ro[0].x,y:v.ytl+ro[0].y});
						coords.push({x:v.xtl+ro[0].x+ds,y:v.ytl+2*ro[0].y});
					}else{
						coords.push({x:v.xtr,y:v.ytr});
						coords.push({x:v.xtr-ds,y:v.ytr+ro[0].y});
						coords.push({x:v.xtr-ro[0].x,y:v.ytr+ro[0].y});
						coords.push({x:v.xtr-ro[0].x-ds,y:v.ytr+2*ro[0].y});
					}
					block_list.push(v.block);
					switch_list.push(v.switchA);
					switch_list.push(v.switchB);
				}
				else if(v.type == "dc"){
					block_list.push(v.blockA);
					block_list.push(v.blockB);

					coords.push({x:v.xtl,y:v.ytl});
					coords.push({x:v.xtl+ro[0].x,y:v.ytl});
					coords.push({x:v.xtl+2*ro[0].x,y:v.ytl});
					coords.push({x:v.xtl+ro[0].x,y:v.ytl+ro[0].y});
					coords.push({x:v.xtl,y:v.ytl+2*ro[0].y});
					coords.push({x:v.xtl+ro[0].x,y:v.ytl+2*ro[0].y});
					coords.push({x:v.xtl+2*ro[0].x,y:v.ytl+2*ro[0].y});

					switch_list.push(v.switchA);
					switch_list.push(v.switchB);
					switch_list.push(v.switchC);
					switch_list.push(v.switchD);
				}

				$.each(coords,function(i,v){
					for(var j = 0;j<=module_v.dotmatrix.length;j++){
						if(j == module_v.dotmatrix.length){
							module_v.dotmatrix.push({x:v.x,y:v.y,blocks:block_list});
							break;
						}
						if(Math.pow(v.x - module_v.dotmatrix[j].x,2)+Math.pow(v.y - module_v.dotmatrix[j].y,2) < 4){
							module_v.dotmatrix[j].blocks = module_v.dotmatrix[j].blocks.concat(block_list);
							break;
						}
					}
				});

				if(Math.max(...block_list) > module_v.blocks.length-1){
					for(0;(Math.max(...block_list) - module_v.blocks.length) > -1;0){
						module_v.blocks.push(10);
					}
				}

				if(Math.max(...switch_list) > module_v.switches.length-1){
					for(0;(Math.max(...switch_list) - module_v.switches.length) > -1;0){
						module_v.switches.push(1);
					}
				}
			});
		});
	},
	draw_dotmatrix: function(){
		c.lineWidth = 0.1;
		//Draw dots
		var dot_radius = Math.min(2.8*this.dimensions.scale,2.8);
		var tmp_this = this;
		$.each(modules,function(module,module_v){
			if (module_v.visible == false){
				return true;
			}
			var ofX = module_v.OffsetX + tmp_this.dimensions.ofX;
			var ofY = module_v.OffsetY + tmp_this.dimensions.ofY;
			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);
			$.each(module_v.dotmatrix,function(i,v){
				c.beginPath();
				var block_state = 255;
				$.each(v.blocks,function(i2,v2){
					if(module_v.blocks[v2] < block_state){
						block_state = module_v.blocks[v2];
					}
				});
				c.arc(ofX+rvX*v.x+rvY_*v.y,ofY+rvY*v.y+rvX_*v.x,dot_radius,0,2*Math.PI,false);
				tmp_this.setStrokeColor(block_state,0);
				c.fillStyle = c.strokeStyle;
				c.fill();
				c.stroke();
			});
		});
	},

	draw_background: function(){
		c.lineWidth = 6;
		var tmp_this = this;
		//Draw background lines
		$.each(modules,function(module,module_v){
			if (module_v.visible == false){
				return true;
			}
			var ofX = module_v.OffsetX + tmp_this.dimensions.ofX;
			var ofY = module_v.OffsetY + tmp_this.dimensions.ofY;
			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);
			$.each(module_v.data,function(i,v){
				c.beginPath();
				c.strokeStyle = v.stroke;

				if(v.type == "ds"){
					test = 13;
				}

				if(v.type == "arc"){

				}else{
					nx = rvX*v.x + rvY_*v.y + ofX;
					ny = rvY*v.y + rvX_*v.x + ofY;
				}
				nr = v.r - module_v.r;

				if(v.type == "line"){
					tmp_this.setStrokeColor(module_v.blocks[v.block], 1);
					if(v.if != undefined){
						counter = 0;
						for (var i = v.if.length - 1; i >= 0; i--) {
							if(module_v.switches[v.if[i].sw] == v.if[i].st)
								counter++;
						}
						if(counter == v.if.length){
							return true; // Print on foreground
						}
					}
					c.moveTo(nx, ny);
					c.lineTo(rvX*v.x2+rvY_*v.y2+ofX,rvY*v.y2+rvX_*v.x2+ofY);
				}
				else if(v.type == "arc"){
					tmp_this.setStrokeColor(module_v.blocks[v.block], 1);
					if(v.if != undefined){
						counter = 0;
						for (var i = v.if.length - 1; i >= 0; i--) {
							if(module_v.switches[v.if[i].sw] == v.if[i].st)
								counter++;
						}
						if(counter == v.if.length){
							 return true//Print on foreground
						}
					}
					c.arc(rvX*v.cx+rvY_*v.cy+ofX,rvY*v.cy+rvX_*v.cx+ofY,v.r,Math.PI*(-module_v.r+v.start),Math.PI*(-module_v.r+v.end),v.CW);
				}
				else if(v.type == "swr" || v.type == "swl"){ // Switch Right
					if(v.if != undefined){
						var j = 0;
						for (var i = v.if.length - 1; i >= 0; i--) {
							if(module_v.switches[v.if[i].sw] != v.if[i].st){
								j++;
							}
						}
						if(j != v.if.length){
							Draw.switch(nx, ny, nr, v.type, "B",module_v.blocks[v.block],module_v.switches[v.switch]);
							c.stroke();
							return true
						}
						Draw.switch(nx, ny, nr, v.type, "G",module_v.blocks[v.block],module_v.switches[v.switch]);
					}
					else{
						Draw.switch(nx, ny, nr, v.type, "B",module_v.blocks[v.block],module_v.switches[v.switch]);
					}
				}
				else if(v.type == "ds"){ //Double slip
					Draw.double_slip(nx, ny, nr, "B",module_v.blocks[v.block],module_v.switches[v.switchA], module_v.switches[v.switchB]);
				}
				else if(v.type == "dsf"){ //Double slip flipped
					Draw.double_slip_mirrored(nx, ny, nr, "B",module_v.blocks[v.block],module_v.switches[v.switchA], module_v.switches[v.switchB]);
				}
				else if(v.type == "dc"){ //Double crossover
					if(v.xtl != undefined && v.ytl != undefined){
						tmp_this.setStrokeColor(module_v.blocks[v.blockA],1);
						if(module_v.switches[v.switchA] == 1 || module_v.switches[v.switchB] == 1){
							c.moveTo(v.xtl+ofX,v.ytl+ofY);
							c.lineTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY);
							c.stroke();c.beginPath();
							c.moveTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY);
							c.lineTo(v.xtl+ofX+2*ro[0].x,v.ytl+ofY);
							c.stroke();c.beginPath();
						}
						if(module_v.switches[v.switchA] == 0){
							c.arc(v.xtl+ofX,v.ytl+ofY+radia[0],radia[0],Math.PI*-0.5+0.05,Math.PI*-0.25,false);
							c.stroke();c.beginPath();
						}

						if(module_v.switches[v.switchB] == 0){
							c.arc(v.xtl+ofX+2*ro[0].x,v.ytl+ofY+radia[0],radia[0],Math.PI*-0.5-0.05,Math.PI*-0.75,true);
							c.stroke();c.beginPath();
						}

						tmp_this.setStrokeColor(module_v.blocks[v.blockB],1);
						if(module_v.switches[v.switchC] == 1 || module_v.switches[v.switchD] == 1){
							c.moveTo(v.xtl+ofX,v.ytl+ofY+2*ro[0].y);
							c.lineTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY+2*ro[0].y);
							c.stroke();c.beginPath();
							c.moveTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY+2*ro[0].y);
							c.lineTo(v.xtl+ofX+2*ro[0].x,v.ytl+ofY+2*ro[0].y);
							c.stroke();c.beginPath();
						}
						if(module_v.switches[v.switchC] == 0){
							c.arc(v.xtl+ofX,v.ytl+ofY+2*ro[0].y-radia[0],radia[0],Math.PI*0.5-0.05,Math.PI*0.25,true);
							c.stroke();c.beginPath();
						}

						if(module_v.switches[v.switchD] == 0){
							c.arc(v.xtl+ofX+2*ro[0].x,v.ytl+ofY+2*ro[0].y-radia[0],radia[0],Math.PI*0.5+0.05,Math.PI*0.75,false);
							c.stroke();c.beginPath();
						}

					}
				}
				c.stroke();
			});
		});
	},

	draw_foreground: function(){
		var tmp_this = this;
		//Draw lines
		c.lineWidth = 6;
		$.each(modules,function(module,module_v){
			if (module_v.visible == false){
				return true;
			}
			var ofX = module_v.OffsetX + tmp_this.dimensions.ofX;
			var ofY = module_v.OffsetY + tmp_this.dimensions.ofY;
			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);
			$.each(module_v.data,function(i,v){
				c.beginPath();
				c.strokeStyle = v.stroke;

				if(v.type == "ds"){
					test = 13;
				}

				if(v.type == "arc"){

				}else{
					nx = rvX*v.x + rvY_*v.y + ofX;
					ny = rvY*v.y + rvX_*v.x + ofY;
				}
				nr = v.r - module_v.r

				if(v.type == "line"){
					tmp_this.setStrokeColor(module_v.blocks[v.block]);
					if(v.if != undefined){
						where = "F";
						for (var i = v.if.length - 1; i >= 0; i--) {
							if(module_v.switches[v.if[i].sw] != v.if[i].st)
								return true; // Print on background
						}
					}
					c.moveTo(nx, ny);
					c.lineTo(rvX*v.x2+rvY_*v.y2+ofX,rvY*v.y2+rvX_*v.x2+ofY);
				}
				else if(v.type == "arc"){
					tmp_this.setStrokeColor(module_v.blocks[v.block]);
					if(v.if != undefined){
						where = "F";
						for (var i = v.if.length - 1; i >= 0; i--) {
							if(module_v.switches[v.if[i].sw] != v.if[i].st)
								return true //Print on background
						}
					}
					c.arc(rvX*v.cx+rvY_*v.cy+ofX,rvY*v.cy+rvX_*v.cx+ofY,v.r,Math.PI*(-module_v.r+v.start),Math.PI*(-module_v.r+v.end),v.CW);
				}
				else if(v.type == "swr" || v.type == "swl"){ // Switch Right
					if(v.if != undefined){
						where = "F";
						for (var i = v.if.length - 1; i >= 0; i--) {
							if(module_v.switches[v.if[i].sw] != v.if[i].st)
								return true;
						}
					}
					Draw.switch(nx, ny, nr, v.type, "F",module_v.blocks[v.block],module_v.switches[v.switch]);
				}
				else if(v.type == "ds"){ //Double slip
					Draw.double_slip(nx, ny, nr, "F",module_v.blocks[v.block],module_v.switches[v.switchA], module_v.switches[v.switchB]);
				}
				else if(v.type == "dsf"){ //Double slip flipped
					Draw.double_slip_mirrored(nx, ny, nr, "F",module_v.blocks[v.block],module_v.switches[v.switchA], module_v.switches[v.switchB]);
				}
				else if(v.type == "dc"){
					if(v.xtl != undefined && v.ytl != undefined){
						tmp_this.setStrokeColor(module_v.blocks[v.blockA],0);
						if(module_v.switches[v.switchA] == 0 && module_v.switches[v.switchB] == 0){
							c.moveTo(v.xtl+ofX,v.ytl+ofY);
							c.lineTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY);
							c.stroke();c.beginPath();
							c.moveTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY);
							c.lineTo(v.xtl+ofX+2*ro[0].x,v.ytl+ofY);
							c.stroke();c.beginPath();
						}
						else{
							if(module_v.switches[v.switchA] == 1){
								c.arc(v.xtl+ofX,v.ytl+ofY+radia[0],radia[0],Math.PI*-0.5,Math.PI*-0.25,false);
								c.stroke();c.beginPath();
							}

							if(module_v.switches[v.switchB] == 1){
								c.arc(v.xtl+ofX+2*ro[0].x,v.ytl+ofY+radia[0],radia[0],Math.PI*-0.5,Math.PI*-0.75,true);
								c.stroke();c.beginPath();
							}
						}

						tmp_this.setStrokeColor(module_v.blocks[v.blockB],0);
						if(module_v.switches[v.switchC] == 0 && module_v.switches[v.switchD] == 0){
							c.moveTo(v.xtl+ofX,v.ytl+ofY+2*ro[0].y);
							c.lineTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY+2*ro[0].y);
							c.stroke();c.beginPath();
							c.moveTo(v.xtl+ofX+1*ro[0].x,v.ytl+ofY+2*ro[0].y);
							c.lineTo(v.xtl+ofX+2*ro[0].x,v.ytl+ofY+2*ro[0].y);
							c.stroke();c.beginPath();
						}
						else{
							if(module_v.switches[v.switchC] == 1){
								c.arc(v.xtl+ofX,v.ytl+ofY+2*ro[0].y-radia[0],radia[0],Math.PI*0.5,Math.PI*0.25,true);
								c.stroke();c.beginPath();
							}

							if(module_v.switches[v.switchD] == 1){
								c.arc(v.xtl+ofX+2*ro[0].x,v.ytl+ofY+2*ro[0].y-radia[0],radia[0],Math.PI*0.5,Math.PI*0.75,false);
								c.stroke();c.beginPath();
							}
						}

					}
				}
				c.stroke();
			});
		});
	},

	setStrokeColor: function(b, s = 0){
		if(b == 0){ // Blocked
			(s == 0)?this.c.strokeStyle = "#900":this.c.strokeStyle = "#daa";
	   	}
	   	else if(b == 1){ // DANGER
	    	(s == 0)?this.c.strokeStyle = "#f00":this.c.strokeStyle = "#fcc";
	    }
	   	else if(b == 2){ // RESTRICTED
	    	(s == 0)?this.c.strokeStyle = "#f40":this.c.strokeStyle = "#fcc";
	    }
	    else if(b == 3){ // CAUTION
	    	(s == 0)?this.c.strokeStyle = "#f70":this.c.strokeStyle = "#fe8";
	    }
	    else if(b == 4){ // PROCEED
	    	(s == 0)?this.c.strokeStyle = "#aaa":this.c.strokeStyle = "#ddd";
	    }
	    else if(b == 5){ // RESERVED
	    	(s == 0)?this.c.strokeStyle = "#ccc":this.c.strokeStyle = "#eee";
	    }
	    else if(b == 6){ // RESERVED_SWICTH
	    	(s == 0)?this.c.strokeStyle = "#aaa":this.c.strokeStyle = "#fcc";
	    }
	    else{ //Unknown
	    	(s == 0)?this.c.strokeStyle = "#ddd":this.c.strokeStyle = "#eee";
	    }
	},

    update_frame: function(){
		c.clearRect(0, 0, this.dimensions.width/this.dimensions.scale, this.dimensions.height/this.dimensions.scale);
		var tmp_this = this;

		this.draw_background();
		this.draw_dotmatrix();
		this.draw_foreground();

		

		//Draw Tooltips
		$.each(modules,function(module,module_v){
			if (module_v.visible == false){
				return true;
			}
			var ofX = module_v.OffsetX + tmp_this.dimensions.ofX;
			var ofY = module_v.OffsetY + tmp_this.dimensions.ofY;
			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);
			c.beginPath();
			c.moveTo(ofX,ofY);
			c.lineTo(rvX*module_v.width+ofX,rvX_*module_v.width+ofY);
			c.lineTo(rvX*module_v.width+rvY_*module_v.height+ofX,rvY*module_v.height+rvX_*module_v.width+ofY);
			c.lineTo(rvY_*module_v.height+ofX,rvY*module_v.height+ofY);
			c.lineTo(ofX,ofY);
			c.lineWidth = 0.5;
			c.strokeStyle = "#000";
			c.stroke();
			// $.each(module_v.data,function(i,v){
			// 	if(v.tooltip){
			// 		if(v.type == "dc"){
			// 			c.lineWidth = 1;
			// 			c.beginPath();
			// 			c.rect(v.xtl+ofX,v.ytl+ofY+3*ro[0].y,2*ro[0].x,(1.6+0.5+0.5+2.5)*ro[0].y);
			// 			c.strokeStyle = "#000";
			// 			c.fillStyle = "#fff";
			// 			c.fill();
			// 			c.stroke(); 

			// 			var mid = v.xtl+ofX+ro[0].x;

			// 			c.strokeStyle = "#ddd";
			// 			// c.lineWidth = 0.5;
			// 			c.beginPath();
			// 			c.moveTo(mid-ro[0].x,v.ytl+ofY+(3+1.75)*ro[0].y);
			// 			c.lineTo(mid+ro[0].x,v.ytl+ofY+(3+1.75)*ro[0].y);
			// 			c.stroke();c.beginPath();
			// 			c.moveTo(mid-ro[0].x,v.ytl+ofY+(3+3.25)*ro[0].y);
			// 			c.lineTo(mid+ro[0].x,v.ytl+ofY+(3+3.25)*ro[0].y);
			// 			c.stroke();
			// 			var y_top = v.ytl+ofY+3.5*ro[0].y;
			// 			//Write all options
			// 			//A
			// 			c.strokeStyle = "#ddd";
			// 			c.lineWidth = 0.5*6;
			// 			c.beginPath();
			// 			c.arc(mid-0.5*ro[0].x,y_top+0.5*radia[0],0.5*radia[0],Math.PI*-0.5,Math.PI*-0.25,false);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid+0.5*ro[0].x,y_top+0.5*radia[0],0.5*radia[0],Math.PI*-0.5,Math.PI*-0.75,true);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid-0.5*ro[0].x,y_top+ro[0].y-0.5*radia[0],0.5*radia[0],Math.PI*0.5,Math.PI*0.25,true);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid+0.5*ro[0].x,y_top+ro[0].y-0.5*radia[0],0.5*radia[0],Math.PI*0.5,Math.PI*0.75,false);
			// 			c.stroke();c.beginPath();
			// 			c.strokeStyle = "#aaa";
			// 			c.moveTo(mid-0.5*ro[0].x,y_top);
			// 			c.lineTo(mid+0.5*ro[0].x,y_top);
			// 			c.stroke();
			// 			c.beginPath();
			// 			c.moveTo(mid-0.5*ro[0].x,y_top+ro[0].y);
			// 			c.lineTo(mid+0.5*ro[0].x,y_top+ro[0].y);
			// 			c.stroke();
			// 			//B
			// 			y_top += 1.5*ro[0].y;

			// 			c.strokeStyle = "#ddd";
			// 			c.lineWidth = 0.5*6;
			// 			c.beginPath();
			// 			c.arc(mid+0.5*ro[0].x,y_top+0.5*radia[0],0.5*radia[0],Math.PI*-0.5,Math.PI*-0.75,true);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid-0.5*ro[0].x,y_top+ro[0].y-0.5*radia[0],0.5*radia[0],Math.PI*0.5,Math.PI*0.25,true);
			// 			c.stroke();c.beginPath();
			// 			c.moveTo(mid-0.5*ro[0].x,y_top);
			// 			c.lineTo(mid+0.5*ro[0].x,y_top);
			// 			c.stroke();
			// 			c.beginPath();
			// 			c.moveTo(mid-0.5*ro[0].x,y_top+ro[0].y);
			// 			c.lineTo(mid+0.5*ro[0].x,y_top+ro[0].y);
			// 			c.stroke();c.beginPath();
			// 			c.strokeStyle = "#aaa";
			// 			c.arc(mid+0.5*ro[0].x,y_top+ro[0].y-0.5*radia[0],0.5*radia[0],Math.PI*0.5,Math.PI*0.75,false);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid-0.5*ro[0].x,y_top+0.5*radia[0],0.5*radia[0],Math.PI*-0.5,Math.PI*-0.25,false);
			// 			c.stroke();c.beginPath();
			// 			//C
			// 			y_top += 1.5*ro[0].y;

			// 			c.strokeStyle = "#ddd";
			// 			c.lineWidth = 0.5*6;
			// 			c.beginPath();
			// 			c.arc(mid+0.5*ro[0].x,y_top+ro[0].y-0.5*radia[0],0.5*radia[0],Math.PI*0.5,Math.PI*0.75,false);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid-0.5*ro[0].x,y_top+0.5*radia[0],0.5*radia[0],Math.PI*-0.5,Math.PI*-0.25,false);
			// 			c.stroke();c.beginPath();
			// 			c.moveTo(mid-0.5*ro[0].x,y_top);
			// 			c.lineTo(mid+0.5*ro[0].x,y_top);
			// 			c.stroke();
			// 			c.beginPath();
			// 			c.moveTo(mid-0.5*ro[0].x,y_top+ro[0].y);
			// 			c.lineTo(mid+0.5*ro[0].x,y_top+ro[0].y);
			// 			c.stroke();c.beginPath();
			// 			c.strokeStyle = "#aaa";
			// 			c.arc(mid+0.5*ro[0].x,y_top+0.5*radia[0],0.5*radia[0],Math.PI*-0.5,Math.PI*-0.75,true);
			// 			c.stroke();c.beginPath();
			// 			c.arc(mid-0.5*ro[0].x,y_top+ro[0].y-0.5*radia[0],0.5*radia[0],Math.PI*0.5,Math.PI*0.25,true);
			// 			c.stroke();c.beginPath();
			// 		}
			// 	}
			// });
		});
    },

    // Events //
    // Control functions

	click: function(x,y){
		var update = false;
		var tmp_this = this;
		$.each(modules,function(module,module_v){
			var ofX = module_v.OffsetX;
			var ofY = module_v.OffsetY;
			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);
			_x = x/tmp_this.dimensions.scale - ofX - tmp_this.dimensions.ofX;
			_y = y/tmp_this.dimensions.scale - ofY - tmp_this.dimensions.ofY;
			_l = Math.sqrt(Math.pow(_x,2)+Math.pow(_y,2));
			_r = Math.atan(_y/_x);
			(_x<0&&_y<0)?_r=-(Math.PI-_r):1;
			_r -= module_v.r*Math.PI;
			_x = _l*Math.cos(_r);
			_y = _l*Math.sin(_r);
			console.log(_x,_y);
			$.each(module_v.hitboxes,function(i,box){
				if(eval(box.eval_code) == 1){
					console.log("Clicked something");
					if(box.action == "tSw"){ // Throw switch
						throw_Switch(module,box.switch);
					}
					else if(box.action == "tDS"){
						throw_doubleSlib(module,box.switch);
					}
					else if(box.action == "tDC"){
						throw_doubleCrossover(module,box.switch);
					}
					else{
						alert("Hit " + box.name + "\n with no action");
					}
					update = true
				}
			});
		});
		if(update)
			this.update_frame();
	},

	moved: function(){
		console.log("Moved");
		if(this.dimensions.width < this.dimensions.content_width){
			//Scroll limiter
			if((this.dimensions.x_left) > -this.dimensions.ofX){
				this.dimensions.ofX = -this.dimensions.x_left;
			}
			if((this.dimensions.x_right) < -this.dimensions.ofX+this.canvas.width / this.dimensions.scale){
				this.dimensions.ofX = -(this.dimensions.x_right - this.canvas.width / this.dimensions.scale);
			}
		}
		if(this.dimensions.height < this.dimensions.content_height){
			//Scroll limiter
			if((this.dimensions.y_top) > -this.dimensions.ofY){
				this.dimensions.ofY = this.dimensions.y_top;
			}
			if((this.dimensions.y_bottom) < -this.dimensions.ofY+this.canvas.height / this.dimensions.scale){
				this.dimensions.ofY = -(this.dimensions.y_bottom - this.canvas.height / this.dimensions.scale);
			}
		}

		this.update_frame();
	},

	longClick: function(evt){
		alert("Long Click");
		Longclick = true;
	},

	// Events //

	LongclickTimer: undefined,
	Longclick: false,

	evt_mousedown: function(evt){
		Canvas.Longclick = false;
		Canvas.LongclickTimer = setTimeout(function(){Canvas.longClick(evt)},400);
		Train.hide(evt);
	},
	evt_mouseup: function(evt){
		clearTimeout(Canvas.LongclickTimer);
		if(!Canvas.touchClicked && !Canvas.Longclick)
			Canvas.click(evt.offsetX,evt.offsetY);
		else
			Canvas.touchClicked = false;
	},


	touchStart: {x:0,y:0},
	touchMove: {x:0,y:0},
	touchMoved: false,
	touchClicked: false,
	evt_drag_start: function(evt){
		Canvas.touchStart.x = (Canvas.touchMove.x = evt.touches[0].pageX);
		Canvas.touchStart.y = (Canvas.touchMove.y = evt.touches[0].pageY);
		Canvas.touchMoved = false;

		Canvas.LongclickTimer = setTimeout(function(){longClick(evt)},600);

		Train.hide(evt);
	},
	evt_drag_move: function(evt){
		if(Canvas.touchMoved == true){
			if(Canvas.dimensions.width < Canvas.dimensions.content_width){
				Canvas.dimensions.ofX += (evt.touches[0].pageX - Canvas.touchMove.x)/Canvas.dimensions.scale;
			}
			if(Canvas.dimensions.height < Canvas.dimensions.content_height){
				Canvas.dimensions.ofY += (evt.touches[0].pageY - Canvas.touchMove.y)/Canvas.dimensions.scale;
			}

			Canvas.moved()

			Canvas.touchMove.x = evt.touches[0].pageX;
			Canvas.touchMove.y = evt.touches[0].pageY;

			return false;
		}else{
			if(Math.sqrt(Math.pow(Canvas.touchStart.x - evt.touches[0].pageX,2)+Math.pow(Canvas.touchStart.y - evt.touches[0].pageY,2)) >= 10){
				Canvas.touchMoved = true;
				clearTimeout(Canvas.LongclickTimer);
				console.log("Touch Moved");
			}
		}
	},
	evt_drag_end: function(evt){
		if(Canvas.touchMoved == false){
			Canvas.touchClicked = true;
			Canvas.click(Canvas.touchStart.x,Canvas.touchStart.y);
			clearTimeout(Canvas.LongclickTimer);
		}

		evt.preventDefault();
		return false;
	},

	evt_scrolled: function(evt){
		console.log("Scroll");

		if(Canvas.dimensions.width < Canvas.dimensions.content_width){
			Canvas.dimensions.ofX -= evt.originalEvent.deltaX/10;
		}
		if(Canvas.dimensions.height < Canvas.dimensions.content_height){
			Canvas.dimensions.ofY -= evt.originalEvent.deltaY/10;
		}

		Canvas.moved();
	}
}

Canvas.init_calc();
ro = Canvas.ro;
ds = Canvas.ds;
radia = Canvas.radia;


var modules = {
	20:{Name: "Straight",visible: true, OffsetX:250,OffsetY:20,width:800,height:160,r:0,blocks:[5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5],switches:[],dotmatrix:[],
		data:[
			{type:"line",block:0, x:0,y:20,          x2:5,y2:20},
			{type:"swr" ,block:0, x:5,y:20,          x2:5,y2:20,r:0,switch:0},
			{type:"line",block:6, x:0,y:20+2*ro[0].y,x2:5+ro[0].x+ds,y2:20+2*ro[0].y},

			{type:"ds"  ,block:6, x:5+ro[0].x,y:20+ro[0].y,r:0,switchA:1,switchB:2},

			{type:"swl" ,block:6, x:5+ds+2*ro[0].x,y:20+3*ro[0].y,switch:3,r:0.25,if:[{sw:2,st:1}]},
			{type:"arc" ,block:6,cx:5+2*ds+4*ro[0].x,cy:20+6*ro[0].y-radia[0],r:radia[0],start:0.75,end:0.5,CW:true,if:[{sw:2,st:1},{sw:3,st:0}]},

			{type:"line",block:1,   x:5+ro[0].x,y:20,x2:250,y2:20},
			{type:"line",block:7,   x:5+2*ro[0].x,y:20+2*ro[0].y,x2:250,y2:20+2*ro[0].y},
			
			{type:"line",block:2,   x:250,y:20,                       x2:400,y2:20},
			{type:"line",block:8,   x:250,y:20+2*ro[0].y,             x2:400,y2:20+2*ro[0].y},
			{type:"line",block:12,  x:5+3*ro[0].x+ds  ,y:20+4*ro[0].y,x2:400,y2:20+4*ro[0].y},
			{type:"line",block:15,  x:5+4*ro[0].x+2*ds,y:20+6*ro[0].y,x2:400,y2:20+6*ro[0].y},

			{type:"line",block:3,   x:400,y:20,          x2:550,y2:20},
			{type:"line",block:9,   x:400,y:20+2*ro[0].y,x2:550,y2:20+2*ro[0].y},
			{type:"line",block:13,  x:400,y:20+4*ro[0].y,x2:550,y2:20+4*ro[0].y},
			{type:"line",block:16,  x:400,y:20+6*ro[0].y,x2:550,y2:20+6*ro[0].y},

			{type:"line",block:4,   x:550,y:20,x2:795-ro[0].x,y2:20},
			{type:"line",block:10,  x:550,y:20+2*ro[0].y,x2:795-2*ro[0].x,y2:20+2*ro[0].y},
			{type:"line",block:14,  x:550,y:20+4*ro[0].y,x2:795-3*ro[0].x-ds,y2:20+4*ro[0].y},
			{type:"arc" ,block:14,  cx:795-3*ro[0].x-ds,cy:20+4*ro[0].y-radia[0],r:radia[0],start:0.5,end:0.25,CW:true},
			{type:"line",block:17,  x:550,y:20+6*ro[0].y,x2:800-3*ro[0].x-ds,y2:20+6*ro[0].y},
			{type:"line",block:18,  x:800-3*ro[0].x-ds,y:20+6*ro[0].y,x2:700,y2:20+6*ro[0].y},


			{type:"swl" ,block:5,  x:795,y:20,r:1,switch:6},
			{type:"line",block:5,  x:800,y:20,x2:795,y2:20},
			{type:"line",block:11, x:800,y:20+2*ro[0].y,x2:795-ro[0].x-ds,y2:20+2*ro[0].y},

			{type:"dsf"  ,block:11, x:795-ro[0].x,y:20+ro[0].y,r:0,switchA:5,switchB:4},
		],
		hitboxes:[
			{eval_code:"(_x > 2.5 && _x < 52.5 && _y > 15 && _y < 40)?1:0;",name:"Switch 1",action:"tSw",switch:0},
			{eval_code:"(_x > 42.5 && _x < 97.5 && _y > 40 && _y < 80)?1:0;",name:"Switch 2&3",action:"tDS",switch:[1,2]},
			{eval_code:"(_x > 90 && _x < 150 && _y > 80 && _y < 120)?1:0;",name:"Switch 4",action:"tSw",switch:3},
			{eval_code:"(_x > 702.5 && _x < 757.5 && _y > 40 && _y < 80)?1:0;",name:"Switch 5&6",action:"tDS",switch:[4,5]},
			{eval_code:"(_x > 748.5 && _x < 797.5 && _y > 15 && _y < 40)?1:0;",name:"Switch 6",action:"tSw",switch:6},
		]
	},
	21:{Name: "Straight",visible: true, OffsetX:1050,OffsetY:460,width:800,height:160,r:1,blocks:[5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5],switches:[],dotmatrix:[],
		data:[
		{type:"line",block:0,   x:0,y:20,          x2:200,y2:20},
		{type:"line",block:1,   x:200,y:20,        x2:400,y2:20},
		{type:"line",block:2,   x:400,y:20,        x2:600,y2:20},
		{type:"line",block:3,   x:600,y:20,        x2:800,y2:20},

		{type:"line",block:4,   x:0,y:20+2*ro[0].y,x2:5,y2:20+2*ro[0].y},
		{type:"swr" ,block:4,   x:5,y:20+2*ro[0].y,r:0,switch:0},

		{type:"line",block:5,   x:5+ro[0].x,y:20+2*ro[0].y,x2:200,y2:20+2*ro[0].y},
		{type:"line",block:6,   x:200,y:20+2*ro[0].y,x2:400,y2:20+2*ro[0].y},
		{type:"line",block:7,   x:400,y:20+2*ro[0].y,x2:600,y2:20+2*ro[0].y},
		{type:"line",block:8,   x:795-ro[0].x,y:20+2*ro[0].y,x2:600,y2:20+2*ro[0].y},

		{type:"swl" ,block:9,   x:795,y:20+2*ro[0].y,r:0,r:-1,switch:1},
		{type:"line",block:9,   x:800,y:20+2*ro[0].y,x2:795,y2:20+2*ro[0].y},
		
		{type:"arc" ,block:10, cx:5+2*ro[0].x,cy:20+4*ro[0].y-radia[0],r:radia[0],start:0.75,end:0.5,CW:true},
		{type:"line",block:10,  x:5+2*ro[0].x,y:20+4*ro[0].y,x2:200,y2:20+4*ro[0].y},
		{type:"line",block:11,  x:200,y:20+4*ro[0].y,x2:400,y2:20+4*ro[0].y},
		{type:"line",block:12,  x:400,y:20+4*ro[0].y,x2:600,y2:20+4*ro[0].y},
		{type:"line",block:13,  x:795-2*ro[0].x,y:20+4*ro[0].y,x2:600,y2:20+4*ro[0].y},
		{type:"arc" ,block:13, cx:795-2*ro[0].x,cy:20+4*ro[0].y-radia[0],r:radia[0],start:0.5,end:0.25,CW:true},
		],hitboxes:[
			{eval_code:"(_x > 2.5 && _x < 52.5 && _y > 45 && _y < 70)?1:0;",name:"Switch 1",action:"tSw",switch:0},
			{eval_code:"(_x > 748.5 && _x < 797.5 && _y > 45 && _y < 70)?1:0;",name:"Switch 2",action:"tSw",switch:1},
		]
	},
	22:{Name: "Bocht",visible: true, OffsetX:1050,OffsetY:20,width:250,height:440,r:0,blocks:[5,5,5,5],switches:[],dotmatrix:[],
		data:[
		{type:"arc",block:0,cx:0,cy:220,r:200,start:-0.5,end:0,CW:false},
		{type:"arc",block:2,cx:0,cy:220,r:200-2*ro[0].y,start:-0.5,end:0,CW:false},
		{type:"arc",block:1,cx:0,cy:220,r:200,start:0,end:0.5,CW:false},
		{type:"arc",block:3,cx:0,cy:220,r:200-2*ro[0].y,start:0,end:0.5,CW:false}
		]
	},
	23:{Name: "Bocht",visible: true, OffsetX:250,OffsetY:460,width:250,height:440,r:1,blocks:[5,5,5,5],switches:[],dotmatrix:[],
		data:[
		{type:"arc",block:0,cx:0,cy:220,r:200,start:-0.5,end:0,CW:false},
		{type:"arc",block:2,cx:0,cy:220,r:200-2*ro[0].y,start:-0.5,end:0,CW:false},
		{type:"arc",block:1,cx:0,cy:220,r:200,start:0,end:0.5,CW:false},
		{type:"arc",block:3,cx:0,cy:220,r:200-2*ro[0].y,start:0,end:0.5,CW:false}
		]
	},
}

var Draw = {
	switch: function(x,y,r,lr,type,bl,sw){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = Math.sin((r)*Math.PI);
		if((sw == 0 && type == "F") || (sw == 1 && type == "B") || type == "G"){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);
			if(r == 0.25 || r == 0.75 || r == 1.25 || r == 1.75 || r == -0.25 || r == -0.75 || r == -1.25 || r == -1.75){
				c.moveTo(x,y);
				if(r == -0.25 || r == 1.75){
					c.lineTo(x+ro[0].x+ds,y-2*ro[0].y);
				}else if(r == -0.75 || r == 1.25){
					c.lineTo(x-ro[0].x-ds,y-2*ro[0].y);
				}else if(r == -1.25 || r == 0.75){
					c.lineTo(x-ro[0].x-ds,y+2*ro[0].y);
				}else if(r == -1.75 || r == 0.25){
					c.lineTo(x+ro[0].x+ds,y+2*ro[0].y);
				}
			}else if(r == 0 || r == 0.5 || r == 1 || r == 1.5 || r == 2 || r == -0.5 || r == -1 || r == -1.5 || r == -2){
				c.moveTo(x,y);
				c.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
			}
		}
		if((sw == 1 && type == "F") || (sw == 0 && type == "B") || type == "G"){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);
			if(lr == "swr"){
				if(r == 0.25 || r == -1.75){
					c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}else if(r == -0.25 || r == 1.75){
					c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}else if(r == 0.75 || r == -1.25){
					c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}else if(r == -0.75 || r == 1.25){
					c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}else if(r == 0){
					c.arc(x,y+radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}else if(r == -1 || r == 1){
					c.arc(x,y-radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}
				else if(r == -0.5){
					c.arc(x+radia[0],y,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}
				else if(r == 0.5){
					c.arc(x-radia[0],y,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
				}
			}
			else{
				if(r == 0.25 || r == -1.75){
					c.arc(x+rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);//
				}else if(r == -0.25 || r == 1.75){
					c.arc(x-rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);//
				}else if(r == 0.75 || r == -1.25){
					c.arc(x-rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);//
				}else if(r == -0.75 || r == 1.25){
					c.arc(x+rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
				}else if(r == 0){
					c.arc(x,y-radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
				}else if(r == -1 || r == 1){
					c.arc(x,y+radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
				}
				else if(r == -0.5){
					c.arc(x+radia[0],y,radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
				}
				else if(r == 0.5){
					c.arc(x+radia[0],y,radia[0], Math.PI*(r+0.5), Math.PI*(r+0.25), true)
				}
			}
		}
	},

	double_slip: function(x,y,r,type,bl,swA,swB){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = -Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = -Math.sin((r)*Math.PI);

		if((swA == 1 && swB == 0 && type == "F") || ((swA == 0 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			if(r == 0.25 || r == -1.75){
				c.arc(x+radia[0],y,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);//
			}else if(r == -0.25 || r == 1.75){
				c.arc(x,y-radia[0],radia[0],Math.PI*(r+0.750),Math.PI*(r+0.5),true);
			}else if(r == 0.75 || r == -1.25){
				c.arc(x,y+radia[0],radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);//
			}else if(r == -0.75 || r == 1.25){
				c.arc(x-radia[0],y,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);
			}
			else if(r == 0){
				c.arc(x+ro[0].x,y-radia[0]+ro[0].y,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.50),true);
			}else if(r == -1 || r == 1){
				c.arc(x-ro[0].x,y-ro[0].y+radia[0],radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);
			}else if(r == -0.5){
				c.arc(x-radia[0]+ro[0].y,y-ro[0].x,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);
			}else if(r == 0.5){
				c.arc(x-ro[0].y+radia[0],y+ro[0].x,radia[0], Math.PI*(r+0.75), Math.PI*(r+0.5), true)
			}

			c.stroke();c.beginPath();
		}
		if((swA == 1 && swB == 1 && type == "F") || ((swA == 0 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			dis_X = ro[0].x+ds;
			dis_Y = 2 * ro[0].y;

			if(r == 0.25 || r == 0.75 || r == 1.25 || r == 1.75 || r == -0.25 || r == -0.75 || r == -1.25 || r == -1.75){
				c.moveTo(x,y);
				if(r == -0.25 || r == 1.75){
					c.lineTo(x+ro[0].x-ds,y);
				}else if(r == -0.75 || r == 1.25){
					c.lineTo(x,y-ro[0].x+ds);
				}else if(r == -1.25 || r == 0.75){
					c.lineTo(x-ro[0].x+ds,y);
				}else if(r == -1.75 || r == 0.25){
					c.lineTo(x,y+ro[0].x-ds);
				}
			}else if(r == 0 || r == 0.5 || r == 1 || r == 1.5 || r == 2 || r == -0.5 || r == -1 || r == -1.5 || r == -2){
				c.moveTo(x,y);
				c.lineTo(x+rvX*dis_X+rvY_*dis_Y, y+rvX_*dis_X+rvY*dis_Y);
			}
			c.stroke();c.beginPath();
		}

		if((swA == 0 && swB == 0 && type == "F") || ((swA == 1 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);
			tx = x; ty = y;

			x += rvX*ds + rvY_*ro[0].y;
			y += rvX_*ds + rvY*ro[0].y;
			dis_X = ro[0].x - ds;
			dis_Y = 0;

			if(r == 0.25 || r == 0.75 || r == 1.25 || r == 1.75 || r == -0.25 || r == -0.75 || r == -1.25 || r == -1.75){
				dis_X += -ds;
			}

			c.moveTo(x,y);
			c.lineTo(x+rvX*dis_X+rvY_*dis_Y, y+rvX_*dis_X+rvY*dis_Y);
			c.stroke();c.beginPath();

			x = tx; y = ty;
		}
		if((swA == 0 && swB == 1 && type == "F") || ((swA == 1 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			x += rvX*ds + rvY_*ro[0].y;
			y += rvX_*ds + rvY*ro[0].y;

			if(r == 0.25 || r == -1.75){
				c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == -0.25 || r == 1.75){
				c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == 0.75 || r == -1.25){
				c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == -0.75 || r == 1.25){
				c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}
			else if(r == 0){
				c.arc(x,y+radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == -1 || r == 1){
				c.arc(x,y-radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == -0.5){
				c.arc(x+radia[0],y,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == 0.5){
				c.arc(x-radia[0],y,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}

			c.stroke();c.beginPath();
		}
	},

	double_slip_mirrored: function(x,y,r,type,bl,swA,swB){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = -Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = -Math.sin((r)*Math.PI);

		if((swA == 1 && swB == 0 && type == "F") || ((swA == 0 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			if(r == 0.25 || r == -1.75){
				// c.arc(x+radia[0],y,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);//
			}else if(r == -0.25 || r == 1.75){
				// c.arc(x,y-radia[0],radia[0],Math.PI*(r+0.750),Math.PI*(r+0.5),true);
			}else if(r == 0.75 || r == -1.25){
				// c.arc(x,y+radia[0],radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);//
			}else if(r == -0.75 || r == 1.25){
				// c.arc(x-radia[0],y,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);
			}
			else if(r == 0){
				c.arc(x-ro[0].x,y-radia[0]+ro[0].y,radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
			}else if(r == -1 || r == 1){
				c.arc(x+ro[0].x,y-ro[0].y+radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
			}else if(r == -0.5){
				// c.arc(x-radia[0]+ro[0].y,y-ro[0].x,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.5),true);
			}else if(r == 0.5){
				// c.arc(x-ro[0].y+radia[0],y+ro[0].x,radia[0], Math.PI*(r+0.75), Math.PI*(r+0.5), true)
			}

			c.stroke();c.beginPath();
		}
		if((swA == 1 && swB == 1 && type == "F") || ((swA == 0 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			dis_X = -(ro[0].x+ds);
			dis_Y = 2 * ro[0].y;

			if(r == 0.25 || r == 0.75 || r == 1.25 || r == 1.75 || r == -0.25 || r == -0.75 || r == -1.25 || r == -1.75){
				c.moveTo(x,y);
				if(r == -0.25 || r == 1.75){
					c.lineTo(x-ro[0].x-ds,y);
				}else if(r == -0.75 || r == 1.25){
					c.lineTo(x,y-ro[0].x+ds);
				}else if(r == -1.25 || r == 0.75){
					c.lineTo(x+ro[0].x+ds,y);
				}else if(r == -1.75 || r == 0.25){
					c.lineTo(x,y+ro[0].x-ds);
				}
			}else if(r == 0 || r == 0.5 || r == 1 || r == 1.5 || r == 2 || r == -0.5 || r == -1 || r == -1.5 || r == -2){
				c.moveTo(x,y);
				c.lineTo(x+rvX*dis_X+rvY_*dis_Y, y+rvX_*dis_X+rvY*dis_Y);
			}
			c.stroke();c.beginPath();
		}

		if((swA == 0 && swB == 0 && type == "F") || ((swA == 1 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);
			tx = x; ty = y;

			x += -rvX*ds + rvY_*ro[0].y;
			y += -rvX_*ds + rvY*ro[0].y;
			dis_X = -(ro[0].x - ds);
			dis_Y = 0;

			if(r == 0.25 || r == 0.75 || r == 1.25 || r == 1.75 || r == -0.25 || r == -0.75 || r == -1.25 || r == -1.75){
				dis_X += -ds;
			}

			c.moveTo(x,y);
			c.lineTo(x+rvX*dis_X+rvY_*dis_Y, y+rvX_*dis_X+rvY*dis_Y);
			c.stroke();c.beginPath();

			x = tx; y = ty;
		}
		if((swA == 0 && swB == 1 && type == "F") || ((swA == 1 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			x += -rvX*ds + rvY_*ro[0].y;
			y += -rvX_*ds + rvY*ro[0].y;

			if(r == 0.25 || r == -1.75){
				// c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == -0.25 || r == 1.75){
				// c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == 0.75 || r == -1.25){
				// c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == -0.75 || r == 1.25){
				// c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}
			else if(r == 0){
				c.arc(x,y+radia[0],radia[0],Math.PI*(r-0.5),Math.PI*(r-0.75),true);
			}else if(r == -1 || r == 1){
				c.arc(x,y-radia[0],radia[0],Math.PI*(r-0.5),Math.PI*(r-0.75),true);
			}else if(r == -0.5){
				// c.arc(x+radia[0],y,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}else if(r == 0.5){
				// c.arc(x-radia[0],y,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
			}

			c.stroke();c.beginPath();
		}
			// if(swB == 0){
			// 	c.moveTo(v.xtl+ofX+ds,v.ytl+ofY+ro[0].y);
			// 	c.lineTo(v.xtl+ofX+ro[0].x,v.ytl+ofY+ro[0].y);
			// 	c.stroke();c.beginPath();
			// }
			// else{
			// 	c.arc(v.xtl+ofX-8.28,v.ytl+ofY+radia[0]+ro[0].y,radia[0],Math.PI*-0.50,Math.PI*-0.25,false);
			// 	c.stroke();c.beginPath();
			// }
		






		// if((sw == 0 && type == "P") || (sw == 1 && type == "B")){
		// 	(type == "P")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);
		// 	if(r == 0.25 || r == 0.75 || r == 1.25 || r == 1.75 || r == -0.25 || r == -0.75 || r == -1.25 || r == -1.75){
		// 		c.moveTo(x,y);
		// 		if(r == -0.25 || r == 1.75){
		// 			c.lineTo(x+ro[0].x+ds,y-2*ro[0].y);
		// 		}else if(r == -0.75 || r == 1.25){
		// 			c.lineTo(x-ro[0].x-ds,y-2*ro[0].y);
		// 		}else if(r == -1.25 || r == 0.75){
		// 			c.lineTo(x-ro[0].x-ds,y+2*ro[0].y);
		// 		}else if(r == -1.75 || r == 0.25){
		// 			c.lineTo(x+ro[0].x+ds,y+2*ro[0].y);
		// 		}
		// 	}else if(r == 0 || r == 0.5 || r == 1 || r == 1.5 || r == 2 || r == -0.5 || r == -1 || r == -1.5 || r == -2){
		// 		c.moveTo(x,y);
		// 		c.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
		// 	}
		// }
		// else{
		// 	(type == "P")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);
		// 	if(lr == "swr"){
		// 		if(r == 0.25 || r == -1.75){
		// 			c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		// 		}else if(r == -0.25 || r == 1.75){
		// 			c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		// 		}else if(r == 0.75 || r == -1.25){
		// 			c.arc(x+rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		// 		}else if(r == -0.75 || r == 1.25){
		// 			c.arc(x-rvX*radia[0],y+rvY*radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		// 		}else if(r == 0){
		// 			c.arc(x,y+radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		// 		}else{
		// 			c.arc(x,y-radia[0],radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		// 		}
		// 	}
		// 	else{
		// 		if(r == 0.25 || r == -1.75){
		// 			c.arc(x+rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);//
		// 		}else if(r == -0.25 || r == 1.75){
		// 			c.arc(x-rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);//
		// 		}else if(r == 0.75 || r == -1.25){
		// 			c.arc(x-rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);//
		// 		}else if(r == -0.75 || r == 1.25){
		// 			c.arc(x+rvX*radia[0],y-rvY*radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
		// 		}else if(r == 0){
		// 			c.arc(x,y-radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
		// 		}else if(r == -1 || r == 1){
		// 			c.arc(x,y+radia[0],radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
		// 		}
		// 	}
		// }
	}
}