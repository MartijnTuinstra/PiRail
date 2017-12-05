
LayoutDragging = {"x1":0,"y1":0,"target":undefined,"parent":undefined,"Box":undefined,
                  "Dropable":0,"Keys":[],"AC_En":false,"AC_Stage":0,"AC_Elements":[],"AC_Click":false};

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

    rotation = parseFloat(transform_str[1].slice(7,-1));
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

  if(LayoutDragging.AC_En && part != "CRail"){
    Layout_AC_Abort();
  }

  var box;
  var box_childs = [];
  var part_nr = 0;

  switch(part){
    case "RS": //Straight Rail
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"size":"1","transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      length = 1;
      steps = parts_list.RS.length_steps;

      $.each(parts_list.RS.part,function(index,element){
        part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RS","length":1};
      break;
    case "RC":
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0

      angle = 4;
      radius = 2;
      a_steps = parts_list.RC.angle_steps;
      r_steps = parts_list.RC.radius_steps;

      $.each(parts_list.RC.part,function(index,element){
        part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RC","angle":angle,"radius":radius};
      break;
    case "SwN":
      part_nr = $("#LayoutContainer g#Nodes").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      box_childs[0] = CreateSvgElement("path",{"d":"M 3.75,-7.5 v 15 l -7.5,-7.5 Z","style":"stroke-width:0;fill:#41b7dd;"});
      box_childs[1] = CreateSvgElement("circle",{"cx":"0","cy":"0","r":10,"style":"fill:rgba(100,100,100,0.1);cursor:move;"});

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"SwN","Connectors":2,"heading":180};
      break;
    case "CRail":
      if($('.tool_button',target).hasClass('active')){
        Layout_AC_Abort();
      }else{
        Layout_AC_Init();
      }
      return;
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

  //If right mouse button???? or if Auto_Complete is enabled
  if(evt.which == 3 || LayoutDragging.AC_En){
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
  rotation = parseFloat(transfrom_str[1].slice(7,-1));

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
  rotation = parseFloat(transfrom_str[1].slice(7,-1));

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
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

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
        if(element1.angle-180 != parseFloat(Anchor.element.attr("_r")) && attached == false){
          JoinSide = parseInt(element1.element.get(0).className.baseVal.slice(6));
          JoinSideSign = 1;
          if(JoinSide == 2){
            JoinSideSign = 1;
          }
          rotation = (element1.angle - parseFloat(Anchor.element.attr("_r")) + 180) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
        }
        sx -= Anchor.loc.left - element1.x;
        sy -= Anchor.loc.top - element1.y;
        LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
        Anchor.element.get(0).style.fill = "green";
        element1.element.get(0).style.fill = "green";

        Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
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
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
      Layout_createFAnchor(Anchor,rotation);
      //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
    }
  });
}

function Layout_deleteFAnchor(){
  Drop = 0;
  transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  translation = transfrom_str[0].slice(10,-1).split(',');
  sx = parseFloat(translation[0]);
  sy = parseFloat(translation[1]);
  rotation = parseFloat(transfrom_str[1].slice(7,-1));

  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

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
      EditObj.Layout.FreeAnchors.push({"x":x_y.left,"y":x_y.top,"angle":(p_rotation+parseFloat(element.attr("_r")))%360,"element":element,"Join":-1});
      Anchor.element.attr("attached","");
    }
  });
  LayoutDragging.Dropable = Drop

}

function Layout_createFAnchor(Anchor,rotation){
  EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseFloat(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
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
	    context.options[2] = {"type":"in_step","name":"Length","b_type":"length","value":"EditObj.Layout.Setup.Rail[item_ID].length"};
      break;
    case "RC":
      context.name = "Curved Rail";
      context.options[0] = {"type":"in_nr","name":"ID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Radius","b_type":"radius","value":"EditObj.Layout.Setup.Rail[item_ID].radius"};
      context.options[3] = {"type":"in_step","name":"Angle","b_type":"angle","value":"EditObj.Layout.Setup.Rail[item_ID].angle"};
      break;
  }

  box = $('#LayoutContextMenu');
  box.css("display","block");
    loc = $(evt.currentTarget).offset();
  box.css("left",(evt.pageX - loc.left + 10)+'px');
  box.css("top",(evt.pageY - loc.top + 10)+'px');

  //Header Name
  $('.header',box).html(context.name);

  Layout_ContextMenuRedrawOptions(box,context);
  

  /*
  <div class="id" style="width:100%;height:20px;float:left;">ID:<input type="text" style="float:right;width:100px;"/></div>
  <div class="size" style="width:100%;height:20px;float:left;">Size:<span style="float:right;margin-right:45px;font-weight:bold;">1</span></div>
  <div class="test" style="width:100%;height:20px;float:left;">Test:<span style="float:right;margin-right:45px;font-weight:bold;">2</span></div>
  */

  LayoutEdit.contextmenu = true;
  return false;
}

