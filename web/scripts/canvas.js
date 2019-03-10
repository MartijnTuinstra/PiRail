
var c;
var hit_radius = 8;

var Canvas = {

	radia: [68.2843],
	ds: -8.28,
	ro: [],
	c: undefined,
	canvas: undefined,
	dimensions: {scale:1,width:0,height:0,ofX:0,ofY:0,box_width:0,box_height:0,x_left:0,x_right:0,y_top:0,y_bottom:0, content:{width:0, height: 0}},
	init_calc: function(){
		this.ro = [{x:this.radia[0]*Math.cos(Math.PI*0.25),y:this.radia[0] - this.radia[0]*Math.sin(Math.PI*0.25),yr:this.radia[0]*Math.sin(Math.PI*0.25)}];
		this.ro[0].l = Math.sqrt(Math.pow(this.ro[0].x-this.ds,2)+Math.pow(this.ro[0].y,2));
	},
	init: function(){
		this.canvas_jquery = $('#LayoutContainer canvas');
		this.canvas = this.canvas_jquery[0];
		this.c = this.canvas.getContext('2d');
		c = this.c;

	    this.dimensions.height = (this.canvas.height = $('#LayoutContainer').height());
		this.dimensions.width  = (this.canvas.width  = $('#LayoutContainer').width());

		this.canvas_jquery.on('mousedown',this.evt_mousedown);
		this.canvas_jquery.on('mouseup',this.evt_mouseup);

		// this.canvas_jquery.on("scroll", this.evt_scrolled.bind(this));
		this.canvas_jquery.on("wheel", Canvas.evt_scrolled.bind(this));
		// this.canvas_jquery.on("DOMMouseScroll", Canvas.evt_DOMscrolled.bind(this));

		this.canvas_jquery.on("touchstart",this.evt_drag_start);
		this.canvas_jquery.on("touchmove",this.evt_drag_move);
		this.canvas_jquery.on("touchend",this.evt_drag_end);
		
		this.dimensions.ofX = 0.5*this.dimensions.width-0.5*this.dimensions.content.width;
		this.dimensions.ofY = 0;

		this.calc_limits();

		if(window.innerWidth < 764){
			this.rescale(0.5);
		}

		this.resize();

		this.get_content_box();

		this.update_frame();
	},

	get_content_box: function(){

		var l = 1e100;
		var r = 0;
		var t = 1e100;
		var b = 0;

		for(const i in modules){
			if(!modules[i].visible){
				continue;
			}

			// console.log("Module "+i, modules[i].width, modules[i].height, modules[i].OffsetX, modules[i].OffsetY, modules[i].r);
			// console.log([l, r, t, b]);

			if(modules[i].r == 0){
				if(modules[i].OffsetX < l){
					l = modules[i].OffsetX - 20;
				}

				if((modules[i].OffsetX + modules[i].width) > r){
					r = modules[i].OffsetX + modules[i].width + 20;
				}

				if(modules[i].OffsetY < t){
					t = modules[i].OffsetY - 20;
				}

				if((modules[i].OffsetY + modules[i].height) > b){
					b = modules[i].OffsetY + modules[i].height + 20;
				}
			}
			else if(modules[i].r == 1){
				if((modules[i].OffsetX - modules[i].width) < l){
					l = modules[i].OffsetX - modules[i].width - 20;
				}

				if((modules[i].OffsetX) > r){
					r = modules[i].OffsetX + 20;
				}

				if((modules[i].OffsetY - modules[i].height) < t){
					t = modules[i].OffsetY - modules[i].height - 20;
				}

				if((modules[i].OffsetY) > b){
					b = modules[i].OffsetY + 20;
				}
			}

			// console.log([l, r, t, b]);
		}

		this.dimensions.content.width = (r - l)*this.dimensions.scale;
		this.dimensions.content.height = (b - t)*this.dimensions.scale;
		this.dimensions.content.ofX = l*this.dimensions.scale;
		this.dimensions.content.ofY = t*this.dimensions.scale;

		// console.log("content_box", [r-l, b-t])
		this.calc_limits();
	},
	resize: function(){
		console.log("resize");

	    this.dimensions.height = (this.canvas.height = $('#LayoutContainer').height());
		this.dimensions.width  = (this.canvas.width  = $('#LayoutContainer').width());
		
		//Recenter content
		this.dimensions.ofX = 0.5*this.dimensions.width/this.dimensions.scale-0.5*this.dimensions.content.width;
		this.dimensions.ofY = 0;

		this.get_content_box();
		this.calc_limits();

		this.update_frame();
	},
	rescale: function(new_scale){
		console.log("rescale");
		this.c.scale(1/this.dimensions.scale,1/this.dimensions.scale);

		this.dimensions.scale = new_scale;
		this.c.scale(this.dimensions.scale,this.dimensions.scale);

		this.get_content_box();

		//Recenter content
		this.dimensions.ofX = 0.5*(this.dimensions.width-this.dimensions.content.width);
		this.dimensions.ofY = 0;

		this.moved();

		this.update_frame();
	},

	calc_limits: function(){
		if(this.dimensions.content.width == 0 || this.dimensions.content.height == 0){
			return;
		}
		this.dimensions.x_left   = 0;
		this.dimensions.x_right  = this.dimensions.content.width;
		this.dimensions.y_top    = 0;
		this.dimensions.y_bottom = this.dimensions.content.height;
	},

	calc_dotmatrix: function(){
		$.each(modules,function(module,module_v){
			module_v.dotmatrix = [];
			$.each(module_v.data,function(i,v){
				var coords = [];
				var block_list = [];
				var switch_list = [];
				
				data = v.dotmatrix()
				coords.push(...data[0]);
				block_list.push(data[1]);
				if(data[2] != undefined){
					if(data[2].isArray != undefined && data[2].isArray()){
						switch_list.push(...data[2]);
					}
					else{
						switch_list.push(data[2]);
					}
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
			});
		});
	},
	draw_dotmatrix: function(){
		c.lineWidth = 0.1;
		//Draw dots
		var dot_radius = 2.8;
		var obj = this;

		$.each(modules,function(module,module_v){
			if (module_v.visible){
				module_v.draw_dotmatrix(c, obj.setStrokeColor, dot_radius);
			}
		});
	},

	draw_background: function(){
		c.lineWidth = 6;
		var obj = this;
		//Draw background lines
		$.each(modules,function(module,module_v){
			if (module_v.visible){
				module_v.draw_background(c, obj.setStrokeColor);
			}
		});
	},

	draw_foreground: function(){
		c.lineWidth = 6;
		var obj = this;
		//Draw lines
		c.lineWidth = 6;
		$.each(modules,function(module,module_v){
			if (module_v.visible){
				module_v.draw_foreground(c, obj.setStrokeColor);
			}
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
	    	(s == 0)?this.c.strokeStyle = "#aaf":this.c.strokeStyle = "#ddf";
	    }
	    else if(b == 6){ // RESERVED_SWICTH
	    	(s == 0)?this.c.strokeStyle = "#aaa":this.c.strokeStyle = "#fcc";
	    }
	    else{ //Unknown
	    	(s == 0)?this.c.strokeStyle = "#ddd":this.c.strokeStyle = "#eee";
	    }
	},

    update_frame: function(){
    	this.c.setTransform(1, 0, 0, 1, 0, 0);
		this.c.clearRect(-10, -10, 10 + this.dimensions.width/this.dimensions.scale, 10 + this.dimensions.height/this.dimensions.scale);
		var tmp_this = this;

		this.c.setTransform(1, 0, 0, 1, this.dimensions.ofX, this.dimensions.ofY);

		this.c.beginPath();
		this.c.lineWidth = "6";
		this.c.rect(0, 0,this.dimensions.content.width, this.dimensions.content.height);

		this.c.stroke();

		this.c.setTransform(this.dimensions.scale, 0, 0, this.dimensions.scale, this.dimensions.ofX-this.dimensions.content.ofX, this.dimensions.ofY-this.dimensions.content.ofY);


		for (var w = this.dimensions.content.ofX; w < (this.dimensions.content.ofX + this.dimensions.content.width/this.dimensions.scale) + 50; w += 100) {
		    for (var h = this.dimensions.content.ofY; h < (this.dimensions.content.ofY + this.dimensions.content.height/this.dimensions.scale) + 50; h += 100) {
		      var x = Math.round((w) / 100) * 100;
		      var y = Math.round((h) / 100) * 100;

		      this.c.beginPath();

		      this.c.strokeStyle = "#ff0000";
		      this.c.lineWidth = 0;
				this.c.fillStyle = "#ff0000";

				this.c.arc(x, y, 1, 0, 2*Math.PI, false);

				this.c.fill();
				this.c.stroke();

		      this.c.fillText(x + ',' + y, x, y+20);
		    }
		  }

		this.draw_background();
		this.draw_dotmatrix();
		this.draw_foreground();

		

		//Draw Tooltips
		$.each(modules,function(module,module_v){
			if (module_v.visible == false){
				return true;
			}
			var ofX = module_v.OffsetX;
			var ofY = module_v.OffsetY;
			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);

			c.beginPath();
			c.lineWidth = 0;

		      c.strokeStyle = "#00FF00";
				c.fillStyle = "#00ff00";

				c.arc(ofX, ofY, 2, 0, 2*Math.PI, false);

				c.fill();
				c.stroke();

				c.fillStyle = "black";
				if(this.r == 0){
		      		c.fillText(ofX + ',' + ofY+' '+this.id, ofX+5, ofY+10);
				}
				else{
					c.fillText(ofX + ',' + ofY+' '+this.id, ofX-40, ofY+10);
				}
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

		_x = (x - tmp_this.dimensions.ofX + tmp_this.dimensions.content.ofX)/tmp_this.dimensions.scale;
		_y = (y - tmp_this.dimensions.ofY + tmp_this.dimensions.content.ofY)/tmp_this.dimensions.scale;

		console.log("xy", [_x, _y]);

		$.each(modules,function(module,module_v){
			if(!module_v.visible){
				console.log("Module "+module+" not visible");
				return true;
			}

			var ofX = module_v.OffsetX;
			var ofY = module_v.OffsetY;

			var rvX = Math.cos(module_v.r*Math.PI);
			var rvX_ = Math.cos((module_v.r+0.5)*Math.PI);
			var rvY = Math.sin((module_v.r+0.5)*Math.PI);
			var rvY_ = Math.sin((module_v.r)*Math.PI);

			_x = _x - ofX;
			_y = _y - ofY;

			_l = Math.sqrt(Math.pow(_x,2)+Math.pow(_y,2));
			_r = Math.atan2(_y, _x);
			_r -= module_v.r*Math.PI;
			_x = _l*Math.cos(_r);
			_y = _l*Math.sin(_r);

			if(_x < 0 || _y < 0 || _x > module_v.width || _y > module_v.height){
				console.log("Outside module "+module);
				return true;
			}
			console.log("Hit module" + module);
			for(var i = 0; i < module_v.hitboxes.length; i++){
				if(module_v.hitboxes[i] && {}.toString.call(module_v.hitboxes[i]) === '[object Function]'){
					console.log("Hit switch");
					module_v.hitboxes[i](_x, _y);
				}
			}
		});
		//if(update)
		//	this.update_frame();
	},

	moved: function(){
		// console.log("Moved");
		// console.log([this.dimensions.ofX, this.dimensions.ofY, this.dimensions.width, this.dimensions.content]);
		// console.log([this.dimensions.x_left, this.dimensions.x_right, this.dimensions.y_top, this.dimensions.y_bottom]);
		if(this.dimensions.width < this.dimensions.content.width){
			//Scroll limiter
			if((this.dimensions.x_left) > -this.dimensions.ofX){
				this.dimensions.ofX = -this.dimensions.x_left;
			}
			if((this.dimensions.x_right) < -this.dimensions.ofX+this.canvas.width){
				this.dimensions.ofX = -(this.dimensions.x_right - this.canvas.width);
			}
		}
		if(this.dimensions.height < this.dimensions.content.height){
			//Scroll limiter
			if((this.dimensions.y_top) > -this.dimensions.ofY){
				this.dimensions.ofY = this.dimensions.y_top;
			}
			if((this.dimensions.y_bottom) < -this.dimensions.ofY+this.canvas.height){
				this.dimensions.ofY = -(this.dimensions.y_bottom - this.canvas.height);
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
	},
	evt_drag_move: function(evt){
		if(Canvas.touchMoved == true){
			if(Canvas.dimensions.width < Canvas.dimensions.content.width){
				Canvas.dimensions.ofX += (evt.touches[0].pageX - Canvas.touchMove.x)/Canvas.dimensions.scale;
			}
			if(Canvas.dimensions.height < Canvas.dimensions.content.height){
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

		if(this.dimensions.width < this.dimensions.content.width){
			if(Math.abs(evt.originalEvent.deltaX) < 20){
				this.dimensions.ofX -= evt.originalEvent.deltaX*10;
			}
			else{
				this.dimensions.ofX -= evt.originalEvent.deltaX/10;
			}
		}
		if(this.dimensions.height < this.dimensions.content.height){
			if(Math.abs(evt.originalEvent.deltaY) < 20){
				this.dimensions.ofY -= evt.originalEvent.deltaY*10;
			}
			else{
				this.dimensions.ofY -= evt.originalEvent.deltaY/10;
			}
		}

		this.moved();
	}
}


events.add_init(Canvas.init.bind(Canvas));
events.add_resize(Canvas.resize.bind(Canvas));

Canvas.init_calc();
ro = Canvas.ro;
ds = Canvas.ds;
radia = Canvas.radia;

function equation_tester(equation, values) {
	result = 0;
	if(values.x != undefined){
		for(var i = 1; i <= equation.x.length; i++){
			result += equation.x[i-1] * Math.pow(values.x, i);
		}
	}
	if(values.y != undefined){
		for(var i = 1; i <= equation.y.length; i++){
			result += equation.y[i-1] * Math.pow(values.y, i);
		}
	}
	if(equation.v != undefined && equation.op!= undefined ){
		if(equation.op == "lt"){
			return (result < equation.v);
		}
		else{
			return (result > equation.v);
		}
	}
	else{
		return result;
	}
}

class canvas_line {
	constructor(module_id, block, x1, y1, x2, y2, options){
		this.type = "line";
		this.module_id = module_id;
		this.m = undefined;
		this.b = block;
		this.x1 = x1;
		this.y1 = y1;
		this.x2 = x2;
		this.y2 = y2;

		if (options == undefined) { options = {}; }

		if (options.if != undefined){
			this.if = options.if;
		}
	}

	init(){
		this.m = modules[this.module_id]
	}

	dotmatrix(){
		var coords = [{x: this.x1, y: this.y1}, {x: this.x2, y: this.y2}];
		return [coords, this.b]
	}

	draw_fore(cnvs, stroke, rotation, offset){
		if(this.m == undefined){
			this.update_module();
		}
		stroke(this.m.blocks[this.b]);
		if(this.if != undefined){
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] != this.if[i].st)
					return true; // Print on background
			}
		}

		var x1 = rotation.X*this.x1 + rotation.Y_*this.y1 + offset.X;
		var y1 = rotation.Y*this.y1 + rotation.X_*this.x1 + offset.Y;

		var x2 = rotation.X*this.x2 + rotation.Y_*this.y2 + offset.X;
		var y2 = rotation.Y*this.y2 + rotation.X_*this.x2 + offset.Y;

		cnvs.moveTo(x1, y1);
		cnvs.lineTo(x2, y2);
	}

	draw_back(cnvs, stroke, color, rotation, offset){
		if(this.m == undefined){
			this.update_module();
		}
		stroke(this.m.blocks[this.b], color);

		if(this.if != undefined){
			var counter = 0;
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] == this.if[i].st)
					counter++;
			}
			if(counter == this.if.length){
				return true; // Print on foreground
			}
		}

		var x1 = rotation.X*this.x1 + rotation.Y_*this.y1 + offset.X;
		var y1 = rotation.Y*this.y1 + rotation.X_*this.x1 + offset.Y;

		var x2 = rotation.X*this.x2 + rotation.Y_*this.y2 + offset.X;
		var y2 = rotation.Y*this.y2 + rotation.X_*this.x2 + offset.Y;

		cnvs.moveTo(x1, y1);
		cnvs.lineTo(x2, y2);
	}
}

class canvas_arc {
	constructor(module_id, block, cx, cy, r, arc, options){
		this.type = "arc";
		this.module_id = module_id;
		this.m = undefined;
		this.b = block;
		this.cx = cx;
		this.cy = cy;
		this.r = r;
		this.start = arc[0];
		this.end = arc[1];
		this.cw = arc[2];

		if (options == undefined) { options = {}; }

		if (options.if != undefined){
			this.if = options.if;
		}
	}

	init(){
		this.m = modules[this.module_id]
	}

	dotmatrix(){
		var coords = [{x: this.cx+Math.cos(this.start*Math.PI)*this.r, y: this.cy+Math.sin(this.start*Math.PI)*this.r}, 
					  {x: this.cx+Math.cos(this.end * Math.PI)*this.r, y: this.cy+Math.sin(this.end * Math.PI)*this.r}];
		return [coords, this.b]
	}

	draw_back(cnvs, stroke, color, rotation, offset){
		stroke(this.m.blocks[this.b], color);
		if(this.if != undefined){
			var counter = 0;
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] == this.if[i].st)
					counter++;
			}
			if(counter == this.if.length){
				 return true//Print on foreground
			}
		}
		cnvs.arc(rotation.X*this.cx+rotation.Y_*this.cy+offset.X,
			  rotation.Y*this.cy+rotation.X_*this.cx+offset.Y,
			  this.r,
			  Math.PI*(-this.m.r+this.start),
			  Math.PI*(-this.m.r+this.end),
			  this.cw);
	}

	draw_fore(cnvs, stroke, rotation, offset){
		stroke(this.m.blocks[this.b]);
		if(this.if != undefined){
			var where = "F";
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] != this.if[i].st)
					return true //Print on background
			}
		}
		cnvs.arc(rotation.X*this.cx+rotation.Y_*this.cy+offset.X,
			  rotation.Y*this.cy+rotation.X_*this.cx+offset.Y,
			  this.r,
			  Math.PI*(-this.m.r+this.start),
			  Math.PI*(-this.m.r+this.end),
			  this.cw);
	}
}

