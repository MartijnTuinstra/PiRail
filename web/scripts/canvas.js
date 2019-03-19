
function settings(color, size){
	return '<svg version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" \
	 width="'+size+'px" height="'+size+'px" viewBox="0 0 438.529 438.529" style="display:block; cursor:pointer;"> \
	<path style="fill:'+color+'" d="M436.25,181.438c-1.529-2.002-3.524-3.193-5.995-3.571l-52.249-7.992c-2.854-9.137-6.756-18.461-11.704-27.98 \
		c3.422-4.758,8.559-11.466,15.41-20.129c6.851-8.661,11.703-14.987,14.561-18.986c1.523-2.094,2.279-4.281,2.279-6.567 \
		c0-2.663-0.66-4.755-1.998-6.28c-6.848-9.708-22.552-25.885-47.106-48.536c-2.275-1.903-4.661-2.854-7.132-2.854 \
		c-2.857,0-5.14,0.855-6.854,2.567l-40.539,30.549c-7.806-3.999-16.371-7.52-25.693-10.565l-7.994-52.529 \
		c-0.191-2.474-1.287-4.521-3.285-6.139C255.95,0.806,253.623,0,250.954,0h-63.38c-5.52,0-8.947,2.663-10.278,7.993 \
		c-2.475,9.513-5.236,27.214-8.28,53.1c-8.947,2.86-17.607,6.476-25.981,10.853l-39.399-30.549 \
		c-2.474-1.903-4.948-2.854-7.422-2.854c-4.187,0-13.179,6.804-26.979,20.413c-13.8,13.612-23.169,23.841-28.122,30.69 \
		c-1.714,2.474-2.568,4.664-2.568,6.567c0,2.286,0.95,4.57,2.853,6.851c12.751,15.42,22.936,28.549,30.55,39.403 \
		c-4.759,8.754-8.47,17.511-11.132,26.265l-53.105,7.992c-2.093,0.382-3.9,1.621-5.424,3.715C0.76,182.531,0,184.722,0,187.002 \
		v63.383c0,2.478,0.76,4.709,2.284,6.708c1.524,1.998,3.521,3.195,5.996,3.572l52.25,7.71c2.663,9.325,6.564,18.743,11.704,28.257 \
		c-3.424,4.761-8.563,11.468-15.415,20.129c-6.851,8.665-11.709,14.989-14.561,18.986c-1.525,2.102-2.285,4.285-2.285,6.57 \
		c0,2.471,0.666,4.658,1.997,6.561c7.423,10.284,23.125,26.272,47.109,47.969c2.095,2.094,4.475,3.138,7.137,3.138 \
		c2.857,0,5.236-0.852,7.138-2.563l40.259-30.553c7.808,3.997,16.371,7.519,25.697,10.568l7.993,52.529 \
		c0.193,2.471,1.287,4.518,3.283,6.14c1.997,1.622,4.331,2.423,6.995,2.423h63.38c5.53,0,8.952-2.662,10.287-7.994 \
		c2.471-9.514,5.229-27.213,8.274-53.098c8.946-2.858,17.607-6.476,25.981-10.855l39.402,30.84c2.663,1.712,5.141,2.563,7.42,2.563 \
		c4.186,0,13.131-6.752,26.833-20.27c13.709-13.511,23.13-23.79,28.264-30.837c1.711-1.902,2.569-4.09,2.569-6.561 \
		c0-2.478-0.947-4.862-2.857-7.139c-13.698-16.754-23.883-29.882-30.546-39.402c3.806-7.043,7.519-15.701,11.136-25.98l52.817-7.988 \
		c2.279-0.383,4.189-1.622,5.708-3.716c1.523-2.098,2.279-4.288,2.279-6.571v-63.376 \
		C438.533,185.671,437.777,183.438,436.25,181.438z M270.946,270.939c-14.271,14.277-31.497,21.416-51.676,21.416 \
		c-20.177,0-37.401-7.139-51.678-21.416c-14.272-14.271-21.411-31.498-21.411-51.673c0-20.177,7.135-37.401,21.411-51.678 \
		c14.277-14.272,31.504-21.411,51.678-21.411c20.179,0,37.406,7.139,51.676,21.411c14.274,14.277,21.413,31.501,21.413,51.678 \
		C292.359,239.441,285.221,256.669,270.946,270.939z"/></svg>';

}

