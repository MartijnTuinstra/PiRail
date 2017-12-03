
LayoutDragging = {"x1":0,"y1":0,"target":undefined,"parent":undefined,"Box":undefined,"Dropable":0,"Keys":[]};

// Converts from degrees to radians.
Math.radians = function(degrees) {
  return degrees * Math.PI / 180;
};

// Converts from radians to degrees.
Math.degrees = function(radians) {
  return radians * 180 / Math.PI;
};

function getMidPoint(x, y, width, height, angle_degrees) {
    var angle_rad = angle_degrees * Math.PI / 180;
    var cosa = Math.cos(angle_rad);
    var sina = Math.sin(angle_rad);
    var wp = width/2;
    var hp = height/2;
    return { px: ( x + wp * cosa - hp * sina ),
             py: ( y + wp * sina + hp * cosa ) };
}

function getRotatedPoint(x, y, width, height, angle_degrees) {
    var angle_rad = angle_degrees * Math.PI / 180;
    var cosa = Math.cos(angle_rad);
    var sina = Math.sin(angle_rad);
    var wp = width;
    var hp = height;
    return { left: ( x + wp * cosa - hp * sina ),
             top: ( y + wp * sina + hp * cosa ) };
}

function MDegrees(degrees){
  return (degrees % 360);
}

function Layout_dragToolbar(evt){
  x1 = evt.clientX;
  y1 = evt.clientY;

  LayoutDragging.target = $(evt.target);
  LayoutDragging.ID = LayoutDragging.target.attr("nr");

  LayoutDragging.targetParent = evt.target.parentNode;

  $(LayoutDragging.targetParent).toggleClass("dragging");

  LayoutDragging.targetPParent = $(evt.target.parentNode.parentNode);

  console.log(LayoutDragging);
  $(LayoutDragging.target).bind({
    mousemove : Layout_dragToolbarMove,
    mouseup   : Layout_dragToolbarStop
  });
  $('body').bind({
    mousemove : Layout_dragToolbarMove,
    mouseup   : Layout_dragToolbarStop
  });

}

function Layout_dragToolbarMove(event){
  var evt = event;
  translation = $(LayoutDragging.targetParent).attr("transform").slice(10,-1).split(',');
  sx = parseInt(translation[0]);
  sy = parseInt(translation[1]);

  $(LayoutDragging.targetParent).attr("transform", "translate(" + (sx + evt.clientX - x1) + "," + (sy + evt.clientY - y1) + ")");

  x1 = evt.clientX;
  y1 = evt.clientY;
}

function Layout_dragToolbarStop(){
  console.log("MouseUP");

  translation = $(LayoutDragging.targetParent).attr("transform").slice(10,-1).split(',');
  sx = Math.round(parseInt(translation[0])/10)*10;
  sy = Math.round(parseInt(translation[1])/10)*10;

  if(sx < 0){sx = 0;}
  if(sy < 0){sy = 0;}

  $(LayoutDragging.targetParent).attr("transform", "translate("+sx+","+sy+")");

  $(LayoutDragging.targetParent).toggleClass("dragging");

  $(LayoutDragging.target).unbind('mousemove',Layout_dragToolbarMove);
  $(LayoutDragging.target).unbind('mouseup',Layout_dragToolbarStop);

  $('body').unbind('mousemove',Layout_dragToolbarMove);
  $('body').unbind('mouseup',Layout_dragToolbarStop);
}

function Layout_KeyHandler(event){
  if(event.key == "r" || event.key == "R"){ //Rotating

    transform_str = LayoutDragging.parent.attr("transform").split(' ');

    rotation = parseInt(transform_str[1].slice(7,-1));
    if(event.shiftKey){ //CCW
      rotation -= 45;
    }else{ //CW
      rotation += 45;
    }
    LayoutDragging.parent.attr("transform", transform_str[0] + " rotate("+rotation+")");
  }
}

