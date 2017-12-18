
var LayoutDragging = {"x1":0,"y1":0,"target":undefined,"parent":undefined,"Box":undefined,"part_settings":{"RC":{"radius":2,"angle":3},"RS":{"length":1},"Sig":{"type":0},"MAN":{"lenght":2}},
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

function Layout_Clear(){
  $('#LayoutContainer svg g#Rail').empty();
  $('#LayoutContainer svg g#Nodes').empty();
  $('#LayoutContainer svg g#Signals').empty();
  $('#LayoutContainer svg g#Drawing').empty();

  EditObj.Layout.Setup = {"Rail":[],"Nodes":[],"Signals":[]};
  EditObj.Layout.Anchors = [];
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
  var target = evt.target

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
  var offset = $(target.parentNode.parentNode).offset();
  var mouseloc = {"x":evt.clientX,"y":evt.clientY};
  var part_nr;
  var i_counter;
  var length,angle,radius,type;

  var xy = {"x":(mouseloc.x - offset.left),"y":(mouseloc.y - offset.top)};

  var part = $(target).attr("part");

  if(LayoutDragging.AC_En && part != "CRail"){
    Layout_AC_Abort();
  }

  var box;
  var box_childs = [];
  var part_nr = 0;

  switch(part){
    case "RS": //Straight Rail
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Rail[i] == undefined){
          part_nr = i;
          break;
        }
      }
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"size":"1","transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      length = LayoutDragging.part_settings.RS.length;
      var steps = parts_list.RS.length_steps;

      $.each(parts_list.RS.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RS","BlockID":-1,"length":length};
      break;
    case "RC":
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Rail[i] == undefined){
          part_nr = i;
          break;
        }
      }
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      angle = LayoutDragging.part_settings.RC.angle;
      radius = LayoutDragging.part_settings.RC.radius;
      var a_steps = parts_list.RC.angle_steps;
      var r_steps = parts_list.RC.radius_steps;

      $.each(parts_list.RC.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RC","BlockID":-1,"angle":angle,"radius":radius};
      break;
    case "SwN":
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Nodes[i] == undefined){
          part_nr = i;
          break;
        }
      }
      part_nr = $("#LayoutContainer g#Nodes").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      $.each(parts_list.SwN.part[0],function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"SwN","SwitchPID":-1,"SwitchNID":-1,"Nstates":2,"Pstates":1,"heading":180};
      break;
    case "CRail":
      if($('.tool_button',target).hasClass('active')){
        Layout_AC_Abort();
      }else{
        Layout_AC_Init();
      }
      return;
      break;
    case "BI":
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Nodes[i] == undefined){
          part_nr = i;
          break;
        }
      }
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      $.each(parts_list.BI.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"BI"};
      break;
    case "MAN":
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Nodes[i] == undefined){
          part_nr = i;
          break;
        }
      }
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      length = 2;

      $.each(parts_list.MAN.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });
      i = 0;
      for(i;i<(length-1);i++){
        number = i+2;
        part_attributes = {};
        $.each(parts_list.MAN.node[0].attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(parts_list.MAN.node[0].element,part_attributes);
        part_attributes = {};
        $.each(parts_list.MAN.node[1].attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(parts_list.MAN.node[1].element,part_attributes);
      }
      part_attributes = {};
      $.each(parts_list.MAN.mousebox,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      box_childs[i_counter++] = CreateSvgElement("rect",part_attributes);

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"MAN","length":2};
      break;
    case "Sig":
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Signals[i] == undefined){
          part_nr = i;
          break;
        }
      }
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      type = LayoutDragging.part_settings.Sig.type;

      $.each(parts_list.Sig.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });
      $.each(parts_list.Sig.types[type],function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Signals[part_nr] = {"type":"Sig","SignalID":-1,"type":0,"heading":180};
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#LayoutContainer #Drawing').append(box);
  $('#LayoutContainer g#Drawing g').bind("mousedown",Layout_dragPart);
  /*$.each(parts_list.Sig.types[type],function(index,element){
    $('#LayoutContainer #Drawing g.SignalBox').get(0).appendChild(CreateSvgElement(element.element,element.attr))
  });*/

  

  //Add to list


  var N_evt = {};
  N_evt.clientX = evt.clientX;
  N_evt.clientY = evt.clientY;
  N_evt.new = true;
  N_evt.target = $('#LayoutContainer g#Drawing line').get(0);

  if(N_evt.target == undefined){
    N_evt.target = $('#LayoutContainer g#Drawing path').get(0);
  }
  if(N_evt.target == undefined){
    N_evt.target = $('#LayoutContainer g#Drawing circle').get(0);
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
    if(evt.target == undefined){
      evt.target = $('#LayoutContainer g#Drawing circle').get(0);
    }

    LayoutDragging.parent = $(evt.target.parentNode);
  }

  LayoutDragging.target = $(evt.target);

  LayoutDragging.Box = $(evt.target.parentNode.parentNode);

  if(evt.new != true){
    Layout_deleteAnchor();
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
  var transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  var translation = transfrom_str[0].slice(10,-1).split(',');
  var sx = parseFloat(translation[0]);
  var sy = parseFloat (translation[1]);
  var rotation = parseFloat(transfrom_str[1].slice(7,-1));

  LayoutDragging.parent.attr("transform", "translate(" + (sx + evt.clientX - LayoutDragging.x1) + "," + (sy + evt.clientY - LayoutDragging.y1) + ") rotate("+rotation+")");

  LayoutDragging.x1 = evt.clientX;
  LayoutDragging.y1 = evt.clientY;

  Layout_dragCheckDrop();
}

function Layout_dragEnd(evt,place = false){
  console.log("Layout DragEnd");

  var transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  var translation = transfrom_str[0].slice(10,-1).split(',');
  var sx = parseFloat(translation[0]);
  var sy = parseFloat(translation[1]);
  var rotation = parseFloat(transfrom_str[1].slice(7,-1));

  //EditObj.Blocks[nr].Connector = sx;
  //Place object in corresponding group
  var part_type = LayoutDragging.parent.attr('part');
  if(part_type == "RS" || part_type == "RC"){
      var element = LayoutDragging.parent.detach();
      $('#LayoutContainer #Rail').append(element);
  }
  else if(part_type == "SwN" || part_type == "BI" || part_type == "MAN"){
    var element = LayoutDragging.parent.detach();
    $("#LayoutContainer #Nodes").append(element);
  }
  else if(part_type == "Sig"){
    var element = LayoutDragging.parent.detach();
    $("#LayoutContainer #Signals").append(element);
  }

  if(!place){
    LayoutDragging.Box.unbind('mousemove',Layout_dragMove);
    LayoutDragging.Box.unbind('mouseup',Layout_dragEnd);
    //,'mouseup']);
    $('body').unbind('mousemove',Layout_dragMove);
    $('body').unbind('mouseup',Layout_dragEnd);
    $('body').unbind('keypress',Layout_KeyHandler);
  }

  var attached = false;

  //Attach Rail
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element.className.baseVal.slice(6));
    var OwnSideSign = 1;
    if(MySide == 2){
      OwnSideSign = -1;
    }

    var MyID = parseInt(LayoutDragging.parent.attr("nr"));
    var MyJoin = MyID + "_" + MySide;

    var set = -1;

    if(set < 0){
      var for_each_counter = 0;
      var array_length = EditObj.Layout.Anchors.length;
      for(for_each_counter;for_each_counter<array_length;for_each_counter++){
        var element1 = EditObj.Layout.Anchors[for_each_counter];
		if(element1 == undefined){continue;}
        if((element1.prev.length < element1.prev_states || element1.next.length < element1.next_states) && 
            (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && set < 0){
          var a = "";
          r_rotation = element1.angle-rotation-parseFloat(Anchor.element.attr("_r")) % 360;
          if(place){
            if((r_rotation < 90 && r_rotation > -90) || r_rotation > 270 || r_rotation < -270){
              if(element1.prev.length < element1.prev_states){
                a = "prev";
              }else{
                a = "next";
              }
            }else if((r_rotation > 90 && r_rotation < 270) || (r_rotation > -270 && r_rotation < -90) && attached == false){
              if(element1.next.length < element1.next_states){
                a = "next";
              }else{
                a = "prev";
              }
            }
          }else{
            if((r_rotation < 90 && r_rotation > -90) || r_rotation > 270 || r_rotation < -270 && attached == false){
              if(element1.prev.length < element1.prev_states){
                a = "prev";
                rotation = (element1.angle - parseFloat(Anchor.element.attr("_r"))) % 360;
                LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
                Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
              }else{
                a = "next";
                rotation = (element1.angle - parseFloat(Anchor.element.attr("_r"))+180) % 360;
                LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
                Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);              
              }
            }else if((r_rotation > 90 && r_rotation < 270) || (r_rotation > -270 && r_rotation < -90) && attached == false){
              if(element1.next.length < element1.next_states){
                a = "next";
                rotation = (element1.angle - parseFloat(Anchor.element.attr("_r"))+180) % 360;
                LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
                Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);              
              }else{
                a = "prev";
                rotation = (element1.angle - parseFloat(Anchor.element.attr("_r"))) % 360;
                LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
                Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
              }
            }
          }
          sx -= Anchor.loc.left - element1.x;
          sy -= Anchor.loc.top - element1.y;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.element.get(0).style.fill = "green";
          Anchor.element.attr("connected",for_each_counter);
          set = for_each_counter;

          if(a == "prev"){
            $.each(element1.next,function(index2,element2){
              element2.get(0).style.fill = "green";
            });
            EditObj.Layout.Anchors[for_each_counter].prev.push(Anchor.element);
          }else{
            $.each(element1.prev,function(index2,element2){
              element2.get(0).style.fill = "green";
            });
            EditObj.Layout.Anchors[for_each_counter].next.push(Anchor.element);
          }
        }
        if(set < 0){Anchor.element.get(0).style.fill = "red"};
      }
    }
    if(set < 0){
      Anchor.element.get(0).style.fill = "red";
    }
  });
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element1.className.baseVal.slice(6));

    if(Anchor.element.attr("connected") == "" || Anchor.element.attr("connected") == undefined){ //Add when not connected
      Layout_createAnchor(Anchor,rotation);
      Anchor.element.attr("connected",EditObj.Layout.Anchors.length-1);
      console.log("Create New Anchor for "+Anchor.element.parent().attr("nr"));
    }
  });

  //Attach Signals
  $.each($('[class^=SigAttach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element.className.baseVal.slice(6));
    var MyID = parseInt(LayoutDragging.parent.attr("nr"));
    var MyJoin = MyID + "_" + MySide;

    var set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
	  if(element1 == undefined){return}
      if((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
        if(((rotation-90)-element1.angle <= 90 && (rotation-90)-element1.angle > -90) || ((rotation-90)-element1.angle >= 270 && (rotation-90)-element1.angle < -360) || ((rotation-90)-element1.angle >= -360 && (rotation-90)-element1.angle < -270)){
          rotation = (element1.angle - 270) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
        }else{
          rotation = (element1.angle - 90) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
        }
        sx -= Anchor.loc.left - element1.x;
        sy -= Anchor.loc.top - element1.y;
        LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
        Anchor.element.get(0).style.fill = "purple";

        Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
        set = index1;

        attached = true;

        if(element1.angle-90 == rotation || element1.angle+90 == rotation-180){
          EditObj.Layout.Anchors[index1].Signal.L = Anchor.element;
        }else if(element1.angle+90 == rotation || element1.angle-90 == rotation+180){
          EditObj.Layout.Anchors[index1].Signal.R = Anchor.element;
        }
        Anchor.element.attr("attached",index1);
      }
    });
  });

  //Attach Node
  $.each($('[class^=NodeAttach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MyID = parseInt(LayoutDragging.parent.attr("nr"));

    var set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
	  if(element1 == undefined){return}
      if((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
        if(((rotation)-element1.angle <= 90 && (rotation)-element1.angle > -90) || ((rotation)-element1.angle >= 270 && (rotation)-element1.angle < -360) || ((rotation)-element1.angle >= -360 && (rotation)-element1.angle < -270)){
          rotation = (element1.angle) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

          if(part_type == "SwN"){
            EditObj.Layout.Anchors[index1].next_states = EditObj.Layout.Setup.Nodes[MyID].Pstates;
            EditObj.Layout.Anchors[index1].prev_states = EditObj.Layout.Setup.Nodes[MyID].Nstates;
            EditObj.Layout.Setup.Nodes[MyID].heading = 180;
          }
        }else{
          rotation = (element1.angle - 180) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

          if(part_type == "SwN"){
            EditObj.Layout.Anchors[index1].next_states = EditObj.Layout.Setup.Nodes[MyID].Nstates;
            EditObj.Layout.Anchors[index1].prev_states = EditObj.Layout.Setup.Nodes[MyID].Pstates;
            EditObj.Layout.Setup.Nodes[MyID].heading = 0;
          }
        }
        sx -= Anchor.loc.left - element1.x;
        sy -= Anchor.loc.top - element1.y;
        LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
        Anchor.element.get(0).style.fill = "rgba(200,200,200,0)";

        EditObj.Layout.Setup.Nodes[Anchor.element.parent().attr("nr")].Anchor = index1;

        Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
        set = index1;

        attached = true;

        if(part_type == "BI"){
          EditObj.Layout.Anchors[index1].Node.BI = Anchor.element.parent().attr("nr");
        }else if(part_type == "SwN"){
          EditObj.Layout.Anchors[index1].Node.SwN = Anchor.element.parent().attr("nr");
          console.log("Sw heading: "+EditObj.Layout.Setup.Nodes[MyID].heading);
        }
        Anchor.element.attr("attached",index1);
      }
    });
  });

  console.log({"x":sx,"y":sy});
}