function round(number, decimals){
	return Math.round(number * Math.pow(10, decimals))/Math.pow(10, decimals);
}


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

		this.canvas.height = $('#LayoutContainer').height();
		this.canvas.width  = $('#LayoutContainer').width();

	    this.dimensions.height = this.canvas.height;
		this.dimensions.width  = this.canvas.width;

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

		for(i in modules){
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

	setStrokeColor: function(b, s = 0, cnvs){

		if(b == 0){ // Blocked
			(s == 0)?cnvs.strokeStyle = "#900":cnvs.strokeStyle = "#daa";
	   	}
	   	else if(b == 1){ // DANGER
	    	(s == 0)?cnvs.strokeStyle = "#f00":cnvs.strokeStyle = "#fcc";
	    }
	   	else if(b == 2){ // RESTRICTED
	    	(s == 0)?cnvs.strokeStyle = "#f40":cnvs.strokeStyle = "#fcc";
	    }
	    else if(b == 3){ // CAUTION
	    	(s == 0)?cnvs.strokeStyle = "#f70":cnvs.strokeStyle = "#fe8";
	    }
	    else if(b == 4){ // PROCEED
	    	(s == 0)?cnvs.strokeStyle = "#aaa":cnvs.strokeStyle = "#ddd";
	    }
	    else if(b == 5){ // RESERVED
	    	(s == 0)?cnvs.strokeStyle = "#aaf":cnvs.strokeStyle = "#ddf";
	    }
	    else if(b == 6){ // RESERVED_SWICTH
	    	(s == 0)?cnvs.strokeStyle = "#aaa":cnvs.strokeStyle = "#fcc";
	    }
	    else{ //Unknown
	    	(s == 0)?cnvs.strokeStyle = "#ddd":cnvs.strokeStyle = "#eee";
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
	constructor(module, data){//module_id, block, x1, y1, x2, y2, options){
		this.type = "line";
		this.edit_type = "line";
		this.module_id = module.id;
		this.m = module;
		this.b = data.b;
		this.x1 = data.x1;
		this.y1 = data.y1;
		this.x2 = data.x2;
		this.y2 = data.y2;

		if (data.options == undefined) { data.options = {}; }

		if (data.options.if != undefined){
			this.if = data.options.if;
		}
	}

	init(){
		return;
	}

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>Line</td><td>"+round(this.x1, 3)+
		       ", "+round(this.y1, 3)+"</td><td>"+round(this.x2, 3)+", "+round(this.y2, 3)+"</td><td>-</td><td>-</td><td>"+settings("#ccc", 23)+"</td></tr>";
	}

	dotmatrix(){
		var coords = [{x: this.x1, y: this.y1}, {x: this.x2, y: this.y2}];
		return [coords, this.b]
	}

	draw_fore(cnvs, stroke, rotation, offset){
		if(this.m == undefined){
			this.update_module();
		}
		stroke(this.m.blocks[this.b], 0, cnvs);
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
		stroke(this.m.blocks[this.b], color, cnvs);

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
	constructor(module, data){
		this.type = "arc";
		this.edit_type = "arc";
		this.module_id = module.id;
		this.m = module;
		this.b = data.b;
		this.cx = data.cx;
		this.cy = data.cy;
		this.r = data.r;
		this.start = data.s[0];
		this.end = data.s[1];
		this.cw = data.s[2];

		if (data.options == undefined) { data.options = {}; }

		if (data.options.if != undefined){
			this.if = data.options.if;
		}
	}

	init(){
		return;
	}

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>Arc</td><td>"+round(this.cx, 3)+
		       ", "+round(this.cy, 3)+"</td><td>"+round(this.r, 3)+"</td><td>"+this.start+" - "+this.end+", "+this.cw+", "+"</td><td>-</td><td>"+settings("#ccc", 23)+"</td></tr>";
	}

	dotmatrix(){
		var coords = [{x: this.cx+Math.cos(this.start*Math.PI)*this.r, y: this.cy+Math.sin(this.start*Math.PI)*this.r}, 
					  {x: this.cx+Math.cos(this.end * Math.PI)*this.r, y: this.cy+Math.sin(this.end * Math.PI)*this.r}];
		return [coords, this.b]
	}

	draw_back(cnvs, stroke, color, rotation, offset){
		stroke(this.m.blocks[this.b], color, cnvs);
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
		stroke(this.m.blocks[this.b], 0, cnvs);
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
	constructor(module, data){ //, block, side, sw, x, y, r, options){
		this.type = data.type;
		this.edit_type = "sw";
		this.module_id = module.id;
		this.m = module;
		this.b = data.b;
		this.s = data.sw;
		this.x = data.x;
		this.y = data.y;
		this.r = data.r % 2;

		if (data.options == undefined) { data.options = {}; }

		if (data.options.if != undefined){
			this.if = data.options.if;
		}
	}

	init(){
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

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>"+this.type+"</td><td>"+round(this.x, 3)+
		       ", "+round(this.y, 3)+"</td><td>-</td><td>"+this.r+"</td><td>"+this.s+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
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

	draw_back(cnvs, stroke, color, rotation, offset){
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
				this.draw(cnvs, stroke, x, y, r, this.type, "B",this.m.blocks[this.b],this.m.switches[this.s]);
				cnvs.stroke();
				return true
			}
			this.draw(cnvs, stroke, x, y, r, this.type, "G",this.m.blocks[this.b],this.m.switches[this.s]);
		}
		else{
			this.draw(cnvs, stroke, x, y, r, this.type, "B",this.m.blocks[this.b],this.m.switches[this.s]);
		}
	}

	draw_fore(cnvs, stroke, rotation, offset){
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

		this.draw(cnvs, stroke, x, y, r, this.type, "F",
			        this.m.blocks[this.b],
			        this.m.switches[this.s]);
	}
}

class canvas_switch_l extends canvas_switch{
	constructor(module, data){
		super(module, data);
	}

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

	draw(cnvs, stroke, x,y,r,lr,type,bl,sw){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = Math.sin((r)*Math.PI);
		if((sw == 0 && type == "F") || (sw == 1 && type == "B") || type == "G"){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			cnvs.moveTo(x,y);
			if((r + 0.25) % 0.5 == 0){
				cnvs.lineTo(x+rvX*(ro[0].x-ds),y+rvY_*(ro[0].x-ds));
			}else if(r % 0.5 == 0){
				cnvs.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
			}
		}
		if((sw == 1 && type == "F") || (sw == 0 && type == "B") || type == "G"){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			var tx = x - rvX_ * radia[0];
			var ty = y - rvY * radia[0];
			cnvs.arc(tx, ty, radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
		}
	}
}

class canvas_switch_r extends canvas_switch{
	constructor(module, data){
		super(module, data);
	}
	
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

	draw(cnvs, stroke, x,y,r,lr,type,bl,sw){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = Math.sin((r)*Math.PI);
		if((sw == 0 && type == "F") || (sw == 1 && type == "B") || type == "G"){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			cnvs.moveTo(x,y);
			if((r + 0.25) % 0.5 == 0){
				cnvs.lineTo(x+rvX*(ro[0].x-ds),y+rvY_*(ro[0].x-ds));
			}else if(r % 0.5 == 0){
				cnvs.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
			}
		}
		if((sw == 1 && type == "F") || (sw == 0 && type == "B") || type == "G"){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			var tx = x + rvX_ * radia[0];
			var ty = y + rvY * radia[0];
			cnvs.arc(tx, ty, radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);
		}
	}
}

class canvas_double_slip {
	constructor(module, data){//block, side, sw, x, y, r, options){
		this.type = data.type;
		this.edit_type = "ds";
		this.module_id = module.id;
		this.m = module;
		this.b = data.b;
		if(data.sw == undefined){
			this.sA = undefined; this.sB = undefined;
		}
		else{
			this.sA = data.sw[0];
			this.sB = data.sw[1];
		}
		this.x = data.x;
		this.y = data.y;
		this.r = data.r;

		if (data.options == undefined) { data.options = {}; }

		if (data.options.if != undefined){
			this.if = data.options.if;
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

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>"+this.type+"</td><td>"+round(this.x, 3)+
		       ", "+round(this.y, 3)+"</td><td>-</td><td>"+this.r+"</td><td>"+this.sA+", "+this.sB+"</td><td>"+settings("#ccc", 23)+"</td></tr>";
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

	draw_back(cnvs, stroke, color, rotation, offset){
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
				this.draw(cnvs, stroke, x, y, r, "B",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
				cnvs.stroke();
				return true
			}
			this.draw(cnvs, stroke, x, y, r, "G",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
		}
		else{
			this.draw(cnvs, stroke, x, y, r, "B",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
		}
	}

	draw_fore(cnvs, stroke, rotation, offset){
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

		this.draw(cnvs, stroke, x, y, r, "F",this.m.blocks[this.b], this.m.switches[this.sA], this.m.switches[this.sB]);
	}
}

class canvas_dslip extends canvas_double_slip {
	constructor(module, data){
		super(module, data);
	}
	
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

	draw(cnvs, stroke, x,y,r,type,bl,swA,swB){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = -Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = -Math.sin((r)*Math.PI);
		var tx, ty, dis_X, dis_Y;

		if((swA == 1 && swB == 0 && type == "F") || ((swA == 0 || swB == 1) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			tx = x - rvX*(ds - ro[0].x) - rvY_*radia[0];
			ty = y - rvX_*(ds - ro[0].x) - rvY*radia[0];
			
			cnvs.arc(tx, ty,radia[0],Math.PI*(r+0.75),Math.PI*(r+0.50),true);

			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 1 && swB == 1 && type == "F") || ((swA == 0 || swB == 0) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			dis_X = ro[0].x+ds;
			dis_Y = 2 * ro[0].y;

			tx = x - rvX*ds - rvY_*ro[0].y;
			ty = y - rvX_*ds - rvY*ro[0].y;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}

		if((swA == 0 && swB == 0 && type == "F") || ((swA == 1 || swB == 1) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			tx = x;
			ty = y;
			dis_X = ro[0].x - ds;
			dis_Y = 0;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 0 && swB == 1 && type == "F") || ((swA == 1 || swB == 0) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			tx = x+rvY_*radia[0];
			ty = y+rvY*radia[0];

			cnvs.arc(tx,ty,radia[0],Math.PI*(r-0.25),Math.PI*(r-0.5),true);

			cnvs.stroke();cnvs.beginPath();
		}
	}
}

class canvas_fl_dslip extends canvas_double_slip {
	constructor(module, data){
		super(module, data);
	}
	
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

	draw(cnvs, stroke, x,y,r,type,bl,swA,swB){
		if(r >  1){r -= 2;}
		if(r < -1){r += 2;}

		var rvX = Math.cos(r*Math.PI);
		var rvX_ = -Math.cos((r+0.5)*Math.PI);
		var rvY = Math.sin((r+0.5)*Math.PI);
		var rvY_ = -Math.sin((r)*Math.PI);
		var tx, ty, dis_X, dis_Y;

		if((swA == 1 && swB == 0 && type == "F") || ((swA == 0 || swB == 1) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			tx = x - rvY_*radia[0];
			ty = y - rvY*radia[0];
			
			cnvs.arc(tx, ty,radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);

			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 1 && swB == 1 && type == "F") || ((swA == 0 || swB == 0) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			dis_X = ro[0].x+ds;
			dis_Y = -2 * ro[0].y;

			tx = x - rvX*ds + rvY_*ro[0].y;
			ty = y - rvX_*ds + rvY*ro[0].y;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}

		if((swA == 0 && swB == 0 && type == "F") || ((swA == 1 || swB == 1) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			tx = x;
			ty = y;
			dis_X = ro[0].x - ds;
			dis_Y = 0;

			cnvs.moveTo(tx, ty);
			cnvs.lineTo(tx+rvX*dis_X+rvY_*dis_Y, ty+rvX_*dis_X+rvY*dis_Y);
			cnvs.stroke();cnvs.beginPath();
		}
		if((swA == 0 && swB == 1 && type == "F") || ((swA == 1 || swB == 0) && type == "B")){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			tx = x - rvX*(ds-ro[0].x)+rvY_*radia[0];
			ty = y - rvX_*(ds-ro[0].x)+rvY*radia[0];

			cnvs.arc(tx,ty,radia[0],Math.PI*(r-0.5),Math.PI*(r-0.75),true);

			cnvs.stroke();cnvs.beginPath();
		}
	}
}

class canvas_module {
	constructor(data){
		this.id = data.id;
		this.name = data.name;
		this.OffsetX = 0;
		this.OffsetY = 0;
		this.width = data.dim.w;
		this.height = data.dim.h;
		this.r = 0;
		this.visible = false;
		this.connections = data.anchors;
		this.connection_link = [];
		
		for(var i = 0; i < this.connections.length; i++){
			this.connections[i].connected = false;
		}

		this.blocks = Array.apply(null, Array(data.blocks)).map(function (){return 7});
		this.switches = Array.apply(null, Array(data.switches)).map(function (){return 0});

		this.dotmatrix = [];
		this.data = [];
		this.hitboxes = [];

		for(var i =0; i < data.layout.length; i++){
			switch(data.layout[i].type){
				case "line":
					this.data.push(new canvas_line(this, data.layout[i]));
					break;
				case "arc":
					this.data.push(new canvas_arc(this, data.layout[i]));
					break;
				case "swl":
					this.data.push(new canvas_switch_l(this, data.layout[i]));
					break;
				case "swr":
					this.data.push(new canvas_switch_r(this, data.layout[i]));
					break;
				case "ds":
					this.data.push(new canvas_dslip(this, data.layout[i]));
					break;
				case "dsf":
					this.data.push(new canvas_fl_dslip(this, data.layout[i]));
					break;
			}
		}
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

	configdata(){
		var data = "";
		for(var i = 0; i < this.data.length; i++){
			data += this.data[i].configdata(i);
		};

		return data;
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

			stroke(block_state, 0, cnvs);
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
	console.log("Load module "+id);
	// function callback(a, b, c){
		
	// }
	jQuery.ajax({
        url: "../configs/units/"+id+".txt",
        // dataType: 'plain',
        success: function(data, b,c){
        	loading_modules--;
			var height = 0;

			function JsonParse(obj){
			    return Function('"use strict";return (' + obj + ')')();
			};

			var newdata = JsonParse(data);
			
			modules[newdata.id] = new canvas_module(newdata);

			// function customEval(obj){
			// 	return Function('"use strict";return ('+obj+')')();
			// }

			// console.log(customEval(a));

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
        },
        error: function(){
        	console.warn("Error");
        },
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
var modules_data = {}