function Layout_ContextMenuRedrawOptions(box,context){
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
        ':<span style="float:right;text-align:center;width:50%;font-weight:bold;position:relative">'+eval(element.value)+
        '<svg viewbox="25 0 20 20" class="min_button" type="'+element.b_type+'" style="width:20px;position:absolute;left:0px;">'+min_button+'</svg>'+
        '<svg viewbox="0  0 20 20" class="plus_button" type="'+element.b_type+'" style="width:20px;position:absolute;right:0px;">'+plus_button+'</svg></span></div>';
    }

    $('.content',box).append(text);
  });
  $('.content  .min_button',box).bind('mousedown',Layout_Setup_Change);
  $('.content .plus_button',box).bind('mousedown',Layout_Setup_Change);
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
  
  target.unbind('mousedown',Layout_Setup_Change);

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
  }else if(o_type == "RS" && target.attr("type") == 'length'){
    length = EditObj.Layout.Setup.Rail[ID].length;
    steps  = parts_list.RS.length_steps
    limits = parts_list.RS.limits;
    if(target.attr("class") == "min_button"){
      length = InBounds(--length,limits.length_min,limits.length_max);
    }else if(target.attr("class") == "plus_button"){
      length = InBounds(++length,limits.length_min,limits.length_max);
    }
    console.log(EditObj.Layout.Setup.Rail[ID]);
    Layout_deleteFAnchor();
    part = $("#LayoutContainer g#Rail g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.RS.part,function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    EditObj.Layout.Setup.Rail[ID].length = length;

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createFAnchor(Anchor,rotation);
        //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
      }
    });
  }else if( o_type == "RC" && (target.attr("type") == 'angle' || target.attr("type") == "radius") ){
    angle   = EditObj.Layout.Setup.Rail[ID].angle;
    radius  = EditObj.Layout.Setup.Rail[ID].radius;
    r_steps = parts_list.RC.radius_steps;
    a_steps = parts_list.RC.angle_steps;
    limits  = parts_list.RC.limits;
    if(target.attr("class") == "min_button" && target.attr("type") == 'angle'){
      angle = InBounds(--angle,limits.angle_min[radius],limits.angle_max[radius]);
    }else if(target.attr("class") == "plus_button" && target.attr("type") == 'angle'){
      angle = InBounds(++angle,limits.angle_min[radius],limits.angle_max[radius]);
    }else if(target.attr("class") == "min_button" && target.attr("type") == 'radius'){
      radius = InBounds(--radius,limits.radius_min,limits.radius_max);
    }else if(target.attr("class") == "plus_button" && target.attr("type") == 'radius'){
      radius = InBounds(++radius,limits.radius_min,limits.radius_max);
    }
    angle = InBounds(angle,limits.angle_min[radius],limits.angle_max[radius]);

    EditObj.Layout.Setup.Rail[ID].angle = angle;
    EditObj.Layout.Setup.Rail[ID].radius = radius;

    console.log(EditObj.Layout.Setup.Rail[ID]);
    Layout_deleteFAnchor();
    part = $("#LayoutContainer g#Rail g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.RC.part,function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createFAnchor(Anchor,rotation);
        //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
      }
    });
  }


  setTimeout(
    Layout_ContextMenuRedrawOptions($('#LayoutContextMenu'),context),
    5000);

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

