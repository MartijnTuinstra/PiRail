var B,C,D,E,x1,y1;
var F = [];
var G = [];

function flip(pNode,axis){
  var homeElement = pNode;
  if(homeElement.attr("id").startsWith("BBlock")){
    console.log("Flipping a block");
  }else if(homeElement.attr("id").startsWith("BSwitch")){
    console.log("Flipping a switch");
    if(axis == "h"){
      console.log("Test");
      obj_list = [];
      _list = [];
      //flip Classes and connected points
      //Finding and storing info
      $.each($('path.APoint',homeElement),function(index,element){
        if($(element).attr("class").startsWith("N")){
          var cls = $(element).attr("class").split(" ")[0];
          obj_list.push([cls,$(element).attr("connected"),$('text.'+cls,homeElement).html()])
        }
      });
      //Changing info
      $.each(obj_list,function(index){
        _list.push([$('path.'+obj_list[index][0],homeElement),obj_list[obj_list.length-1-index][0]]);
        _list.push([$('text.'+obj_list[index][0],homeElement),obj_list[obj_list.length-1-index][0]]);

        $('path.'+obj_list[index][0],homeElement).attr("connected",obj_list[obj_list.length-1-index][1]);
        $('text.'+obj_list[index][0],homeElement).html(obj_list[obj_list.length-1-index][2]);
      });
      //Swapping class
	  
      $.each(_list,function(index,element){
        element[0].attr("class",element[1]+" APoint");
      });

      //Flip contact points
	  
      contact = homeElement.attr("contact").split("|");
      contact[0] = contact[0].split(" ");
      contact[0] = contact[0].reverse();
      contact[1] = contact[1].split(" ");
      contact[1] = contact[1].reverse();
      contact_out = "";
      $.each(contact[0],function(i){
        if(i != 0){
          contact_out += " ";
        }
        contact_out += contact[0][i];
      });
      contact_out += "|";
      $.each(contact[1],function(i){
        if(i != 0){
          contact_out += " ";
        }
        contact_out += contact[1][i];
      });
      homeElement.attr("contact",contact_out);
    }
    else if(axis == "v"){
      console.log("Test");
      obj_list = [];
      _list = [];
      //flip Classes and connected points
      //Finding and storing info
      $.each($('path.APoint',homeElement),function(index,element){
        var cls = $(element).attr("class").split(" ")[0];
        obj_list.push([cls,$('path.'+cls,homeElement),$('path.'+cls,homeElement).attr("df"),$('path.'+cls,homeElement).attr("d"),
                          $('text.'+cls,homeElement),$('text.'+cls,homeElement).attr("x"),$('text.'+cls,homeElement).attr("y"),
                          $('text.'+cls,homeElement).attr("locf")]);
      });

      //Changing info
      $.each(obj_list,function(index){
        obj_list[index][1].attr("df",obj_list[index][3]);
        obj_list[index][1].attr("d",obj_list[index][2]);
        if(obj_list[index][4].length != 0){
          obj_list[index][4].attr("locf",obj_list[index][5] + "," + obj_list[index][6]);
          obj_list[index][4].attr("x",obj_list[index][7].split(",")[0]);
        }
      });
      
      //Swapping class
	  /*
      $.each(_list,function(index,element){
        element[0].attr("class",element[1]+" APoint");
      });*/
      
      //Flip contact points

      contact = homeElement.attr("contact").split("|");
      $.each(contact,function(x){
        contact[x] = contact[x].split(" ");
        $.each(contact[x],function(y){
          cont_x = parseInt(contact[x][y].split(",")[0]);
          contact[x][y] = (cont_x - 2*(cont_x - 17.5)) + "," + contact[x][y].split(",")[1];
        });
      });
      contact_out = "";
      $.each(contact[0],function(i){
        if(i != 0){
          contact_out += " ";
        }
        contact_out += contact[0][i];
      });
      contact_out += "|";
      $.each(contact[1],function(i){
        if(i != 0){
          contact_out += " ";
        }
        contact_out += contact[1][i];
      });
      homeElement.attr("contact",contact_out);
    }
  }
}