function Layout_deleteAnchor(){
  var Drop = 0;
  var transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  var translation = transfrom_str[0].slice(10,-1).split(',');
  var sx = parseFloat(translation[0]);
  var sy = parseFloat(translation[1]);
  var rotation = parseFloat(transfrom_str[1].slice(7,-1));

  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element.className.baseVal.slice(6));
    var MyID = parseInt(LayoutDragging.parent.attr("nr"));
    var MyJoin = MyID + "_" + MySide;

    var set = false;

    var x = -1;
    var yN = -1;
    var yP = -1;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
	  if(element1 == undefined){return}
      if(x == -1 && Anchor.element.attr("connected") == index1){
        x = index1;
        $.each(element1.next,function(indexN,elementN){
          console.log($(elementN.parentNode).attr("nr"));
          if(parseInt(elementN.parent().attr("nr")) == parseInt(Anchor.element.parent().attr("nr"))){
            yN = indexN;
          }
        });
        $.each(element1.prev,function(indexP,elementP){
          if(parseInt(elementP.parent().attr("nr")) == parseInt(Anchor.element.parent().attr("nr"))){
            yP = indexP;
          }
        });
      }
    });
    if(x != -1){
      if(yN != -1){
        EditObj.Layout.Anchors[x].next.splice(yN,1);
      }else{
        EditObj.Layout.Anchors[x].prev.splice(yP,1);
      }
      if(EditObj.Layout.Anchors[x].next.length == 0 && EditObj.Layout.Anchors[x].prev.length == 0){
        delete EditObj.Layout.Anchors[x];
      }else if(EditObj.Layout.Anchors[x].next.length == 0 && EditObj.Layout.Anchors[x].prev.length == 1){
        EditObj.Layout.Anchors[x].prev[0].get(0).style.fill = "red";
      }else if(EditObj.Layout.Anchors[x].next.length == 1 && EditObj.Layout.Anchors[x].prev.length == 0){
        EditObj.Layout.Anchors[x].next[0].get(0).style.fill = "red";
      }
      Anchor.element.attr("connected","");
    }
  });
  LayoutDragging.Dropable = Drop
}

function Layout_DeletePart(part){
  LayoutDragging.parent = part;

  Layout_deleteAnchor();

  if(part.parent().attr("id") == "Rail"){
    EditObj.Layout.Setup.Rail[parseInt(part.attr("nr"))] = undefined;
  }else if(part.parent().attr("id") == "Nodes"){
    EditObj.Layout.Setup.Nodes[parseInt(part.attr("nr"))] = undefined;
  }else if(part.parent().attr("id") == "Signals"){
    EditObj.Layout.Setup.Signals[parseInt(part.attr("nr"))] = undefined;
  }

  part.remove();
}

function Layout_createAnchor(Anchor,rotation){
    EditObj.Layout.Anchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseFloat(Anchor.element.attr("_r")))%360,
        "prev":[Anchor.element],"next":[],"prev_states":1,"next_states":1,"Signal":{"R":undefined,"L":undefined},"type":"RailNode","Node":{}});
	return EditObj.Layout.Anchors.length;
}

function Layout_dragCheckDrop(){
  var svgContainerOffset = $('#LayoutContainer svg').offset();


  var transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  var translation = transfrom_str[0].slice(10,-1).split(',');
  var sx = parseFloat(translation[0]);
  var sy = parseFloat(translation[1]);
  var rotation = parseFloat(transfrom_str[1].slice(7,-1));

  var Drop = 0;
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element.className.baseVal.slice(6));
    var MyID = parseInt(LayoutDragging.parent.attr("nr"));
    var MyJoin = MyID + "_" + MySide;

    var set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
	  	if(element1 == undefined){return}
      if((element1.prev.length < element1.prev_states || element1.next.length < element1.next_states) && 
          (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
        Anchor.element.get(0).style.fill = "green";
        if(EditObj.Layout.Anchors[index1].next.length == 0 && EditObj.Layout.Anchors[index1].prev.length == 1){
          EditObj.Layout.Anchors[index1].prev[0].get(0).style.fill = "green";
        }else if(EditObj.Layout.Anchors[index1].next.length == 1 && EditObj.Layout.Anchors[index1].prev.length == 0){
          EditObj.Layout.Anchors[index1].next[0].get(0).style.fill = "green";
        }
        set = true;
      }else if(EditObj.Layout.Anchors[index1].next.length == 0 && EditObj.Layout.Anchors[index1].prev.length == 1 && !set && EditObj.Layout.Anchors[index1].prev[0].get(0).style.fill != "red"){
        EditObj.Layout.Anchors[index1].prev[0].get(0).style.fill = "red";
      }else if(EditObj.Layout.Anchors[index1].next.length == 1 && EditObj.Layout.Anchors[index1].prev.length == 0 && !set && EditObj.Layout.Anchors[index1].next[0].get(0).style.fill != "red"){
        EditObj.Layout.Anchors[index1].next[0].get(0).style.fill = "red";
      }
      if(!set){
        Anchor.element.get(0).style.fill = "red";
      }
    });
  });
  $.each($('[class^=SigAttach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element.className.baseVal.slice(6));
    var MyID = parseInt(LayoutDragging.parent.attr("nr"));
    var MyJoin = MyID + "_" + MySide;

    var set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
	  if(element1 == undefined){return}
      if((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
        if(Drop == 0){
          Anchor.element.get(0).style.fill = "pink";
        }
        set = true;
      }else{
        if(!set){Anchor.element.get(0).style.fill = "purple"};
      }
      if(set){
        Drop += 1;
      }
    });
  });
  $.each($('[class^=NodeAttach]',LayoutDragging.parent),function(index,element){
    var Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    var MySide = parseInt(element.className.baseVal.slice(6));
    var MyID = parseInt(LayoutDragging.parent.attr("nr"));
    var MyJoin = MyID + "_" + MySide;

    var part = Anchor.element.parent().attr("part");

    var set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
	  if(element1 == undefined){return}
      if((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set
            && ((part == "BI" && element1.Node.BI == undefined) || (part == "SwN" && element1.Node.SwN == undefined))){
        if(Drop == 0){
          Anchor.element.get(0).style.fill = "rgba(0,128,0,0.4)";
        }
        set = true;
      }else{
        if(!set){Anchor.element.get(0).style.fill = "rgba(0,0,0,0)"};
      }
      if(set){
        Drop += 1;
      }
    });
  });
  LayoutDragging.Dropable = Drop
}