function Layout_AC_MouseMoveCheck(evt){
  Offset = $('#LayoutContainer svg').offset();
  evt.clientX;
  pos = {"x":(evt.pageX - Offset.left),"y":(evt.pageY - Offset.top)};
  enable_Click = false;
  $.each(EditObj.Layout.FreeAnchors,function(index,element){
    allreadyDone = false;
    $.each(LayoutDragging.AC_Elements,function(index2,element2){
      if(element == element2){
        allreadyDone = true;
      }
    })
    if((Math.pow(pos.x - element.x,2) + Math.pow(pos.y - element.y,2)) < 100 && !allreadyDone){
      enable_Click = true;
      element.element.css("fill","orange");
    }else if(!allreadyDone){
      element.element.css("fill","red");
    }
  });
  if(enable_Click && !LayoutDragging.AC_Click){
    $('#LayoutContainer svg').on("click",Layout_AC_MouseClick);
    LayoutDragging.AC_Click = true;
  }else if(!enable_Click && LayoutDragging.AC_Click){
    $('#LayoutContainer svg').off("click",Layout_AC_MouseClick);
    LayoutDragging.AC_Click = false;
  }
}

function Layout_AC_MouseClick(evt){
  set = false;
  $.each(EditObj.Layout.FreeAnchors,function(index,element){
    if((Math.pow(pos.x - element.x,2) + Math.pow(pos.y - element.y,2)) < 100 && !set){
      element.element.css("fill","green");
      LayoutDragging.AC_Elements.push(element);
      LayoutDragging.AC_Stage++;
      set = true;
    }
  });
  $('#LayoutContainer svg').off("click",Layout_AC_MouseClick);
  LayoutDragging.AC_Click = false;

  if(LayoutDragging.AC_Stage == 2){
    console.log("Start Auto Complete");
    Layout_AC_Run();
  }
}

function Layout_AC_Init(){
  LayoutDragging.AC_En = true;
  $.each(LayoutDragging.AC_Elements,function(index,element){
    $(element).css("fill","red");
  });
  $('#LayoutContainer svg').on("mousemove",Layout_AC_MouseMoveCheck);
  $('.tool_button','#LayoutContainer #toolbar').toggleClass('active');
  LayoutDragging.AC_En = true;
}

function Layout_AC_Abort(){
  $('#LayoutContainer svg').off("mousemove",Layout_AC_MouseMoveCheck);
  if(LayoutDragging.AC_Click){
    $('#LayoutContainer svg #Drawing').off("click",Layout_AC_MouseClick);
  }
  if(LayoutDragging.AC_Stage != 0){
    $.each(LayoutDragging.AC_Elements,function(index,element){
      element.element.css("fill","red");
    });
    LayoutDragging.AC_Elements = [];
    LayoutDragging.AC_Stage = 0;
  }
  $('.tool_button','#LayoutContainer #toolbar').toggleClass('active');
  LayoutDragging.AC_En = false;
}

function Layout_AC_Finish(){
  $('#LayoutContainer svg').off("mousemove",Layout_AC_MouseMoveCheck);
  if(LayoutDragging.AC_Click){
    $('#LayoutContainer svg #Drawing').off("click",Layout_AC_MouseClick);
  }
  if(LayoutDragging.AC_Stage != 0){
    $.each(LayoutDragging.AC_Elements,function(index,element){
      element.element.css("fill","red");
    });
    LayoutDragging.AC_Elements = [];
    LayoutDragging.AC_Stage = 0;
  }
}

