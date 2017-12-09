
LayoutDragging = {"x1":0,"y1":0,"target":undefined,"parent":undefined,"Box":undefined,"part_settings":{"RC":{"radius":2,"angle":3},"RS":{"length":1},"Sig":{"type":1},"MAN":{"lenght":2}},
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
      for(i=0;i<10000;i++){
        if(EditObj.Layout.Setup.Rail[i] == undefined){
          part_nr = i;
          break;
        }
      }
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"size":"1","transform":"translate("+(xy.x-21)+","+(xy.y-21)+") rotate(0)"});
      i_counter = 0;

      length = LayoutDragging.part_settings.RS.length;
      steps = parts_list.RS.length_steps;

      $.each(parts_list.RS.part,function(index,element){
        part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Rail[part_nr] = {"type":"RS","length":length};
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
        part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Nodes[part_nr] = {"type":"SwN","Nstates":2,"Pstates":1,"heading":180};
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
        part_attributes = {};
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
        part_attributes = {};
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
        part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });
      $.each(parts_list.Sig.types[type],function(index,element){
        part_attributes = {};
        $.each(element.attr,function(index2,element2){
          part_attributes[index2] = eval(element2);
        });
        box_childs[i_counter++] = CreateSvgElement(element.element,part_attributes);
      });

      EditObj.Layout.Setup.Signals[part_nr] = {"type":"Sig","type":0,"heading":180};
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


  N_evt = {};
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
  else if(part_type == "SwN" || part_type == "BI" || part_type == "MAN"){
    var element = LayoutDragging.parent.detach();
    $("#LayoutContainer #Nodes").append(element);
  }
  else if(part_type == "Sig"){
    var element = LayoutDragging.parent.detach();
    $("#LayoutContainer #Signals").append(element);
  }

  LayoutDragging.Box.unbind('mousemove',Layout_dragMove);
  LayoutDragging.Box.unbind('mouseup',Layout_dragEnd);
  //,'mouseup']);
  $('body').unbind('mousemove',Layout_dragMove);
  $('body').unbind('mouseup',Layout_dragEnd);
  $('body').unbind('keypress',Layout_KeyHandler);

  attached = false;

  //Attach Rail
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

    if(set < 0){
      for_each_counter = 0;
      array_length = EditObj.Layout.Anchors.length;
      for(for_each_counter;for_each_counter<array_length;for_each_counter++){
        element1 = EditObj.Layout.Anchors[for_each_counter];
        if((element1.prev.length < element1.prev_states || element1.next.length < element1.next_states) && 
            (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && set < 0){
          a = "";
          if(element1.angle != parseFloat(Anchor.element.attr("_r")) && attached == false && element1.prev.length < element1.prev_states){
            a = "prev";
            rotation = (element1.angle - parseFloat(Anchor.element.attr("_r"))) % 360;
            LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
            Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
          }else if(element1.angle-180 != parseFloat(Anchor.element.attr("_r")) && attached == false && element1.next.length < element1.next_states){
            a = "next";
            rotation = (element1.angle - parseFloat(Anchor.element.attr("_r"))+180) % 360;
            LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
            Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
          }
          sx -= Anchor.loc.left - element1.x;
          sy -= Anchor.loc.top - element1.y;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.element.get(0).style.fill = "green";
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
        if(!set){Anchor.element.get(0).style.fill = "red"};
      }
    }
    if(set < 0){
      Anchor.element.get(0).style.fill = "red";
    }
  });
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element1.className.baseVal.slice(6));

    if(Anchor.element.get(0).style.fill == "red"){ //Add when not connected
      Layout_createAnchor(Anchor,rotation);
    }
  });

  //Attach Signals
  $.each($('[class^=SigAttach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
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
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
      if((Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
        if(((rotation)-element1.angle <= 90 && (rotation)-element1.angle > -90) || ((rotation)-element1.angle >= 270 && (rotation)-element1.angle < -360) || ((rotation)-element1.angle >= -360 && (rotation)-element1.angle < -270)){
          rotation = (element1.angle) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

          EditObj.Layout.Anchors[index1].next_states = EditObj.Layout.Setup.Nodes[MyID].Pstates;
          EditObj.Layout.Anchors[index1].prev_states = EditObj.Layout.Setup.Nodes[MyID].Nstates;
          EditObj.Layout.Setup.Nodes[MyID].heading = rotation;
        }else{
          rotation = (element1.angle - 180) % 360;
          LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
          Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

          EditObj.Layout.Anchors[index1].next_states = EditObj.Layout.Setup.Nodes[MyID].Nstates;
          EditObj.Layout.Anchors[index1].prev_states = EditObj.Layout.Setup.Nodes[MyID].Pstates;
          EditObj.Layout.Setup.Nodes[MyID].heading = rotation-180;
        }
        sx -= Anchor.loc.left - element1.x;
        sy -= Anchor.loc.top - element1.y;
        LayoutDragging.parent.attr("transform", "translate("+sx+","+sy+") rotate("+rotation+")");
        Anchor.element.get(0).style.fill = "rgba(200,200,200,0.4)";

        Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
        set = index1;

        attached = true;

        EditObj.Layout.Anchors[index1].Node = Anchor.element;
        Anchor.element.attr("attached",index1);
      }
    });
  });
}