var context = {};

function Layout_ContextMenu(evt){
  if($(evt.target).is("rect") && $(evt.target).attr("class") == "grid"){
    return true;
  }
  LayoutDragging.parent = $(evt.target.parentNode);
  $(evt.currentTarget).bind("mousedown",Layout_HideContextMenu);
  console.log($(evt.target.parentElement).attr("part"));
  context = {};
  context.options = [];
  context.parent = $(evt.target.parentElement);
  var item_ID = parseInt(context.parent.attr("nr"));
  context.ID = item_ID;
  switch(context.parent.attr("part")){
    case "RS":
      context.name = "'Straight Rail'";
      context.options[0] = {"type":"in_nr","name":"ID","b_type":"RID","value":"EditObj.Layout.Setup.Rail[item_ID].BlockID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
	    context.options[2] = {"type":"in_step","name":"Length","b_type":"length","value":"Math.round(parts_list.RS.length_steps[EditObj.Layout.Setup.Rail[item_ID].length]*100)/100"};
      break;
    case "RC":
      context.name = "'Curved Rail'";
      context.options[0] = {"type":"in_nr","name":"ID","b_type":"RID","value":"EditObj.Layout.Setup.Rail[item_ID].BlockID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Radius","b_type":"radius","value":"Math.round(parts_list.RC.radius_steps[EditObj.Layout.Setup.Rail[item_ID].radius]*100)/100"};
      context.options[3] = {"type":"in_step","name":"Angle","b_type":"angle","value":"Math.round(Math.degrees(parts_list.RC.angle_steps[EditObj.Layout.Setup.Rail[item_ID].angle])*100)/100+'&deg;'"};
      break;
    case "CRail":
      context.name = "'Custom Rail'";
      context.options[0] = {"type":"in_nr","name":"ID","b_type":"RID","value":"EditObj.Layout.Setup.Rail[item_ID].BlockID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      break;
    case "SwN":
    case "BI":
      if(context.parent.attr("part") == "SwN" || (context.parent.attr("part") == "BI" && EditObj.Layout.Anchors[EditObj.Layout.Setup.Nodes[item_ID].Anchor].Node.SwN != undefined)){
        context.name = "'Switch Node'";
        if(context.parent.attr("part") == "BI"){
          item_ID = parseInt(EditObj.Layout.Anchors[EditObj.Layout.Setup.Nodes[item_ID].Anchor].Node.SwN);
        }
        var Anchor = EditObj.Layout.Anchors[EditObj.Layout.Setup.Nodes[item_ID].Anchor];

        var heading = EditObj.Layout.Setup.Nodes[item_ID].heading;

        context.options[0] = {"type":"show","name":"Own_ID","value":item_ID};
        if(Anchor.next.length == 1 && Anchor.prev.length > 1){
          context.options[1] = {"type":"in_nr","name":"ID","b_type":"SwPID","value":"EditObj.Layout.Setup.Nodes[item_ID].SwitchPID"};
        }else if(Anchor.prev.length == 1 && Anchor.next.length > 1){
          context.options[1] = {"type":"in_nr","name":"ID","b_type":"SwNID","value":"EditObj.Layout.Setup.Nodes[item_ID].SwitchNID"};
        }else{
		  if(heading < 180){
		    context.options[1] = {"type":"in_nr","name":"ID Forw.","b_type":"SwNID","value":"EditObj.Layout.Setup.Nodes[item_ID].SwitchNID"};
		    context.options[2] = {"type":"in_nr","name":"ID Backw.","b_type":"SwPID","value":"EditObj.Layout.Setup.Nodes[item_ID].SwitchPID"};
		  }else{
			context.options[1] = {"type":"in_nr","name":"ID Forw.","b_type":"SwPID","value":"EditObj.Layout.Setup.Nodes[item_ID].SwitchPID"};
		    context.options[2] = {"type":"in_nr","name":"ID Backw.","b_type":"SwNID","value":"EditObj.Layout.Setup.Nodes[item_ID].SwitchNID"};
		  }
        }

        context.options.push({"type":"in_step","name":"Forw. States","b_type":"Nstates","value":"EditObj.Layout.Setup.Nodes[item_ID].Nstates"});
        context.options.push({"type":"in_step","name":"Back. States","b_type":"Pstates","value":"EditObj.Layout.Setup.Nodes[item_ID].Pstates"});
        context.options.push({"type":"in_flip","name":"Flip","b_type":"flip","value":""});

        if(Anchor.next.length > 1){
          if(heading < 180){context.options.push({"type":"Sw_State","name":"Next States"});}
                       else{context.options.push({"type":"Sw_State","name":"Prev States"});}
          $.each(Anchor.next,function(index,element){
            context.options.push({"type":"Sw_show","name":("N"+index),"index":index,"part_nr":item_ID,"Anchor":EditObj.Layout.Setup.Nodes[item_ID].Anchor,"NAttach":index});
          });
        }
        if(Anchor.prev.length > 1){
          if(heading < 180){context.options.push({"type":"Sw_State","name":"Prev States"});}
                       else{context.options.push({"type":"Sw_State","name":"Next States"});}
          $.each(Anchor.prev,function(index,element){
            context.options.push({"type":"Sw_show","name":("P"+index),"index":index,"part_nr":item_ID,"Anchor":EditObj.Layout.Setup.Nodes[item_ID].Anchor,"PAttach":index});
          });
        }
      }else{
        context.name = "'Node'";
      }
      break;
    case "MAN":
      context.name = "'Module Attachment Node'";
      context.options[0] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[1] = {"type":"in_step","name":"Nr. of Nodes","b_type":"length","value":"EditObj.Layout.Setup.Nodes[item_ID].length"};
      break;
    case "Sig":
      context.name = "parts_list.Sig.types_name[EditObj.Layout.Setup.Signals[item_ID].type]";
      context.options[0] = {"type":"in_nr","name":"ID","value":"EditObj.Layout.Setup.Signals[item_ID].SignalID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Type","b_type":"type","value":"EditObj.Layout.Setup.Signals[item_ID].type"};
      break;
  }

  var box = $('#LayoutContextMenu');
  box.css("display","block");
  var loc = $(evt.currentTarget).offset();
  box.css("left",(evt.pageX - loc.left + 10)+'px');
  box.css("top",(evt.pageY - loc.top + 10)+'px');

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
  //Header Name
  $('.header',box).html(eval(context.name));

  //Erase previous Options
  $('.content',box).empty();

  var plus_button = '<line x1="6" y1="10" x2="14" y2="10" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
        '<line x1="10" y1="6" x2="10" y2="14" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
        '<circle class="yplus" cx="10" cy="10" r="8" style="fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"/>'
  var min_button = '<line x1="31" y1="10" x2="39" y2="10" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
        '<circle class="ymin" cx="35" cy="10" r="8" style="fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"/>';

  var flip_button = '<path d="M13,1V15a1,1,0,0,0,1.7.7l7-7a1,1,0,0,0,0-1.41l-7-7A1,1,0,0,0,13.3.3,1,1,0,0,0,13,1Zm0,0" style="stroke-width:0px;fill:black"/>'+
            '<path d="M9,15V1A1,1,0,0,0,8.7.3,1,1,0,0,0,7.3.3l-7,7A1,1,0,0,0,.3,8.7l7,7A1,1,0,0,0,9,15Zm0,0" style="stroke-width:0px;fill:black"/>'+
            '<rect x="0" y="0" width="22" height="16" style="fill:rgba(0,0,0,0);cursor:pointer"';

  var item_ID = context.ID;

  //Add each option
  $.each(context.options,function(index,element){
    var text = "";
    if(element.type == "in_nr"){
      text = "<div class=\""+element.name+"\" style=\"width:100%;height:20px;float:left;\">"+element.name+":<input type=\""+element.b_type+"\" value=\""+eval(element.value)+"\" style=\"float:right;width:100px;\"/></div>";
    }else if(element.type == "show"){
      text = "<div class=\""+element.name+"\" style=\"width:100%;height:20px;float:left;\">"+element.name+":<span style=\"float:right;margin-right:45px;font-weight:bold;\">"+element.value+"</span></div>";
    }else if(element.type == "in_step"){
    text = '<div class="'+element.name+'" style="width:100%;height:20px;float:left;">'+element.name+
        ':<span style="float:right;text-align:center;width:50%;font-weight:bold;position:relative">'+eval(element.value)+
        '<svg viewbox="25 0 20 20" class="min_button" type="'+element.b_type+'" style="width:20px;position:absolute;left:0px;">'+min_button+'</svg>'+
        '<svg viewbox="0  0 20 20" class="plus_button" type="'+element.b_type+'" style="width:20px;position:absolute;right:0px;">'+plus_button+'</svg></span></div>';
    }else if(element.type == "in_flip"){
    text = '<div class="'+element.name+'" style="width:100%;height:20px;float:left;">'+element.name+
        ':<span style="float:right;text-align:center;width:50%;font-weight:bold;position:relative">'+
        '<svg viewbox="0 0 22 16" class="flip_button" type="'+element.b_type+'" style="width:20px;margin:auto">'+flip_button+'</svg></span></div>';
    }else if(element.type == "Sw_State"){
      text = '<div class="'+element.name+'" style="width:100%;height:20px;margin-top:20px;float:left;text-align:center;"><b>'+element.name+'</b></div>';
    }else if(element.type == "Sw_show"){
      text = '<div class="'+element.name+'" style="width:100%;height:60px;float:left;">'+
	  '<span style="height:50px;line-height:50px;float:left;width:10px;margin: 5 0 5 10;text-align:center"><b>'+element.index+'</b></span>'+
	  '<svg width="80px" height="50px" viewbox="-40 -25 80 50" style="margin: 5 0 6 40;float:left;"></svg>'+
	  '<div style="height:40px;float:right;width:20px;margin: 10 10 10 0;">';
	  if(element.index != 0){
		text += '<span class="up_downButton" name="'+element.name+'" type="Sw_State_up" style="cursor:pointer;">▲</span>';
	  }
	  text += '<br/>';
	  if((element.name.startsWith("N") && element.index < (EditObj.Layout.Anchors[element.Anchor].next_states)-1) || (element.name.startsWith("P") && (element.index < EditObj.Layout.Anchors[element.Anchor].prev_states-1))){
		text += '<span class="up_downButton" name="'+element.name+'" type="Sw_State_down" style="cursor:pointer;">▼</span>';
	  }
	  text += '</div></div>';
    }

    $('.content',box).append(text);
  });
  $('.content  .min_button'  ,box).bind('mousedown',Layout_Setup_Change);
  $('.content .plus_button'  ,box).bind('mousedown',Layout_Setup_Change);
  $('.content .flip_button'  ,box).bind('mousedown',Layout_Setup_Change);
  $('.content .up_downButton',box).bind('mousedown',Layout_Setup_Change);
  $('.content input[type=RID]',box).bind('focusout',Layout_Assign_BlockNR);
  $('.content input[type=SwNID]',box).bind('focusout',Layout_Setup_Change);
  $('.content input[type=SwPID]',box).bind('focusout',Layout_Setup_Change);

  $.each(context.options,function(index,element){
    if(element.type == "Sw_show"){
      Anchor  = EditObj.Layout.Anchors[element.Anchor];

      $.each(Anchor.next,function(index,element2){
        if(index != element.NAttach){
          color = "#bbb";
          Layout_Context_CreateSwitchState(element,box,element2,color);
        }
      });
      $.each(Anchor.next,function(index,element2){
        if(index == element.NAttach){
          color = "#555";
          Layout_Context_CreateSwitchState(element,box,element2,color);
        }
      });

      $.each(Anchor.prev,function(index,element2){
        if(index != element.PAttach){
          color = "#bbb";
          Layout_Context_CreateSwitchState(element,box,element2,color);
        }
      });
      $.each(Anchor.prev,function(index,element2){
        if(index == element.PAttach){
          color = "#555";
          Layout_Context_CreateSwitchState(element,box,element2,color);
        }
      });


      heading = EditObj.Layout.Setup.Nodes[element.part_nr].heading;

      box2 = CreateSvgElement("g",{"transform":"translate(0,0) rotate("+(Anchor.angle+heading)+")"});

      box2.appendChild(CreateSvgElement("path",{"d":"M 0,-7.5 v 15 l 7.5,-7.5 Z","style":"stroke-width:0;fill:#41b7dd;"}));
      if(EditObj.Layout.Setup.Nodes[element.part_nr].Pstates > 1 && EditObj.Layout.Setup.Nodes[element.part_nr].Nstates > 1){
        box2.appendChild(CreateSvgElement("path",{"d":"M 0,-7.5 v 15 l -7.5,-7.5 Z","style":"stroke-width:0;fill:#ff61fa"}));
      }

      $('.content .'+element.name+' svg',box).append(box2);
    }
  });
}

function Layout_Context_CreateSwitchState(box_name,context_box,element2,color){
  part = element2.parent().attr("part");
  nr = parseInt(element2.parent().attr("nr"));

  length = EditObj.Layout.Setup.Rail[nr].length;
  angle  = EditObj.Layout.Setup.Rail[nr].angle;
  radius = EditObj.Layout.Setup.Rail[nr].radius;
  steps = parts_list.RS.length_steps;
  a_steps = parts_list.RC.angle_steps;
  r_steps = parts_list.RC.radius_steps;

  if(part == "CRail"){length = 1;part = "RS"};

  var transfrom_str = element2.parent().attr("transform").split(' ');

  var translation = transfrom_str[0].slice(10,-1).split(',');
  var sx = parseFloat(translation[0]);
  var sy = parseFloat (translation[1]);
  var rotation = parseFloat(transfrom_str[1].slice(7,-1));

  //var loc = getRotatedPoint(Anchor.x,Anchor.y,sx-Anchor.x,sy-Anchor.y,Anchor.angle)
  var x = sx - Anchor.x;
  var y = sy - Anchor.y;
  var r = rotation;

  box2 = CreateSvgElement("g",{"transform":"translate("+x+","+y+") rotate("+r+")"});
  child = undefined;

  var part_attributes = {};
  $.each(parts_list[part].part[0].attr,function(index4,element4){
    part_attributes[index4] = eval(element4);
  });
  part_attributes['style'] = "stroke-width:6px;stroke:"+color;
  child = CreateSvgElement(parts_list[part].part[0].element,part_attributes);

  box2.appendChild(child);

  $('.content .'+box_name.name+' svg',context_box).append(box2);
}

function Layout_HideContextMenu(evt = {"currentTarget":'#LayoutContainer svg'}){
  $(evt.currentTarget).unbind("mousedown",Layout_HideContextMenu);
  $('#LayoutContextMenu').css("display","none");
  return true;
}

function Find_Anchor_Point(g_element,attach = ""){
  var transform_str = g_element.attr("transform").split(' ');

  var translation = transform_str[0].slice(10,-1).split(',');
  var sx = parseFloat(translation[0]);
  var sy = parseFloat(translation[1]);
  var rotation = parseFloat(transform_str[1].slice(7,-1));

  var loc = {};
  if(attach != ""){
    loc = getRotatedPoint(sx,sy,parseFloat($('.'+attach,g_element).attr("cx")),parseFloat($('.'+attach,g_element).attr("cy")),rotation);
    sx += loc.left;
    sy += loc.top;
  }

  var ID = -1;

  $.each(EditObj.Layout.Anchors,function(index,element){
	if(element == undefined){return}
    if(sx == element.x && sy == element.y){
      ID = index;
    }
  });
  return ID;
}

function Layout_Setup_Change(event){
  var target = $(event.target);
  if($(event.target).is('circle') || $(event.target).is('line') || $(event.target).is('rect')){
    target = $(event.target.parentNode);
  }
  
  target.unbind('mousedown',Layout_Setup_Change);

  var ID = parseInt(context.parent.attr("nr"));
  var o_type = context.parent.attr("part");
  var type = undefined;

  if(o_type == "BI" && (target.attr("type") == "SwNID" || target.attr("type") == "SwPID")){
    o_type = "SwN";
    ID = parseInt(EditObj.Layout.Anchors[EditObj.Layout.Setup.Nodes[ID].Anchor].Node.SwN);
  }

  switch(o_type){
    case "RS":
    case "RC":
      type = "Rail";
      break;
    case "SwN":
      type = "Node"
      break;
  }

  if(o_type == "SwN"){
    var heading = EditObj.Layout.Setup.Nodes[ID].heading;

    var Nstates = EditObj.Layout.Setup.Nodes[ID].Nstates;
    var Pstates = EditObj.Layout.Setup.Nodes[ID].Pstates;

    var limits = parts_list.SwN.limits;
    
    var _ID = ID;
    var AnchorID = Find_Anchor_Point($('#LayoutContainer g#Nodes g[nr='+(ID)+']'));
    var Anchor = EditObj.Layout.Anchors[AnchorID];


    if(target.attr("class") == "min_button" && target.attr("type") == 'Nstates'){
      Nstates = InBounds(--Nstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "plus_button" && target.attr("type") == 'Nstates'){
      Nstates = InBounds(++Nstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "min_button" && target.attr("type") == 'Pstates'){
      Pstates = InBounds(--Pstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "plus_button" && target.attr("type") == 'Pstates'){
      Pstates = InBounds(++Pstates,limits.states_min,limits.states_max);
    }else if(target.attr("type") == "SwPID" && !isNaN(target.val())){
      EditObj.Layout.Setup.Nodes[_ID].SwitchPID = parseInt(target.val());
    }else if(target.attr("type") == "SwNID" && !isNaN(target.val())){
      EditObj.Layout.Setup.Nodes[_ID].SwitchNID = parseInt(target.val());
    }else if(target.attr("class") == "flip_button"){
      if(Anchor.prev.length > Anchor.next_states || Anchor.next.length > Anchor.prev_states){
        alert("To many connections, try to remove some.")
        return;
      }else{
        var temp = Anchor.next_states;
        Anchor.next_states = Anchor.prev_states;
        Anchor.prev_states = temp;

        temp = EditObj.Layout.Setup.Nodes[_ID].SwitchNID;
        EditObj.Layout.Setup.Nodes[_ID].SwitchNID = EditObj.Layout.Setup.Nodes[_ID].SwitchPID;
        EditObj.Layout.Setup.Nodes[_ID].SwitchPID = temp;

        if(heading < 180){
          heading += 180;
          transform_str = $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform").split(' ');

          rotation = parseFloat(transform_str[1].slice(7,-1)) + 180;
          $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform",transform_str[0]+" rotate("+rotation+")");
        }else{
          heading -= 180;
          transform_str = $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform").split(' ');

          rotation = parseFloat(transform_str[1].slice(7,-1)) - 180;
          $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform",transform_str[0]+" rotate("+rotation+")");
        }
        EditObj.Layout.Setup.Nodes[_ID].heading = heading;
        console.log("Sw heading: "+EditObj.Layout.Setup.Nodes[_ID].heading);
      }
    }else if(target.attr("class") == "up_downButton"){
		if(target.attr("type") == "Sw_State_up"){
			console.log("Move up");
			var side = target.attr("name").slice(0,1);
			var id = parseInt(target.attr("name").slice(1));
			
			if(side == "N"){
				var temp = Anchor.next[id-1];
				Anchor.next[id-1] = Anchor.next[id];
				Anchor.next[id] = temp;
			}else{
				var temp = Anchor.prev[id-1];
				Anchor.prev[id-1] = Anchor.prev[id];
				Anchor.prev[id] = temp;
			}
		}else{
			console.log("Move down");
			var side = target.attr("name").slice(0,1);
			var id = parseInt(target.attr("name").slice(1));
			
			if(side == "N"){
				var temp = Anchor.next[id+1];
				Anchor.next[id+1] = Anchor.next[id];
				Anchor.next[id] = temp;
			}else{
				var temp = Anchor.prev[id+1];
				Anchor.prev[id+1] = Anchor.prev[id];
				Anchor.prev[id] = temp;
			}
		}
	}

    if(heading < 180){
      EditObj.Layout.Anchors[AnchorID].next_states = Nstates;
      EditObj.Layout.Anchors[AnchorID].prev_states = Pstates;
    }else{
      EditObj.Layout.Anchors[AnchorID].next_states = Pstates;
      EditObj.Layout.Anchors[AnchorID].prev_states = Nstates;
    }

    var part = $("#LayoutContainer g#Nodes g[nr="+(_ID)+"]");
    part.empty();
    var type = 0;
    if(Pstates > 1 && Nstates > 1){
      type = 1;
    }
    $.each(parts_list.SwN.part[type],function(index,element){
      var part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });

    EditObj.Layout.Setup.Nodes[_ID].Nstates = Nstates;
    EditObj.Layout.Setup.Nodes[_ID].Pstates = Pstates;
    console.log(EditObj.Layout.Setup.Nodes[_ID]);
  }else if(o_type == "MAN" && target.attr("type") == 'length'){
    var length = EditObj.Layout.Setup.Nodes[ID].length;
    var limits = parts_list.MAN.limits;
    if(target.attr("class") == "min_button"){
      length = InBounds(--length,limits.nodes_min,limits.nodes_max);
    }else if(target.attr("class") == "plus_button"){
      length = InBounds(++length,limits.nodes_min,limits.nodes_max);
    }
    console.log(EditObj.Layout.Setup.Nodes[ID]);
    Layout_deleteAnchor();
    var part = $("#LayoutContainer g#Nodes g[nr="+ID+"]");
    part.empty();

    $.each(parts_list.MAN.part,function(index,element){
      var part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    i = 0;
    for(i;i<(length-1);i++){
      var number = i+2;
      var part_attributes = {};
      $.each(parts_list.MAN.node[0].attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(parts_list.MAN.node[0].element,part_attributes));
      var part_attributes = {};
      $.each(parts_list.MAN.node[1].attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(parts_list.MAN.node[1].element,part_attributes));
    }
    var part_attributes = {};
    $.each(parts_list.MAN.mousebox,function(index2,element2){
      part_attributes[index2] = eval(element2);
    });
    part.get(0).appendChild(CreateSvgElement("rect",part_attributes));


    LayoutDragging.part_settings.MAN.length = length;
    EditObj.Layout.Setup.Nodes[ID].length = length;

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      var Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createAnchor(Anchor,rotation);
        //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
      }
    });
  }else if(o_type == "Sig"){
    var type = EditObj.Layout.Setup.Signals[ID].type;
    var limits = parts_list.SwN.limits;
    if(target.attr("class") == "min_button"){
      type = InBounds(--type,limits.type_max,limits.type_min);
    }else if(target.attr("class") == "plus_button"){
      type = InBounds(++type,limits.type_max,limits.type_min);
    }

    EditObj.Layout.Setup.Signals[ID].type = type;
    LayoutDragging.part_settings.Sig.type = type;
    console.log(EditObj.Layout.Setup.Signals[ID]);

    var part = $("#LayoutContainer g#Signals g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.Sig.part,function(index,element){
      var part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    $.each(parts_list.Sig.types[type],function(index,element){
      var part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });

  }else if(o_type == "RS" && target.attr("type") == 'length'){
    var length = EditObj.Layout.Setup.Rail[ID].length;
    var steps  = parts_list.RS.length_steps
    var limits = parts_list.RS.limits;
    if(target.attr("class") == "min_button"){
      length = InBounds(--length,limits.length_min,limits.length_max);
    }else if(target.attr("class") == "plus_button"){
      length = InBounds(++length,limits.length_min,limits.length_max);
    }
    console.log(EditObj.Layout.Setup.Rail[ID]);
    Layout_deleteAnchor();
    var part = $("#LayoutContainer g#Rail g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.RS.part,function(index,element){
      var part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    LayoutDragging.part_settings.RS.length = length;
    EditObj.Layout.Setup.Rail[ID].length = length;

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      var Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createAnchor(Anchor,rotation);
        //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
      }
    });
  }else if( o_type == "RC" && (target.attr("type") == 'angle' || target.attr("type") == "radius") ){
    var angle   = EditObj.Layout.Setup.Rail[ID].angle;
    var radius  = EditObj.Layout.Setup.Rail[ID].radius;
    var r_steps = parts_list.RC.radius_steps;
    var a_steps = parts_list.RC.angle_steps;
    var limits  = parts_list.RC.limits;
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

    LayoutDragging.part_settings.RC.angle = angle;
    LayoutDragging.part_settings.RC.radius = radius;

    EditObj.Layout.Setup.Rail[ID].angle = angle;
    EditObj.Layout.Setup.Rail[ID].radius = radius;

    console.log(EditObj.Layout.Setup.Rail[ID]);
    Layout_deleteAnchor();
    var part = $("#LayoutContainer g#Rail g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.RC.part,function(index,element){
      var part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });

    var transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

    var translation = transfrom_str[0].slice(10,-1).split(',');
    var sx = parseFloat(translation[0]);
    var sy = parseFloat(translation[1]);
    var rotation = parseFloat(transfrom_str[1].slice(7,-1));

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      var Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createAnchor(Anchor,rotation);
        //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
      }
    });
  }


  setTimeout(
    Layout_ContextMenuRedrawOptions($('#LayoutContextMenu'),context),
    5000);
}