function Layout_AC_Run(){
  points = LayoutDragging.AC_Elements;
  points[0].r = parseFloat(points[0].element.attr("_r")) + parseFloat(points[0].element.parent().attr("transform").split(' ')[1].slice(7,-1));
  points[1].r = parseFloat(points[1].element.attr("_r")) + parseFloat(points[1].element.parent().attr("transform").split(' ')[1].slice(7,-1));
  console.log("Point 1: {X:"+points[0].x+",Y:"+points[0].y+",R:"+points[0].r+"}");
  console.log("Point 2: {X:"+points[1].x+",Y:"+points[1].y+",R:"+points[1].r+"}");

  angle_diff = Math.abs(points[0].r - points[1].r) % 360;
  if(angle_diff >= 45 && angle_diff <= 315 && angle_diff != 180){
    console.log("Angle difference: "+((points[0].r - points[1].r) % 360));
    a = {"first":{},"second":{}};
    b = {"first":{},"second":{}};
    a.first.x = points[0].x;a.first.y = points[0].y;
    a.second.x = points[0].x + 5 * Math.cos(Math.radians(points[0].r));
    a.second.y = points[0].y + 5 * Math.sin(Math.radians(points[0].r));
    b.first.x = points[1].x;b.first.y = points[1].y;
    b.second.x = points[1].x + 5 * Math.cos(Math.radians(points[1].r));
    b.second.y = points[1].y + 5 * Math.sin(Math.radians(points[1].r));

    inter_pos = intersection2(a,b);

    console.log(b.second);

    line1 = Math.sqrt(Math.pow(a.first.x - inter_pos.x,2)+Math.pow(a.first.y - inter_pos.y,2));
    line2 = Math.sqrt(Math.pow(b.first.x - inter_pos.x,2)+Math.pow(b.first.y - inter_pos.y,2));

    if(line1 < line2){
      length = line2 - line1;
      node = {"first":{},"second":{}};
      node.first = {"x":points[1].x + length * Math.cos(Math.radians(points[1].r)),"y":points[1].y + length * Math.sin(Math.radians(points[1].r))};

      if(angle_diff < 180){
        node.second = {"x":node.first.x + 5 * Math.cos(Math.radians(points[1].r-90)),"y":node.first.y + 5 * Math.sin(Math.radians(points[1].r-90))};
        a.second.x = points[0].x + 5 * Math.cos(Math.radians(points[0].r-90));
        a.second.y = points[0].y + 5 * Math.sin(Math.radians(points[0].r-90));
      }else{
        node.second = {"x":node.first.x + 5 * Math.cos(Math.radians(points[1].r+90)),"y":node.first.y + 5 * Math.sin(Math.radians(points[1].r+90))};
        a.second.x = points[0].x + 5 * Math.cos(Math.radians(points[0].r+90));
        a.second.y = points[0].y + 5 * Math.sin(Math.radians(points[0].r+90));
      }

      console.log(b.second);
      circle_center = intersection2(node,a);

      radius = Math.sqrt(Math.pow(node.first.x - circle_center.x,2)+Math.pow(node.first.y - circle_center.y,2));

      //Create Custom Part
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+a.first.x+","+a.first.y+") rotate(0)"});
      box_childs = [];

      factor = 0;
      if(((points[0].r - points[1].r) % 360) < -180 || ((points[0].r - points[1].r) % 360) > 0){
        factor = 1;
      }


      box_childs[0] = CreateSvgElement("path",{"d":"M 0,0 a "+radius+","+radius+" 0,0,"+factor+" "+(node.first.x-a.first.x)+","+(node.first.y-a.first.y)+" l "+(b.first.x-node.first.x)+","+(b.first.y-node.first.y),"style":"stroke-width:6px;"});
      box_childs[1] = CreateSvgElement("path",{"d":"M 0,0 a "+radius+","+radius+" 0,0,"+factor+" "+(node.first.x-a.first.x)+","+(node.first.y-a.first.y)+" l "+(b.first.x-node.first.x)+","+(b.first.y-node.first.y),"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"});
      box_childs[2] = CreateSvgElement("circle",{"class":"Attach1","_r":points[0].r-180,"cx":0,"cy":0,"r":3,"style":"fill:green;"});
      box_childs[3] = CreateSvgElement("circle",{"class":"Attach2","_r":points[1].r-180,"cx":(b.first.x-node.first.x)+(node.first.x-a.first.x),"cy":(node.first.y-a.first.y)+(b.first.y-node.first.y),"r":3,"style":"fill:green;"});

      $.each(box_childs,function(index,element){
        box.appendChild(element);
      });

      $('#LayoutContainer #Drawing').append(box);

      $('#LayoutContainer g#Drawing g[nr='+part_nr+']').bind("mousedown",Layout_dragPart);
    }else{
      length = line1 - line2;
      node = {"first":{},"second":{}};
      node.first = {"x":points[0].x + length * Math.cos(Math.radians(points[0].r)),"y":points[0].y + length * Math.sin(Math.radians(points[0].r))};

      if(angle_diff < 180){
        node.second = {"x":node.first.x + 5 * Math.cos(Math.radians(points[0].r-90)),"y":node.first.y + 5 * Math.sin(Math.radians(points[0].r-90))};
        b.second.x = points[1].x + 5 * Math.cos(Math.radians(points[1].r+90));
        b.second.y = points[1].y + 5 * Math.sin(Math.radians(points[1].r+90));
      }else{
        node.second = {"x":node.first.x + 5 * Math.cos(Math.radians(points[0].r+90)),"y":node.first.y + 5 * Math.sin(Math.radians(points[0].r+90))};
        b.second.x = points[1].x + 5 * Math.cos(Math.radians(points[1].r-90));
        b.second.y = points[1].y + 5 * Math.sin(Math.radians(points[1].r-90));
      }

      console.log(b.second);
      circle_center = intersection2(node,b);

      radius = Math.sqrt(Math.pow(node.first.x - circle_center.x,2)+Math.pow(node.first.y - circle_center.y,2))

      //Create Custom Part
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+a.first.x+","+a.first.y+") rotate(0)"});
      box_childs = [];

      factor = 0;
      if(((points[0].r - points[1].r) % 360) > 180 || ((points[0].r - points[1].r) % 360) < 0){
        factor = 1;
      }

      box_childs[0] = CreateSvgElement("path",{"d":"M 0,0 l "+(node.first.x-a.first.x)+","+(node.first.y-a.first.y)+" a "+radius+","+radius+" 0,0,"+factor+" "+(b.first.x-node.first.x)+","+(b.first.y-node.first.y),"style":"stroke-width:6px;"});
      box_childs[1] = CreateSvgElement("path",{"d":"M 0,0 l "+(node.first.x-a.first.x)+","+(node.first.y-a.first.y)+" a "+radius+","+radius+" 0,0,"+factor+" "+(b.first.x-node.first.x)+","+(b.first.y-node.first.y),"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"});
      box_childs[2] = CreateSvgElement("circle",{"class":"Attach1","_r":points[0].r-180,"cx":0,"cy":0,"r":3,"style":"fill:green;"});
      box_childs[3] = CreateSvgElement("circle",{"class":"Attach2","_r":points[1].r-180,"cx":(b.first.x-node.first.x)+(node.first.x-a.first.x),"cy":(node.first.y-a.first.y)+(b.first.y-node.first.y),"r":3,"style":"fill:green;"});

      $.each(box_childs,function(index,element){
        box.appendChild(element);
      });

      $('#LayoutContainer #Drawing').append(box);

      $('#LayoutContainer g#Drawing g[nr='+part_nr+']').bind("mousedown",Layout_dragPart);
    }
    return Layout_AC_Finish();
  }else if(angle_diff == 180){
    console.log("Straight Line!! EASY");
  }else{
    alert("Failed\nTo Large Angle");
  }
  Layout_AC_Abort();
}