function StartMove(evt){
  if(evt.shiftKey){
    flip($(evt.target.parentNode),"v");
  }else if(evt.ctrlKey){
    flip($(evt.target.parentNode),"h");
  }


  x1 = evt.clientX;
  y1 = evt.clientY;

  D = $(evt.target);

  C = evt.target.parentNode;

  B = $(evt.target.parentNode.parentNode);

  E = $("path[connected]",C);

  F = [];
  F[0] = [];
  F[1] = [];

  if(E.length > 0){
    $.each(E,function(index,value){
      if($(E[index]).attr('connected').indexOf("_") >= 0){
        if($(C).attr("id").startsWith("BBlock")){ //If you are dragging a block
          if($(E[index]).attr("class").startsWith("P")){ //If its the previous side
            F[0].push($(E[index]).attr('connected').split('_'));
            F[0][F[0].length-1][1] = parseInt(F[0][F[0].length-1][1]);
          }else if($(E[index]).attr("class").startsWith("N")){ //If its the next side
            F[1].push($(E[index]).attr('connected').split('_'));
            F[1][F[1].length-1][1] = parseInt(F[1][F[1].length-1][1]);
          }
        }
        else if($(C).attr("id").startsWith("BSwitch")){ //If you are dragging a block
          if($(E[index]).attr("class").startsWith("A")){ //If its the approach side
            F[0].push($(E[index]).attr('connected').split('_'));
            F[0][F[0].length-1][1] = parseInt(F[0][F[0].length-1][1]);
          }else if($(E[index]).attr("class").startsWith("N")){                                          //If its not the approach side
			level = parseInt($(E[index]).attr("class").split(" ")[0].slice(1)) - 1;
            F[1][level] = $(E[index]).attr('connected').split('_');
            F[1][level][1] = parseInt(F[1][level][1]);
          }
        }
        else if($(C).attr("id").startsWith("BAnchor")){ //If you are dragging a link
          if($(E[index]).attr("class").startsWith("A")){ //If you selected the A side
            F[0].push($(E[index]).attr('connected').split('_'));
            F[0][F[0].length-1][1] = parseInt(F[0][F[0].length-1][1]);
          }else{                                          //If you selected the B side
            F[1].push($(E[index]).attr('connected').split('_'));
            F[1][F[1].length-1][1] = parseInt(F[1][F[1].length-1][1]);
          }
        }
      }else{
        F.push(undefined);
      }
    });

    G = $(C).attr('contact').split('|');
    $.each(G,function(x){
      G[x] = G[x].split(' ');
      $.each(G[x],function(y){
        G[x][y] = G[x][y].split(',');
        $.each(G[x][y],function(z){
          G[x][y][z] = parseInt(G[x][y][z]);
        });
      });
    });
  }


  if(evt.shiftKey || evt.ctrlKey){
    endMove();
    return;
  }

  console.log(D);
  $(D).bind({
    mousemove : MoveIt,
    mouseup   : endMove
  });
  $('body').bind({
    mousemove : MoveIt,
    mouseup   : endMove
  });

}

function MoveIt(event){
  var evt = event;
  translation = $(C).attr("transform").slice(10,-1).split(',');
  sx = parseInt(translation[0]);
  sy = parseInt(translation[1]);

  $(C).attr("transform", "translate(" + (sx + evt.clientX - x1) + "," + (sy + evt.clientY - y1) + ")");

  $.each(F,function(x){
    $.each(F[x],function(y){
      if(F[x][y]){
        if(F[x][y][1] == 1){
          $("#"+F[x][y][0],B).attr('x1',(sx + evt.clientX - x1) + G[x][y][0]);
          $('#'+F[x][y][0],B).attr('y1',(sy + evt.clientY - y1) + G[x][y][1]);
        }else{
          $('#'+F[x][y][0],B).attr('x2',(sx + evt.clientX - x1) + G[x][y][0]);
          $('#'+F[x][y][0],B).attr('y2',(sy + evt.clientY - y1) + G[x][y][1]);
        }
      }
    });
  });
  
  x1 = evt.clientX;
  y1 = evt.clientY;
}