function Layout_Anchor_partnr(Anchor_side,id){
  var r_value = 0;
  $.each(Anchor_side,function(index,element){
    part_nr = parseInt(element.parent().attr("nr"));
    if(EditObj.Layout.Setup.Rail[part_nr].BlockID == id){
      r_value++;
    }
  });
  return r_value;
}

function Layout_Assign_BlockNR(evt = undefined,org_element = undefined,id = -1,nest = 0){
  console.log("Lost Focus");
  console.log(evt);

  if(nest > 10){
    return false;
  }

  if(evt != undefined){
    id = parseInt(evt.target.value);
    org_element = LayoutDragging.parent;
  }

  var iterations = 0;

  
  for(var reverse_counter = 0;reverse_counter<2;reverse_counter++){
    var value = false;
    var element = org_element;
    EditObj.Layout.Setup.Rail[$(org_element).attr("nr")].BlockID = -1;
    var direction = true;
    var reverse = false;
    if(reverse_counter == 1){
      console.log("REVERSE");
      direction = true;reverse = true
    }else{
      console.log("FORWARD");
    }
    while(!value){
      var part_nr = $(element).attr("nr");
      console.log([part_nr,iterations,direction,reverse,element]);
      if(EditObj.Layout.Setup.Rail[part_nr].BlockID == id){
        value = true;
        break;
      }

      EditObj.Layout.Setup.Rail[part_nr].BlockID = id;

      if((!reverse && direction) || (reverse && !direction)){
        Anchor = EditObj.Layout.Anchors[parseInt($('.Attach1',element).attr("connected"))];

        if(Layout_Anchor_partnr(Anchor.prev,id) > 0 && Anchor.Node.BI == undefined){
          if(Anchor.next.length == 1){
            new_element = Anchor.next[0].parent();
            if(Anchor.next[0].attr("class") == "Attach1"){
              if(!reverse){direction = false}else{direction = true};
            }else{
              if(!reverse){direction = true}else{direction = false};
            }
          }else if(Anchor.next.length > 1){
            $.each(Anchor.next,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
              }
            });
            value = true;
          }else{
            value = true;
          }
          if(Anchor.prev.length > 1 && Layout_Anchor_partnr(Anchor.prev,id) != Anchor.prev.length){
            $.each(Anchor.prev,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
              }
            });
          }
        }else if(Layout_Anchor_partnr(Anchor.next,id) > 0 && Anchor.Node.BI == undefined){
          if(Anchor.prev.length == 1){
            new_element = Anchor.prev[0].parent();
            if(Anchor.prev[0].attr("class") == "Attach1"){
              if(!reverse){direction = false}else{direction = true};
            }else{
              if(!reverse){direction = true}else{direction = false};
            }
          }else if(Anchor.prev.length > 1){
            $.each(Anchor.prev,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
              }
            });
            value = true;
          }else{
            value = true;
          }
          if(Anchor.next.length > 1 && Layout_Anchor_partnr(Anchor.next,id) != Anchor.next.length){
            $.each(Anchor.next,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest);
              }
            });
          }
        }else if(Layout_Anchor_partnr(Anchor.prev,id) > 0 && Anchor.prev.length > 1 && Anchor.Node.BI != undefined){
          $.each(Anchor.prev,function(index,element2){
            if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
              Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
            }
          });
          value = true;
        }else if(Layout_Anchor_partnr(Anchor.next,id) > 0 && Anchor.next.length > 1 && Anchor.Node.BI != undefined){
          $.each(Anchor.next,function(index,element2){
            if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
              Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
            }
          });
          value = true;
        }else{
          value = true;
          break;
        }
      }else{
        Anchor = EditObj.Layout.Anchors[parseInt($('.Attach2',element).attr("connected"))];

        if(Layout_Anchor_partnr(Anchor.next,id) > 0 && Anchor.Node.BI == undefined){
          if(Anchor.prev.length == 1){
            new_element = Anchor.prev[0].parent();
            if(Anchor.prev[0].attr("class") == "Attach1"){
              if(!reverse){direction = false}else{direction = true};
            }else{
              if(!reverse){direction = true}else{direction = false};
            }
          }else if(Anchor.prev.length > 1){
            $.each(Anchor.prev,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
              }
            });
            value = true;
          }else{
            value = true;
          }
          if(Anchor.next.length > 1 && Layout_Anchor_partnr(Anchor.next,id) != Anchor.next.length){
            $.each(Anchor.prev,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest);
              }
            });
            value = true;
          }
        }else if(Layout_Anchor_partnr(Anchor.prev,id) > 0 && Anchor.Node.BI == undefined){
          if(Anchor.next.length == 1){ //A neighbour
            new_element = Anchor.next[0].parent();
            if(Anchor.next[0].attr("class") == "Attach1"){
              if(!reverse){direction = false}else{direction = true};
            }else{
              if(!reverse){direction = true}else{direction = false};
            }
          }else if(Anchor.next.length > 1){ //More than one neighbour
            $.each(Anchor.next,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
              }
            });
            value = true;
          }else{ //No neighbor
            value = true;
          }
          if(Anchor.prev.length > 1 && Layout_Anchor_partnr(Anchor.prev,id) != Anchor.prev.length){
            $.each(Anchor.prev,function(index,element2){
              if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
                Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest);
              }
            });
            value = true;
          }
        }else if(parseInt(Anchor.prev[0].parent().attr("nr")) == part_nr && Anchor.prev.length > 1 && Anchor.Node.BI != undefined){
          $.each(Anchor.prev,function(index,element2){
            if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
              Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
            }
          });
          value = true;
        }else if(parseInt(Anchor.next[0].parent().attr("nr")) == part_nr && Anchor.next.length > 1 && Anchor.Node.BI != undefined){
          $.each(Anchor.next,function(index,element2){
            if(EditObj.Layout.Setup.Rail[$(element2).parent().attr("nr")].BlockID != id){
              Layout_Assign_BlockNR(undefined,element2.parent(),id,++nest)
            }
          });
          value = true;
        }else{
          value = true;
          break;
        }
      }
      if(!value){element = new_element};
      iterations++;
      if(iterations > 5000){
        value = true;
      }
      console.log(iterations);
    }
  }
  console.log("Stepping out");
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
  var Offset = $('#LayoutContainer svg').offset();
  evt.clientX;
  var pos = {"x":(evt.pageX - Offset.left),"y":(evt.pageY - Offset.top)};
  var enable_Click = false;
  $.each(EditObj.Layout.Anchors,function(index,element){
	if(element == undefined){return}
    var allreadyDone = false;
    $.each(LayoutDragging.AC_Elements,function(index2,element2){
      if(element == element2){
        allreadyDone = true;
      }
    })
    if(element.prev.length + element.next.length <= 1){
      if((Math.pow(pos.x - element.x,2) + Math.pow(pos.y - element.y,2)) < 100 && !allreadyDone){
      enable_Click = true;
        if(element.prev.length == 1){
          element.prev[0].css("fill","orange");
        }else{
          element.next[0].css("fill","orange");
        }
      }else if(!allreadyDone){
        if(element.prev.length == 1){
          element.prev[0].css("fill","red");
        }else{
          element.next[0].css("fill","red");
        }
      }
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
  var Offset = $('#LayoutContainer svg').offset();
  var pos = {"x":(evt.pageX - Offset.left),"y":(evt.pageY - Offset.top)};
  var set = false;
  $.each(EditObj.Layout.Anchors,function(index,element){
	if(element == undefined){return}
    if((Math.pow(pos.x - element.x,2) + Math.pow(pos.y - element.y,2)) < 100 && !set){
      if(element.prev.length == 1){
        element.prev[0].css("fill","green");
        LayoutDragging.AC_Elements.push({"element":element.prev[0],"x":element.x,"y":element.y});
      }else{
        element.next[0].css("fill","green");
        LayoutDragging.AC_Elements.push({"element":element.next[0],"x":element.x,"y":element.y});
      }
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
  var points = LayoutDragging.AC_Elements;
  points[0].r = parseFloat(points[0].element.attr("_r")) + parseFloat(points[0].element.parent().attr("transform").split(' ')[1].slice(7,-1));
  points[1].r = parseFloat(points[1].element.attr("_r")) + parseFloat(points[1].element.parent().attr("transform").split(' ')[1].slice(7,-1));
  console.log("Point 1: {X:"+points[0].x+",Y:"+points[0].y+",R:"+points[0].r+"}");
  console.log("Point 2: {X:"+points[1].x+",Y:"+points[1].y+",R:"+points[1].r+"}");

  var angle_diff = Math.abs(points[0].r - points[1].r) % 360;
  if(angle_diff >= 45 && angle_diff <= 315 && angle_diff != 180){
    console.log("Angle difference: "+((points[0].r - points[1].r - 180) % 360));
    var a = {"first":{},"second":{}};
    var b = {"first":{},"second":{}};
    a.first.x = points[0].x;a.first.y = points[0].y;
    a.second.x = points[0].x + 5 * Math.cos(Math.radians(points[0].r));
    a.second.y = points[0].y + 5 * Math.sin(Math.radians(points[0].r));
    b.first.x = points[1].x;b.first.y = points[1].y;
    b.second.x = points[1].x + 5 * Math.cos(Math.radians(points[1].r));
    b.second.y = points[1].y + 5 * Math.sin(Math.radians(points[1].r));

    var inter_pos = intersection(a,b);

    console.log(b.second);

    var line1 = Math.sqrt(Math.pow(a.first.x - inter_pos.x,2)+Math.pow(a.first.y - inter_pos.y,2));
    var line2 = Math.sqrt(Math.pow(b.first.x - inter_pos.x,2)+Math.pow(b.first.y - inter_pos.y,2));

    if(line1 < line2){
      var length = line2 - line1;
      var node = {"first":{},"second":{}};
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
      var circle_center = intersection(node,a);

      var radius = Math.sqrt(Math.pow(node.first.x - circle_center.x,2)+Math.pow(node.first.y - circle_center.y,2));

      //Create Custom Part
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Rail[i] == undefined){
          part_nr = i;
          break;
        }
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"part":"CRail","transform":"translate("+a.first.x+","+a.first.y+") rotate(0)"});
      var box_childs = [];

      var factor = 0;
      if(((points[0].r - points[1].r - 180) % 360) > 0 && ((points[0].r - points[1].r - 180) % 360) < 180){
        factor = 1;
      }

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"CRail","BlockID":-1,"d":"M 0,0 a "+radius+","+radius+" 0,0,"+factor+" "+(node.first.x-a.first.x)+","+(node.first.y-a.first.y)+" l "+(b.first.x-node.first.x)+","+(b.first.y-node.first.y)};

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
      var length = line1 - line2;
      var node = {"first":{},"second":{}};
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
      var circle_center = intersection(node,b);

      var radius = Math.sqrt(Math.pow(node.first.x - circle_center.x,2)+Math.pow(node.first.y - circle_center.y,2))

      //Create Custom Part
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Rail[i] == undefined){
          part_nr = i;
          break;
        }
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"part":"CRail","transform":"translate("+a.first.x+","+a.first.y+") rotate(0)"});
      var box_childs = [];

      var factor = 0;
      if(((points[0].r - points[1].r - 180) % 360) < 0 && ((points[0].r - points[1].r - 180) % 360) > -180){
        factor = 1;
      }

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"CRail","BlockID":-1,"d":"M 0,0 l "+(node.first.x-a.first.x)+","+(node.first.y-a.first.y)+" a "+radius+","+radius+" 0,0,"+factor+" "+(b.first.x-node.first.x)+","+(b.first.y-node.first.y)};

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
    LayoutDragging.parent = $('#LayoutContainer g#Drawing g[nr='+part_nr+']');

    var transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

    var translation = transfrom_str[0].slice(10,-1).split(',');
    var sx = parseFloat(translation[0]);
    var sy = parseFloat(translation[1]);
    var rotation = parseFloat(transfrom_str[1].slice(7,-1));

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
      var Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      var MySide = parseInt(element.className.baseVal.slice(6));
      var OwnSideSign = 1;
      if(MySide == 2){
        OwnSideSign = -1;
      }

      var MyID = parseInt(LayoutDragging.parent.attr("nr"));
      var MyJoin = MyID + "_" + MySide;

      var set = -1;

      var for_each_counter = 0;
      var array_length = EditObj.Layout.Anchors.length;
      for(for_each_counter;for_each_counter<array_length;for_each_counter++){
        element1 = EditObj.Layout.Anchors[for_each_counter];
		if(element1 == undefined){return;}
        if((element1.prev.length < element1.prev_states || element1.next.length < element1.next_states) && 
            (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && set < 0){
          var a = "";
          if(element1.angle != parseFloat(Anchor.element.attr("_r")) && element1.prev.length < element1.prev_states){
            a = "prev";
          }else if(element1.angle-180 != parseFloat(Anchor.element.attr("_r")) && element1.next.length < element1.next_states){
            a = "next";
          }

          Anchor.element.get(0).style.fill = "green";
          Anchor.element.attr("connected",for_each_counter);
          set = for_each_counter;

          if(a == "prev"){
            if(element1.prev.length == 1){
              element1.prev[0].get(0).style.fill = "green";
            }
            EditObj.Layout.Anchors[for_each_counter].prev.push(Anchor.element);
          }else{
            if(element1.next.length == 1){
              element1.next[0].get(0).style.fill = "green";
            }
            EditObj.Layout.Anchors[for_each_counter].next.push(Anchor.element);
          }
        }
      }
      if(set < 0){
        Anchor.element.get(0).style.fill = "red";
      }
    });
	$.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
		var Anchor = {"element":undefined,"loc":{}};
		Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
		Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

		var MySide = parseInt(element1.className.baseVal.slice(6));

		if(Anchor.element.attr("connected") == "" || Anchor.element.attr("connected") == undefined){ //Add when not connected
		  Layout_createAnchor(Anchor,rotation);
		  Anchor.element.attr("connected",EditObj.Layout.Anchors.length-1);
		  console.log("Create New Anchor for "+Anchor.element.parent().attr("nr"));
		}
	  });
    
	var element = LayoutDragging.parent.detach();
    $('#LayoutContainer #Rail').append(element);
    return Layout_AC_Finish();
  }else if(angle_diff == 180){
    console.log("Straight Line!! EASY");
  }else{
    alert("Failed\nTo Large Angle");
  }
  Layout_AC_Abort();
}