class canvas_switch {
	constructor(module_id, block, side, sw, x, y, r, options){
		if(side == "l"){
			Object.setPrototypeOf(this, new canvas_switch_l);
		}
		else if(side == "r"){
			Object.setPrototypeOf(this, new canvas_switch_r);
		}

		this.type = "sw"+side;
		this.module_id = module_id;
		this.m = undefined;
		this.b = block;
		this.s = sw;
		this.x = x;
		this.y = y;
		this.r = r % 2;

		if (options == undefined) { options = {}; }

		if (options.if != undefined){
			this.if = options.if;
		}
	}

	init(){
		this.m = modules[this.module_id];

		this.m.add_hitbox(this.hit.bind(this));

		this.hit_eqn = [];

		//Calculate hitboxes
		//arc hit box
		// (x-a)^2+(y-b)^2 = r^2
		// x^2-2ax+a^2+y^2-2b^2+b^2 = r^2
		// x^2 - 2ax + y^2 - 2b^2 = r^2 - a^2 - b^2
		this.hit_eqn.push({x:[1], y:[], op:"gt", v:0});
		if((this.r % 0.5) == 0){
			this.hit_eqn.push({x:[1], y:[], op:"lt", v:ro[0].x});
		}
		else{
			this.hit_eqn.push({x:[1], y:[], op:"lt", v:(ro[0].x+ds)/Math.cos(Math.PI/4)})
		}
		var a = 0;
		if(this.type == "swr"){
			this.hit_eqn.push({x:[], y:[1], op:"gt", v:-hit_radius});
			this.hit_eqn.push({x:[1], y:[1], op:"lt", v:(ro[0].x+ro[0].y)});
			var b = radia[0];
		}
		else{
			this.hit_eqn.push({x:[], y:[1], op:"lt", v:hit_radius});
			this.hit_eqn.push({x:[1], y:[-1], op:"lt", v:(ro[0].x+ro[0].y)});
			var b = -radia[0];
		}
		this.hit_eqn.push({x:[-2*a, 1], y:[-2*b, 1], v:(Math.pow(radia[0]-hit_radius, 2)-(a*a)-(b*b)), op:"gt"})
	}