var LayoutEdit = {"tooltype":"mouse","tool":undefined,"toolsize":"1x1","rot":0,"contextmenu":false};

EditObj.Layout.gridsize = {"x":42,"y":42};
EditObj.Layout.grid = [];

parts_list = {};
parts_list.RS = {"length_steps":[],"limits":{},"part":[]};
parts_list.RS.limits.length_max = 4;
parts_list.RS.limits.length_min = 1;
parts_list.RS.length_steps = [undefined,42,82,162,322];

parts_list.RS.part = [
          {"element":"line","attr":{"x1":0,"y1":0,"x2":"steps[length]","y2":0,"style":"'stroke-width:6px;'"}},
          {"element":"line","attr":{"x1":0,"y1":0,"x2":"steps[length]","y2":0,"style":"'stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;'"}},
          {"element":"circle","attr":{"class":"'Attach1'","_r":180,"cx":0,"cy":0,"r":3,"style":"'fill:red;'"}},
          {"element":"circle","attr":{"class":"'Attach2'","_r":  0,"cx":"steps[length]","cy":0,"r":3,"style":"'fill:red;'"}}
        ];

parts_list.RC = {"angle_steps":[],"radius_steps":[],"limits":{},"part":[]};
parts_list.RC.angle_steps  = [undefined,Math.radians(11.25),Math.radians(22.5),Math.radians(45),Math.radians(67.5)];
parts_list.RC.radius_steps = [undefined,25,50,75,100,150];
parts_list.RC.limits.angle_max = [undefined,4,4,4,4,4];
parts_list.RC.limits.angle_min = [3,2,1,1];
parts_list.RC.limits.radius_max = 5;
parts_list.RC.limits.radius_min = 1;