function endMove(){
  console.log("MouseUP");

  translation = $(C).attr("transform").slice(10,-1).split(',');
  sx = Math.round(parseInt(translation[0])/10)*10;
  sy = Math.round(parseInt(translation[1])/10)*10;

  $(C).attr("transform", "translate("+sx+","+sy+")");

  $.each(F,function(x){
    $.each(F[x],function(y){
      if(F[x][y]){
        if(F[x][y][1] == 1){
          $("#"+F[x][y][0],B).attr('x1',sx + G[x][y][0]);
          $('#'+F[x][y][0],B).attr('y1',sy + G[x][y][1]);
        }else{
          $('#'+F[x][y][0],B).attr('x2',sx + G[x][y][0]);
          $('#'+F[x][y][0],B).attr('y2',sy + G[x][y][1]);
        }
      }
    });
  });

  if($(C).attr("id").startsWith("BBlock")){
    nr = parseInt($(C).attr("id").slice(6));
    EditObj.Blocks[nr].Connector.x = sx;
    EditObj.Blocks[nr].Connector.y = sy;
  }else if($(C).attr("id").startsWith("BAnchorL")){
    EditObj.LAnchor.x = sx;
    EditObj.LAnchor.y = sy;
  }else if($(C).attr("id").startsWith("BAnchorR")){
    EditObj.RAnchor.x = sx;
    EditObj.RAnchor.y = sy;
  }

  $(D).unbind('mousemove',MoveIt);
  $(D).unbind('mouseup',endMove);
  //,'mouseup']);
  $('body').unbind('mousemove',MoveIt);
  $('body').unbind('mouseup',endMove);
  //,'mouseup']);



  F = [];
  G = [];
}

var draw = false;

function startDraw(event){
  if(draw){
    return;
  }
  var evt = event;
  draw = true;

  console.log("start draw");

  x1 = evt.clientX;
  y1 = evt.clientY;

  C = evt.target.parentNode;

  if($(evt.target).is("path")){
    console.log("Drawing point is not a path");
    D = $(evt.target);
  }else if($(evt.target).is("text")){
    D = $('path.'+$(evt.target).attr("class").split(" ")[0],$(C));
  }


  var obj = D;
  B = $(evt.target.parentNode.parentNode);


  G = $(C).attr('contact').split('|');
  $.each(G,function(x){
    G[x] = G[x].split(' ');
    $.each(G[x],function(y){
      G[x][y] = G[x][y].split(',');
      $.each(G[x][y],function(z){
        G[x][y][z] = parseInt(G[x][y][z]);
      });
    });
  });

  translation = $(C).attr("transform").slice(10,-1).split(',');
  sx = parseInt(translation[0]);
  sy = parseInt(translation[1]);

  var LineID
  if($(D).attr('connected') == ""){
    LineID = 'L'+$('line',B).length;
  }else{
    LineID = $(D).attr('connected').split('_')[0];
    removeLine($('#'+LineID,B));
  }

  if($(obj).attr("class").startsWith('P') || $(obj).attr("class").startsWith('A')){ //P side of block or Approach of Switch
    var newLine = document.createElementNS('http://www.w3.org/2000/svg','line');
    newLine.setAttribute('id',LineID);
    newLine.setAttribute('class','drawing');
    newLine.setAttribute('style','stroke:black;stroke-width:2px');
    newLine.setAttribute('x1',G[0][0][0]+sx);
    newLine.setAttribute('y1',G[0][0][1]+sy);
    newLine.setAttribute('x2',G[0][0][0]+sx);
    newLine.setAttribute('y2',G[0][0][1]+sy);
    B.prepend(newLine);
    $('#L'+LineID,B).attr('x2',(x1 - $(B).position().left));
    $('#L'+LineID,B).attr('y2',(y1 - $(B).position().top));
    //B.append('<line x1="'+G[0][0]+'" y1="'+G[0][1]+'" x2="'+G[0][0]+'" y2="'+G[0][1]+'" class="drawing" style="stroke:black;stroke-width:2px"/>');
  }else if($(obj).attr("class").startsWith('N ')){ //N side of block
    var newLine = document.createElementNS('http://www.w3.org/2000/svg','line');
    newLine.setAttribute('id',LineID);
    newLine.setAttribute('class','drawing');
    newLine.setAttribute('style','stroke:black;stroke-width:2px');
    newLine.setAttribute('x1',G[1][0][0]+sx);
    newLine.setAttribute('y1',G[1][0][1]+sy);
    newLine.setAttribute('x2',G[1][0][0]+sx);
    newLine.setAttribute('y2',G[1][0][1]+sy);
    B.prepend(newLine);
    $('#L'+LineID,B).attr('x2',(x1 - $(B).position().left));
    $('#L'+LineID,B).attr('y2',(y1 - $(B).position().top));

    //B.append('<line x1="'+G[1][0]+'" y1="'+G[1][1]+'" x2="'+G[1][0]+'" y2="'+G[1][1]+'" class="drawing" style="stroke:black;stroke-width:2px"/>')
  }else if($(obj).attr("class").startsWith('N')){ //N side of block
    var pointNr = parseInt($(obj).attr("class").split(" ")[0].slice(1))-1;
    var newLine = document.createElementNS('http://www.w3.org/2000/svg','line');
    newLine.setAttribute('id',LineID);
    newLine.setAttribute('class','drawing');
    newLine.setAttribute('style','stroke:black;stroke-width:2px');
    newLine.setAttribute('x1',G[1][pointNr][0]+sx);
    newLine.setAttribute('y1',G[1][pointNr][1]+sy);
    newLine.setAttribute('x2',G[1][pointNr][0]+sx);
    newLine.setAttribute('y2',G[1][pointNr][1]+sy);
    B.prepend(newLine);
    $('#L'+LineID,B).attr('x2',(x1 - $(B).position().left));
    $('#L'+LineID,B).attr('y2',(y1 - $(B).position().top));

    //B.append('<line x1="'+G[1][0]+'" y1="'+G[1][1]+'" x2="'+G[1][0]+'" y2="'+G[1][1]+'" class="drawing" style="stroke:black;stroke-width:2px"/>')
  };




  console.log(D);
  $(D).bind({
    mousemove : function(event){Drawing(this,event)},
    mouseup   : function(event){StopDrawing(event)}
  });
  $('body').bind({
    mousemove : function(event){Drawing(this,event)},
    mouseup   : function(event){StopDrawing(event)}
  });
  $('#ConnectBox .APoint').bind({
    mouseenter : function(event){allowDrop(event)}
  });
}