	hit_eval(x, y){
		if(this.hit_eqn.length == 0){
			return false;
		}

		var _l = Math.sqrt(Math.pow(x-this.x, 2)+Math.pow(y-this.y, 2));
		var _r = Math.atan2((y-this.y), (x-this.x));
		_r -= this.r*Math.PI;
		x = _l*Math.cos(_r);
		y = _l*Math.sin(_r);

		for(var i = 0; i < this.hit_eqn.length; i++){
			if(!equation_tester(this.hit_eqn[i], {x:x, y:y})){
				return false;
			}
		}
		return true;
	}

	hit(x, y){
		// {eval_code:"(_x > 2.5 && _x < 52.5 && _y > 15 && _y < 40)?1:0;",name:"Switch 1",action:"tSw",switch:0},
		console.log("Check hit switch "+this.s);
		if(this.hit_eval(x,y)){
			console.log("Throw switch "+this.module_id+":"+this.s);
			throw_Switch(this.module_id,this.s);
		}
	}

	draw_back(cnvs, obj, color, rotation, offset){
		var x = rotation.X*this.x + rotation.Y_*this.y + offset.X;
		var y = rotation.Y*this.y + rotation.X_*this.x + offset.Y;
		var r = this.r - this.m.r

		if(this.if != undefined){
			var j = 0;
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] != this.if[i].st){
					j++;
				}
			}
			if(j != this.if.length){
				this.draw(cnvs, x, y, r, this.type, "B",this.m.blocks[this.b],this.m.switches[this.s]);
				cnvs.stroke();
				return true
			}
			this.draw(cnvs, x, y, r, this.type, "G",this.m.blocks[this.b],this.m.switches[this.s]);
		}
		else{
			this.draw(cnvs, x, y, r, this.type, "B",this.m.blocks[this.b],this.m.switches[this.s]);
		}
	}

	draw_fore(cnvs, obj, rotation, offset){
		if(this.if != undefined){
			var where = "F";
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] != this.if[i].st)
					return true;
			}
		}

		var x = rotation.X*this.x + rotation.Y_*this.y + offset.X;
		var y = rotation.Y*this.y + rotation.X_*this.x + offset.Y;
		var r = this.r - this.m.r

		this.draw(x, y, r, this.type, "F",
			        this.m.blocks[this.b],
			        this.m.switches[this.s]);
	}
}