parts_list.RC.part = [
          {"element":"path","attr":{"d":"'M 0,0 a '+r_steps[radius]+','+r_steps[radius]+' 0,0,0 '+Math.sin(a_steps[angle])*r_steps[radius]+','+r_steps[radius]*(Math.cos(a_steps[angle])-1)","style":"'stroke-width:6px;'"}},
          {"element":"path","attr":{"d":"'M 0,0 a '+r_steps[radius]+','+r_steps[radius]+' 0,0,0 '+Math.sin(a_steps[angle])*r_steps[radius]+','+r_steps[radius]*(Math.cos(a_steps[angle])-1)","style":"'stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;'"}},
          {"element":"circle","attr":{"class":"'Attach1'","_r":   180,"cx":    0,"cy":    0,"r":3,"style":"'fill:red;'"}},
          {"element":"circle","attr":{"class":"'Attach2'","_r":"-Math.degrees(a_steps[angle])","cx":"Math.sin(a_steps[angle])*r_steps[radius]","cy":"r_steps[radius]*(Math.cos(a_steps[angle])-1)","r":3,"style":"'fill:red;'"}}
        ];


vector = {};
vector.dot = function (u, v){
  return u.x * v.x + u.y * v.y;   
}

vector.norm2 = function (v){
  return v.x * v.x + v.y * v.y;
}

vector.norm = function (v){
  return sqrt(vector.norm2(v));
}

vector.cross = function (b, c) // cross product
{
  return {"x":(b.y * c.x - c.y * b.x),"y":(b.x * c.y - c.x * b.y)};
}

function line(p1, p2){
  A = (p.first.y - p.second.y);
  B = (p.second.x - p.first.x);
  C = -(p.first.x*p.second.y - p.second.x*p1.y);
  return [A, B, C];
}

function intersection3(L1, L2){
    D  = L1[0] * L2[1] - L1[1] * L2[0];
    Dx = L1[2] * L2[1] - L1[1] * L2[2];
    Dy = L1[0] * L2[2] - L1[2] * L2[0];
    if(D != 0){
      x = Dx / D;
      y = Dy / D;
      return [x,y];
    }else{
      return False;
    }
}

function intersection2(L1,L2){
  A1 = (L1.first.y - L1.second.y);
  B1 = (L1.second.x - L1.first.x);
  C1 = -(L1.first.x*L1.second.y - L1.second.x*L1.first.y);

  A2 = (L2.first.y - L2.second.y);
  B2 = (L2.second.x - L2.first.x);
  C2 = -(L2.first.x*L2.second.y - L2.second.x*L2.first.y);

  D  = A1 * B2 - B1 * A2;
  Dx = C1 * B2 - B1 * C2;
  Dy = A1 * C2 - C1 * A2;
  if(D != 0){
    x = Dx / D;
    y = Dy / D;
  }else{
    return False;
  }

  return {"x":x,"y":y};
}

function intersection(a, b)
// http://mathworld.wolfram.com/Line-LineIntersection.html
// in 3d; will also work in 2d if z components are 0
{
  da = {"x":(a.second.x - a.first.x),"y":(a.second.y - a.first.y)}; 
  db = {"x":(b.second.x - b.first.x),"y":(b.second.y - b.first.y)}; 
  ac = (da.y*a.first.x) + (da.x*a.first.y); 
  bc = (db.y*b.first.x) + (db.x*b.first.y);

  det = da.y*db.x - da.x*db.y;
  if(det != 0){
    x = (db.x*ac - da.x*bc)/det;
    y = (da.y*bc - db.y*ac)/det;
  }else{
    return {};
  }

  if(x < 0 || y < 0){
    console.log("No intersection of ray");
    return {};
  }

  return {"x":x,"y":y};
}
/*

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