function Layout_deleteAnchor(){
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
    yN = -1;
    yP = -1;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
      if(x == -1 && element1.x == Anchor.loc.left && element1.y == Anchor.loc.top){
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
        EditObj.Layout.Anchors.splice(x,1);
      }
    }
    /*if(Anchor.element.attr("attached") != "" && Anchor.element.attr("attached") != undefined){
      IDs = Anchor.element.attr("attached").split("_");
      element = $('g[nr='+IDs[0]+'] .Attach'+IDs[1]);
      x_y = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);
      element.get(0).style.fill = "red";

      //Parent Rotation
      p_rotation = parseFloat(element.parent().attr("transform").split(" ")[1].slice(7,-1));
      EditObj.Layout.FreeAnchors.push({"x":x_y.left,"y":x_y.top,"angle":(p_rotation+parseFloat(element.attr("_r")))%360,"element":element,"Join":-1});
      Anchor.element.attr("attached","");
    }*/
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
        "prev":[Anchor.element],"next":[],"prev_states":1,"next_states":1,"Signal":{"R":undefined,"L":undefined},"type":"RailNode","Node":undefined});
}

function Layout_dragCheckDrop(){
  svgContainerOffset = $('#LayoutContainer svg').offset();


  transfrom_str = LayoutDragging.parent.attr("transform").split(' ');

  translation = transfrom_str[0].slice(10,-1).split(',');
  sx = parseFloat(translation[0]);
  sy = parseFloat(translation[1]);
  rotation = parseFloat(transfrom_str[1].slice(7,-1));

  Drop = 0;
  $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

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
    if(set == false){
      $.each(EditObj.Layout.Anchors,function(index1,element1){
        if((element1.prev.length < element1.prev_states || element1.next.length < element1.next_states) && 
            (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
          Anchor.element.get(0).style.fill = "green";
          set = true;
        }
        if(!set){Anchor.element.get(0).style.fill = "red"};
      });
    }
  });
  $.each($('[class^=SigAttach]',LayoutDragging.parent),function(index,element){
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
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
    Anchor = {"element":undefined,"loc":{}};
    Anchor.element = $('.'+element.className.baseVal,LayoutDragging.parent);
    Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

    MySide = parseInt(element.className.baseVal.slice(6));
    MyID = parseInt(LayoutDragging.parent.attr("nr"));
    MyJoin = MyID + "_" + MySide;

    set = false;

    $.each(EditObj.Layout.Anchors,function(index1,element1){
      if(element1.Node == undefined && (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && !set){
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
      context.name = "'Straight Rail'";
      context.options[0] = {"type":"in_nr","name":"ID","value":""};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
	    context.options[2] = {"type":"in_step","name":"Length","b_type":"length","value":"Math.round(parts_list.RS.length_steps[EditObj.Layout.Setup.Rail[item_ID].length]*100)/100"};
      break;
    case "RC":
      context.name = "'Curved Rail'";
      context.options[0] = {"type":"in_nr","name":"ID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Radius","b_type":"radius","value":"Math.round(parts_list.RC.radius_steps[EditObj.Layout.Setup.Rail[item_ID].radius]*100)/100"};
      context.options[3] = {"type":"in_step","name":"Angle","b_type":"angle","value":"Math.round(Math.degrees(parts_list.RC.angle_steps[EditObj.Layout.Setup.Rail[item_ID].angle])*100)/100+'&deg;'"};
      break;
    case "CRail":
      context.name = "'Custom Rail'";
      context.options[0] = {"type":"in_nr","name":"ID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      break;
    case "SwN":
      context.name = "'Switch Node'";
      context.options[0] = {"type":"in_nr","name":"ID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Forw. States","b_type":"Nstates","value":"EditObj.Layout.Setup.Nodes[item_ID].Nstates"};
      context.options[3] = {"type":"in_step","name":"Back. States","b_type":"Pstates","value":"EditObj.Layout.Setup.Nodes[item_ID].Pstates"};
      context.options[4] = {"type":"in_flip","name":"Flip","b_type":"flip","value":""};
      break;
    case "MAN":
      context.name = "'Module Attachment Node'";
      context.options[0] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[1] = {"type":"in_step","name":"Nr. of Nodes","b_type":"length","value":"EditObj.Layout.Setup.Nodes[item_ID].length"};
      break;
    case "Sig":
      context.name = "parts_list.Sig.types_name[EditObj.Layout.Setup.Signals[item_ID].type]";
      context.options[0] = {"type":"in_nr","name":"ID"};
      context.options[1] = {"type":"show","name":"Own_ID","value":item_ID};
      context.options[2] = {"type":"in_step","name":"Type","b_type":"type","value":"EditObj.Layout.Setup.Signals[item_ID].type"};
      break;
  }

  box = $('#LayoutContextMenu');
  box.css("display","block");
    loc = $(evt.currentTarget).offset();
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

  plus_button = '<line x1="6" y1="10" x2="14" y2="10" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
        '<line x1="10" y1="6" x2="10" y2="14" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
        '<circle class="yplus" cx="10" cy="10" r="8" style="fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"/>'
  min_button = '<line x1="31" y1="10" x2="39" y2="10" style="fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"/>'+
        '<circle class="ymin" cx="35" cy="10" r="8" style="fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"/>';

  flip_button = '<path d="M13,1V15a1,1,0,0,0,1.7.7l7-7a1,1,0,0,0,0-1.41l-7-7A1,1,0,0,0,13.3.3,1,1,0,0,0,13,1Zm0,0" style="stroke-width:0px;fill:black"/>'+
            '<path d="M9,15V1A1,1,0,0,0,8.7.3,1,1,0,0,0,7.3.3l-7,7A1,1,0,0,0,.3,8.7l7,7A1,1,0,0,0,9,15Zm0,0" style="stroke-width:0px;fill:black"/>'+
            '<rect x="0" y="0" width="22" height="16" style="fill:rgba(0,0,0,0);cursor:pointer"';

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
    }else if(element.type == "in_flip"){
    text = '<div class="'+element.name+'" style="width:100%;height:20px;float:left;">'+element.name+
        ':<span style="float:right;text-align:center;width:50%;font-weight:bold;position:relative">'+
        '<svg viewbox="0 0 22 16" class="flip_button" type="'+element.b_type+'" style="width:20px;margin:auto">'+flip_button+'</svg></span></div>';
    }

    $('.content',box).append(text);
  });
  $('.content  .min_button',box).bind('mousedown',Layout_Setup_Change);
  $('.content .plus_button',box).bind('mousedown',Layout_Setup_Change);
  $('.content .flip_button',box).bind('mousedown',Layout_Setup_Change);
}

function Layout_HideContextMenu(evt = {"currentTarget":'#LayoutContainer svg'}){
  $(evt.currentTarget).unbind("mousedown",Layout_HideContextMenu);
  $('#LayoutContextMenu').css("display","none");
  return true;
}

function Layout_ClearAll(){
  $.each($('#LayoutContainer svg + g:not(#toolbar,#Rail,#Signals,#Nodes,#Drawing)'),function(index,element){
    element.remove();
  });
}

function Find_Anchor_Point(g_element,attach = ""){
  transform_str = g_element.attr("transform").split(' ');

  translation = transform_str[0].slice(10,-1).split(',');
  sx = parseFloat(translation[0]);
  sy = parseFloat(translation[1]);
  rotation = parseFloat(transform_str[1].slice(7,-1));

  if(attach != ""){
    loc = getRotatedPoint(sx,sy,parseFloat($('.'+attach,g_element).attr("cx")),parseFloat($('.'+attach,g_element).attr("cy")),rotation);
    sx += loc.left;
    sy += loc.top;
  }

  ID = -1;

  $.each(EditObj.Layout.Anchors,function(index,element){
    if(sx == element.x && sy == element.y){
      ID = index;
    }
  });
  return ID;
}

function Layout_Setup_Change(event){
  target = $(event.target);
  if($(event.target).is('circle') || $(event.target).is('line') || $(event.target).is('rect')){
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

  if(o_type == "SwN"){
    Nstates = EditObj.Layout.Setup.Nodes[ID].Nstates;
    Pstates = EditObj.Layout.Setup.Nodes[ID].Pstates;
    limits = parts_list.SwN.limits;
    
    heading = EditObj.Layout.Setup.Nodes[ID].heading;
    _ID = ID;
    AnchorID = Find_Anchor_Point($('#LayoutContainer g#Nodes g[nr='+(ID)+']'));
    Anchor = EditObj.Layout.Anchors[AnchorID];


    if(target.attr("class") == "min_button" && target.attr("type") == 'Nstates'){
      Nstates = InBounds(--Nstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "plus_button" && target.attr("type") == 'Nstates'){
      Nstates = InBounds(++Nstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "min_button" && target.attr("type") == 'Pstates'){
      Pstates = InBounds(--Pstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "plus_button" && target.attr("type") == 'Pstates'){
      Pstates = InBounds(++Pstates,limits.states_min,limits.states_max);
    }else if(target.attr("class") == "flip_button"){
      if(Anchor.prev.length >= Anchor.next_states && Anchor.next.length >= Anchor.prev_states){
        alert("To many connections, try to remove some.")
        return;
      }else{
        temp = Anchor.next_states;
        Anchor.next_states = Anchor.prev_states;
        Anchor.prev_states = temp;
      }

      if(heading < 180){
        heading += 180;
        transform_str = $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform").split(' ');

        rotation = parseFloat(transform_str[1].slice(7,-1)) + 180;
        $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform",transform_str[0]+" rotate("+rotation+")");
      }else{
        heading -= 180;
        transform_str = $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform").split(' ');

        rotation = parseFloat(transfrom_str[1].slice(7,-1)) - 180;
        $('#LayoutContainer g#Nodes g[nr='+(_ID)+']').attr("transform",transform_str[0]+" rotate("+rotation+")");
      }
      EditObj.Layout.Setup.Nodes[_ID].heading = heading;
    }

    if(heading < 180){
      EditObj.Layout.Anchors[AnchorID].next_states = Pstates;
      EditObj.Layout.Anchors[AnchorID].prev_states = Nstates;
    }else{
      EditObj.Layout.Anchors[AnchorID].next_states = Nstates;
      EditObj.Layout.Anchors[AnchorID].prev_states = Pstates;
    }

    part = $("#LayoutContainer g#Nodes g[nr="+(_ID)+"]");
    part.empty();
    type = 0;
    if(Pstates > 1 && Nstates > 1){
      type = 1;
    }
    $.each(parts_list.SwN.part[type],function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });

    EditObj.Layout.Setup.Nodes[_ID].Nstates = Nstates;
    EditObj.Layout.Setup.Nodes[_ID].Pstates = Pstates;
    console.log(EditObj.Layout.Setup.Nodes[_ID]);
  }else if(o_type == "MAN" && target.attr("type") == 'length'){
    length = EditObj.Layout.Setup.Nodes[ID].length;
    limits = parts_list.MAN.limits;
    if(target.attr("class") == "min_button"){
      length = InBounds(--length,limits.nodes_min,limits.nodes_max);
    }else if(target.attr("class") == "plus_button"){
      length = InBounds(++length,limits.nodes_min,limits.nodes_max);
    }
    console.log(EditObj.Layout.Setup.Nodes[ID]);
    Layout_deleteAnchor();
    part = $("#LayoutContainer g#Nodes g[nr="+ID+"]");
    part.empty();

    $.each(parts_list.MAN.part,function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    i = 0;
    for(i;i<(length-1);i++){
      number = i+2;
      part_attributes = {};
      $.each(parts_list.MAN.node[0].attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(parts_list.MAN.node[0].element,part_attributes));
      part_attributes = {};
      $.each(parts_list.MAN.node[1].attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(parts_list.MAN.node[1].element,part_attributes));
    }
    part_attributes = {};
    $.each(parts_list.MAN.mousebox,function(index2,element2){
      part_attributes[index2] = eval(element2);
    });
    part.get(0).appendChild(CreateSvgElement("rect",part_attributes));


    LayoutDragging.part_settings.MAN.length = length;
    EditObj.Layout.Setup.Nodes[ID].length = length;

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createAnchor(Anchor,rotation);
        //EditObj.Layout.FreeAnchors.push({"x":Anchor.loc.left,"y":Anchor.loc.top,"angle":(rotation+parseInt(Anchor.element.attr("_r")))%360,"element":Anchor.element,"Join":-1});
      }
    });
  }else if(o_type == "Sig"){
    type = EditObj.Layout.Setup.Signals[ID].type;
    limits = parts_list.SwN.limits;
    if(target.attr("class") == "min_button"){
      type = InBounds(--type,limits.type_max,limits.type_min);
    }else if(target.attr("class") == "plus_button"){
      type = InBounds(++type,limits.type_max,limits.type_min);
    }

    EditObj.Layout.Setup.Signals[ID].type = type;
    LayoutDragging.part_settings.Sig.type = type;
    console.log(EditObj.Layout.Setup.Signals[ID]);

    part = $("#LayoutContainer g#Signals g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.Sig.part,function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    $.each(parts_list.Sig.types[type],function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });

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
    Layout_deleteAnchor();
    part = $("#LayoutContainer g#Rail g[nr="+ID+"]");
    part.empty();
    $.each(parts_list.RS.part,function(index,element){
      part_attributes = {};
      $.each(element.attr,function(index2,element2){
        part_attributes[index2] = eval(element2);
      });
      part.get(0).appendChild(CreateSvgElement(element.element,part_attributes));
    });
    LayoutDragging.part_settings.RS.length = length;
    EditObj.Layout.Setup.Rail[ID].length = length;

    $.each($('[class^=Attach]',LayoutDragging.parent),function(index,element1){
      Anchor = {"element":undefined,"loc":{}};
      Anchor.element = $('.'+element1.className.baseVal,LayoutDragging.parent);
      Anchor.loc = getRotatedPoint(sx,sy,parseFloat(Anchor.element.attr("cx")),parseFloat(Anchor.element.attr("cy")),rotation);

      if(Anchor.element.attr("attached") == undefined || Anchor.element.attr("attached") == ""){ //Test
        Layout_createAnchor(Anchor,rotation);
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

    LayoutDragging.part_settings.RC.angle = angle;
    LayoutDragging.part_settings.RC.radius = radius;

    EditObj.Layout.Setup.Rail[ID].angle = angle;
    EditObj.Layout.Setup.Rail[ID].radius = radius;

    console.log(EditObj.Layout.Setup.Rail[ID]);
    Layout_deleteAnchor();
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
        Layout_createAnchor(Anchor,rotation);
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
  $.each(EditObj.Layout.Anchors,function(index,element){
    allreadyDone = false;
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
  set = false;
  $.each(EditObj.Layout.Anchors,function(index,element){
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
  points = LayoutDragging.AC_Elements;
  points[0].r = parseFloat(points[0].element.attr("_r")) + parseFloat(points[0].element.parent().attr("transform").split(' ')[1].slice(7,-1));
  points[1].r = parseFloat(points[1].element.attr("_r")) + parseFloat(points[1].element.parent().attr("transform").split(' ')[1].slice(7,-1));
  console.log("Point 1: {X:"+points[0].x+",Y:"+points[0].y+",R:"+points[0].r+"}");
  console.log("Point 2: {X:"+points[1].x+",Y:"+points[1].y+",R:"+points[1].r+"}");

  angle_diff = Math.abs(points[0].r - points[1].r) % 360;
  if(angle_diff >= 45 && angle_diff <= 315 && angle_diff != 180){
    console.log("Angle difference: "+((points[0].r - points[1].r - 180) % 360));
    a = {"first":{},"second":{}};
    b = {"first":{},"second":{}};
    a.first.x = points[0].x;a.first.y = points[0].y;
    a.second.x = points[0].x + 5 * Math.cos(Math.radians(points[0].r));
    a.second.y = points[0].y + 5 * Math.sin(Math.radians(points[0].r));
    b.first.x = points[1].x;b.first.y = points[1].y;
    b.second.x = points[1].x + 5 * Math.cos(Math.radians(points[1].r));
    b.second.y = points[1].y + 5 * Math.sin(Math.radians(points[1].r));

    inter_pos = intersection(a,b);

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
      circle_center = intersection(node,a);

      radius = Math.sqrt(Math.pow(node.first.x - circle_center.x,2)+Math.pow(node.first.y - circle_center.y,2));

      //Create Custom Part
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+a.first.x+","+a.first.y+") rotate(0)"});
      box_childs = [];

      factor = 0;
      if(((points[0].r - points[1].r - 180) % 360) > 0 && ((points[0].r - points[1].r - 180) % 360) < 180){
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
      circle_center = intersection(node,b);

      radius = Math.sqrt(Math.pow(node.first.x - circle_center.x,2)+Math.pow(node.first.y - circle_center.y,2))

      //Create Custom Part
      part_nr = $("#LayoutContainer g#Rail").children().length;
      box = CreateSvgElement("g",{"nr":part_nr,"part":part,"transform":"translate("+a.first.x+","+a.first.y+") rotate(0)"});
      box_childs = [];

      factor = 0;
      if(((points[0].r - points[1].r - 180) % 360) < 0 && ((points[0].r - points[1].r - 180) % 360) > -180){
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
    LayoutDragging.parent = $('#LayoutContainer g#Drawing g[nr='+part_nr+']');

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
      OwnSideSign = 1;
      if(MySide == 2){
        OwnSideSign = -1;
      }

      MyID = parseInt(LayoutDragging.parent.attr("nr"));
      MyJoin = MyID + "_" + MySide;

      set = -1;

      for_each_counter = 0;
      array_length = EditObj.Layout.Anchors.length;
      for(for_each_counter;for_each_counter<array_length;for_each_counter++){
        element1 = EditObj.Layout.Anchors[for_each_counter];
        if((element1.prev.length < element1.prev_states || element1.next.length < element1.next_states) && 
            (Math.pow(Anchor.loc.left - element1.x,2) + Math.pow(Anchor.loc.top - element1.y,2)) < 100 && set < 0){
          a = "";
          if(element1.angle != parseFloat(Anchor.element.attr("_r")) && attached == false && element1.prev.length < element1.prev_states){
            a = "prev";
          }else if(element1.angle-180 != parseFloat(Anchor.element.attr("_r")) && attached == false && element1.next.length < element1.next_states){
            a = "next";
          }

          Anchor.element.get(0).style.fill = "green";
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

var LayoutEdit = {"tooltype":"mouse","tool":undefined,"toolsize":"1x1","rot":0,"contextmenu":false};

EditObj.Layout.gridsize = {"x":42,"y":42};
EditObj.Layout.grid = [];

parts_list = {};
parts_list.RS = {"length_steps":[],"limits":{},"part":[]};
parts_list.RS.limits.length_max = 5;
parts_list.RS.limits.length_min = 1;
parts_list.RS.length_steps = [undefined,35.36,42,82,162,322];

parts_list.RS.part = [
          {"element":"line","attr":{"x1":0,"y1":0,"x2":"steps[length]","y2":0,"style":"'stroke-width:6px;'"}},
          {"element":"line","attr":{"x1":0,"y1":0,"x2":"steps[length]","y2":0,"style":"'stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;'"}},
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
          {"element":"path","attr":{"d":"'M 0,0 a '+r_steps[radius]+','+r_steps[radius]+' 0,0,0 '+Math.sin(a_steps[angle])*r_steps[radius]+','+r_steps[radius]*(Math.cos(a_steps[angle])-1)","style":"'stroke-width:6px;'"}},
          {"element":"path","attr":{"d":"'M 0,0 a '+r_steps[radius]+','+r_steps[radius]+' 0,0,0 '+Math.sin(a_steps[angle])*r_steps[radius]+','+r_steps[radius]*(Math.cos(a_steps[angle])-1)","style":"'stroke-width:10px;stroke-opacity:0.2;stroke:black;cursor:move;'"}},
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
          {"element":"path","attr":{"d":"'M 0,-7.5 l 7.5,7.5 l -7.5,7.5 l -7.5,-7.5 Z'","style":"'stroke-width:0;fill:#41b7dd;'"}},
          {"element":"circle","attr":{"class":"'NodeAttach1'","cx":0,"cy":0,"r":10,"style":"'fill:rgba(200,200,200,0.4);cursor:move;'"}}
        ];

parts_list.BI = {"limits":{},"part":[]};
parts_list.BI.limits.states_max = 4;
parts_list.BI.limits.states_min = 1;
parts_list.BI.part = [
          {"element":"circle","attr":{"cx":0,"cy":0,"r": 3,"style":"'fill:cyan;'"}},
          {"element":"circle","attr":{"class":"'Attach1'","cx":0,"cy":0,"r":10,"style":"'fill:rgb(100,100,100);opacity:0.5;cursor:move;'"}}
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