class canvas_switch_l extends canvas_switch{
	dotmatrix(){
		var coords = [];

		var rvX = Math.cos(this.r*Math.PI);
		var rvX_ = Math.cos((this.r+0.5)*Math.PI);
		var rvY = Math.sin((this.r+0.5)*Math.PI);
		var rvY_ = Math.sin((this.r)*Math.PI);

		coords.push({x:this.x,y:this.y});
		if((this.r + 0.25) % 0.5 == 0){
			coords.push({x: this.x+rvX*(ro[0].x-ds), y: this.y+rvY_*(ro[0].x-ds)});
		}else if(this.r % 0.5 == 0){
			coords.push({x: this.x+rvX*ro[0].x, y: this.y+rvY_*ro[0].x});
		}

		var tx = this.x + rvX * ro[0].x - rvX_ * ro[0].y;
		var ty = this.y + rvY_* ro[0].x - rvY * ro[0].y;
		coords.push({x: tx, y: ty});

		return [coords, this.b, this.s];
	}

	draw(cnvs, x,y,r,lr,type,bl,sw){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = Math.sin((r)*Math.PI);
		if((sw == 0 && type == "F") || (sw == 1 && type == "B") || type == "G"){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			cnvs.moveTo(x,y);
			if((r + 0.25) % 0.5 == 0){
				cnvs.lineTo(x+rvX*(ro[0].x-ds),y+rvY_*(ro[0].x-ds));
			}else if(r % 0.5 == 0){
				cnvs.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
			}
		}
		if((sw == 1 && type == "F") || (sw == 0 && type == "B") || type == "G"){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			var tx = x - rvX_ * radia[0];
			var ty = y - rvY * radia[0];
			cnvs.arc(tx, ty, radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
		}
	}
}

class canvas_switch_r extends canvas_switch{
	dotmatrix(){
		var coords = [];
		
		var rvX = Math.cos(this.r*Math.PI);
		var rvX_ = Math.cos((this.r+0.5)*Math.PI);
		var rvY = Math.sin((this.r+0.5)*Math.PI);
		var rvY_ = Math.sin((this.r)*Math.PI);

		coords.push({x:this.x,y:this.y});
		if((this.r + 0.25) % 0.5 == 0){
			coords.push({x: this.x+rvX*(ro[0].x-ds), y: this.y+rvY_*(ro[0].x-ds)});
		}else if(this.r % 0.5 == 0){
			coords.push({x: this.x+rvX*ro[0].x, y: this.y+rvY_*ro[0].x});
		}

		var tx = this.x + rvX * ro[0].x + rvX_ * ro[0].y;
		var ty = this.y + rvY_* ro[0].x + rvY * ro[0].y;
		coords.push({x: tx, y: ty});

		return [coords, this.b, this.s];
	}