function Layout_createPart(evt){
  target = evt.target

  if(evt.which != 1){
    return false;
  }

  while(true){
    if($(target).is("g") && $(target).attr("class") == "tool"){
      break;
    }else if($(target).is("svg")){
      return;
    }else{
      target = target.parentNode;
    }
  }
  offset = $(target.parentNode.parentNode).offset();
  mouseloc = {"x":evt.clientX,"y":evt.clientY};

  xy = {"x":(mouseloc.x - offset.left),"y":(mouseloc.y - offset.top)};

  part = $(target).attr("part");
  console.log(xy);

  var box;
  var box_childs = [];
  var part_nr = 0;

  switch(part){
    case "RS": //Straight Rail
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"size":"1","transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0
      $.each(parts_list[part][1],function(index,element){
        box_childs[i_counter] = CreateSvgElement(element.element,element.attr);
        i_counter++;
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RS","length":1};
      break;
    case "RC":
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0
      $.each(parts_list[part][1],function(index,element){
        box_childs[i_counter] = CreateSvgElement(element.element,element.attr);
        i_counter++;
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RC","length":1,"radius":2};
      break;
    case "SwN":
      part_nr = $("#LayoutContainer g#Nodes").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      box_childs[0] = CreateSvgElement("path",{"d":"M 3.75,-7.5 v 15 l -7.5,-7.5 Z","style":"stroke-width:0;fill:#41b7dd;"});
      box_childs[1] = CreateSvgElement("circle",{"cx":"0","cy":"0","r":10,"style":"fill:rgba(100,100,100,0.1);cursor:move;"});

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"SwN","Connectors":2,"heading":180};
      break;
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer #Drawing').append(box);

  $('#LayoutContainer g#Drawing g').bind("mousedown",Layout_dragPart);

  //Add to list


  N_evt = {};
  N_evt.clientX = evt.clientX;
  N_evt.clientY = evt.clientY;
  N_evt.new = true;
  N_evt.target = $('#LayoutContainer g#Drawing line').get(0);

  if(N_evt.target == undefined){
    N_evt.target = $('#LayoutContainer g#Drawing path').get(0);
  }

  Layout_dragPart(N_evt);
}

function Layout_dragPart(evt){

  //If right mouse button????
  if(evt.which == 3){
    return false;
  }

  LayoutDragging.x1 = evt.clientX;
  LayoutDragging.y1 = evt.clientY;

  LayoutDragging.parent = $(evt.target.parentNode);


  if(evt.new == true){
    //Get from group and place in Drawing
    var element = LayoutDragging.parent.detach();
    $('#LayoutContainer #Drawing').append(element);

    evt.target = $('#LayoutContainer g#Drawing line').get(0);

    if(evt.target == undefined){
      evt.target = $('#LayoutContainer g#Drawing path').get(0);
    }

    LayoutDragging.parent = $(evt.target.parentNode);
  }

  LayoutDragging.target = $(evt.target);

  LayoutDragging.Box = $(evt.target.parentNode.parentNode);

  if(evt.new != true){
    Layout_deleteFAnchor();
  }

  LayoutDragging.Box.bind({
    mousemove : Layout_dragMove,
    mouseup   : Layout_dragEnd
  });
  $("body").bind({
    mousemove : Layout_dragMove,
    mouseup   : Layout_dragEnd,
    keypress  : Layout_KeyHandler
  });
}

function Layout_dragMove(evt){
  transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  translation = transfrom_str[0].slice(10,-1).split(',');
  sx = parseFloat(translation[0]);
  sy = parseFloat (translation[1]);
  rotation = parseInt(transfrom_str[1].slice(7,-1));

  LayoutDragging.parent.attr("transform", "translate(" + (sx + evt.clientX - LayoutDragging.x1) + "," + (sy + evt.clientY - LayoutDragging.y1) + ") rotate("+rotation+")");

  LayoutDragging.x1 = evt.clientX;
  LayoutDragging.y1 = evt.clientY;

  Layout_dragCheckDrop();
}

function Layout_dragEnd(){
  console.log("Layout MouseUP");

  transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  translation = transfrom_str[0].slice(10,-1).split(',');
  sx = parseFloat(translation[0]);
  sy = parseFloat(translation[1]);
  rotation = parseInt(transfrom_str[1].slice(7,-1));

  LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");

  //EditObj.Blocks[nr].Connector = sx;
  //Place object in corresponding group
  part_type = LayoutDragging.parent.attr('part');
  if(part_type == "RS" || part_type == "RC"){
      var element = LayoutDragging.parent.detach();
      $('#LayoutContainer #Rail').append(element);
  }
  else if(part_type == "SwN"){
    var element = LayoutDragging.parent.detach();
    $("#LayoutContainer #Nodes").append(element);
  }

  LayoutDragging.Box.unbind('mousemove',Layout_dragMove);
  LayoutDragging.Box.unbind('mouseup',Layout_dragEnd);
  //,'mouseup']);
  $('body').unbind('mousemove',Layout_dragMove);
  $('body').unbind('mouseup',Layout_dragEnd);
  $('body').unbind('keypress',Layout_KeyHandler);

  attached = false;

  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseInt(Anchor.element.attr("cx")),parseInt(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    OwnSideSign = 1;
    if(MySide == 2){
      OwnSideSign = -1;
    }

    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    set = -1;

    $.each(EditObj.Layout.FreeAnchors,function(index,element1){
      if(set == -1 && ((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && attached == false) ||
                      ((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 5 && attached == true)){
        if(element1.angle-180 != parseInt(Anchor.element.attr("_r")) && attached == false){
          JoinSide = parseInt(element1.element.get(0).className.baseVal.slice(6));
          JoinSideSign = 1;
          if(JoinSide == 2){
            JoinSideSign = 1;
          }
          rotation = (element1.angle - parseInt(Anchor.element.attr("_r")) + 180) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseInt(Anchor.element.attr("cx")),parseInt(Anchor.element.attr("cy")),rotation);
        }
        sx -= Anchor.loc.left - element1.x;
        sy -= Anchor.loc.top - element1.y;
        LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
        Anchor.element.get(0).style.fill = "green";
        element1.element.get(0).style.fill = "green";

        Anchor.loc = getRotatedPoint(sx,sy,parseInt(Anchor.element.attr("cx")),parseInt(Anchor.element.attr("cy")),rotation);
        set = index;

        attached = true;

        Anchor.element.attr("attached",element1.element.parent().attr("nr")+"_"+element1.element.attr("class").slice(6));
        element1.element.attr("attached",Anchor.element.parent().attr("nr")+"_"+MySide);
      }
    });
    if(set >= 0){
      EditObj.Layout.Anchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":rotation,"element":Anchor.element,"Join":element.Join});
      EditObj.Layout.FreeAnchors.splice(set,1);
    }else{
      Anchor.element.get(0).style.fill = "red";
    }
  });
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseInt(Anchor.element.attr("cx")),parseInt(Anchor.element.attr("cy")),rotation);

    if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
      Layout_createFAnchor(Anchor,rotation);
      //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
    }
  });
}