function Layout_Save(){
  var saveList = {"Rail":[],"Nodes":[],"Signals":[]};
  var i = 0;
  $.each($('#LayoutContainer svg g#Rail g'),function(index,element){
    console.log(element);
    part_nr = parseInt($(element).attr("nr"));
    saveList.Rail.push({"nr":part_nr,"part":$(element).attr("part"),"transform":$(element).attr("transform")});
    saveList.Rail[i].BlockID = EditObj.Layout.Setup.Rail[part_nr].BlockID;
    if($(element).attr("part")=="RS"){
      saveList.Rail[i].length = EditObj.Layout.Setup.Rail[part_nr].length;
      saveList.Rail[i].x = $('line',element).attr("x2");
      saveList.Rail[i].y = $('line',element).attr("y2");
    }else if($(element).attr("part")=="RC"){
      saveList.Rail[i].angle  = EditObj.Layout.Setup.Rail[part_nr].angle;
      saveList.Rail[i].radius = EditObj.Layout.Setup.Rail[part_nr].radius;
      saveList.Rail[i].d = $('path',element).attr("d");
      saveList.Rail[i].x = $('.Attach2',element).attr("cx");
      saveList.Rail[i].y = $('.Attach2',element).attr("cy");
    }else if($(element).attr("part")=="CRail"){
      saveList.Rail[i].d = EditObj.Layout.Setup.Rail[part_nr].d;
      saveList.Rail[i].r1 = parseFloat($('.Attach1',element).attr("_r"));
      saveList.Rail[i].r2 = parseFloat($('.Attach2',element).attr("_r"));
      saveList.Rail[i].x  = parseFloat($('.Attach2',element).attr("cx"));
      saveList.Rail[i].y  = parseFloat($('.Attach2',element).attr("cy"));
    }
    i++;
  });
  i = 0;
  $.each($('#LayoutContainer svg g#Nodes g'),function(index,element){
    console.log(element);
    part_nr = parseInt($(element).attr("nr"));
    saveList.Nodes.push({"nr":part_nr,"part":$(element).attr("part"),"transform":$(element).attr("transform")});
    if($(element).attr("part") == "BI"){
      saveList.Nodes[i].AnchorID = EditObj.Layout.Setup.Nodes[part_nr].Anchor;
      saveList.Nodes[i].ConRailPart = EditObj.Layout.Anchors[saveList.Nodes[i].AnchorID].prev[0].parent().attr("nr");
    }else if($(element).attr("part")=="SwN"){
      saveList.Nodes[i].SwitchPID   = EditObj.Layout.Setup.Nodes[part_nr].SwitchPID;
      saveList.Nodes[i].SwitchNID   = EditObj.Layout.Setup.Nodes[part_nr].SwitchNID;
      saveList.Nodes[i].Pstates = [];
      saveList.Nodes[i].Nstates = [];
      $.each(EditObj.Layout.Anchors[EditObj.Layout.Setup.Nodes[part_nr].Anchor].prev,function(index1,element1){
        saveList.Nodes[i].Pstates.push({"nr":element1.parent().attr("nr"),"BID":EditObj.Layout.Setup.Rail[element1.parent().attr("nr")].BlockID});
      });
      $.each(EditObj.Layout.Anchors[EditObj.Layout.Setup.Nodes[part_nr].Anchor].next,function(index1,element1){
        saveList.Nodes[i].Nstates.push({"nr":element1.parent().attr("nr"),"BID":EditObj.Layout.Setup.Rail[element1.parent().attr("nr")].BlockID});
      });
      saveList.Nodes[i].ConRailPart = saveList.Nodes[i].Pstates[0].nr;

      saveList.Nodes[i].Pstate = EditObj.Layout.Setup.Nodes[part_nr].Pstates;
      saveList.Nodes[i].Nstate = EditObj.Layout.Setup.Nodes[part_nr].Nstates;
    }else if($(element).attr("part") == "MAN"){
      saveList.Nodes[i].length = EditObj.Layout.Setup.Nodes[part_nr].length;
    }
    i++;
  });
  i=0;
  $.each($('#LayoutContainer svg g#Signals g'),function(index,element){
    console.log(element);
    part_nr = parseInt($(element).attr("nr"));
    saveList.Signals.push({"nr":part_nr,"transform":$(element).attr("transform"),"setup":EditObj.Layout.Setup.Signals[part_nr]});
    saveList.Signals[i].SwitchID = EditObj.Layout.Setup.Signals[part_nr].SignalID;
    i++;
  });

  //Sort Lists
    saveList.Nodes.sort(function(a,b){return a.nr-b.nr});
     saveList.Rail.sort(function(a,b){return a.nr-b.nr});
  saveList.Signals.sort(function(a,b){return a.nr-b.nr});

  return saveList;
}