	draw(cnvs, x,y,r,lr,type,bl,sw){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = Math.sin((r)*Math.PI);
		if((sw == 0 && type == "F") || (sw == 1 && type == "B") || type == "G"){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			cnvs.moveTo(x,y);
			if((r + 0.25) % 0.5 == 0){
				cnvs.lineTo(x+rvX*(ro[0].x-ds),y+rvY_*(ro[0].x-ds));
			}else if(r % 0.5 == 0){
				cnvs.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
			}
		}
		if((sw == 1 && type == "F") || (sw == 0 && type == "B") || type == "G"){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			var tx = x + rvX_ * radia[0];
			var ty = y + rvY * radia[0];
			cnvs.arc(tx, ty, radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		}
	}
}

class canvas_double_slip {
	constructor(module_id, block, side, sw, x, y, r, options){
		if(side == ""){
			Object.setPrototypeOf(this, new canvas_dslip);
		}
		else if(side == "f"){
			Object.setPrototypeOf(this, new canvas_fl_dslip);
		}

		this.type = "ds"+side;
		this.module_id = module_id;
		this.m = undefined;
		this.b = block;
		if(sw == undefined){
			this.sA = undefined; this.sB = undefined;
		}
		else{
			this.sA = sw[0];
			this.sB = sw[1];
		}
		this.x = x;
		this.y = y;
		this.r = r;

		if (options == undefined) { options = {}; }

		if (options.if != undefined){
			this.if = options.if;
		}
	}

	init(){
		this.m = modules[this.module_id]

		this.m.add_hitbox(this.hit.bind(this));

		this.hit_eqn = []

		//Calculate hitboxes
		//arc hit box
		// (x-a)^2+(y-b)^2 = r^2
		// x^2-2ax+a^2+y^2-2b^2+b^2 = r^2
		// x^2 - 2ax + y^2 - 2b^2 = r^2 - a^2 - b^2
		console.log("r: "+this.r%0.5, this.type)
		if(this.type == "ds"){
			this.hit_eqn.push({x:[1], y:[], op:"gt", v:ds});
			this.hit_eqn.push({x:[1], y:[], op:"lt", v:ro[0].x});
			var a = ds;
			var b = radia[0];
			this.hit_eqn.push({x:[-2*a, 1], y:[-2*b, 1], v:(Math.pow(radia[0]-hit_radius, 2)-(a*a)-(b*b)), op:"gt"})
			this.hit_eqn.push({x:[1], y:[1], op:"gt", v:(-ro[0].y)});
			var a = ro[0].x;
			var b = -radia[0];
			this.hit_eqn.push({x:[-2*a, 1], y:[-2*b, 1], v:(Math.pow(radia[0]-hit_radius, 2)-(a*a)-(b*b)), op:"gt"})
			this.hit_eqn.push({x:[1], y:[1], op:"lt", v:(ro[0].x+ro[0].y+ds)});
		}
		else if(this.type == "dsf"){
			this.hit_eqn.push({x:[1], y:[], op:"gt", v:ds});
			this.hit_eqn.push({x:[1], y:[], op:"lt", v:ro[0].x});
			var a = ds;
			var b = -radia[0];
			console.log(this.sA+"\t-2*"+a+"*x+x^2 - 2*"+b+"*y+y^2 = (r-5)^2-"+a+"^2-"+b+"^2");
			this.hit_eqn.push({x:[-2*a, 1], y:[-2*b, 1], v:(Math.pow(radia[0]-hit_radius, 2)-(a*a)-(b*b)), op:"gt"});
			this.hit_eqn.push({x:[1], y:[-1], op:"gt", v:(-ro[0].y)});
			// this.hit_eqn.push({x:[1], y:[-1], op:"lt", v:(-ro[0].y)});
			var a = ro[0].x;
			var b = radia[0];
			console.log(this.sB+"\t-2*"+a+"*x+x^2 - 2*"+b+"*y+y^2 = (r-5)^2-"+a+"^2-"+b+"^2");
			this.hit_eqn.push({x:[-2*a, 1], y:[-2*b, 1], v:(Math.pow(radia[0]-hit_radius, 2)-(a*a)-(b*b)), op:"gt"});
			this.hit_eqn.push({x:[1], y:[-1], op:"lt", v:(ro[0].x+ro[0].y+ds)});
		}
	}

