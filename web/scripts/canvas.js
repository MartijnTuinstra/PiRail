
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

		$('.btn-layout-zoom-in').on("click", function(){
			Canvas.rescale(Canvas.dimensions.scale * 1.2);
		});
		$('.btn-layout-zoom-out').on("click", function(){
			Canvas.rescale(Canvas.dimensions.scale / 1.2);
		});

		$('.btn-layout-rotate-ccw').on("click", function(){
			var m = modules[Object.keys(modules)[0]];
			m.move({r: m.r + 0.25});
			Canvas.get_content_box();
			Canvas.center();
			Canvas.update_frame();
		});
		$('.btn-layout-rotate-cw').on("click", function(){
			var m = modules[Object.keys(modules)[0]];
			m.move({r: m.r - 0.25});
			Canvas.get_content_box();
			Canvas.center();
			Canvas.update_frame();
		});
	},

	get_content_box: function(){

		var l = 1e100;
		var r = 0;
		var t = 1e100;
		var b = 0;

		var point = [];

		for(i in modules){
			if(!modules[i].visible){
				continue;
			}

			point[0] = rotated_point(modules[i].OffsetX, modules[i].OffsetY, 0);
			point[1] = rotated_point(modules[i].width, 0, modules[i].r, modules[i].OffsetX, modules[i].OffsetY);
			point[2] = rotated_point(modules[i].width, modules[i].height, modules[i].r, modules[i].OffsetX, modules[i].OffsetY);
			point[3] = rotated_point(0, modules[i].height, modules[i].r, modules[i].OffsetX, modules[i].OffsetY);

			for(var i = 0; i < 4; i++){
				if(point[i].x < l){
					l = point[i].x - 100;
				}
				if(point[i].x > r){
					r = point[i].x + 100;
				}
				if(point[i].y < t){
					t = point[i].y - 100;
				}
				if(point[i].y > b){
					b = point[i].y + 100;
				}
			}
		}

		this.dimensions.content.width = (r - l)*this.dimensions.scale;
		this.dimensions.content.height = (b - t)*this.dimensions.scale;
		this.dimensions.content.ofX = l*this.dimensions.scale;
		this.dimensions.content.ofY = t*this.dimensions.scale;

		// console.log("content_box", [r-l, b-t])
		this.calc_limits();
	},
	resize: function(){
		// console.log("resize");

	    this.dimensions.height = (this.canvas.height = $('#LayoutContainer').height());
		this.dimensions.width  = (this.canvas.width  = $('#LayoutContainer').width());
		
		//Recenter content
		this.center();

		this.get_content_box();
		this.calc_limits();

		this.update_frame();
	},
	center: function(){
		this.dimensions.ofX = 0.5*this.dimensions.width-0.5*this.dimensions.content.width;
		this.dimensions.ofY = 0;
	},
	rescale: function(new_scale){
		// console.log("rescale");
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

		
		var _color_ = [
			[[153,0,0],     [255,0,0],     [255,68,0],    [255,119,0],   [170,170,170], [170,170,255], [170,170,255], [221,221,221]],
			[[221,170,170], [255,204,204], [255,204,204], [255,238,136], [221,221,221], [221,221,255], [255,204,204], [221,221,221]]
		];


		if(b > 7)
			b = 7;
		else if(b == undefined)
			b = 7;

		cnvs.strokeStyle = `rgb(${_color_[s][b][0]}, ${_color_[s][b][1]}, ${_color_[s][b][2]})`;
		// cnvs.strokeStyle = color;

		// if(b == 0){ // Blocked
		// 	(s == 0)?cnvs.strokeStyle = "#900":cnvs.strokeStyle = "#daa";
	   	// }
	   	// else if(b == 1){ // DANGER
	    // 	(s == 0)?cnvs.strokeStyle = "#f00":cnvs.strokeStyle = "#fcc";
	    // }
	   	// else if(b == 2){ // RESTRICTED
	    // 	(s == 0)?cnvs.strokeStyle = "#f40":cnvs.strokeStyle = "#fcc";
	    // }
	    // else if(b == 3){ // CAUTION
	    // 	(s == 0)?cnvs.strokeStyle = "#f70":cnvs.strokeStyle = "#fe8";
	    // }
	    // else if(b == 4){ // PROCEED
	    // 	(s == 0)?cnvs.strokeStyle = "#aaa":cnvs.strokeStyle = "#ddd";
	    // }
	    // else if(b == 5){ // RESERVED
	    // 	(s == 0)?cnvs.strokeStyle = "#aaf":cnvs.strokeStyle = "#ddf";
	    // }
	    // else if(b == 6){ // RESERVED_SWICTH
	    // 	(s == 0)?cnvs.strokeStyle = "#aaf":cnvs.strokeStyle = "#fcc";
	    // }
	    // else{ //Unknown
	    // 	(s == 0)?cnvs.strokeStyle = "#ddd":cnvs.strokeStyle = "#ddd";
	    // }
	},

    update_frame: function(){
    	this.c.setTransform(1, 0, 0, 1, 0, 0);
		this.c.clearRect(-10, -10, 10 + this.dimensions.width, 10 + this.dimensions.height);
		var tmp_this = this;

		this.c.setTransform(1, 0, 0, 1, this.dimensions.ofX, this.dimensions.ofY);

		this.c.beginPath();
		this.c.lineWidth = "6";
		this.c.rect(0, 0,this.dimensions.content.width, this.dimensions.content.height);

		this.c.stroke();

		this.c.setTransform(this.dimensions.scale, 0, 0, this.dimensions.scale, this.dimensions.ofX-this.dimensions.content.ofX, this.dimensions.ofY-this.dimensions.content.ofY);


		for (var w = this.dimensions.content.ofX; w < (this.dimensions.content.ofX + this.dimensions.content.width/this.dimensions.scale); w += 100) {
		    for (var h = this.dimensions.content.ofY; h < (this.dimensions.content.ofY + this.dimensions.content.height/this.dimensions.scale); h += 100) {
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

		
		if(this.fitlist != undefined){
			for(var i = 0; i < this.fitlist.length; i++){
		        this.c.strokeStyle = "#0000ff";
		        this.c.lineWidth = 0;
				this.c.fillStyle = "#0000ff";
				this.c.beginPath();

				this.c.arc(this.fitlist[i].originX, this.fitlist[i].originY, 1, 0, 2*Math.PI, false);

				this.c.fill();
				this.c.stroke();
				this.c.fillText(i, this.fitlist[i].originX, this.fitlist[i].originY+10);
				this.c.beginPath();

				this.c.arc(this.fitlist[i].originX+this.fitlist[i].w, this.fitlist[i].originY+this.fitlist[i].h, 1, 0, 2*Math.PI, false);

				this.c.fill();
				this.c.stroke();
				this.c.fillText(i, this.fitlist[i].originX+this.fitlist[i].w, this.fitlist[i].originY+this.fitlist[i].h+20);
		        this.c.strokeStyle = "#00ffff";
		        this.c.lineWidth = 0;
				this.c.fillStyle = "#00ffff";
				this.c.beginPath();

				this.c.arc(this.fitlist[i].originX+this.fitlist[i].dx, this.fitlist[i].originY+this.fitlist[i].dy, 2, 0, 2*Math.PI, false);

				this.c.fill();
				this.c.stroke();
			}
		}

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
			c.lineWidth = 0;

			c.stokeStyle = "#00FF00";
			c.fillStyle = "#00ff00";

			c.beginPath();

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

		click_x = (x - tmp_this.dimensions.ofX + tmp_this.dimensions.content.ofX)/tmp_this.dimensions.scale;
		click_y = (y - tmp_this.dimensions.ofY + tmp_this.dimensions.content.ofY)/tmp_this.dimensions.scale;

		// console.log("xy", [click_x, click_y]);

		$.each(modules,function(module,module_v){
			if(!module_v.visible){
				return true;
			}

			var ofX = module_v.OffsetX;
			var ofY = module_v.OffsetY;

			_x = click_x - ofX;
			_y = click_y - ofY;

			_l = Math.sqrt(Math.pow(_x,2)+Math.pow(_y,2));
			_r = Math.atan2(_y, _x);
			_r += module_v.r*Math.PI;
			_x = _l*Math.cos(_r);
			_y = _l*Math.sin(_r);

			if(_x < 0 || _y < 0 || _x > module_v.width || _y > module_v.height){
				return true;
			}

			for(var i = 0; i < module_v.hitboxes.length; i++){
				if(module_v.hitboxes[i] && {}.toString.call(module_v.hitboxes[i]) === '[object Function]'){
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
		Canvas.touchStart.x = (Canvas.touchMove.x = evt.pageX);
		Canvas.touchStart.y = (Canvas.touchMove.y = evt.pageY);
		Canvas.touchMoved = false;
		Canvas.Longclick = false;
		Canvas.LongclickTimer = setTimeout(function(){Canvas.longClick(evt)},600);
		Canvas.canvas_jquery.on("mousemove",Canvas.evt_drag);
	},
	evt_mouseup: function(evt){
		clearTimeout(Canvas.LongclickTimer);
		if(!Canvas.touchClicked && !Canvas.Longclick)
			Canvas.click(evt.offsetX,evt.offsetY);
		else
			Canvas.touchClicked = false;
		
		Canvas.canvas_jquery.off("mousemove");
	},


	evt_drag: function(evt){
		if(Canvas.touchMoved == true){
			if(Canvas.dimensions.width < Canvas.dimensions.content.width){
				Canvas.dimensions.ofX += (evt.pageX - Canvas.touchMove.x)/Canvas.dimensions.scale;
			}
			if(Canvas.dimensions.height < Canvas.dimensions.content.height){
				Canvas.dimensions.ofY += (evt.pageY - Canvas.touchMove.y)/Canvas.dimensions.scale;
			}

			Canvas.moved()

			Canvas.touchMove.x = evt.pageX;
			Canvas.touchMove.y = evt.pageY;

			return false;
		}else{
			if(Math.sqrt(Math.pow(Canvas.touchStart.x - evt.pageX,2)+Math.pow(Canvas.touchStart.y - evt.pageY,2)) >= 10){
				Canvas.touchMoved = true;
				clearTimeout(Canvas.LongclickTimer);
				console.log("Mouse drag Moved");
			}
		}
	},

	touchStart: {x:0,y:0},
	touchMove: {x:0,y:0},
	touchMoved: false,
	touchClicked: false,
	evt_drag_start: function(evt){
		Canvas.touchStart.x = (Canvas.touchMove.x = evt.touches[0].pageX);
		Canvas.touchStart.y = (Canvas.touchMove.y = evt.touches[0].pageY);
		Canvas.touchMoved = false;
		Canvas.Longclick = false;
		Canvas.LongclickTimer = setTimeout(function(){Canvas.longClick(evt)},600);
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
	},

	fit: function(){
		var reference = {w:0, h:0};
		var blocks = [];
		var m = [];

		function fit_func(module, prev, depth, origin, blocks, blockid, module_list){
			for(var i = 0; i < module.connection_link.length; i++){
				if(module.connection_link[i] == undefined)
					continue;

				if(!module.connections[i].connected)
					continue;

				if(module.connection_link[i] == prev || module.connection_link[i] == origin.id)
					continue;

				// Neighbournode
				var m = modules[module.connection_link[i]];

				if(module_list.includes(m.id))
					continue;

				module_list.push(m.id);
				blocks[blockid].m.push(m.id);

				// rotated_point(x, y, r, ofX=0, ofY=0)
				pos = rotated_point(m.width, m.height, m.r, m.OffsetX, m.OffsetY);

				if(m.OffsetX < blocks[blockid].originX){
					blocks[blockid].dx += blocks[blockid].originX - m.OffsetX;
					blocks[blockid].w += blocks[blockid].originX - m.OffsetX;
					blocks[blockid].originX = m.OffsetX;
				}
				else if(m.OffsetX > blocks[blockid].originX + blocks[blockid].w){
					blocks[blockid].w = m.OffsetX - blocks[blockid].originX;
				}
				if(m.OffsetY < blocks[blockid].originY){
					blocks[blockid].dy += blocks[blockid].originY - m.OffsetY;
					blocks[blockid].h += blocks[blockid].originY - m.OffsetY;
					blocks[blockid].originY = m.OffsetY;
				}
				else if(m.OffsetY > blocks[blockid].originY + blocks[blockid].h){
					blocks[blockid].h = m.OffsetY - blocks[blockid].originY;
				}

				if(pos.y > blocks[blockid].originY + blocks[blockid].h){
					blocks[blockid].h = pos.y - blocks[blockid].originY;
				}
				else if(pos.y < blocks[blockid].originY){
					blocks[blockid].dy += blocks[blockid].originY - pos.y;
					blocks[blockid].h += blocks[blockid].originY - pos.y;
					blocks[blockid].originY = pos.y;
				}
				if(pos.x > blocks[blockid].originX + blocks[blockid].w){
					blocks[blockid].w = pos.x - blocks[blockid].originX;
				}
				else if(pos.x < blocks[blockid].originX){
					blocks[blockid].dx += blocks[blockid].originX - pos.x;
					blocks[blockid].w += blocks[blockid].originX - pos.x;
					blocks[blockid].originX = pos.x;
				}

				// Calculate new offset point
				fit_func(m, module.id, depth+1, origin, blocks, blockid, module_list);
			}
		}

		var module_keys = Object.keys(modules);
		for(var i = 0; i < module_keys.length; i++){
			var this_module = modules[module_keys[i]];

			if(!this_module.visible){
				continue;
			}

			if(!m.includes(parseInt(module_keys[i]))){
				m.push(parseInt(module_keys[i]));
				var block_id = blocks.length;
				var obj = {};
				pos = rotated_point(this_module.width, this_module.height, this_module.r, this_module.OffsetX, this_module.OffsetY);
				pos = {x: round(pos.x, 2), y: round(pos.y, 2)};

				obj.dx = 0;
				obj.dy = 0;

				if(pos.x < this_module.OffsetX){
					obj.originX = pos.x;
					obj.w = this_module.OffsetX - pos.x;
					obj.dx += obj.w;
				}else{
					obj.originX = this_module.OffsetX;
					obj.w = pos.x - this_module.OffsetX;
				}
				if(pos.y < this_module.OffsetY){
					obj.originY = pos.y;
					obj.h = this_module.OffsetY - pos.y;
					obj.dy += obj.h;
				}else{
					obj.originY = this_module.OffsetY;
					obj.h = pos.y - this_module.OffsetY;
				}

				obj.m = [parseInt(module_keys[i])];
				obj.widest = 0;

				blocks.push(obj);

				fit_func(this_module, undefined, 0, this_module, blocks, block_id, m);

				blocks[block_id].originX -= 50;
				blocks[block_id].originY -= 50;
				blocks[block_id].w += 100;
				blocks[block_id].h += 100;
				blocks[block_id].dx += 50;
				blocks[block_id].dy += 50;
			}
		}
		this.fitlist = blocks;
	},
	fitOptimize: function(){
		var fitObj = {width: this.canvas.width, height: 0, y: 0, spaces: []};

		function findWidest(){
			var w = 0;
			var x = -1;
			for(var n = 0; n < Canvas.fitlist.length; n++){
				if(Canvas.fitlist[n].widest == 0 && Canvas.fitlist[n].w > w){
					w = Canvas.fitlist[n].w;
					x = n;
				}
			}

			Canvas.fitlist[x].widest = 1;
			return x;
		}

		function findRoom(obj, block){
			for(var i = 0; i < obj.spaces.length; i++){
				if(obj.spaces[i].w > block.w){
					var pos = {x: obj.spaces[i].x, y: obj.spaces[i].y};
					obj.spaces[i].x += block.w;
					obj.spaces[i].w -= block.w;
					return pos;
				}
			}

			var y = obj.height;
			obj.height += block.h;
			obj.y = obj.height;

			if(block.w < obj.width)
				obj.spaces.push({x: block.w, w: obj.width - block.w, y: y, h: obj.height - y});

			return {x: 0, y: y};
		}

		for(var i = 0; i < this.fitlist.length; i++){
			var n = findWidest();
			if (n == -1)
				continue;
			var pos = findRoom(fitObj, this.fitlist[n]);

			console.log("Move "+n + " to "+pos.x+", "+pos.y);

			this.fitlist[n].originX = pos.x;
			this.fitlist[n].originY = pos.y;

			modules[this.fitlist[n].m[0]].move({OffsetX: pos.x+this.fitlist[n].dx, OffsetY: pos.y+this.fitlist[n].dy});
		}
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

//Dotmatrix dot
class dot {
	constructor(x, y, block){
		this.x = Math.round(x);
		this.y = Math.round(y);
		this.block = [block];
	}

	equals(dot){
		return (Math.round(dot.x)==this.x && Math.round(dot.y)==this.y);
	}

	toString(){
		return this.x + ", " + this.y + "; "+this.block.toString();
	}

	combine(dot){
		for(var i = 0; i < dot.block.length; i++){
			if(this.block.indexOf(dot.block[i]) < 0){
				this.block.push(dot.block[i]);
			}
		}
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
		this.m.add_dot(new dot(this.x1, this.y1, this.b));
		this.m.add_dot(new dot(this.x2, this.y2, this.b));
	}

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>Line</td><td>"+round(this.x1, 3)+
		       ", "+round(this.y1, 3)+"</td><td>"+round(this.x2, 3)+", "+round(this.y2, 3)+"</td><td>-</td><td>-</td><td>"+settings("#ccc", 23)+"</td></tr>";
	}

	draw_fore(cnvs, stroke, rotation, offset){
		if(this.m == undefined){
			this.update_module();
		}
		stroke(this.m.blocks[this.b], 0, cnvs);
		if(this.if != undefined){
			for (var i = this.if.length - 1; i >= 0; i--) {
				if(this.if[i].sw != undefined && this.m.switches[this.if[i].sw] != this.if[i].st)
					return true; // Print on background
				if(this.if[i].mssw != undefined && this.m.msswitches[this.if[i].mssw] != this.if[i].st)
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
				if(this.if[i].sw != undefined && this.m.switches[this.if[i].sw] == this.if[i].st)
					counter++;
				if(this.if[i].mssw != undefined && this.m.msswitches[this.if[i].mssw] == this.if[i].st)
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
		this.m.add_dot(new dot(this.cx+Math.cos(this.start*Math.PI)*this.r, this.cy+Math.sin(this.start*Math.PI)*this.r, this.b));
		this.m.add_dot(new dot(this.cx+Math.cos(this.end * Math.PI)*this.r, this.cy+Math.sin(this.end * Math.PI)*this.r, this.b));
	}

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>Arc</td><td>"+round(this.cx, 3)+
		       ", "+round(this.cy, 3)+"</td><td>"+round(this.r, 3)+"</td><td>"+this.start+" - "+this.end+", "+this.cw+", "+"</td><td>-</td><td>"+settings("#ccc", 23)+"</td></tr>";
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
		this.add_dots();
		this.m.add_hitbox(this.hit.bind(this));

		this.hit_eqn = [];

		//Calculate hitboxes
		//arc hit box
		// (x-a)^2+(y-b)^2 >= r^2
		// x^2-2ax+a^2+y^2-2b^2+b^2 = r^2
		// x^2 - 2ax + y^2 - 2b^2 = r^2 - a^2 - b^2
		this.hit_eqn.push({x:[1], y:[], op:"gt", v:0});  // x > 0
		if((this.r % 0.5) == 0){
			this.hit_eqn.push({x:[1], y:[], op:"lt", v:ro[0].x}); // x < ro[0].x
		}
		else{
			this.hit_eqn.push({x:[1], y:[], op:"lt", v:(ro[0].x+ds)/Math.cos(Math.PI/4)}) // x < (ro[0].x+ds)/cos(pi/4)
		}
		var a = 0;
		if(this.type == "swr"){
			this.hit_eqn.push({x:[], y:[1], op:"gt", v:-hit_radius});         // y > -hitradius
			this.hit_eqn.push({x:[1], y:[1], op:"lt", v:(ro[0].x+ro[0].y)});  // x + y < (ro[0].x+ro[0].y)
			var b = radia[0];
		}
		else{
			this.hit_eqn.push({x:[], y:[1], op:"lt", v:hit_radius});
			this.hit_eqn.push({x:[1], y:[-1], op:"lt", v:(ro[0].x+ro[0].y)});
			var b = -radia[0];
		}
		this.hit_eqn.push({x:[-2*a, 1], y:[-2*b, 1], v:(Math.pow(radia[0]-hit_radius, 2)-(a*a)-(b*b)), op:"gt"}) // -2ax^2+x -2by^2+y > (r-hr)^2
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
		if(this.hit_eval(x,y)){
			throw_Switch(this.module_id,this.s);
		}
	}

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

	add_dots(){
		this.m.add_dot(new dot(this.x, this.y, this.b));

		var pointA; // Line
		//Arc
		var pointB = rotated_point(ro[0].x, -ro[0].y, -this.r, this.x, this.y);
		if((this.r + 0.25) % 0.5 == 0){
			pointA = rotated_point(ro[0].x-ds, 0, -this.r, this.x, this.y);
		}else if(this.r % 0.5 == 0){
			pointA = rotated_point(ro[0].x, 0, -this.r, this.x, this.y);
		}

		this.m.add_dot(new dot(pointA.x, pointA.y, this.b));
		this.m.add_dot(new dot(pointB.x, pointB.y, this.b));
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
			var pointA;
			if((r + 0.25) % 0.5 == 0){
				pointA = rotated_point(ro[0].x-ds, 0, -r, x, y);
				// cnvs.lineTo(x+rvX*(ro[0].x-ds),y+rvY_*(ro[0].x-ds));
			}else if(r % 0.5 == 0){
				// cnvs.lineTo(x+rvX*ro[0].x,y+rvY_*ro[0].x);
				pointA = rotated_point(ro[0].x, 0, -r, x, y);
			}
			cnvs.lineTo(pointA.x, pointA.y);
		}
		if((sw == 1 && type == "F") || (sw == 0 && type == "B") || type == "G"){
			(type == "F")?stroke(bl,0, cnvs):stroke(bl,1, cnvs);

			var pointB = rotated_point(0, -radia[0], -r, x, y);

			// var tx = x - rvX_ * radia[0];
			// var ty = y - rvY * radia[0];
			cnvs.arc(pointB.x, pointB.y, radia[0],Math.PI*(r+0.5),Math.PI*(r+0.25),true);
		}
	}
}

class canvas_switch_r extends canvas_switch{
	constructor(module, data){
		super(module, data);
	}

	add_dots(){
		this.m.add_dot(new dot(this.x, this.y, this.b));

		var pointA; //Line
		// Arc
		var pointB = rotated_point(ro[0].x, ro[0].y, -this.r, this.x, this.y);
		if((this.r + 0.25) % 0.5 == 0){
			pointA = rotated_point(ro[0].x-ds, 0, -this.r, this.x, this.y);
		}else if(this.r % 0.5 == 0){
			pointA = rotated_point(ro[0].x, 0, -this.r, this.x, this.y);
		}

		this.m.add_dot(new dot(pointA.x, pointA.y, this.b));
		this.m.add_dot(new dot(pointB.x, pointB.y, this.b));
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
		this.add_dots();

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

	add_dots(){
		this.m.add_dot(new dot(this.x, this.y, this.b));

		//  A\
		//  x---B
		//     \C

		var pointA; // Arc
		var pointB; // Line
		var pointC; // Arc

		pointA = rotated_point(-ds, -ro[0].y, -this.r, this.x, this.y);
		pointB = rotated_point(ro[0].x-ds, 0, -this.r, this.x, this.y);
		pointC = rotated_point(ro[0].x, ro[0].y, -this.r, this.x, this.y);

		this.m.add_dot(new dot(pointA.x, pointA.y, this.b));
		this.m.add_dot(new dot(pointB.x, pointB.y, this.b));
		this.m.add_dot(new dot(pointC.x, pointC.y, this.b));
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

	add_dots(){
		this.m.add_dot(new dot(this.x, this.y, this.b));

		//     /A
		//  x---B
		//  C/

		var pointA; // Arc
		var pointB; // Line
		var pointC; // Arc

		pointA = rotated_point(ro[0].x, -ro[0].y, -this.r, this.x, this.y);
		pointB = rotated_point(ro[0].x-ds, 0, -this.r, this.x, this.y);
		pointC = rotated_point(-ds, ro[0].y, -this.r, this.x, this.y);

		this.m.add_dot(new dot(pointA.x, pointA.y, this.b));
		this.m.add_dot(new dot(pointB.x, pointB.y, this.b));
		this.m.add_dot(new dot(pointC.x, pointC.y, this.b));
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

class canvas_turntable {
	constructor(module, data){
		this.type = "turntable";
		this.edit_type = "turntable";
		this.module_id = module.id;
		this.m = module;
		this.b = data.b;
		this.cx = data.cx;
		this.cy = data.cy;
		this.r = data.r;
		this.pos = data.pos;
		this.mssw = data.mssw;

		this.posxy = [];

		if (data.options == undefined) { data.options = {}; }

		if (data.options.if != undefined){
			this.if = data.options.if;
		}
	}

	init(){
		for(var i = 0; i < this.pos.length; i++){

			this.posxy[i] = {x1: this.cx+Math.cos(this.pos[i]*Math.PI)*this.r, y1: this.cy+Math.sin(this.pos[i]*Math.PI)*this.r, x2:this.cx-Math.cos(this.pos[i]*Math.PI)*this.r, y2: this.cy-Math.sin(this.pos[i]*Math.PI)*this.r}
			this.m.add_dot(new dot(this.posxy[i].x1, this.posxy[i].y1, this.b));
			this.m.add_dot(new dot(this.posxy[i].x2, this.posxy[i].y2, this.b));
		}

		this.m.add_hitbox(this.hit_eval.bind(this));

		this.hit_eqn = [];

		//Calculate hitboxes
		//arc hit box
		// x^2+y^2 <= r^2

		this.hit_eqn.push({x:[0, 1], y:[0, 1], op: "lt", v: Math.pow(Math.ceil(this.r/10)*10, 2)});

		// this.hit_eqn.push({x:[1], y:[], op:"gt", v:0});  // x > 0
		// if((this.r % 0.5) == 0){
		// 	this.hit_eqn.push({x:[1], y:[], op:"lt", v:ro[0].x}); // x < ro[0].x
		// }
		// else{
		// 	this.hit_eqn.push({x:[1], y:[], op:"lt", v:(ro[0].x+ds)/Math.cos(Math.PI/4)}) // x < (ro[0].x+ds)/cos(pi/4)
		// }
		// var a = 0;
		// if(this.type == "swr"){
		// 	this.hit_eqn.push({x:[], y:[1], op:"gt", v:-hit_radius});         // y > -hitradius
		// 	this.hit_eqn.push({x:[1], y:[1], op:"lt", v:(ro[0].x+ro[0].y)});  // x + y < (ro[0].x+ro[0].y)
		// 	var b = radia[0];
		// }
		// else{
		// 	this.hit_eqn.push({x:[], y:[1], op:"lt", v:hit_radius});
		// 	this.hit_eqn.push({x:[1], y:[-1], op:"lt", v:(ro[0].x+ro[0].y)});
		// 	var b = -radia[0];
		// }
		// this.
	}

	hit_eval(x, y){
		console.log("Check hit turntable "+this.mssw);

		var _l = Math.sqrt(Math.pow(x-this.cx, 2)+Math.pow(y-this.cy, 2));
		var _r = Math.atan2((y-this.cy), (x-this.cx));
		x = _l*Math.cos(_r);
		y = _l*Math.sin(_r);

		if(!equation_tester(this.hit_eqn[0], {x:x, y:y})){
			console.log("Not in circle");
			return false;
		}

		this.hit_eqn_cw1 = [];
		this.hit_eqn_cw2 = [];
		this.hit_eqn_ccw1 = [];
		this.hit_eqn_ccw2 = [];

		var r = this.m.r - this.pos[this.m.msswitches[this.mssw]];

		var a = Math.sin(r * Math.PI);
		var b = Math.cos(r * Math.PI);

		this.hit_eqn_cw1.push({x:[b], y:[-a], op: "lt", v:0});
		this.hit_eqn_cw1.push({x:[a], y:[b],  op: "lt", v:0});

		this.hit_eqn_cw2.push({x:[b], y:[-a], op: "gt", v:0});
		this.hit_eqn_cw2.push({x:[a], y:[b],  op: "gt", v:0});

		this.hit_eqn_ccw1.push({x:[b], y:[-a], op: "lt", v:0});
		this.hit_eqn_ccw1.push({x:[a], y:[b],  op: "gt", v:0});

		this.hit_eqn_ccw2.push({x:[b], y:[-a], op: "gt", v:0});
		this.hit_eqn_ccw2.push({x:[a], y:[b],  op: "lt", v:0});

		var hit = 0;

		for(var i = 0; i < this.hit_eqn_cw1.length; i++){
			if(equation_tester(this.hit_eqn_cw1[i], {x:x, y:y})){
				hit++;
			}
		}
		if(this.hit_eqn_cw1.length == hit){
			// console.log("CW1");
			this.hit_cw();
			return;
		}

		hit = 0;

		for(var i = 0; i < this.hit_eqn_cw2.length; i++){
			if(equation_tester(this.hit_eqn_cw2[i], {x:x, y:y})){
				hit++;
			}
		}
		if(this.hit_eqn_cw2.length == hit){
			// console.log("CW2");
			this.hit_cw();
			return;
		}

		hit = 0;

		for(var i = 0; i < this.hit_eqn_ccw1.length; i++){
			if(equation_tester(this.hit_eqn_ccw1[i], {x:x, y:y})){
				hit++;
			}
		}
		if(this.hit_eqn_ccw1.length == hit){
			// console.log("CCW1");
			this.hit_ccw();
			return;
		}
		// console.log("Not in CCW1");

		hit = 0;

		for(var i = 0; i < this.hit_eqn_ccw2.length; i++){
			if(equation_tester(this.hit_eqn_ccw2[i], {x:x, y:y})){
				hit++;
			}
		}
		if(this.hit_eqn_ccw2.length == hit){
			// console.log("CCW2");
			this.hit_ccw();
			return;
		}
		// console.log("Not in CCW2");
	}

	hit_cw(){
		console.log("Hit CW");
		throw_MSSwitch(this.m.id, this.mssw, (this.m.msswitches[this.mssw] + 1) % this.pos.length);
	}
	hit_ccw(){
		console.log("Hit CCW");
		throw_MSSwitch(this.m.id, this.mssw, (this.m.msswitches[this.mssw] + this.pos.length - 1) % this.pos.length);
	}

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>TT</td><td>"+round(this.cx, 3)+
		       ", "+round(this.cy, 3)+"</td><td>"+round(this.r, 3)+"</td><td>"+this.pos+"</td><td>-</td><td>"+settings("#ccc", 23)+"</td></tr>";
	}

	draw_back(cnvs, stroke, color, rotation, offset){
		var pos = rotated_point(this.cx, this.cy, this.m.r, offset.X, offset.Y);
		cnvs.arc(pos.x, pos.y, this.r, 0, 2 * Math.PI, false);
		cnvs.strokeStyle = "#000";
		cnvs.lineWidth = 0.5;
		cnvs.stroke();
		cnvs.lineWidth = 6;
		cnvs.beginPath();

		stroke(this.m.blocks[this.b], color, cnvs);
		cnvs.beginPath();
		for(var i = 0; i < this.pos.length; i++){
			if(i != this.m.msswitches[this.mssw]){
				var x1 = rotation.X*this.posxy[i].x1 + rotation.Y_*this.posxy[i].y1 + offset.X;
				var y1 = rotation.Y*this.posxy[i].y1 + rotation.X_*this.posxy[i].x1 + offset.Y;

				var x2 = rotation.X*this.posxy[i].x2 + rotation.Y_*this.posxy[i].y2 + offset.X;
				var y2 = rotation.Y*this.posxy[i].y2 + rotation.X_*this.posxy[i].x2 + offset.Y;

				cnvs.moveTo(x1, y1);
				cnvs.lineTo(x2, y2);
				cnvs.stroke();cnvs.beginPath();
			}
		}
	}

	draw_fore(cnvs, stroke, rotation, offset){
		stroke(this.m.blocks[this.b], 0, cnvs);
		cnvs.beginPath();

		var i = this.m.msswitches[this.mssw];

		var centerpos = rotated_point(this.cx, this.cy, this.m.r);

		var pos1 = rotated_point( this.r, 0, this.m.r - this.pos[i], offset.X + centerpos.x, offset.Y + centerpos.y);
		var pos2 = rotated_point(-this.r, 0, this.m.r - this.pos[i], offset.X + centerpos.x, offset.Y + centerpos.y);

		// var x1 = rotation.X*this.posxy[i].x1 + rotation.Y_*this.posxy[i].y1 + offset.X;
		// var y1 = rotation.Y*this.posxy[i].y1 + rotation.X_*this.posxy[i].x1 + offset.Y;

		// var x2 = rotation.X*this.posxy[i].x2 + rotation.Y_*this.posxy[i].y2 + offset.X;
		// var y2 = rotation.Y*this.posxy[i].y2 + rotation.X_*this.posxy[i].x2 + offset.Y;

		cnvs.moveTo(pos1.x, pos1.y);
		cnvs.lineTo(pos2.x, pos2.y);
		cnvs.stroke();cnvs.beginPath();

		cnvs.fillStyle = cnvs.strokeStyle;

		cnvs.arc(pos1.x, pos1.y, 3, 0, 2 * Math.PI, true);


		// var a = Math.sin((this.m.r-this.pos[i]) * Math.PI) * this.r / 2;
		// var b = Math.cos((this.m.r-this.pos[i]) * Math.PI) * this.r / 2;

		// var c = Math.sin((this.m.r-this.pos[i]+0.5) * Math.PI) * this.r / 2;
		// var d = Math.cos((this.m.r-this.pos[i]+0.5) * Math.PI) * this.r / 2;

		// var pos1 = rotated_point(a, b, this.m.r, offset.X + centerpos.x, offset.Y + centerpos.y);
		// var pos2 = rotated_point(a, b, this.m.r + 1, offset.X + centerpos.x, offset.Y + centerpos.y);

		// var pos3 = rotated_point(c, d, this.m.r, offset.X + centerpos.x, offset.Y + centerpos.y);
		// var pos4 = rotated_point(c, d, this.m.r + 1, offset.X + centerpos.x, offset.Y + centerpos.y);

		// cnvs.strokeStyle = "#0f0";
		// cnvs.moveTo(pos1.x, pos1.y);
		// cnvs.lineTo(pos2.x, pos2.y);
		// cnvs.stroke();cnvs.beginPath();
		// cnvs.strokeStyle = "#0ff";
		// cnvs.moveTo(pos3.x, pos3.y);
		// cnvs.lineTo(pos4.x, pos4.y);
		cnvs.stroke();cnvs.beginPath();
	}
}

class canvas_parallelogram {
	constructor(data){
		this.x = data.x;
		this.y = data.y;
		this.w = data.w;
		this.h = data.h;
		this.s = data.s;
	}

	init(){}

	draw_back(cnvs, stroke, color, rotation, offset){}

	draw_fore(cnvs, stroke, rotation, offset){
		cnvs.moveTo(offset.X+this.x, offset.Y+this.y);
		cnvs.lineTo(offset.X+this.x+this.w, offset.Y+this.y);
		cnvs.lineTo(offset.X+this.x+this.w+this.s, offset.Y+this.y+this.h);
		cnvs.lineTo(offset.X+this.x+this.s, offset.Y+this.y+this.h);
		cnvs.fillStyle = "rgba(1,1,1,0.5)";
		cnvs.lineWidth = 0;
		cnvs.stroke();
		cnvs.lineWidth = 6;
	}
}

class canvas_polarity {
	constructor(module, data){//module_id, block, x1, y1, x2, y2, options){
		this.type = "polarity";
		this.edit_type = "polarity";
		this.module_id = module.id;
		this.m = module;
		this.b = data.b;
		this.x = data.x;
		this.y = data.y;
		this.r = data.r;

		if (data.options == undefined) { data.options = {}; }

		if (data.options.if != undefined){
			this.if = data.options.if;
		}
	}

	init(){}

	configdata(id){
		return "<tr><th scope='row'>"+id+"</th><td>"+this.b+"</td><td>Line</td><td>"+round(this.x, 3)+
		       ", "+round(this.y, 3)+"</td><td>"+round(this.r, 3)+", </td><td>-</td><td>-</td><td>"+settings("#ccc", 23)+"</td></tr>";
	}

	draw_fore(cnvs, stroke, rotation, offset){
		if(this.m.blocksstate[this.b].polarity == 0)
			stroke(5, 0, cnvs);
		else
			stroke(1, 0, cnvs);

		var x1 = rotation.X*this.x + rotation.Y_*this.y + offset.X;
		var y1 = rotation.Y*this.y + rotation.X_*this.x + offset.Y;

		// var xy1 = rotated_point(this.x, this.y, rotation.X, offset.X, offset.Y);

		var xy2 = rotated_point( 0,  +1, this.r+rotation.X, x1, y1);
		var xy4 = rotated_point( 0,  -1, this.r+rotation.X, x1, y1);
		var xy5 = rotated_point( 0,   0, this.r+rotation.X, x1, y1);

		if(this.m.blocksstate[this.b].direction == 0){
			var xy3 = rotated_point(-2.5, 0, this.r+rotation.X, x1, y1);
	
			cnvs.moveTo(x1, y1);
			cnvs.lineTo(xy2.x, xy2.y);
			cnvs.lineTo(xy3.x, xy3.y);
			cnvs.lineTo(xy4.x, xy4.y);
			cnvs.lineTo(xy5.x, xy5.y);
		}
		else{
			var xy3 = rotated_point(+2.5, 0, this.r+rotation.X, x1, y1);
	
			cnvs.moveTo(x1, y1);
			cnvs.lineTo(xy2.x, xy2.y);
			cnvs.lineTo(xy3.x, xy3.y);
			cnvs.lineTo(xy4.x, xy4.y);
			cnvs.lineTo(xy5.x, xy5.y);
		}
	}

	draw_back(cnvs, stroke, color, rotation, offset){ }
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

		this.config = {};
		
		for(var i = 0; i < this.connections.length; i++){
			this.connections[i].connected = false;
		}

		this.blocks      = Array.apply(null, Array(data.blocks)).map(function (){return 7});
		this.blocksstate = Array.apply(null, Array(data.blocks)).map(function (){return {"state": 7, "direction": 0, "polarity": 0}});
		this.switches    = Array.apply(null, Array(data.switches)).map(function (){return 0});
		if(data.msswitches != undefined){
			this.msswitches = Array.apply(null, Array(data.msswitches)).map(function (){return 0});
		}
		else{
			this.msswitches = [];
		}

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
				case "turntable":
					this.data.push(new canvas_turntable(this, data.layout[i]));
					break;
				case "parallelogram":
					this.data.push(new canvas_parallelogram(this, data.layout[i]));
					break;
				case "polarity":
					this.data.push(new canvas_polarity(this, data.layout[i]));
			}

			this.data[i].init();
		}
	}

	move(options = {}){
		console.log("Move module "+this.id, options);
		if(options.originID != undefined && options.originID == this.id){
			return;
		}
		else if(options.originID == undefined){
			options.originID = this.id;
		}

		var depth;
		if(options.depth == undefined){
			depth = 0;
		}
		else{
			depth = options.depth + 1;
		}
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
			if(this.r < 0)
				this.r += 2;
			this.r = this.r % 2;
		}

		if(depth > 10){
			return;
		}

		for(var i = 0; i < this.connection_link.length; i++){
			if(this.connection_link[i] == undefined)
				continue;

			if(!this.connections[i].connected)
				continue;

			if(this.connection_link[i] == options.prev)
				continue;

			var m = modules[this.connection_link[i]];

			// Calculate new offset point
			var j = m.connection_link.indexOf(this.id);
			var rotation = (this.r + this.connections[i].r + 1);
			var pointA = rotated_point(this.connections[i].x, this.connections[i].y, this.r, this.OffsetX, this.OffsetY);
			var pointB = reversed_rotated_point(m.connections[j].x, m.connections[j].y, (rotation+m.connections[j].r)%2, pointA.x, pointA.y);

			m.move({OffsetX: pointB.x, OffsetY: pointB.y, r: (rotation+m.connections[j].r)%2, originID: options.originID, prev: this.id, depth: depth});
		}


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
			var module_b = modules[this.connection_link[i]];

			if(module_b == undefined || !module_b.connection_link.includes(this.id)){
				continue;
			}

			var anchor = rotated_point(this.connections[i].x, this.connections[i].y, this.r, this.OffsetX, this.OffsetY)

			var rotation = (this.r + this.connections[i].r + 1) % 2;

			var j = module_b.connection_link.indexOf(this.id);

			if(rotation == (module_b.r + module_b.connections[j].r)){
				// Just move
				var newpoint = reversed_rotated_point(module_b.connections[j].x, module_b.connections[j].y, module_b.r, anchor.x, anchor.y);
				module_b.move({OffsetX: newpoint.x, OffsetY: newpoint.y, originID: this.id});
			}
			else{
				// rotate and move
				// module_b.move({r: (rotation+module_b.connections[j].r)%2})
				var newpoint = reversed_rotated_point(module_b.connections[j].x, module_b.connections[j].y, (rotation+module_b.connections[j].r)%2, anchor.x, anchor.y);
				module_b.move({OffsetX: newpoint.x, OffsetY: newpoint.y, r: (rotation+module_b.connections[j].r)%2, originID: this.id});
			}

			console.log("Module "+module_b.id+" and "+this.id+" connected");
			module_b.connections[j].connected = true;
			this.connections[i].connected = true;
		}
	}

	add(obj){
		this.data.push(obj);
	}

	add_hitbox(hit){
		this.hitboxes.push(hit);
	}

	add_dot(dot){
		// for(var i = 0; i < this.dotmatrix.length; i++){
		// 	if(this.dotmatrix[i].equals(dot)){
		// 		this.dotmatrix[i].combine(dot);
		// 		break;
		// 	}

		// 	if(i == (this.dotmatrix.length-1)){
		// 		this.dotmatrix.push(dot);
		// 	}
		// }

		if(this.dotmatrix.length == 0){
			this.dotmatrix.push(dot);
		}
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
		for(var i = 0; i < this.dotmatrix.length; i++){
			cnvs.beginPath();

			var dot = this.dotmatrix[i];

			var block_state = 255;
			for(var j = 0; j < dot.block.length; j++){
				if(dot.block[j] < block_state){
					block_state = dot.block[i];
				}
			};

			stroke(block_state, 0, cnvs);
			cnvs.fillStyle = cnvs.strokeStyle;

			var point = rotated_point(dot.x, dot.y, this.r, this.OffsetX, this.OffsetY);

			cnvs.arc(point.x, point.y, r, 0, 2*Math.PI, false);

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

function reversed_rotated_point(x, y, r, ofX=0, ofY=0){
	r_ = {X: Math.cos(r*Math.PI), X_: Math.cos((r+0.5)*Math.PI), Y: Math.sin((r+0.5)*Math.PI), Y_: Math.sin((r)*Math.PI)};
	return {x: ofX - r_.X*x - r_.Y_*y , y: ofY - r_.Y*y - r_.X_*x } ;
}

var loading_modules = 0;


events.add_init(function(){
	// load_module(20);
	// load_module(21);
	// load_module(22);
	// load_module(23);
	// load_module(25);
	// load_module(26);
});

function conf_modules(){
	modules[10].init({visible: true, OffsetX: 0, OffsetY: 0, r: 0});
	Canvas.rescale(1);
}

var modules = {};

var modules_data = {}