function Layout_deleteFAnchor(){
  transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  translation = transfrom_str[0].slice(10,-1).split(',');
  sx = parseFloat(translation[0]);
  sy = parseFloat(translation[1]);
  rotation = parseInt(transfrom_str[1].slice(7,-1));

  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseInt(Anchor.element.attr("cx")),parseInt(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    set = false;

    x = -1;

    $.each(EditObj.Layout.FreeAnchors,function(index,element1){
      if(x == -1 && element1.x == Anchor.loc.left && element1.y == Anchor.loc.top){
        x = index;
      }
    });
    if(x != -1){
      EditObj.Layout.FreeAnchors.splice(x,1);
    }
    if(Anchor.element.attr("attached") != "" && Anchor.element.attr("attached") != undefined){
      IDs = Anchor.element.attr("attached").split("_");
      element = $('g[nr='+IDs[0]+'] .Attach'+IDs[1]);
      x_y = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
      element.get(0).style.fill = "red";

      //Parent Rotation
      p_rotation = parseFloat(element.parent().attr("transform").split(" ")[1].slice(7,-1));
      EditObj.Layout.FreeAnchors.push({"x":x_y.left,"y":x_y.top,"angle":(p_rotation+parseInt(element.attr("_r")))%360,"element":element,"Join":-1});
      Anchor.element.attr("attached","");
    }
  });
  LayoutDragging.Dropable = Drop

}

function Layout_createFAnchor(Anchor,rotation){
  EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
}

function Layout_dragCheckDrop(){
  svgContainerOffset = $('#LayoutContainer svg').offset();
  Drop = 0;
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = Anchor.element.offset();

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    Anchor.loc.left -= svgContainerOffset.left;
    Anchor.loc.top -= svgContainerOffset.top;

    set = false;

    $.each(EditObj.Layout.FreeAnchors,function(index,element){
      if((Math.pow(Anchor.loc.left - element.x,2) + Math.pow(Anchor.loc.top - element.y,2)) < 100 && !set){
        if(Drop == 0){
          Anchor.element.get(0).style.fill = "green";
          element.element.get(0).style.fill = "green";
        }else{
          Anchor.element.get(0).style.fill = "orange";
          element.element.get(0).style.fill = "orange";
        }
        element.Join = MyJoin;
        set = true;
      }else{
        if(element.Join == MyJoin){
          element.element.get(0).style.fill = "red";
          console.log("I id:"+MyJoin+" reset the color of element\tx"+element.x+"\ty"+element.y);
          element.Join = -1;
        }
        if(!set){Anchor.element.get(0).style.fill = "red"};
      }
      if(set){
        Drop += 1;
      }
    });
  });
  LayoutDragging.Dropable = Drop
}

context = {};

function Layout_ContextMenu(evt){
  if($(evt.target).is("rect") && $(evt.target).attr("class") == "grid"){
    return true;
  }
  LayoutDragging.parent = $(evt.target.parentNode);
  $(evt.currentTarget).bind("mousedown",Layout_HideContextMenu);
  console.log($(evt.target.parentElement).attr("part"));
  context = {};
  context.options = [];
  context.parent = $(evt.target.parentElement)
  item_ID = parseInt(context.parent.attr("nr"));
  switch(context.parent.attr("part")){
    case "RS":
      context.name = "Straight Rail";
      context.options[0] = {"type":"in_nr","name":"ID","value":""};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
	  context.options[2] = {"type":"in_step","name":"Length","b_type":"length","value":EditObj.Layout.Setup.Rail[item_ID].length};
      break;
    case "RC":
      context.name = "Curved Rail";
      context.options[0] = {"type":"in_nr","name":"ID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Radius","value":EditObj.Layout.Setup.Rail[item_ID].radius};
      break;
  }

  box = $('#LayoutContextMenu');
  box.css("display","block");
    loc = $(evt.currentTarget).offset();
  box.css("left",(evt.pageX - loc.left)+'px');
  box.css("top",(evt.pageY - loc.top)+'px');

  //Header Name
  $('.header',box).html(context.name);

  //Erase previous Options
  $('.content',box).empty();

  plus_button = '<line x1="6" y1="10" x2="14" y2="10" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
				'<line x1="10" y1="6" x2="10" y2="14" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
				'<circle class="yplus" cx="10" cy="10" r="8" style="fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"/>'
  min_button = '<line x1="31" y1="10" x2="39" y2="10" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
				'<circle class="ymin" cx="35" cy="10" r="8" style="fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"/>';

  //Add each option
  $.each(context.options,function(index,element){
    if(element.type == "in_nr"){
      text = "<div class=\""+element.name+"\" style=\"width:100%;height:20px;float:left;\">"+element.name+":<input type=\"type\" value=\""+element.value+"\" style=\"float:right;width:100px;\"/></div>";
    }else if(element.type == "show"){
      text = "<div class=\""+element.name+"\" style=\"width:100%;height:20px;float:left;\">"+element.name+":<span style=\"float:right;margin-right:45px;font-weight:bold;\">"+element.value+"</span></div>";
    }else if(element.type == "in_step"){
	  text = '<div class="'+element.name+'" style="width:100%;height:20px;float:left;">'+element.name+
				':<span style="float:right;text-align:center;width:50%;font-weight:bold;position:relative">'+element.value+
				'<svg viewbox="25 0 20 20" class="min_button" type="'+element.b_type+'" style="width:20px;position:absolute;left:0px;">'+min_button+'</svg>'+
				'<svg viewbox="0  0 20 20" class="plus_button" type="'+element.b_type+'" style="width:20px;position:absolute;right:0px;">'+plus_button+'</svg></span></div>';
	}

    $('.content',box).append(text);

	if(element.type == "in_step"){
		$('.content  .min_button',box).bind('click',Layout_Setup_Change);
		$('.content .plus_button',box).bind('click',Layout_Setup_Change);
	}
  });

  /*
  <div class="id" style="width:100%;height:20px;float:left;">ID:<input type="text" style="float:right;width:100px;"/></div>
  <div class="size" style="width:100%;height:20px;float:left;">Size:<span style="float:right;margin-right:45px;font-weight:bold;">1</span></div>
  <div class="test" style="width:100%;height:20px;float:left;">Test:<span style="float:right;margin-right:45px;font-weight:bold;">2</span></div>
  */

  LayoutEdit.contextmenu = true;
  return false;
}