	hit_eval(x, y){
		if(this.hit_eqn.length == 0){
			return false;
		}

		var _l = Math.sqrt(Math.pow(x-this.x, 2)+Math.pow(y-this.y, 2));
		var _r = Math.atan2((y-this.y), (x-this.x));
		_r -= this.r*Math.PI;
		x = _l*Math.cos(_r);
		y = _l*Math.sin(_r);

		for(var i = 0; i < this.hit_eqn.length; i++){
			if(!equation_tester(this.hit_eqn[i], {x:x, y:y})){
				return false;
			}
		}
		return true;
	}

	hit(x, y){
		// {eval_code:"(_x > 2.5 && _x < 52.5 && _y > 15 && _y < 40)?1:0;",name:"Switch 1",action:"tSw",switch:0},
		if(this.hit_eval(x,y)){
			console.warn("Throw double slip "+this.module_id+":"+this.sA +"+"+this.sB);
			throw_doubleSlib(this.module_id, [this.sA, this.sB]);
		}
	}

	// dotmatrix(){
	// 	var coords = [];
	// 	coords.push({x:this.xtr,y:this.ytr});
	// 	coords.push({x:this.xtr-ds,y:this.ytr+ro[0].y});
	// 	coords.push({x:this.xtr-ro[0].x,y:this.ytr+ro[0].y});
	// 	coords.push({x:this.xtr-ro[0].x-ds,y:this.ytr+2*ro[0].y});

	// 	return [coords, this.b, [this.sA, this.sB]];
	// }

	draw_back(cnvs, obj, color, rotation, offset){
		var x = rotation.X*this.x + rotation.Y_*this.y + offset.X;
		var y = rotation.Y*this.y + rotation.X_*this.x + offset.Y;
		var r = this.r - this.m.r;

		if(this.if != undefined){
			var j = 0;
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] != this.if[i].st){
					j++;
				}
			}
			if(j != this.if.length){
				this.draw(cnvs, x, y, r, "B",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
				cnvs.stroke();
				return true
			}
			this.draw(cnvs, x, y, r, "G",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
		}
		else{
			this.draw(cnvs, x, y, r, "B",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
		}
	}

	draw_fore(cnvs, obj, rotation, offset){
		if(this.if != undefined){
			var where = "F";
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.m.switches[this.if[i].sw] != this.if[i].st)
					return true;
			}
		}

		var x = rotation.X*this.x + rotation.Y_*this.y + offset.X;
		var y = rotation.Y*this.y + rotation.X_*this.x + offset.Y;
		var r = this.r - this.m.r

		this.draw(cnvs, x, y, r, "F",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
	}
}

class canvas_dslip extends canvas_double_slip {
	dotmatrix(){
		var coords = [];

		var rvX = Math.cos(this.r*Math.PI);
		var rvX_ = -Math.cos((this.r+0.5)*Math.PI);
		var rvY = Math.sin((this.r+0.5)*Math.PI);
		var rvY_ = -Math.sin((this.r)*Math.PI);
		var tx, ty, dis_X, dis_Y;

		tx = this.x - rvX*ds - rvY_*ro[0].y;
		ty = this.y - rvX_*ds - rvY*ro[0].y;
		coords.push({x: tx, y: ty});

		dis_X = ro[0].x+ds;
		dis_Y = 2 * ro[0].y;
		coords.push({x: tx+rvX*dis_X+rvY_*dis_Y, y: ty+rvX_*dis_X+rvY*dis_Y});

		tx = this.x;
		ty = this.y;
		coords.push({x: tx, y: ty});

		dis_X = ro[0].x - ds;
		dis_Y = 0;
		coords.push({x: tx+rvX*dis_X+rvY_*dis_Y, y: ty+rvX_*dis_X+rvY*dis_Y});

		return [coords, this.b, [this.sA, this.sB]];
	}