function Drawing(obj,evt){

  sx = parseInt($('.drawing',B).attr("x2"));
  sy = parseInt($('.drawing',B).attr("y2"));

  $('.drawing',B).attr("x2", (sx + evt.clientX - x1));
  $('.drawing',B).attr("y2", (sy + evt.clientY - y1));


  x1 = evt.clientX;
  y1 = evt.clientY;
}

function StopDrawing(evt){
  console.log("Stop Drawing");
  $(D).unbind('mousemove');
  $(D).unbind('mouseup');
  $('line.drawing',B).remove();
  //,'mouseup']);
  $('body').unbind('mousemove');
  $('body').unbind('mouseup');

  draw = false;
  $('#ConnectBox .APoint').unbind('mouseenter');
}

function attachdraw(evt){
  console.log('attach');
  
  var element;
  if(evt.target.tagName == 'text'){
	element = $('path.'+$(evt.target).attr("class").split(" ")[0],$(evt.target.parentNode));
  }else if(evt.target.tagName == 'path'){
	element = $(evt.target);
  }
  
  element.css("fill","black");
  
  if(element.attr('connected') != ""){
    removeLine($('#'+element.attr('connected').split('_')[0],B));
  }

  $(D).attr('connected',$('line.drawing',B).attr('id')+"_1");
  element.attr('connected',$('line.drawing',B).attr('id')+"_2");

  G = $(evt.target.parentNode).attr('contact').split('|');
  $.each(G,function(x){
    G[x] = G[x].split(' ');
    $.each(G[x],function(y){
      G[x][y] = G[x][y].split(',');
      $.each(G[x][y],function(z){
        G[x][y][z] = parseInt(G[x][y][z]);
      });
    });
  });

  var Loc = {};
  
  translation = $(evt.target.parentNode).attr("transform").slice(10,-1).split(',');
  Loc.left = parseInt(translation[0]);
  Loc.top  = parseInt(translation[1]);

  class_name = element.attr('class').split(" ")[0];
  if(class_name == "P" || class_name == "A"){
    $("#"+$('line.drawing',B).attr('id'),B).attr('x2',Loc.left + G[0][0][0]);
    $('#'+$('line.drawing',B).attr('id'),B).attr('y2',Loc.top + G[0][0][1]);
  }else if(class_name == "N"){
    $("#"+$('line.drawing',B).attr('id'),B).attr('x2',Loc.left + G[1][0][0]);
    $('#'+$('line.drawing',B).attr('id'),B).attr('y2',Loc.top + G[1][0][1]);
  }else if(class_name.startsWith("N")){ //N side of block
    var pointNr = parseInt(class_name.slice(1))-1;
    $("#"+$('line.drawing',B).attr('id'),B).attr('x2',Loc.left + G[1][pointNr][0]);
    $('#'+$('line.drawing',B).attr('id'),B).attr('y2',Loc.top + G[1][pointNr][1]);
  }

  $('line.drawing',B).toggleClass('drawing');
}