function Layout_Load(saveList){
  //Clear Previous setup
  Layout_Clear();

  //Load new Setup
  $.each(saveList.Nodes,function(index,element){
    console.log(element);
    if(element.part != "BI"){
      element.ConRail = saveList.Rail[element.ConRailPart];
      Layout_placePart(element.part,element.nr,element);
    }
  });
  $.each(saveList.Rail,function(index,element){
    console.log(element);
    Layout_placePart(element.part,element.nr,element);
  });
  $.each(saveList.Signals,function(index,element){
    console.log(element);
    Layout_placePart("Sig",element.nr,element);
  });
  $.each(saveList.Nodes,function(index,element){
    console.log(element);
    if(element.part == "BI"){
      element.ConRail = saveList.Rail[element.ConRailPart];
      Layout_placePart(element.part,element.nr,element);
    }
  });
}

function Layout_LoadNew(moduleNr){
  $.ajax({
    url: './../modules/'+moduleNr+'/layout.txt',
    type: 'GET',
    async: true,
    cache: false,
    timeout: 30000,
    error: function(){
      return true;
    },
    success: function(msg){
      Layout_Load(JSON.parse(msg));
    }
  });
}

function Layout_placePart(part,nr,data = {}){
  var box;
  var box_childs = [];
  var part_nr = 0;

  var part_nr = nr;

  switch(part){
    case "RS": //Straight Rail
      if(EditObj.Layout.Setup.Rail[part_nr] != undefined){
        return false;
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"part":part,"size":"1","transform":data.transform});
      var i_counter = 0;

      var length = data.length;
      var steps = parts_list.RS.length_steps;

      $.each(parts_list.RS.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RS","BlockID":data.BlockID,"length":length};
      break;
    case "RC":
      if(EditObj.Layout.Setup.Rail[part_nr] != undefined){
        return false;
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":data.transform});
      var i_counter = 0;

      var angle = data.angle;
      var radius = data.radius;
      var a_steps = parts_list.RC.angle_steps;
      var r_steps = parts_list.RC.radius_steps;

      $.each(parts_list.RC.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RC","BlockID":data.BlockID,"angle":angle,"radius":radius};
      break;
    case "SwN":
      if(EditObj.Layout.Setup.Nodes[part_nr] != undefined){
        return false;
      }
      var data2 = data.ConRail
      Layout_placePart(data2.part,data2.nr,data2);

      var box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":data.transform});
      var i_counter = 0;
      var type = 0;
      if(data.Nstate > 1 && data.Pstate > 1){
        type = 1;
      }

      $.each(parts_list.SwN.part[type],function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"SwN","SwitchNID":data.SwitchNID,"SwitchPID":data.SwitchPID,"Nstates":data.Nstate,"Pstates":data.Pstate,"heading":data.heading};
      break;
    case "CRail":
      var box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":data.transform});
      var box_childs = [];

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"CRail","BlockID":data.BlockID,"d":data.d};

      box_childs[0] = CreateSvgElement("path",{"d":data.d,"style":"stroke-width:6px;"});
      box_childs[1] = CreateSvgElement("path",{"d":data.d,"style":"stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;"});
      box_childs[2] = CreateSvgElement("circle",{"class":"Attach1","_r":data.r1,"cx":0,"cy":0,"r":3,"style":"fill:green;"});
      box_childs[3] = CreateSvgElement("circle",{"class":"Attach2","_r":data.r2,"cx":data.x,"cy":data.y,"r":3,"style":"fill:green;"});

      $.each(box_childs,function(index,element){
        box.appendChild(element);
      });

      $('#LayoutContainer #Drawing').append(box);

      $('#LayoutContainer g#Drawing g[nr='+part_nr+']').bind("mousedown",Layout_dragPart);
      break;
    case "BI":
      if(EditObj.Layout.Setup.Nodes[part_nr] != undefined){
        return false;
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":data.transform});
      var i_counter = 0;

      $.each(parts_list.BI.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"BI"};
      break;
    case "MAN":
      if(EditObj.Layout.Setup.Nodes[part_nr] != undefined){
        return false;
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":data.transform});
      var i_counter = 0;

      var length = data.length;

      $.each(parts_list.MAN.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });
      i = 0;
      for(i;i<(length-1);i++){
        number = i+2;
        var part_attributes = {};
        $.each(parts_list.MAN.node[0].attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(parts_list.MAN.node[0].element,part_attributes);
        var part_attributes = {};
        $.each(parts_list.MAN.node[1].attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(parts_list.MAN.node[1].element,part_attributes);
      }
      var part_attributes = {};
      $.each(parts_list.MAN.mousebox,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      box_childs[i_counter++] = CreateSvgElement("rect",part_attributes);

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"MAN","length":data.length};
      break;
    case "Sig":
      if(EditObj.Layout.Setup.Signals[part_nr] != undefined){
        return false;
      }
      var box = CreateSvgElement("g",{"nr":part_nr,"SignalID":data.SignalID,"part":part,"transform":data.transform});
      var i_counter = 0;

      var type = data.setup.type;

      $.each(parts_list.Sig.part,function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });
      $.each(parts_list.Sig.types[type],function(index,element){
        var part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Signals[part_nr] = {"type":"Sig","type":data.setup.type,"heading":data.setup.type};
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });


  //Add or connect to Anchor list

  switch(part){
    case "RC":
    case "RS":
    case "CRail":
      console.log("Add to Rail with nr:"+part_nr);
      $('#LayoutContainer #Rail').append(box);
      $('#LayoutContainer g#Rail g[nr='+part_nr+']').bind("mousedown",Layout_dragPart);
      LayoutDragging.parent = $('#LayoutContainer g#Rail g[nr='+part_nr+']');
      break;
    case "SwN":
    case "MAN":
    case "BI":
      $('#LayoutContainer #Nodes').append(box);
      $('#LayoutContainer g#Nodes g[nr='+part_nr+']').bind("mousedown",Layout_dragPart);
      LayoutDragging.parent = $('#LayoutContainer g#Nodes g[nr='+part_nr+']');
      break;
    case "Sig":
      $('#LayoutContainer #Signals').append(box);
      $('#LayoutContainer g#Signals g[nr='+part_nr+']').bind("mousedown",Layout_dragPart);
      LayoutDragging.parent = $('#LayoutContainer g#Signals g[nr='+part_nr+']')
      break;
    default:
      console.log("Parttype not found");
      LayoutDragging.parent = undefined;
      break;
  }

  Layout_dragEnd({},true);
}

var LayoutEdit = {"tooltype":"mouse","tool":undefined,"toolsize":"1x1","rot":0,"contextmenu":false};

EditObj.Layout.gridsize = {"x":42,"y":42};
EditObj.Layout.grid = [];

var parts_list = {};
parts_list.RS = {"length_steps":[],"limits":{},"part":[]};
parts_list.RS.limits.length_max = 5;
parts_list.RS.limits.length_min = 1;
parts_list.RS.length_steps = [undefined,35.36,42,82,162,322];

parts_list.RS.part = [
          {"element":"line","attr":{"x1":-0.2,"y1":0,"x2":"steps[length]+0.2","y2":0,"style":"'stroke-width:6px;'"}},
          {"element":"line","attr":{"x1":-0.2,"y1":0,"x2":"steps[length]+0.2","y2":0,"style":"'stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;'"}},
          {"element":"circle","attr":{"class":"'Attach1'","_r":180,"cx":0,"cy":0,"r":3,"style":"'fill:red;'"}},
          {"element":"circle","attr":{"class":"'Attach2'","_r":  0,"cx":"steps[length]","cy":0,"r":3,"style":"'fill:red;'"}}
        ];

parts_list.RC = {"angle_steps":[],"radius_steps":[],"limits":{},"part":[]};
parts_list.RC.angle_steps  = [undefined,Math.radians(11.25),Math.radians(22.5),Math.radians(45),Math.radians(67.5),Math.radians(90)];
parts_list.RC.radius_steps = [undefined,20.7,25,50,79.29,100,150,179.289];
parts_list.RC.limits.angle_max = [undefined,5,5,5,5,5];
parts_list.RC.limits.angle_min = [undefined,3,2,1,1];
parts_list.RC.limits.radius_max = 7;
parts_list.RC.limits.radius_min = 1;


parts_list.RC.part = [
          {"element":"path","attr":{"d":"'M -0.2,0 h 0.2 a '+r_steps[radius]+','+r_steps[radius]+' 0,0,0 '+Math.sin(a_steps[angle])*r_steps[radius]+','+r_steps[radius]*(Math.cos(a_steps[angle])-1)+' l '+0.2*Math.cos(a_steps[angle])+','+0.2*-Math.sin(a_steps[angle])","style":"'stroke-width:6px;'"}},
          {"element":"path","attr":{"d":"'M -0.2,0 h 0.2 a '+r_steps[radius]+','+r_steps[radius]+' 0,0,0 '+Math.sin(a_steps[angle])*r_steps[radius]+','+r_steps[radius]*(Math.cos(a_steps[angle])-1)+' l '+0.2*Math.cos(a_steps[angle])+','+0.2*-Math.sin(a_steps[angle])","style":"'stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;'"}},
          {"element":"circle","attr":{"class":"'Attach1'","_r":   180,"cx":    0,"cy":    0,"r":3,"style":"'fill:red;'"}},
          {"element":"circle","attr":{"class":"'Attach2'","_r":"-Math.degrees(a_steps[angle])","cx":"Math.sin(a_steps[angle])*r_steps[radius]","cy":"r_steps[radius]*(Math.cos(a_steps[angle])-1)","r":3,"style":"'fill:red;'"}}
        ];

parts_list.SwN = {"limits":{},"part":[]};
parts_list.SwN.limits.states_max = 4;
parts_list.SwN.limits.states_min = 1;
parts_list.SwN.part[0] = [
          {"element":"path","attr":{"d":"'M 0,-7.5 v 15 l -7.5,-7.5 Z'","style":"'stroke-width:0;fill:#41b7dd;'"}},
          {"element":"circle","attr":{"class":"'NodeAttach1'","cx":0,"cy":0,"r":10,"style":"'fill:rgba(200,200,200,0.4);cursor:move;'"}}
        ];
parts_list.SwN.part[1] = [
          {"element":"path","attr":{"d":"'M 0,-7.5 v 15 l -7.5,-7.5 Z'","style":"'stroke-width:0;fill:#41b7dd;'"}},
          {"element":"path","attr":{"d":"'M 0,-7.5 v 15 l 7.5,-7.5 Z'","style":"'stroke-width:0;fill:#ff61fa'"}},
          {"element":"circle","attr":{"class":"'NodeAttach1'","cx":0,"cy":0,"r":10,"style":"'fill:rgba(200,200,200,0);cursor:move;'"}}
        ];

parts_list.BI = {"limits":{},"part":[]};
parts_list.BI.limits.states_max = 4;
parts_list.BI.limits.states_min = 1;
parts_list.BI.part = [
          {"element":"circle","attr":{"cx":0,"cy":0,"r": 3,"style":"'fill:purple;'"}},
          {"element":"circle","attr":{"class":"'NodeAttach1'","cx":0,"cy":0,"r":10,"style":"'fill:rgb(100,100,100);opacity:0.5;cursor:move;'"}}
        ];

parts_list.MAN = {"limits":{},"part":[]};
parts_list.MAN.limits.nodes_max = 4;
parts_list.MAN.limits.nodes_min = 1;
parts_list.MAN.part = [
          {"element":"line","attr":{"x1":0,"y1":-14.6446,"x2":0,"y2":"14.6446+(length-1)*29.2893","style":"'fill:black;stroke-width:1;stroke:black'"}},
          {"element":"circle","attr":{"class":"'Attach1'","cx":0,"cy":0,"r":5,"_r":0,"style":"'fill:rgb(100,100,100);opacity:0.5;'"}},
          {"element":"path","attr":{"d":"'M -5,-5 h 5 a 5,5 0,0,1 0,10 h -5 Z'","style":"'fill:cyan;stroke-width:0;'"}}
        ];
parts_list.MAN.node = [{"element":"circle","attr":{"class":"'Attach'+number","cx":0,"cy":"29.2893*(1+i)","r":5,"_r":0,"style":"'fill:rgb(100,100,100);opacity:0.5;'"}},
                        {"element":"path","attr":{"d":"'M -5,'+(29.2893*(1+i)-5)+' h 5 a 5,5 0,0,1 0,10 h -5 Z'","style":"'fill:cyan;stroke-width:0;'"}}];
parts_list.MAN.mousebox = {"x":-10,"width":20,"y":-14.6446,"height":"length*29.2893","style":"'cursor:move;opacity:0.2'"}

parts_list.Sig = {"limits":{},"part":[],"types":[],"types_name":[]};
parts_list.Sig.limits.type_max = 1;
parts_list.Sig.limits.type_min = 0;

parts_list.Sig.part = [
          {"element":"circle","attr":{"class":"'SigAttach'","cx":0,"cy":0,"r":1.5,"style":"'fill:purple'"}}
        ];

parts_list.Sig.types_name[0] = "3 Light Signal";
parts_list.Sig.types[0] = [
          {"element":"path","attr":{"d":"'M 15,5 v 10 a 5,5 0,0,1 -10,0 v -10 a 5,5 0,0,1 10,0'","style":"'stroke-width:0px;stroke:black;fill:black;'"}},
          {"element":"circle","attr":{"cx":10,"cy": 5,"r":2,"style":"'stroke-width:0px;fill:red'"}},
          {"element":"circle","attr":{"cx":10,"cy":10,"r":2,"style":"'stroke-width:0px;fill:orange'"}},
          {"element":"circle","attr":{"cx":10,"cy":15,"r":2,"style":"'stroke-width:0px;fill:lime'"}},
          {"element":"path","attr":{"d":"'M 17.5,2.5 v 15 a 7.5,7.5 0,0,1 -15,0 v -15 a 7.5,7.5 0,0,1 15,0'","style":"'fill:rgb(100,100,100);opacity:0.1;cursor:move;stroke-width:0px;'"}}
        ];

parts_list.Sig.types_name[1] = "2 Light Signal";
parts_list.Sig.types[1] = [
          {"element":"path","attr":{"d":"'M 15,5 v 5 a 5,5 0,0,1 -10,0 v -5 a 5,5 0,0,1 10,0'","style":"'stroke-width:0px;stroke:black;fill:black;'"}},
          {"element":"circle","attr":{"cx":10,"cy": 5,"r":2,"style":"'stroke-width:0px;fill:red'"}},
          {"element":"circle","attr":{"cx":10,"cy":10,"r":2,"style":"'stroke-width:0px;fill:green'"}},
          {"element":"path","attr":{"d":"'M 17.5,2.5 v 10 a 7.5,7.5 0,0,1 -15,0 v -10 a 7.5,7.5 0,0,1 15,0'","style":"'fill:rgb(100,100,100);opacity:0.1;cursor:move;stroke-width:0px;'"}}
        ];

function intersection(L1,L2){
  var A1 = (L1.first.y - L1.second.y);
  var B1 = (L1.second.x - L1.first.x);
  var C1 = -(L1.first.x*L1.second.y - L1.second.x*L1.first.y);

  var A2 = (L2.first.y - L2.second.y);
  var B2 = (L2.second.x - L2.first.x);
  var C2 = -(L2.first.x*L2.second.y - L2.second.x*L2.first.y);

  var D  = A1 * B2 - B1 * A2;
  var Dx = C1 * B2 - B1 * C2;
  var Dy = A1 * C2 - C1 * A2;
  if(D != 0){
    var x = Dx / D;
    var y = Dy / D;
  }else{
    return False;
  }

  return {"x":x,"y":y};
}