	draw(cnvs, x,y,r,type,bl,swA,swB){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = -Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = -Math.sin((r)*Math.PI);
		var tx, ty, dis_X, dis_Y;

		if((swA == 1 && swB == 0 && type == "F") || ((swA == 0 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			tx = x - rvX*(ds - ro[0].x) - rvY_*radia[0];
			ty = y - rvX_*(ds - ro[0].x) - rvY*radia[0];
			
			cnvs.arc(tx, ty,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.50),true);

			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 1 && swB == 1 && type == "F") || ((swA == 0 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			dis_X = ro[0].x+ds;
			dis_Y = 2 * ro[0].y;

			tx = x - rvX*ds - rvY_*ro[0].y;
			ty = y - rvX_*ds - rvY*ro[0].y;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}

		if((swA == 0 && swB == 0 && type == "F") || ((swA == 1 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			tx = x;
			ty = y;
			dis_X = ro[0].x - ds;
			dis_Y = 0;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 0 && swB == 1 && type == "F") || ((swA == 1 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			tx = x+rvY_*radia[0];
			ty = y+rvY*radia[0];

			cnvs.arc(tx,ty,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);

			cnvs.stroke();cnvs.beginPath();
		}
	}
}

class canvas_fl_dslip extends canvas_double_slip {
	dotmatrix(){
		var coords = [];

		var rvX = Math.cos(this.r*Math.PI);
		var rvX_ = -Math.cos((this.r+0.5)*Math.PI);
		var rvY = Math.sin((this.r+0.5)*Math.PI);
		var rvY_ = -Math.sin((this.r)*Math.PI);
		var tx, ty, dis_X, dis_Y;

		tx = this.x - rvX*ds + rvY_*ro[0].y;
		ty = this.y - rvX_*ds + rvY*ro[0].y;
		coords.push({x: tx, y: ty});

		dis_X = ro[0].x+ds;
		dis_Y = -2 * ro[0].y;
		coords.push({x: tx+rvX*dis_X+rvY_*dis_Y, y: ty+rvX_*dis_X+rvY*dis_Y});

		tx = this.x;
		ty = this.y;
		coords.push({x: tx, y: ty});

		dis_X = ro[0].x - ds;
		dis_Y = 0;
		coords.push({x: tx+rvX*dis_X+rvY_*dis_Y, y: ty+rvX_*dis_X+rvY*dis_Y});

		return [coords, this.b, [this.sA, this.sB]];
	}

	draw(cnvs, x,y,r,type,bl,swA,swB){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = -Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = -Math.sin((r)*Math.PI);
		var tx, ty, dis_X, dis_Y;

		if((swA == 1 && swB == 0 && type == "F") || ((swA == 0 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			tx = x - rvY_*radia[0];
			ty = y - rvY*radia[0];
			
			cnvs.arc(tx, ty,radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);

			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 1 && swB == 1 && type == "F") || ((swA == 0 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			dis_X = ro[0].x+ds;
			dis_Y = -2 * ro[0].y;

			tx = x - rvX*ds + rvY_*ro[0].y;
			ty = y - rvX_*ds + rvY*ro[0].y;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}

		if((swA == 0 && swB == 0 && type == "F") || ((swA == 1 || swB == 1) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			tx = x;
			ty = y;
			dis_X = ro[0].x - ds;
			dis_Y = 0;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 0 && swB == 1 && type == "F") || ((swA == 1 || swB == 0) && type == "B")){
			(type == "F")?Canvas.setStrokeColor(bl,0):Canvas.setStrokeColor(bl,1);

			tx = x - rvX*(ds-ro[0].x)+rvY_*radia[0];
			ty = y - rvX_*(ds-ro[0].x)+rvY*radia[0];

			cnvs.arc(tx,ty,radia[0],Math.PI*(r-0.5),Math.PI*(r-0.75),true);

			cnvs.stroke();cnvs.beginPath();
		}
	}
}

class canvas_module {
	constructor(id, name, dimensions, blocks, switches, connections){
		this.id = id;
		this.name = name;
		this.OffsetX = 0;
		this.OffsetY = 0;
		this.width = dimensions.w;
		this.height = dimensions.h;
		this.r = 0;
		this.visible = false;
		this.connections = connections;
		this.connection_link = [];
		
		for(var i = 0; i < this.connections.length; i++){
			this.connections[i].connected = false;
		}

		this.blocks = Array.apply(null, Array(blocks)).map(function (){return 5});
		this.switches = Array.apply(null, Array(switches)).map(function (){return 0});

		this.dotmatrix = [];
		this.data = [];
		this.hitboxes = [];
	}

	move(options = {}){
		console.log("Move module "+this.id, options);
		var dx = 0;
		var dy = 0;
		var dr = 0;
		if(options.OffsetX != undefined){
			dx = options.OffsetX - this.OffsetX;
			this.OffsetX = Math.round(options.OffsetX * 1000) / 1000;
		}
		if(options.OffsetY != undefined){
			dy = options.OffsetY - this.OffsetY;
			this.OffsetY = Math.round(options.OffsetY * 1000) / 1000;
		}
		if(options.r != undefined){
			dr = options.r - this.r;
			this.r = options.r;
			this.r = this.r % 2;
		}

		console.log("Moved module "+this.id, {x: this.OffsetX, y: this.OffsetY, r:this.r});

		// if((options.nonest == undefined || options.nonest == false) && this.connection_link){
		// 	for(var i = 0; i < this.connections.length; i++){
		// 		if(this.connections[i].connected && this.connection_link[i] != undefined && this.connection_link[i] != options.parent){
		// 			var m = modules[this.connection_link[i]];
		// 			console.log("Move connected module", m.id);
		// 			var point = rotated_point(m.OffsetX - this.OffsetX, m.OffsetY - this.OffsetY, m.r + dr, this.OffsetX, this.OffsetY);
		// 			if((m.r + dr) % 2 == 0){
		// 				point.y += dy;
		// 				point.x += dx;
		// 			}
		// 			else if((m.r + dr) % 2 == 1){
		// 				point.y -= dy;
		// 				point.x += dx;
		// 			}
		// 			else if((m.r + dr) % 2 == 0.5){
		// 				point.y += dy;
		// 				point.x -= dx;	
		// 			}
		// 			m.move({OffsetX: point.xC, OffsetY: point.y, r: m.r + dr, parent: this.id});
		// 		}
		// 	}
		// }

		Canvas.get_content_box();
	}

	connect(){
		console.log("Module connect");
		for(var i = 0; i < this.connection_link.length; i++){
			if(this.connections[i].connected){
				continue;
			}
			var aModule = modules[this.connection_link[i]];

			if(!aModule.connection_link.includes(this.id)){
				continue;
			}

			var point = rotated_point(this.connections[i].x, this.connections[i].y, this.r, this.OffsetX, this.OffsetY);

			var anchorX = point.x;
			var anchorY = point.y;

			var anchorR = (this.connections[i].r + 1) % 2;

			var connect_list = aModule.connection_link.indexOf(this.id);

			console.log("Module "+this.id+" anchor "+i+"=>"+connect_list, aModule.id);

			if(anchorR != aModule.connections[connect_list].r){
				this.move({r: aModule.r + aModule.connections[connect_list].r - anchorR})
			}
			else{
				this.move({r: aModule.r});
			}

			point = rotated_point(aModule.connections[connect_list].x, aModule.connections[connect_list].y, aModule.r, aModule.OffsetX, aModule.OffsetY);
			if(this.r == anchorR){
				point = rotated_point(this.connections[i].x, -this.connections[i].y, this.r, point.x, point.y);
			}
			else{
				point = rotated_point(-this.connections[i].x, -this.connections[i].y, this.r, point.x, point.y);
			}

			this.move({OffsetX: point.x, OffsetY: point.y});

			aModule.connections[connect_list].connected = true;
			this.connections[i].connected = true;
		}
	}

	add(obj){
		this.data.push(obj);
	}

	add_hitbox(hit){
		this.hitboxes.push(hit);
	}

	init(options = {}){
		if(options.visible != undefined && options.visible){
			this.visible = true;
		}
		if(options.OffsetX != undefined){
			this.OffsetX = options.OffsetX;
		}
		if(options.OffsetY != undefined){
			this.OffsetY = options.OffsetY;
		}
		if(options.r != undefined){
			this.r = options.r;
		}

		for(var i = 0; i < this.data.length; i++){
			this.data[i].init();
		}

		Canvas.get_content_box();
	}

	draw_dotmatrix(cnvs, stroke, r){
		var offset = {X: this.OffsetX, Y: this.OffsetY};
		var rotation = {X: Math.cos(this.r*Math.PI), X_: Math.cos((this.r+0.5)*Math.PI),
			            Y: Math.sin((this.r+0.5)*Math.PI), Y_: Math.sin((this.r)*Math.PI)};

		for(var i = 0; i < this.dotmatrix.length; i++){
			cnvs.beginPath();

			var block_state = 255;
			for(var j = 0; j < this.dotmatrix[i].blocks.length; j++){
				if(this.dotmatrix[i].blocks[j] < block_state){
					block_state = this.dotmatrix[i].blocks[i];
				}
			};

			stroke(block_state, 0);
			cnvs.fillStyle = cnvs.strokeStyle;

			cnvs.arc(offset.X+rotation.X*this.dotmatrix[i].x+rotation.Y_*this.dotmatrix[i].y,
				  offset.Y+rotation.Y*this.dotmatrix[i].y+rotation.X_*this.dotmatrix[i].x,
				  r, 0, 2*Math.PI, false);

			cnvs.fill();
			cnvs.stroke();
		};
	}

	draw_background(cnvs, stroke, options=undefined){
		var offset = {X: 0, Y: 0};
		var rotation = {X: 1, X_: 0, Y: 1, Y_: 0};

		if(options == undefined){
			offset = {X: this.OffsetX, Y: this.OffsetY};
			rotation = {X: Math.cos(this.r*Math.PI), X_: Math.cos((this.r+0.5)*Math.PI),
				            Y: Math.sin((this.r+0.5)*Math.PI), Y_: Math.sin((this.r)*Math.PI)};
		}
		else{
			if(options.X != undefined){
				offset.X = options.X;
			}
			if(options.Y != undefined){
				offset.Y = options.Y;
			}
			if(options.R != undefined){
				rotation = {X: Math.cos(options.R*Math.PI), X_: Math.cos((options.R+0.5)*Math.PI),
				            Y: Math.sin((options.R+0.5)*Math.PI), Y_: Math.sin((options.R)*Math.PI)};
			}
		}

		for(var i = 0; i < this.data.length; i++){
			cnvs.beginPath();
			this.data[i].draw_back(cnvs, stroke, 1, rotation, offset);
			cnvs.stroke();
		};
	}

	draw_foreground(cnvs, stroke, options=undefined){
		var offset = {X: 0, Y: 0};
		var rotation = {X: 1, X_: 0, Y: 1, Y_: 0};

		if(options == undefined){
			offset = {X: this.OffsetX, Y: this.OffsetY};
			rotation = {X: Math.cos(this.r*Math.PI), X_: Math.cos((this.r+0.5)*Math.PI),
				            Y: Math.sin((this.r+0.5)*Math.PI), Y_: Math.sin((this.r)*Math.PI)};
		}
		else{
			if(options.X != undefined){
				offset.X = options.X;
			}
			if(options.Y != undefined){
				offset.Y = options.Y;
			}
			if(options.R != undefined){
				rotation = {X: Math.cos(options.R*Math.PI), X_: Math.cos((options.R+0.5)*Math.PI),
				            Y: Math.sin((options.R+0.5)*Math.PI), Y_: Math.sin((options.R)*Math.PI)};
			}
		}

		for(var i = 0; i < this.data.length; i++){
			cnvs.beginPath();
			this.data[i].draw_fore(cnvs, stroke, rotation, offset);
			cnvs.stroke();
		};

		for(const connect of this.connections){
			cnvs.beginPath();
			cnvs.lineWidth = 10;
			cnvs.strokeStyle = "#0000ff";
			var point = rotated_point(connect.x, connect.y, this.r, offset.X, offset.Y);
			cnvs.moveTo(point.x, point.y);
			point = rotated_point(20, 0, this.r+connect.r, point.x, point.y);
			cnvs.lineTo(point.x, point.y);
			cnvs.stroke();
			cnvs.lineWidth = 6;
		}
	}
}

function load_module(id){
	loading_modules++;
	function callback(){
		loading_modules--;
		var height = 0;

		if(loading_modules != 0){
			return;
		}

		for(const key of Object.keys(modules)){
			modules[key].init({visible: true, OffsetX: 0, OffsetY: height, r: 0});
			height += modules[key].height + 100;
		}

		Canvas.calc_dotmatrix();
		Canvas.rescale(Canvas.dimensions.scale);
		Canvas.update_frame();
	}
	jQuery.ajax({
        url: "../configs/units/"+id+".js",
        dataType: 'script',
        success: callback,
        async: true
    });
};

function rotated_point(x, y, r, ofX=0, ofY=0){
	r_ = {X: Math.cos(r*Math.PI), X_: Math.cos((r+0.5)*Math.PI), Y: Math.sin((r+0.5)*Math.PI), Y_: Math.sin((r)*Math.PI)};
	return {x: ofX + r_.X*x + r_.Y_*y , y: ofY + r_.Y*y + r_.X_*x } ;
}

var loading_modules = 0;


events.add_init(function(){
	load_module(20);
	load_module(21);
	load_module(22);
	load_module(23);
	load_module(25);
	load_module(26);
});

function conf_modules(){
	modules[20].move({OffsetX:  50, OffsetY:  0, nonest: true});
	modules[21].move({OffsetX: 850, OffsetY: 440, r: 1, nonest: true});
	modules[22].move({OffsetX: 1650, OffsetY:  0, nonest: true});
	modules[23].move({OffsetX:  50, OffsetY: 440, r: 1, nonest: true});
	modules[25].move({OffsetX: 850, OffsetY:  0, nonest: true});
	modules[26].move({OffsetX: 1650, OffsetY: 440, r: 1, nonest: true});
	Canvas.calc_dotmatrix();
	Canvas.rescale(Canvas.dimensions.scale);
	Canvas.update_frame();
}

var modules = {};