function allowDrop(event){
  var evt = event;
  console.log("Allow Drop");
  console.log(evt.target.tagName);
  var element;
  if(evt.target.tagName == 'text'){
	element = $('path.'+$(evt.target).attr("class").split(" ")[0],$(evt.target.parentNode));
  }else if(evt.target.tagName == 'path'){
	element = $(evt.target);
  }
  
  if(draw == true){
    element.css('fill','green');
  }else{
    element.css('fill','black');
  }

  if(element.attr('connected') != ""){
    $('#'+element.attr('connected').split('_')[0],B).css('stroke','red');
  }



  $(evt.target).bind({
    mouseout : function(event){stopDrop(event)},
    mouseup  : function(event){attachdraw(event)}
  });
}

function stopDrop(event){
  $(event.target).unbind('mouseout');
  $(event.target).unbind('mouseup');
  
  if(event.target.tagName == 'text'){
	//element = $('path.'+$(event.target).attr("class").split(" ")[0],$(event.target.parentNode));
  }else if(event.target.tagName == 'path'){
	element = $(event.target);
	element.css('fill','black');
	if(element.attr('connected') != ""){
	  $('#'+element.attr('connected').split('_')[0],B).css('stroke','black');
	}
  }
}

function removeLine(obj){
  var IDs = [];
  $("path[connected^='"+obj.attr('id')+"']",B).each(function(){ IDs.push({"id":$(this.parentNode).attr('id'),"class":$(this).attr('class')}); });
  console.log(IDs);
  $("path[connected^='"+obj.attr('id')+"']",B).attr('connected','');

  for(var i = 0;i<IDs.length;i++){
    IDs[i].class = IDs[i].class.split(" ");

    if(IDs[i].id.startsWith('BBlock')){ //Its a block
      if(IDs[i].class[0] == 'N'){
        EditObj.Blocks[IDs[i].id.slice(6)].Next[0] = 'E';
        EditObj.Blocks[IDs[i].id.slice(6)].Next[1] = 'E';
        EditObj.Blocks[IDs[i].id.slice(6)].Next[2] = 'E';
      }
      else{
        EditObj.Blocks[IDs[i].id.slice(6)].Prev[0] = 'E';
        EditObj.Blocks[IDs[i].id.slice(6)].Prev[1] = 'E';
        EditObj.Blocks[IDs[i].id.slice(6)].Prev[2] = 'E';
      }
    }
    else if(IDs[i].id.startsWith('BAnchorR')){
      if(IDs[i].class[0] == 'A'){
        EditObj.RAnchor.A = [];
        EditObj.RAnchor.A[0] = 'E';
        EditObj.RAnchor.A[1] = 'E';
        EditObj.RAnchor.A[2] = 'E';
      }
      else{
        EditObj.RAnchor.B = [];
        EditObj.RAnchor.B[0] = 'E';
        EditObj.RAnchor.B[1] = 'E';
        EditObj.RAnchor.B[2] = 'E';
      }
    }
    else if(IDs[i].id.startsWith('BAnchorL')){
      if(IDs[i].class[0] == 'A'){
        EditObj.LAnchor.A = [];
        EditObj.LAnchor.A[0] = 'E';
        EditObj.LAnchor.A[1] = 'E';
        EditObj.LAnchor.A[2] = 'E';
      }
      else{
        EditObj.LAnchor.B = [];
        EditObj.LAnchor.B[0] = 'E';
        EditObj.LAnchor.B[1] = 'E';
        EditObj.LAnchor.B[2] = 'E';
      }
    }
  }

  obj.remove();
}

