
var LayoutEdit = {"tooltype":"mouse","tool":undefined,"toolsize":"1x1","rot":0,"contextmenu":false};

EditObj.Layout = {};
EditObj.Layout.gridsize = {"x":42,"y":42};
EditObj.Layout.grid = [];

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

function Layout_ContextMenu(evt){
  box = $('#LayoutContextMenu');
  box.css("display","block");
  loc = $(evt.target).offset();
  console.log(evt);
  box.css("left",(evt.pageX - loc.left)+'px');
  box.css("top",(evt.pageY - loc.top)+'px');
  LayoutEdit.contextmenu = true;
  return false;
}

function Layout_HideContextMenu(){
  $('#LayoutContextMenu').css("display","none");
  return true;
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
    $('#LayoutContainer svg g[transform="translate('+x+','+y+')"]')[0].remove();
  }
  catch(e){
    console.log(e);
  }
}

function Layout_ClearAll(){
  $.each($('#LayoutContainer svg g'),function(index,element){
    element.remove();
  });
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