function Layout_HideContextMenu(evt){
  $(evt.currentTarget).unbind("mousedown",Layout_HideContextMenu);
  $('#LayoutContextMenu').css("display","none");
  return true;
}

function Layout_ClearAll(){
  $.each($('#LayoutContainer svg + g:not(#toolbar,#Rail,#Signals,#Nodes,#Drawing)'),function(index,element){
    element.remove();
  });
}

function Layout_Setup_Change(event){
  target = $(event.target);
  if($(event.target).is('circle') || $(event.target).is('line')){
    target = $(event.target.parentNode);
  }

  ID = parseInt(context.parent.attr("nr"));
  o_type = context.parent.attr("part");
  type = undefined;
  switch(o_type){
    case "RS":
    case "RC":
      type = "Rail";
      break;
    case "SwN":
      type = "Node"
      break;
  }

  if(type == "Node"){
    console.log(EditObj.Layout.Setup.Nodes[ID]);
  }else if(type == "Rail"){
    if(o_type == "RS" && target.attr("type") == 'length'){
      length = EditObj.Layout.Setup.Rail[ID].length;
      limits = parts_list.limits.RS;
      if(target.attr("class") == "min_button"){
        length = InBounds(--length,limits.length_min,limits.length_max);
      }else if(target.attr("class") == "plus_button"){
        length = InBounds(++length,limits.length_min,limits.length_max);
      }
      console.log(EditObj.Layout.Setup.Rail[ID]);
      Layout_deleteFAnchor();
      part = $("#LayoutContainer g#Rail g[nr="+ID+"]");
      part.empty();
      $.each(parts_list["RS"][length],function(index,element){
        part.get(0).appendChild(CreateSvgElement(element.element,element.attr));
      });
      EditObj.Layout.Setup.Rail[ID].length = length;

      $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
        Anchor = {"element":undefined,"loc":{}};
        Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
        Anchor.loc = getRotatedPoint(sx,sy,parseInt(Anchor.element.attr("cx")),parseInt(Anchor.element.attr("cy")),rotation);

        if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
          Layout_createFAnchor(Anchor,rotation);
          //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
        }
      });
    }
  }
}

function InBounds(x,min,max){
  if(x < min){
    return min;
  }else if(x > max){
    return max;
  }else{
    return x;
  }
}

var LayoutEdit = {"tooltype":"mouse","tool":undefined,"toolsize":"1x1","rot":0,"contextmenu":false};

EditObj.Layout.gridsize = {"x":42,"y":42};
EditObj.Layout.grid = [];

parts_list = {"RS":[],"Limits":{}};
parts_list.limits = {"RS":{},"RC":{}};
parts_list.limits.RS.length_max = 4;
parts_list.limits.RS.length_min = 1;