var SVGResize = {};

function SetResizeSVG(x,y){
	x = parseInt(x);
	y = parseInt(y);
	
	$("#ConnectBox").css("width",x+"px");
	$("#ConnectBox").css("height",y+"px");
	$("#ConnectBox").attr("viewBox","0 0 "+x+" "+y);
	
	$("#ConnectBox #vResize").attr("transform","translate("+((x/2)-20)+","+(y-25)+")");
	$("#ConnectBox #hResize").attr("transform","translate("+(x-25)+","+((y/2)-20)+")");
}

function ResizeSVG(event){
	var axis = event.data[0];
	var dir  = event.data[1];
	var step = 50;
	
	if(event.shiftKey){
		step *= 4;
	}else if(event.ctrlKey){
		step *= 10;
	}
	
	console.log("Resize SVG"+axis+dir);
	sx = parseInt($('#ConnectBox').css('width').slice(0,-2));
	sy = parseInt($('#ConnectBox').css('height').slice(0,-2));
	console.log(sx + "|"+sy);
	if(dir == '+'){
		if(axis == "y"){
			sy += step;
		}else if(axis == "x"){
			sx += step;
		}
	}
	if(dir == '-'){
		if(axis == "y"){
			sy -= step;
		}else if(axis == "x"){
			sx -= step;
		}
	}
	$('#ConnectBox').css('width',sx  + "px");
	$('#ConnectBox').css('height',sy + "px");
	$('#ConnectBox').attr("viewBox","0 0 "+sx+" "+sy);
	
	$("#ConnectBox #vResize").attr("transform","translate("+((sx/2)-20)+","+(sy-25)+")");
	$("#ConnectBox #hResize").attr("transform","translate("+(sx-25)+","+((sy/2)-20)+")");
	
	
	EditObj.ConW = sx;
	EditObj.ConH = sy;
}

function rescaleSVG(event,type){
  SVGResize.Handle = $(event.target);

  if(type == 'x' || type == 'y' || type == 'xy'){
    SVGResize.Element = $('svg',event.target.parentNode);
    SVGResize.Container = $(event.target.parentNode);

    SVGResize.MouseX1 = event.clientX;
    SVGResize.MouseY1 = event.clientY;

    SVGResize.Handle.bind({
      mousemove : function(event){rescaleSVG(event,'m'+type)},
      mouseup   : function(event){rescaleSVG(event,'e'+type)}
    });
    $('body').bind({
      mousemove : function(event){rescaleSVG(event,'m'+type)},
      mouseup   : function(event){rescaleSVG(event,'e'+type)}
    });
  }else if(type.startsWith('e')){
    SVGResize.Handle.unbind('mousemove');
    SVGResize.Handle.unbind('mouseup');
    $('body').unbind('mousemove');
    $('body').unbind('mouseup');
  }


  if(type == 'x' || type == 'xy' || type == 'mx' || type == 'mxy'){
    nX = (sx + event.clientX - SVGResize.MouseX1);
  }else{nX = sx}
  if(type == 'y' || type == 'xy' || type == 'my' || type == 'mxy'){
    nY = (sy + event.clientY - SVGResize.MouseY1);
  }else{nY = sy}

  if(nX < 0){nX = 0;}
  if(nY < 0){nY = 0;}

  SVGResize.Element.css("width", nX + "px");
  SVGResize.Element.css("height",nY + "px");
  SVGResize.Element.attr("viewBox","0 0 "+nX+" "+nY);

  $('.x',SVGResize.Container).css("height",nY+"px");
  $('.y',SVGResize.Container).css("width",nX+"px");

  SVGResize.Container.css("width",(nX+5)+"px");
  SVGResize.Container.css("height",(nY+5)+"px");

  EditObj.ConW = nX;
  EditObj.ConH = nY;


  SVGResize.MouseX1 = event.clientX;
  SVGResize.MouseY1 = event.clientY;



}