parts_list['RS'][1] = [
          {"element":"line","attr":{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke-width:6px;"}},
          {"element":"line","attr":{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":180,"cx":0,"cy":21,"r":3,"style":"fill:red;"}},
          {"element":"circle","attr":{"class":"Attach2","_r":  0,"cx":42,"cy":21,"r":3,"style":"fill:red;"}},
        ];
parts_list['RS'][2] = [
          {"element":"line","attr":{"x1":0,"y1":21,"x2":84,"y2":21,"style":"stroke-width:6px;"}},
          {"element":"line","attr":{"x1":0,"y1":21,"x2":84,"y2":21,"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":180,"cx":0,"cy":21,"r":3,"style":"fill:red;"}},
          {"element":"circle","attr":{"class":"Attach2","_r":  0,"cx":84,"cy":21,"r":3,"style":"fill:red;"}},
        ];
parts_list['RS'][3] = [
          {"element":"line","attr":{"x1":0,"y1":21,"x2":168,"y2":21,"style":"stroke-width:6px;"}},
          {"element":"line","attr":{"x1":0,"y1":21,"x2":168,"y2":21,"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":180,"cx":0,"cy":21,"r":3,"style":"fill:red;"}},
          {"element":"circle","attr":{"class":"Attach2","_r":  0,"cx":168,"cy":21,"r":3,"style":"fill:red;"}},
        ];
parts_list['RS'][4] = [
          {"element":"line","attr":{"x1":0,"y1":21,"x2":336,"y2":21,"style":"stroke-width:6px;"}},
          {"element":"line","attr":{"x1":0,"y1":21,"x2":336,"y2":21,"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":180,"cx":0,"cy":21,"r":3,"style":"fill:red;"}},
          {"element":"circle","attr":{"class":"Attach2","_r":  0,"cx":336,"cy":21,"r":3,"style":"fill:red;"}},
        ];

parts_list.limits.RC.length_max = 4;
parts_list.limits.RC.length_min = 1;
parts_list.limits.RC.radius_max = 4;
parts_list.limits.RC.radius_min = 1;

parts_list['RC'][1][1] = [
          {"element":"path","attr":{"d":"M 0,0 a 56.33,56.33 0,0,0 40,-26.5","style":"stroke-width:6px;"}},//a
          {"element":"path","attr":{"d":"M 0,0 a 56.33,56.33 0,0,0 40,-26.5","style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":180,"cx":0,"cy":0,"r":3,"style":"fill:red;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":-45,"cx":40"cy":26.5,"r":3,"style":"fill:red;"}}
        ];
parts_list['RC'][1][2] = [
          {"element":"path","attr":{"d":"M 0,21 a 56.33,56.33 0,0,0 40,-15.5","style":"stroke-width:6px;"}},
          {"element":"path","attr":{"d":"M 0,21 a 56.33,56.33 0,0,0 40,-15.5","style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":180,"cx":0,"cy":21,"r":3,"style":"fill:red;"}},
          {"element":"circle","attr":{"class":"Attach1","_r":-45,"cx":40"cy":5.5,"r":3,"style":"fill:red;"}}
        ];


/*
function Layout_selectTool(evt){
  var tool;
  if($(evt.target).is('svg')){
    tool = $(evt.target);
  }else{
    tool = $(evt.target.parentNode);
  }
  console.log(tool);
  console.log(tool.attr("tool"));
  $.each($('svg',tool.parent().parent()),function(index,element){
    $(element).removeClass("active");
  });
  tool.toggleClass("active");
  if(tool.attr("tool") != "tot"){
    LayoutEdit.tool = tool.attr("tool");
    if(typeof tool.attr("size") === undefined || tool.attr("size") == undefined){
      LayoutEdit.toolsize = "1x1";
    }else{
      LayoutEdit.toolsize = tool.attr("size");
    }
  }else{
    LayoutEdit.tooltype = tool.attr("type");
    if(LayoutEdit.tooltype == "edit"){
      $('#Layout #small_grid').css("display","block");
    }else{
      $('#Layout #small_grid').css("display","none");
    }
  }
}

function Layout_Click(evt){
  if(LayoutEdit.contextmenu == true){
    Layout_HideContextMenu();
    LayoutEdit.contextmenu = false;
    return;
  }

  if(LayoutEdit.tooltype == "mouse"){
    Layout_placeRail(evt);
  }
}

function scroll_Toolbox(evt){
  console.log(evt);
  console.log($(evt.target));
  element = $(evt.target);
  while(true){
    if(element.is("div") && element[0].id == "Toolitems"){
      break;
    }else if(element.is("body")){
      return false;
    }else{
      element = element.parent();
    }
  }
  console.log(element);
  var margin = parseInt(element.css("margin-left").slice(0,-2));
  margin -= evt.deltaY * 0.25;
  margin -= evt.deltaX * 0.25;
  if(margin > 0){
    margin = 0;
  }else if(margin < -(element.width() - element.parent().width())){
    margin = -(element.width() - element.parent().width());
  }
  element.css("margin-left",margin+"px")
  return false;
}

function Layout_placeRail(evt){
  //User must have selected a tool
  if(LayoutEdit.tool == undefined){
    return;
  }

  console.log("It Works!!!");
  console.log("X:"+evt.clientX+"\tY:"+evt.clientY);
  var pos = $(evt.target.parentNode).offset();
  console.log("dX:"+(evt.clientX-pos.left)+"\tdY"+(evt.clientY-pos.top));

  var gX = parseInt((evt.clientX-pos.left)/EditObj.Layout.gridsize.x);
  var gY = parseInt((evt.clientY-pos.top) /EditObj.Layout.gridsize.y);

  var lX = gX * EditObj.Layout.gridsize.x;
  var lY = gY * EditObj.Layout.gridsize.y;
  var rot = LayoutEdit.rot;

  if(evt.shiftKey && !evt.ctrlKey){
    Layout_Empty(gX,gY,lX,lY);
    return;
  }
  if(!evt.shiftKey && evt.ctrlKey){
    Layout_Rotate(gX,gY,1);
    return;
  }
  if(evt.shiftKey && evt.ctrlKey){
    Layout_Rotate(gX,gY,-1);
    return;
  }

  if(LayoutEdit.toolsize == "1x1"){
    $('#LayoutContainer .grid').on("mousemove",Layout_DrawRail);
    $('#LayoutContainer .grid').on("mouseup",Layout_drawRailUnbind);

    if(typeof EditObj.Layout.grid[gX] === 'undefined'){
      EditObj.Layout.grid[gX] = [];
    }
    if(typeof EditObj.Layout.grid[gX][gY] === 'undefined'){
      EditObj.Layout.grid[gX][gY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0}};
    }else{
      if(EditObj.Layout.grid[gX][gY].type != undefined){
        if(EditObj.Layout.grid[gX][gY].type != LayoutEdit.tool){
          //Remove current Item
          Layout_Empty(gX,gY,lX,lY);
        }else{
          return;
        }
      }
      EditObj.Layout.grid[gX][gY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0}};
    }

  }else{
    if(typeof EditObj.Layout.grid[gX] === 'undefined'){
      EditObj.Layout.grid[gX] = [];
    }
    if(typeof EditObj.Layout.grid[gX][gY] === 'undefined'){
      EditObj.Layout.grid[gX][gY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0},"parent":{"x":gX,"y":gY}};
    }else{
      if(EditObj.Layout.grid[gX][gY].type != undefined){
        if(EditObj.Layout.grid[gX][gY].type != LayoutEdit.tool){
          //Remove current Item
          Layout_Empty(gX,gY,gX * EditObj.Layout.gridsize.x,gY * EditObj.Layout.gridsize.y);
        }else{
          return;
        }
      }
      EditObj.Layout.grid[gX][gY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0},"parent":{"x":gX,"y":gY}};
    }

    var Size = LayoutEdit.toolsize.split("x");
    Size[0] = parseInt(Size[0]);
    Size[1] = parseInt(Size[1]);
    var iX = gX + Size[0] - 1;
    var iY = gY + Size[1] - 1;

    if(typeof EditObj.Layout.grid[iX] === 'undefined'){
      EditObj.Layout.grid[iX] = [];
    }
    if(typeof EditObj.Layout.grid[iX][iY] === 'undefined'){
      EditObj.Layout.grid[iX][iY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0},"parent":{"x":gX,"y":gY}};
    }else{
      if(EditObj.Layout.grid[iX][iY].type != undefined){
        if(EditObj.Layout.grid[iX][iY].type != LayoutEdit.tool){
          //Remove current Item
          Layout_Empty(iX,iY,iX * EditObj.Layout.gridsize.x,iY * EditObj.Layout.gridsize.y);
        }else{
          return;
        }
      }
      EditObj.Layout.grid[iX][iY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0},"parent":{"x":gX,"y":gY}};
    }
  }

  Layout_placeElement(gX,gY,lX,lY);
}

function Layout_DrawRail(evt){
  var pos = $(evt.target.parentNode).offset();

  var gX = parseInt((evt.clientX-pos.left)/EditObj.Layout.gridsize.x);
  var gY = parseInt((evt.clientY-pos.top) /EditObj.Layout.gridsize.y);

  var lX = gX * EditObj.Layout.gridsize.x;
  var lY = gY * EditObj.Layout.gridsize.y;
  var rot = LayoutEdit.rot;

  if(typeof EditObj.Layout.grid[gX] === 'undefined'){
    EditObj.Layout.grid[gX] = [];
  }
  if(typeof EditObj.Layout.grid[gX][gY] === 'undefined'){
    EditObj.Layout.grid[gX][gY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0}};
  }else{
    if(EditObj.Layout.grid[gX][gY].type != undefined){
      if(EditObj.Layout.grid[gX][gY].type != LayoutEdit.tool){
        //Remove current Item
        Layout_Empty(gX,gY,lX,lY);
      }else{
        return;
      }
    }
    EditObj.Layout.grid[gX][gY] = {"type":LayoutEdit.tool,"rotation":0,"size":LayoutEdit.toolsize,"id":{"block":0,"switch":0,"signal":0}};
  }

  Layout_placeElement(gX,gY,lX,lY);
}

function Layout_drawRailUnbind(evt){
  $('#LayoutContainer .grid').off("mousemove");
  $('#LayoutContainer .grid').off("mouseup");
}

function Layout_placeElement(gX,gY,lX,lY){
  switch(LayoutEdit.tool){
    case "straight":
      Layout_placeStraight(lX,lY,rot);
      break;
    case "End_of_Line":
      Layout_placeEnd_of_Line(lX,lY,rot);
      break;
    case "turn_small":
      Layout_placeCurveSmall(lX,lY,rot);
      break;
    case "turn_medium":
      Layout_placeCurveMedium(lX,lY,rot);
      break;
    case "turn_large":
      Layout_placeCurveLarge(lX,lY,rot);
      break;
    case "cross45":
      Layout_placeCross45(lX,lY,rot);
      break;
    case "cross90":
      Layout_placeCross90(lX,lY,rot);
      break;
    case "switch2R":
      Layout_placeSwitch2R(lX,lY,rot);
      break;
    case "switch2L":
      Layout_placeSwitch2L(lX,lY,rot);
      break;
    case "switch3":
      Layout_placeSwitch3(lX,lY,rot);
      break;
    case "switchW":
      Layout_placeSwitchWye(lX,lY,rot);
      break;
    case "switchSS":
      Layout_placeSwitchSingleSlip(lX,lY,rot);
      break;
    case "switchDS":
      Layout_placeSwitchDoubleSlip(lX,lY,rot);
      break;
    case "decoupler":
      Layout_placeDecoupler(lX,lY,rot);
      break;
    case "Clear":
      Layout_Clear(lX,lY);
      EditObj.Layout.grid[gX][gY] = {};
      break;
    default:
      var box = CreateSvgElement("g",{"transform":"translate("+lX+","+lY+")"});
      box_childs = CreateSvgElement("rect",{"x":0,"y":0,"width":42,"height":42,"style":"fill:grey"});

      box.appendChild(box_childs);

      $('#LayoutContainer svg .grid').before(box);
      break;
  }
}

function Layout_Rotate(gX,gY,dir){
  var lX = gX * EditObj.Layout.gridsize.x;
  var lY = gY * EditObj.Layout.gridsize.y;
  var rot = 0;

  if(typeof EditObj.Layout.grid[gX] === 'undefined' || typeof EditObj.Layout.grid[gX][gY] === 'undefined'){
    return;
  }
  rot  = (EditObj.Layout.grid[gX][gY].rotation + dir*45)%360;
  if(rot < 0){
    rot += 360;
  }
  EditObj.Layout.grid[gX][gY].rotation = rot;

  switch(EditObj.Layout.grid[gX][gY].type){
    case "straight":
      Layout_Clear(lX,lY);
      Layout_placeStraight(lX,lY,rot);
      break;
    case "cross45":
      Layout_Clear(lX,lY);
      Layout_placeCross45(lX,lY,rot);
      break;
    case "cross90":
      Layout_Clear(lX,lY);
      Layout_placeCross90(lX,lY,rot);
      break;
    case "End_of_Line":
      Layout_Clear(lX,lY);
      Layout_placeEnd_of_Line(lX,lY,rot);
      break;
    case "turn_small":
      Layout_Clear(lX,lY);
      Layout_placeCurveSmall(lX,lY,rot);
      break;
    case "turn_medium":
      Layout_Clear(lX,lY);
      Layout_placeCurveMedium(lX,lY,rot);
      break;
    case "switch2R":
      Layout_Clear(lX,lY);
      Layout_placeSwitch2R(lX,lY,rot);
      break;
    case "switch2L":
      Layout_Clear(lX,lY);
      Layout_placeSwitch2L(lX,lY,rot);
      break;
    case "switch3":
      Layout_Clear(lX,lY);
      Layout_placeSwitch3(lX,lY,rot);
      break;
    case "switchW":
      Layout_Clear(lX,lY);
      Layout_placeSwitchWye(lX,lY,rot);
      break;
    case "switchSS":
      Layout_Clear(lX,lY);
      Layout_placeSwitchSingleSlip(lX,lY,rot);
      break;
    case "switchDS":
      Layout_Clear(lX,lY);
      Layout_placeSwitchDoubleSlip(lX,lY,rot);
      break;
    case "decoupler":
      Layout_Clear(lX,lY);
      Layout_placeDecoupler(lX,lY,rot);
      break;
  }
}

function Layout_Clear(x,y){
  try{
    $('#LayoutContainer svg g[transform^="translate('+x+','+y+')"]')[0].remove();
  }
  catch(e){
    console.log(e);
  }
}

function Layout_Empty(gX,gY,x,y){
  if(typeof EditObj.Layout.grid[gX] === 'undefined' || typeof EditObj.Layout.grid[gX][gY] === 'undefined'){
    console.log("Nothing to delete");
    return false;
  }
  if(EditObj.Layout.grid[gX][gY].parent != undefined || EditObj.Layout.grid[gX][gY].size != "1x1"){
    loc = EditObj.Layout.grid[gX][gY].parent;
    size = EditObj.Layout.grid[loc.x][loc.y].size.split("x");
    EditObj.Layout.grid[loc.x][loc.y] = {};
    EditObj.Layout.grid[loc.x+parseInt(size[0])-1][loc.y+parseInt(size[1])-1] = {};
    Layout_Clear(loc.x*EditObj.Layout.gridsize.x,loc.y*EditObj.Layout.gridsize.y);
  }else{
    EditObj.Layout.grid[gX][gY] = {};
    Layout_Clear(x,y);
  }
}

function Layout_placeStraight(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs
  if(r == 0 || r == 180){
    box_childs = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
  }else if(r == 45 || r == 225){
    box_childs = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
  }else if(r == 90 || r == 270){
    box_childs = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
  }else if(r == 135 || r == 315){
    box_childs = CreateSvgElement("line",{"x1":0,"y1":42,"x2":42,"y2":0});
  }

  box.appendChild(box_childs);

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeEnd_of_Line(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":30,"y2":21});
    box_childs[1] = CreateSvgElement("line",{"x1":30,"y1":14,"x2":30,"y2":28,"style":"stroke-width:4px;"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":30,"y2":30});
    box_childs[1] = CreateSvgElement("line",{"x1":25.05,"y1":34.95,"x2":34.95,"y2":25.05,"style":"stroke-width:4px;"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":30});
    box_childs[1] = CreateSvgElement("line",{"x1":14,"y1":30,"x2":28,"y2":30,"style":"stroke-width:4px;"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":12,"y2":30});
    box_childs[1] = CreateSvgElement("line",{"x1":16.95,"y1":34.95,"x2":7.05,"y2":25.05,"style":"stroke-width:4px;"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":21,"x2":12,"y2":21});
    box_childs[1] = CreateSvgElement("line",{"x1":12,"y1":14,"x2":12,"y2":28,"style":"stroke-width:4px;"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":42,"x2":12,"y2":12});
    box_childs[1] = CreateSvgElement("line",{"x1":16.95,"y1":7.05,"x2":7.05,"y2":16.95,"style":"stroke-width:4px;"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":42,"x2":21,"y2":12});
    box_childs[1] = CreateSvgElement("line",{"x1":14,"y1":12,"x2":28,"y2":12,"style":"stroke-width:4px;"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":42,"x2":30,"y2":12});
    box_childs[1] = CreateSvgElement("line",{"x1":25.05,"y1":7.05,"x2":34.95,"y2":16.95,"style":"stroke-width:4px;"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeCurveSmall(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeCurveMedium(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];

  box_childs[0] = CreateSvgElement("rect",{"width":"42px","height":"42px","style":"fill:#ccc"});
  if(r == 0){
    box_childs[1] = CreateSvgElement("rect",{"x":"126px","y":"42px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 h 15.96,0 a215.04,215.04 0 0,1 152.04,63"});
  }else if(r == 45){
    box_childs[1] = CreateSvgElement("rect",{"x":"42px","y":"126px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M63,168 v -15.96,0 a215.04,215.04 0 0,0 -63,-152.04"});
  }else if(r == 90){
    box_childs[1] = CreateSvgElement("rect",{"x":"-42px","y":"126px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,0 v 15.96,0 a215.04,215.04 0 0,1 -63,152.04"});
  }else if(r == 135){
    box_childs[1] = CreateSvgElement("rect",{"x":"-126px","y":"42px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M-126,63 h 15.96,0 a215.04,215.04 0 0,0 152.04,-63"});
  }else if(r == 180){
    box_childs[1] = CreateSvgElement("rect",{"x":"-126px","y":"-42px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M42,21 h -15.96,0 a215.04,215.04 0 0,1 -152.04,-63"});
  }else if(r == 225){
    box_childs[1] = CreateSvgElement("rect",{"x":"-42px","y":"-126px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M-21,-126 v 15.96,0 a215.04,215.04 0 0,0 63,152.04"});
  }else if(r == 270){
    box_childs[1] = CreateSvgElement("rect",{"x":"42px","y":"-126px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,42 v -15.96,0 a215.04,215.04 0 0,1 63,-152.04"});
  }else if(r == 315){
    box_childs[1] = CreateSvgElement("rect",{"x":"126px","y":"-42px","width":"42px","height":"42px","style":"fill:#eee"});
    box_childs[2] = CreateSvgElement("path",{"d":"M168,-21 h -15.96,0 a215.04,215.04 0 0,0 -152.04,63"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeCross45(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  var box_childs
  if(r == 0 || r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
  }else if(r == 45 || r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
  }else if(r == 90 || r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
  }else if(r == 135 || r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeCross90(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if((r % 90) == 0){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":21,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
  }else if((r % 90) == 45){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":42,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeSwitch2L(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
  }



  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeSwitch2R(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }



  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeSwitch3(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
    box_childs[2] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeSwitchWye(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
    box_childs[1] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeSwitchSingleSlip(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
  }else if(r == 45){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
  }else if(r == 90){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
  }else if(r == 135){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
  }else if(r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
  }else if(r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeSwitchDoubleSlip(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 0 || r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,1 35.7,14.7 l 6.3,6.3"});
    box_childs[3] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,1 -35.7,-14.7 l -6.3,-6.3"});
  }else if(r == 45 || r == 225){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":0,"x2":42,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,0 -14.7,-35.7 l -6.3,-6.3"});
    box_childs[3] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,0 14.7,35.7 l 6.3,6.3"});
  }else if(r == 90 || r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M21,0 a50.82,50.82 0 0,1 -14.7,35.7 l -6.3,6.3"});
    box_childs[3] = CreateSvgElement("path",{"d":"M21,42 a50.82,50.82 0 0,1 14.7,-35.7 l 6.3,-6.3"});
  }else if(r == 135 || r == 315){
    box_childs[0] = CreateSvgElement("line",{"x1":42,"y1":0,"x2":0,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M0,21 a50.82,50.82 0 0,0 35.7,-14.7 l 6.3,-6.3"});
    box_childs[3] = CreateSvgElement("path",{"d":"M42,21 a50.82,50.82 0 0,0 -35.7,14.7 l -6.3,6.3"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}

function Layout_placeDecoupler(x,y,r){
  var box = CreateSvgElement("g",{"transform":"translate("+x+","+y+")"});
  var box_childs = [];
  if(r == 45 || r == 135 || r == 225 || r == 315){
    r += 45;
    r = r % 360;
    EditObj.Layout.grid[x/EditObj.Layout.gridsize.x][y/EditObj.Layout.gridsize.y].rotation = r;
  }
  if(r == 0 || r == 180){
    box_childs[0] = CreateSvgElement("line",{"x1":0,"y1":21,"x2":18,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":24,"y1":21,"x2":42,"y2":21,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M6,16 h 30 l -6,-4 h -18 Z","style":"fill:black;stroke-width:0;"});
    box_childs[3] = CreateSvgElement("path",{"d":"M6,26 h 30 l -6,4 h -18 Z","style":"fill:black;stroke-width:0;"});
  }else if(r == 90 || r == 270){
    box_childs[0] = CreateSvgElement("line",{"x1":21,"y1":0,"x2":21,"y2":18,"style":"stroke:black;stroke-width:6px;"});
    box_childs[1] = CreateSvgElement("line",{"x1":21,"y1":24,"x2":21,"y2":42,"style":"stroke:black;stroke-width:6px;"});
    box_childs[2] = CreateSvgElement("path",{"d":"M16,6 v 30 l -4,-6 v -18 Z","style":"fill:black;stroke-width:0;"});
    box_childs[3] = CreateSvgElement("path",{"d":"M26,6 v 30 l 4,-6 v -18 Z","style":"fill:black;stroke-width:0;"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer svg .grid').before(box);
}
*/
