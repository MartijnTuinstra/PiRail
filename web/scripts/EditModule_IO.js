var EditObj = {};
EditObj.Blocks = [];
EditObj.Switch = [];
EditObj.Signals = [];
EditObj.Stations = [];
EditObj.LineCount = 1;
EditObj.NrSignals = 0;
EditObj.NrStation = 0;
EditObj.HOA = 0; //Highest Output Address
EditObj.HIA = 0; //Highest Input Address
EditObj.ModuleNr;

function LoadModule(evt,nr){
  clearTables();
  $(evt.target).attr("src","./img/loading.svg");
  var PropLines;

  //Load properties
  $.ajax({
    url: './../modules/'+nr+'/prop.txt',
    type: 'GET',
    async: true,
    cache: false,
    timeout: 30000,
    error: function(){
      return true;
    },
    success: function(msg){
      PropLines = msg.split("\r\n");

      EditObj.ModuleNr = nr;

      for(var i = 0;i<PropLines.length;i++){
        if(typeof PropLines[i] == "string"){
          if(i > 0 && PropLines[i].startsWith('\'')){
            PropLines.splice(i,1);
            i--;
            continue;
          }
          try {PropLines[i] = PropLines[i].split('\t');}catch(err){}

          $.each(PropLines[i],function(index){
            if(!isNaN(PropLines[i][index])){
              PropLines[i][index] = parseInt(PropLines[i][index]);
            }
          });

          if(PropLines[i][0] == "CB"){}
          switch(PropLines[i][0]){
            case "CB":
              addBlockTable(PropLines[i]);
              break;
            case "CSw":
              addSwitchTable(PropLines[i]);
              break;
            case "CMSw":
              addSwitchTable(PropLines[i]);
              break;
            case "CSig":
              addSignalTable(PropLines[i]);
              break;
            case "CSt":
              addStationTable(PropLines[i]);
              break;
            default:
              if(PropLines[i][0].startsWith('\'')){
                EditObj.Name = PropLines[i][0].slice(1);
                $('#EMtitle span').html(EditObj.Name);
              }else{
                console.log("Unknown Command: "+PropLines[i][0]);
              }
              break;
          }
        }
      }

      //Load positions for the connectors
      $.ajax({
        url: './../modules/'+nr+'/connect.txt',
        type: 'GET',
        async: true,
        cache: false,
        timeout: 30000,
        error: function(){
          return true;
        },
        success: function(msg){
          ConnectLines = msg.split("\n");

          $.each(ConnectLines,function(i){
            if(ConnectLines[0] == '\''){
              ConnectLines = ConnectLines.splice(i,1);
            }else{
              ConnectLines[i] = ConnectLines[i].split('\t');

              switch(ConnectLines[i][0]){
                case "B":
                  var z = ConnectLines[i][1];
                  EditObj.Blocks[z].Connector = {"x":ConnectLines[i][2],"y":ConnectLines[i][3]};

                  $('#ConnectBox #BBlock'+z).attr('transform','translate('+EditObj.Blocks[z].Connector.x+','+EditObj.Blocks[z].Connector.y+')');
                  break;
                case "Sw":
                  var z = ConnectLines[i][1];
                  EditObj.Switch[z].Connector = {"x":ConnectLines[i][2],"y":ConnectLines[i][3],"flip":ConnectLines[i][4]};
                  $('#ConnectBox #BSwitch'+z).attr('transform','translate('+EditObj.Switch[z].Connector.x+','+EditObj.Switch[z].Connector.y+')');
                  if(EditObj.Switch[z].Connector.flip == "v" || EditObj.Switch[z].Connector.flip == "vh"){
                    flip($('#ConnectBox #BSwitch'+z),"v");
                  }
                  if(EditObj.Switch[z].Connector.flip == "h" || EditObj.Switch[z].Connector.flip == "vh"){
                    flip($('#ConnectBox #BSwitch'+z),"h");
                  }
                  break;
                case "LA":
                  EditObj.LAnchor = {"x":ConnectLines[i][1],"y":ConnectLines[i][2]};

                  $('#ConnectBox #BAnchorL').attr('transform','translate('+EditObj.LAnchor.x+','+EditObj.LAnchor.y+')');
                  break;
                case "RA":
                  EditObj.RAnchor = {"x":ConnectLines[i][1],"y":ConnectLines[i][2]};

                  $('#ConnectBox #BAnchorR').attr('transform','translate('+EditObj.RAnchor.x+','+EditObj.RAnchor.y+')');
                  break;
                case "XY":
				  SetResizeSVG(ConnectLines[i][1],ConnectLines[i][2]);

                  EditObj.ConW = parseInt(ConnectLines[i][1]);
                  EditObj.ConH = parseInt(ConnectLines[i][2]);
              }
            }
          });


          $.each(EditObj.Blocks,function(i){
            if(EditObj.Blocks[i].Prev.Connect == 0){
              ConnectLine({"Type":"P","Source":i,"Dest":EditObj.Blocks[i].Prev});
            }
            if(EditObj.Blocks[i].Next.Connect == 0){
              ConnectLine({"Type":"N","Source":i,"Dest":EditObj.Blocks[i].Next});
            }
          });

          $.each(EditObj.Switch,function(i){
			$.each(EditObj.Switch[i].Appr,function(j){
			  if(EditObj.Switch[i].Appr[j].Connect == 0){
				ConnectLine({"Type":"A","Source":i,"Dest":EditObj.Switch[i].Appr[j]});
			  }
			});

			$.each(EditObj.Switch[i].Dest,function(j){
			  if(EditObj.Switch[i].Dest[j].Connect == 0){
				ConnectLine({"Type":"N"+(j+1),"Source":i,"Dest":EditObj.Switch[i].Dest[j]});
			  }
			});
          });

          setTimeout(function(){
            $(evt.target).attr("src","./img/setting_cog.svg");
            $('#EModules').css('display','block');
            $('#MModules').css('display','none');
          },100);
        }//End Succes Load
      });
    } //End Success Load
  });
}

function SaveModule(){
  $.post("./modules_edit_upload.php",EditObj);
}

function clearTables(){
  $('#TableBlock').html('<div class="row header red"><div class="cell">IO Address</div><div class="cell">ID nr</div><div class="cell">Type</div><div class="cell">Max speed</div><div class="cell">Direction</div><div class="cell">Length</div><div class="cell last"></div></div>');
  $('#TableSwitch').html('<div class="row header green"><div class="cell first"></div><div class="cell">ID nr</div><div class="cell">States</div><div class="cell">IO Addresses</div><div class="cell">Max speed</div><div class="cell last"></div></div>');
  $('#TableSignal').html('<div class="row header blue"><div class="cell" style="width:20px"></div><div class="cell">Nr - Name</div><div class="cell">Block</div><div class="cell">States</div><div class="cell">IO Addresses</div><div class="cell last"></div></div>');
  $('#TableStation').html('<div class="row header yellow"><div class="cell">Name</div><div class="cell">Type</div><div class="cell">Nr of Blocks</div><div class="cell">Block IDs</div><div class="cell last"></div></div>');

  EditObj.Name = "";
  EditObj.Blocks = [];
  EditObj.Switch = [];
  EditObj.Signals = [];
  EditObj.Stations = [];
  EditObj.LineCount = 0;
  EditObj.NrSignals = 0;
  EditObj.NrStation = 0;
  EditObj.HOA = 0; //Highest Output Address
  EditObj.HIA = 0; //Highest Input Address
  EditObj.Layout.grid = [];

  $('#ConnectBox').empty();

  Layout_ClearAll();

  var path = CreateSvgElement('path',{"d":"M0,0 L0,6 L9,3 z"});
  var marker = CreateSvgElement('marker',{"id":"arrow","markerWidth":"10","markerHeight":"10","refX":"0","refY":"3","orient":"auto","markerUnits":"strokeWidth"});
  var defs = CreateSvgElement('defs',{});

  marker.appendChild(path);
  defs.appendChild(marker);

  $('#ConnectBox').append(defs);

  //Left Anchor

  var box = [];
  box[0] = CreateSvgElement('path',{"class":"Container","d":"M0,0 h25 a10,10 0 0 1 10,10 v50 a10,10 0 0 1 -10,10 h-25 a10,10 0 0 1 -10,-10 v-50 a10,10 0 0 1 10,-10 z","style":"fill:#aaa"});
  box[1] = CreateSvgElement('path',{"class":"A APoint","d":"M25,15 h10 v15 h-10 a 5,5 0 0 1 -5,-5 v-5 a 5,5 0 0 1 5,-5 z","style":"cursor:crosshair","connected":""});
  box[2] = CreateSvgElement('path',{"class":"B APoint","d":"M25,40 h10 v15 h-10 a 5,5 0 0 1 -5,-5 v-5 a 5,5 0 0 1 5,-5 z","style":"cursor:crosshair","connected":""});
  box[3] = CreateSvgElement('circle',{"cx":"5","cy":"35","r":"5","style":"cursor:move"});

  var group = CreateSvgElement('g',{"id":"BAnchorL","transform":"translate(300,100)","contact":"25,22.5|25,47.5"});

  $.each(box,function(index,value){
    group.appendChild(value);
  });

  $('#ConnectBox').append(group);

  //Right Anchor

  box = [];
  box[0] = CreateSvgElement('path',{"class":"Container","d":"M0,0 h25 a10,10 0 0 1 10,10 v50 a10,10 0 0 1 -10,10 h-25 a10,10 0 0 1 -10,-10 v-50 a10,10 0 0 1 10,-10 z","style":"fill:#aaa"});
  box[1] = CreateSvgElement('path',{"class":"A APoint","d":"M-10,15 h10 a5,5 0 0 1 5,5 v5 a5,5 0 0 1 -5,5 h-10 v-15 z","style":"cursor:crosshair","connected":""});
  box[2] = CreateSvgElement('path',{"class":"B APoint","d":"M-10,40 h10 a5,5 0 0 1 5,5 v5 a5,5 0 0 1 -5,5 h-10 v-15 z","style":"cursor:crosshair","connected":""});
  box[3] = CreateSvgElement('circle',{"cx":"20","cy":"35","r":"5","style":"cursor:move"});

  group = CreateSvgElement('g',{"id":"BAnchorR","transform":"translate(300,100)","contact":"-10,22.5|-10,47.5"});

  $.each(box,function(index,value){
    group.appendChild(value);
  });

  $('#ConnectBox').append(group);

  $('#ConnectBox .Apoint').bind({mousedown:startDraw});
  $('#ConnectBox g circle').bind({mousedown:StartMove});

  //Bottom Rescale

  box = [];
  box[0] = CreateSvgElement('line',{"x1":"6","y1":"10","x2":"14","y2":"10","style":"fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"});
  box[1] = CreateSvgElement('line',{"x1":"10","y1":"6","x2":"10","y2":"14","style":"fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"});
  box[2] = CreateSvgElement('circle',{"class":"yplus","cx":"10","cy":"10","r":"8","style":"fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"});

  box[3] = CreateSvgElement('line',{"x1":"31","y1":"10","x2":"39","y2":"10","style":"fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"});
  box[4] = CreateSvgElement('circle',{"class":"ymin","cx":"35","cy":"10","r":"8","style":"fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"});

  group = CreateSvgElement('g',{"id":"vResize","transform":"translate(300,200)"});

  $.each(box,function(index,value){
    group.appendChild(value);
  });

  $('#ConnectBox').append(group);

  //Right Rescale

  box = [];
  box[0] = CreateSvgElement('line',{"x1":"6","y1":"10","x2":"14","y2":"10","style":"fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"});
  box[1] = CreateSvgElement('line',{"x1":"10","y1":"6","x2":"10","y2":"14","style":"fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"});
  box[2] = CreateSvgElement('circle',{"class":"xplus","cx":"10","cy":"10","r":"8","style":"fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"});

  box[3] = CreateSvgElement('line',{"x1":"6","y1":"35","x2":"14","y2":"35","style":"fill:none;stroke:#777777;stroke-width:1.5px;stroke-linecap:round;stroke-miterlimit:10"});
  box[4] = CreateSvgElement('circle',{"class":"xmin","cx":"10","cy":"35","r":"8","style":"fill:rgba(0,0,0,0);cursor:pointer;stroke:#777777;stroke-width:1.5px"});

  group = CreateSvgElement('g',{"id":"hResize","transform":"translate(1200,200)"});

  $.each(box,function(index,value){
    group.appendChild(value);
  });

  $('#ConnectBox').append(group);

  $('#ConnectBox .yplus').click(['y','+'],ResizeSVG);
  $('#ConnectBox .ymin' ).click(['y','-'],ResizeSVG);
  $('#ConnectBox .xplus').click(['x','+'],ResizeSVG);
  $('#ConnectBox .xmin' ).click(['x','-'],ResizeSVG);
}

function CreateSvgElement(type,list,content = ""){
  var box = document.createElementNS("http://www.w3.org/2000/svg",type);
  $.each(list,function(index,element){
    box.setAttribute(index,element);
  });

  box.appendChild(document.createTextNode(content));
  return box;
}

function addBlockTable(Line){
  //EditObj.Blocks[Line[2]] = Line;
  X = Line[2]
  EditObj.Blocks[X] = {};
  EditObj.Blocks[X].IOAddr = Line[1];
  if(Line[1] > EditObj.HIA){EditObj.HIA = Line[1];}

  EditObj.Blocks[X].Addr = Line[2];
  EditObj.Blocks[X].Type = Line[3];
  EditObj.Blocks[X].Next = {0:Line[4],1:Line[5],2:Line[6],"Connect":0};
  EditObj.Blocks[X].Prev = {0:Line[7],1:Line[8],2:Line[9],"Connect":0};
  EditObj.Blocks[X].Speed = Line[10];
  EditObj.Blocks[X].Dir = Line[11];
  EditObj.Blocks[X].Length = Line[12];
  EditObj.Blocks[X].Connector = {};
  EditObj.Blocks[X].Connector.Hidden = false;

  if(Line[13] == "N"){
    EditObj.Blocks[X].OneWay = false;
  }else{
    EditObj.Blocks[X].OneWay = true;
  }

  Type = ""
  if(Line[3] == "R"){
    Type = "Rail";
  }else if(Line[3] == "S"){
    Type ="Station";
  }else if(Line[3] == "T"){
    Type = "Sw. Det."
  }

  var text = '<div class="row row'+X+'">'+
    '<div class="cell">'+Line[1]+'</div>' +
    '<div class="cell">'+Line[2]+'</div>' +
    '<div class="cell">'+Type+'</div>' +
    '<div class="cell">'+Line[10]+'</div>' +
    '<div class="cell">'+Line[11]+'</div>' +
    '<div class="cell">'+Line[12]+'</div>' +
    '<div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editBlock(event)"/></div>' +
  '</div>';
  $('#TableBlock').append(text);

  var ID = "BBlock" + Line[2];
  var box;
  if(Line[3] != 'T'){
    box = CreateSvgElement("g",{"id":ID,"class":"BBlock","transform":"translate(0,0)","contact":"-9,17.5|44,17.5"});
  }else{
    box = CreateSvgElement("g",{"id":ID,"class":"BBlock","transform":"translate(0,0)","contact":"-9,17.5|44,17.5","style":"display:none"});
    EditObj.Blocks[X].Connector.Hidden = false;
  }
  var box_childs = [];
  box_childs[0] = CreateSvgElement("path",{"class":"BAnchor","d":"M0,0 h35 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-35 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z","style":"fill:#aaa"});
  box_childs[1] = CreateSvgElement("path",{"class":"P APoint","d":"M0,0 h5 v35 h-5 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z","style":"fill:#000;cursor:crosshair;","connected":""},"");
  box_childs[2] = CreateSvgElement("path",{"class":"N APoint","d":"M30,0 h5 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-5 z","style":"fill:#000;cursor:crosshair;","connected":""},"");
  box_childs[3] = CreateSvgElement("text",{"x":"17.5","y":"22.5","style":"line-height:5px;font-weight:bold","text-anchor":"middle"},Line[2]);
  box_childs[4] = CreateSvgElement("path",{"class":"MoveHandle","d":"M5,0 h25 v35 h-25 v-35 z","style":"fill:rgba(0,0,0,0);cursor:move"},"");

  if(Line[11] == 0){
    box_childs[5] = CreateSvgElement("line",{"x1":"10","y1":"30","x2":"20","y2":"30","style":"stroke-width:1;stroke:#000","marker-end":"url(#arrow)"});
  }else{
    box_childs[5] = CreateSvgElement("line",{"x1":"25","y1":"30","x2":"15","y2":"30","style":"stroke-width:1;stroke:#000","marker-end":"url(#arrow)"});
  }

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#BoxConnector svg').append(box);

  $('#'+ID+' .APoint').bind({mousedown:startDraw});
  $('#'+ID+' .MoveHandle').bind({mousedown:StartMove});
};

function addSwitchTable(Line){
  //EditObj.Switch[Line[1]] = Line;
  X = Line[1]
  EditObj.Switch[X] = {};
  EditObj.Switch[X].Addr = Line[1];
  if(Line[0]=="CSw"){type = "S"}else{type = "MS"}
  EditObj.Switch[X].Type = type;

  EditObj.Switch[X].Appr = [];
  EditObj.Switch[X].Appr[0] = {0:Line[2],1:Line[3],2:Line[4],"Connect":0};

  EditObj.Switch[X].Dest = [];
  EditObj.Switch[X].Dest[0] = {0:Line[5],1:Line[6],2:Line[7],"Connect":0};
  EditObj.Switch[X].Dest[1] = {0:Line[8],1:Line[9],2:Line[10],"Connect":0};

  EditObj.Switch[X].States = Line[11];
  EditObj.Switch[X].Speed = Line[13].split(" ");
  EditObj.Switch[X].IOAddr = Line[12].split(" ");

  for(var i = 0;i<Line[11];i++){
    EditObj.Switch[X].Speed[i] = parseInt(EditObj.Switch[X].Speed[i]);
    EditObj.Switch[X].IOAddr[i] = parseInt(EditObj.Switch[X].IOAddr[i]);

    if(EditObj.Switch[X].IOAddr[i] > EditObj.HOA){EditObj.HOA = EditObj.Switch[X].IOAddr[i];}
  }

  var text = '<div class="row row'+X+'">'+
    '<div class="cell first">';
    text += type + '</div>' +
    '<div class="cell">'+Line[1]+'</div>' +
    '<div class="cell">'+Line[11]+'</div>' +
    '<div class="cell">'+Line[12]+'</div>' +
    '<div class="cell">'+Line[13]+'</div>' +
    '<div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSwitch(event)"/></div>' +
  '</div>';

  $('#TableSwitch').append(text);

  var ID = "BSwitch" + Line[1];
  var box;

  box = CreateSvgElement("g",{"id":ID,"class":"BBlock","transform":"translate(0,0)","contact":"-9,17.5|44,12.5 44,22.5"});

  paths = {"A":"M0,0 h5 v35 h-5 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z","Af":"M30,0 h5 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-5 z",
          "N1":"M30,0 h5 a10,10 0 0 1 10,10 v7 h-15 z","N1f":"M0,0 h5 v17 h-15 v-7 a10,10 0 0 1 10,-10 z",
          "N2":"M30,18 h15 v7 a10,10 0 0 1 -10,10 h-5 z","N2f":"M-10,18 h15 v17 h-5 a10,10 0 0 1 -10,-10 v-7 z"};


  var box_childs = [];
  box_childs[0] = CreateSvgElement("path",{"class":"BAnchor","d":"M0,0 h35 a10,10 0 0 1 10,10 v15 a10,10 0 0 1 -10,10 h-35 a10,10 0 0 1 -10,-10 v-15 a10,10 0 0 1 10,-10 z","style":"fill:#aaa"});
  box_childs[1] = CreateSvgElement("path",{"class":"A APoint","d":paths.A,"df":paths.Af,"style":"fill:#000;cursor:crosshair;","connected":""},"");
  box_childs[2] = CreateSvgElement("path",{"class":"N1 APoint","d":paths.N1,"df":paths.N1f,"style":"fill:#000;cursor:crosshair;","connected":""},"");
  box_childs[3] = CreateSvgElement("text",{"class":"N1 APoint","x":34,"y":12.5,"locf":"-4,12.5","style":"cursor:crosshair;fill:white;font-size:10px;"},"S");
  box_childs[4] = CreateSvgElement("path",{"class":"N2 APoint","d":paths.N2,"df":paths.N2f,"style":"fill:#000;cursor:crosshair;","connected":""},"");
  box_childs[5] = CreateSvgElement("text",{"class":"N2 APoint","x":34,"y":29.5,"locf":"-4,29.5","style":"cursor:crosshair;fill:white;font-size:10px;"},"D");
  box_childs[6] = CreateSvgElement("text",{"x":"17.5","y":"22.5","style":"line-height:5px;font-weight:bold","text-anchor":"middle"},Line[1]);
  box_childs[7] = CreateSvgElement("path",{"class":"MoveHandle","d":"M5,0 h25 v35 h-25 v-35 z","style":"fill:rgba(0,0,0,0);cursor:move"},"");

  $.each(box_childs,function(index,element){
    box.appendChild(element);
  });

  $('#BoxConnector svg').append(box);

  console.log('Append Switch');

  $('#'+ID+' .APoint').bind({mousedown:startDraw});
  $('#'+ID+' .MoveHandle').bind({mousedown:StartMove});
};

function addSignalTable(Line){

  console.log(Line);

  X = EditObj.NrSignals++;
  EditObj.Signals[X] = {};
  EditObj.Signals[X].Name = Line[1];
  EditObj.Signals[X].Type = Line[2];
  EditObj.Signals[X].Block = Line[7];
  EditObj.Signals[X].Side = Line[8];
  EditObj.Signals[X].EnStates = parseInt(Line[5],16);
  //EditObj.Signals[X].IOAddress = [];
  //EditObj.Signals[X].IOAddress = Line[3].split(" ");
  if(typeof Line[3] === "string"){
    Line[3] = Line[3].split(" ");
  }
  $.each(Line[3], function(index){
    Line[3][index] = parseInt(Line[3][index]);
    if(Line[3][index] > EditObj.HOA){EditObj.HOA = Line[3][index];}
  });
  EditObj.Signals[X].IOAddress = Line[3];

  if(typeof Line[6] === "string"){
    Line[6] = Line[6].split(" ");
  }
  $.each(Line[6], function(index){
    Line[6][index] = parseInt(Line[6][index],16);
  });

  //Make Line 6 the same length as line 3
  if(Line[3].length < Line[6].length){
    Line[6] = Lin[6].slice(0,Line[3].length);
  }else if(Line[3].length > Line[6].length){
    for(var i = 0;i<(Line[3].length-Line[6].length);i++){
      Line[6].push(0);
    }
  }
  EditObj.Signals[X].States = Line[6];
  EditObj.Signals[X].Output = {'FREE':Line[3][0],'Restricted':Line[3][1],'Stop':Line[3][2]};

  var text = '<div class="row row'+X+'">'+
    '<div class="cell" style="position:relative;"><img src="./../signals/'+Line[2]+'.svg" width="20px"/></div>' +
    '<div class="cell">'+X+' - '+Line[1]+'</div>' +
    '<div class="cell">'+Line[7]+Line[8]+'</div>' +
    '<div class="cell">'+Line[4]+'</div>' +
    '<div class="cell">'+Line[3]+'</div>' +
    '<div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editSignal(event)"/></div>' +
  '</div>';
  $('#TableSignal').append(text);
}

function addStationTable(Line){
  //EditObj.Stations.push(Line);
  X = EditObj.NrStation++;
  Station = {};
  Station.Name = Line[1];
  Station.Type = Line[2];
  Station.NrOf = Line[3];
  Station.Blocks = Line[4].split(" ");
  //EditObj.Stations[X] = {};
  EditObj.Stations[X] = Station;
  type = "";
  if(Line[2] == 0){
    type = "None";
  }else if(Line[2] == 1){
    type = "Passengers";
  }else if(Line[2] == 2){
    type = "Cargo";
  }else if(Line[2] == 3){
    type = "Both";
  }
  text = '<div class="row row'+EditObj.Stations.length+'">' +
    '<div class="cell">'+Line[1]+'</div>' +
    '<div class="cell">'+type+'</div>' +
    '<div class="cell">'+Line[3]+'</div>' +
    '<div class="cell">'+Line[4]+'</div>' +
    '<div class="cell last"><img src="./img/setting_cog.svg" style="cursor:pointer;" onClick="editStation(event)"/></div>' +
  '</div>';
  $('#TableStation').append(text);
};

function ConnectLine(list){
  if(list.Dest[0] == "E" && list.Dest[1] == "E" && list.Dest[2] == "E"){
    EditObj.Blocks[list.Source].Prev.Connect = -1;
    EditObj.Blocks[list.Source].Next.Connect = -1;
    return;
  }

  var startBlock,endBlock;
  var sX=0,sY=0,eX=0,eY=0;

  if(list.Type == 'P' || list.Type == 'N'){
    startBlock = "#BBlock"+list.Source;
  }else if(list.Type == 'A' || list.Type.startsWith('N')){
	startBlock = "#BSwitch"+list.Source;
  }

  if(list.Dest[0] == "I1" || list.Dest[0] == "I2"){
    endBlock = "#BAnchorL";
  }else if(list.Dest[0] == "E1" || list.Dest[0] == "E2"){
    endBlock = "#BAnchorR"
  }else if(list.Dest[2] == "R"){ //Rail
    endBlock = "#BBlock"+list.Dest[1];
  }else if(list.Dest[2] == "S"){ //Switch Approach
	endBlock = "#BSwitch"+list.Dest[1];
  }else if(list.Dest[2] == "s"){
	return;
  }

  translationStart = $(startBlock,'#ConnectBox').attr("transform").slice(10,-1).split(',');
  sX = parseInt(translationStart[0]);
  sY = parseInt(translationStart[1]);

  var contactStart;

  if(list.Type == 'N'){ //Block Next
    EditObj.Blocks[list.Source].Next.Connect = true;

    contactStart = $(startBlock,'#ConnectBox').attr("contact").split('|')[0].split(',');
    $($('.APoint',startBlock,'#ConnectBox')[0]).attr("connected","L"+EditObj.LineCount+"_1");
  }
  else if(list.Type == 'P'){ //Block Previous
    EditObj.Blocks[list.Source].Prev.Connect = true;

    contactStart = $(startBlock,'#ConnectBox').attr("contact").split('|')[1].split(',');
    $($('.APoint',startBlock,'#ConnectBox')[1]).attr("connected","L"+EditObj.LineCount+"_1");
  }
  else if(list.Type == 'A'){ //Switch Approach
	EditObj.Switch[list.Source].Appr[0].Connect = true;

    contactStart = $(startBlock,'#ConnectBox').attr("contact").split('|')[0].split(',');
    $($('.A.APoint',startBlock,'#ConnectBox')).attr("connected","L"+EditObj.LineCount+"_1");
  }
  else if(list.Type.startsWith('N')){ //Switch Next
	nr = parseInt(list.Type.slice(1));
	EditObj.Switch[list.Source].Dest[nr-1].Connect = true;

    contactStart = $(startBlock,'#ConnectBox').attr("contact").split('|')[1].split(" ")[nr-1].split(',');
    $($('.N'+(nr)+'.APoint',startBlock,'#ConnectBox')).attr("connected","L"+EditObj.LineCount+"_1");
  }

  sX += parseInt(contactStart[0]);
  sY += parseInt(contactStart[1]);


  if(list.Dest[0] == "X" && list.Dest[2] == "R"){ //Next is Rail
    translationEnd = $("#BBlock"+list.Dest[1],'#ConnectBox').attr("transform").slice(10,-1).split(',');
    eX = parseInt(translationEnd[0]);
    eY = parseInt(translationEnd[1]);

    if(list.Type == "P"){
      EditObj.Blocks[list.Dest[1]].Next.Connect = true;

      contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[0].split(',');
      $($('.APoint',endBlock,'#ConnectBox')[0]).attr("connected","L"+EditObj.LineCount+"_2");
    }else if(list.Type == "N"){
      EditObj.Blocks[list.Dest[1]].Prev.Connect = true;

      contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[1].split(',');
      $($('.APoint',endBlock,'#ConnectBox')[1]).attr("connected","L"+EditObj.LineCount+"_2");
    }else{
	  if(!EditObj.Blocks[list.Dest[1]].Prev.Connect){
		EditObj.Blocks[list.Dest[1]].Prev.Connect = true;

		contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[1].split(',');
		$($('.APoint',endBlock,'#ConnectBox')[1]).attr("connected","L"+EditObj.LineCount+"_2");
	  }else if(!EditObj.Blocks[list.Dest[1]].Next.Connect){
		EditObj.Blocks[list.Dest[1]].Next.Connect = true;

		contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[0].split(',');
		$($('.APoint',endBlock,'#ConnectBox')[0]).attr("connected","L"+EditObj.LineCount+"_2");
	  }
	}
    eX += parseInt(contactEnd[0]);
    eY += parseInt(contactEnd[1]);

  }
  else if(list.Dest[0] == "X" && list.Dest[2] == "S"){ //Next is Switch
    translationEnd = $("#BSwitch"+list.Dest[1],'#ConnectBox').attr("transform").slice(10,-1).split(',');
    eX = parseInt(translationEnd[0]);
    eY = parseInt(translationEnd[1]);

	EditObj.Switch[list.Dest[1]].Appr[0].Connect = true;

	contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[0].split(' ')[0].split(',');
	$($('.A.APoint',endBlock,'#ConnectBox')).attr("connected","L"+EditObj.LineCount+"_2");

    eX += parseInt(contactEnd[0]);
    eY += parseInt(contactEnd[1]);
  }
  else if(list.Dest[0] == "I1"){ //Left Anchor Position 1
    translationEnd = $(endBlock,'#ConnectBox').attr("transform").slice(10,-1).split(',');
    eX = parseInt(translationEnd[0]);
    eY = parseInt(translationEnd[1]);
    contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[0].split(',');
    $($('.APoint',endBlock,'#ConnectBox')[0]).attr("connected","L"+EditObj.LineCount+"_2");
    eX += parseInt(contactEnd[0]);
    eY += parseInt(contactEnd[1]);
  }
  else if(list.Dest[0] == "I2"){ //Left Anchor Position 2
    translationEnd = $(endBlock,'#ConnectBox').attr("transform").slice(10,-1).split(',');
    eX = parseInt(translationEnd[0]);
    eY = parseInt(translationEnd[1]);
    contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[1].split(',');
    $($('.APoint',endBlock,'#ConnectBox')[1]).attr("connected","L"+EditObj.LineCount+"_2");
    eX += parseInt(contactEnd[0]);
    eY += parseInt(contactEnd[1]);

  }
  else if(list.Dest[0] == "E1"){ //Right Anchor Position 1
    translationEnd = $(endBlock,'#ConnectBox').attr("transform").slice(10,-1).split(',');
    eX = parseInt(translationEnd[0]);
    eY = parseInt(translationEnd[1]);
    contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[0].split(',');
    $($('.APoint',endBlock,'#ConnectBox')[0]).attr("connected","L"+EditObj.LineCount+"_2");
    eX += parseInt(contactEnd[0]);
    eY += parseInt(contactEnd[1]);
  }
  else if(list.Dest[0] == "E2"){ //Right Anchor Position 2
    translationEnd = $(endBlock,'#ConnectBox').attr("transform").slice(10,-1).split(',');
    eX = parseInt(translationEnd[0]);
    eY = parseInt(translationEnd[1]);
    contactEnd = $(endBlock,'#ConnectBox').attr("contact").split('|')[1].split(',');
    $($('.APoint',endBlock,'#ConnectBox')[1]).attr("connected","L"+EditObj.LineCount+"_2");
    eX += parseInt(contactEnd[0]);
    eY += parseInt(contactEnd[1]);
  }


  if(sX != 0 && sY != 0 && eX != 0 && eY != 0){
    line = CreateSvgElement('line',{"id":("L"+EditObj.LineCount),"x1":sX+"px","y1":sY+"px","x2":eX+"px","y2":eY+"px"})
    $('#ConnectBox').append(line);
  }
  console.log(list);

  EditObj.LineCount += 1;
}

function editBlock(evt){
  $('#EModulesBox .Block').css('display','block');
  $('#EModulesBox').css('display','block');

  var id = parseInt($($('div',evt.target.parentNode.parentNode)[1]).html());

  $('#EModulesBox .Block .Addr')[0].disabled = true;

  $('#EModulesBox .Block .Addr').val(id);
  $('#EModulesBox .Block .IOAddr').val(EditObj.Blocks[id].IOAddr);
  $('#EModulesBox .Block .MaxSPD').val(EditObj.Blocks[id].Speed);
  $('#EModulesBox .Block .Len').val(EditObj.Blocks[id].Length);

  if((t = EditObj.Blocks[id].Type) == "S"){
    $('#EModulesBox .Block .Type').val("Station");
  }else if(t == "R"){
    $('#EModulesBox .Block .Type').val("Rail");
  }else if(t == "T"){
    $('#EModulesBox .Block .Type').val("Switch Detection");
  }

  if((t = EditObj.Blocks[id].Dir) == 0){
    $('#EModulesBox .Block .Dir').val("Clockwise (->)");
  }else if(t == 1){
    $('#EModulesBox .Block .Dir').val("CounterClockwise (<-)");
  }else if(t == 2){
    $('#EModulesBox .Block .Dir').val("Switching");
  }

  if(EditObj.Blocks[id].OneWay){
    $('#EModulesBox .Block .OneWay')[0].checked = true;
  }else{
    $('#EModulesBox .Block .OneWay')[0].checked = false;
  }
  //Setting select
  //document.getElementById('personlist').value
}

function SaveEditBlock(evt){
  box = $(evt.target.parentNode);


  nr = parseInt($('.Addr',box).val());
  if($('.Addr',box)[0].disabled == false){
    if((typeof EditObj.Blocks[nr] === 'undefined') == false){
      alert("Address is already in use");
      return false;
    }else{
      var T,D;
      if((t = $('#EModulesBox .Type').val()) == "Station"){
        T = "S";
      }else if(t == "Rail"){
        T = "R";
      }else if(t == "Switch Detection"){
        T = "T";
      }
      if((t = $('#EModulesBox .Dir').val()) == "Clockwise (->)"){
        D = 0;
      }else if(t == "CounterClockwise (<-)"){
        D = 1;
      }else if(t == "Switching"){
        D = 2;
      }
      addBlockTable(["CB",parseInt($('.IOAddr',box).val()),parseInt($('.Addr',box).val()),T,"E","E","E","E","E","E",parseInt($('.MaxSPD',box).val()),D,parseInt($('.Len',box).val())]);
      if((typeof EditObj.Blocks[nr+1] === 'undefined') == false){
        $('.IO_table_wrapper.blocks .row'+(nr-1)).after($('.IO_table_wrapper.blocks .row'+nr));
      }
    }
  }

  EditObj.Blocks[nr].IOAddr = parseInt($('.IOAddr',box).val());
  $('.row'+nr+' div:nth-child(1)','.IO_table_wrapper.blocks').html(parseInt($('.IOAddr',box).val()));

  EditObj.Blocks[nr].Speed = parseInt($('.MaxSPD',box).val());
  $('.row'+nr+' div:nth-child(4)','.IO_table_wrapper.blocks').html(parseInt($('.MaxSPD',box).val()));

  EditObj.Blocks[nr].Length = parseInt($('.Len',box).val());
  $('.row'+nr+' div:nth-child(6)','.IO_table_wrapper.blocks').html(parseInt($('.Len',box).val()));

  $('.row'+nr+' div:nth-child(3)','.IO_table_wrapper.blocks').html($('#EModulesBox .Type').val());

  if((t = $('.Type,box').val()) == "Station"){
    EditObj.Blocks[nr].Type = "S";
  }else if(t == "Rail"){
    EditObj.Blocks[nr].Type = "R";
  }else if(t == "Switch Detection"){
    EditObj.Blocks[nr].Type = "T";
  }
  if((t = $('.Dir',box).val()) == "Clockwise (->)"){
    EditObj.Blocks[nr].Dir = 0;
  }else if(t == "CounterClockwise (<-)"){
    EditObj.Blocks[nr].Dir = 1;
  }else if(t == "Switching"){
    EditObj.Blocks[nr].Dir = 2;
  }

  if($('.OneWay',box)[0].checked == true){
    EditObj.Blocks[nr].OneWay = true;
  }else{
    EditObj.Blocks[nr].OneWay = false;
  }

  $('.row'+nr+' div:nth-child(5)','.IO_table_wrapper.blocks').html(EditObj.Blocks[nr].Dir);


  return true;
  //$('.row'+nr+' div:nth-child(2)','.IO_table_wrapper.blocks').html("Test");
}

function DeleteBlock(evt){
  box = $(evt.target.parentNode);
  id = $('.Addr',box).val();
  if (confirm("You're gonna delete Block "+id+"\nAre you sure??") == false) {
    return false;
  }

  $('.row'+id,'.IO_table_wrapper.blocks').remove();
  $('#BBlock'+id+' path[connected]','#ConnectBox').each(function(){
    if((ido = $(this).attr("connected").slice(0,-2)) != ""){
      line = $('#'+ido,'#ConnectBox').get(0);
      parent = line.parentNode;
      parent.removeChild(line);
    }
  });
  //$('#BBlock'+id,'#BoxConnector').remove();
  box = $('#BBlock'+id,'#BoxConnector').get(0);
  parent = box.parentNode;
  parent.removeChild(box);
  delete EditObj.Blocks[id];
  return true;
}

function editSwitch(evt){

}

function editSignal(evt){
  $('#EModulesBox .Signal').css('display','block');
  $('#EModulesBox').css('display','block');

  var box = $('#EModulesBox .Signal');

  var id = parseInt($($('div',evt.target.parentNode.parentNode)[1]).html().split(" - ")[0]);

  $('.Addr',box)[0].disabled = true;

  $('.Addr',box).val(id);
  $('.Name',box).val(EditObj.Signals[id].Name);
  $('.S_Block',box).val(EditObj.Signals[id].Block);

  $('[name=type][value='+EditObj.Signals[id].Type+']',box)[0].checked = true;

  if(EditObj.Signals[id].Side == "N"){
    $('[name=side][value=N]',box)[0].checked = true;
  }else{
    $('[name=side][value=P]',box)[0].checked = true;
  }

  var j = 0;

  $('.IOACon',box).empty();

  $('.IOACon',box).css("width","40px");

  var text = '<div class="header">IO</div>'+
  '<div class="IOAddr Add"><div><img src="./img/plus.png" style="margin-top:2px;height:16px;cursor:pointer"/></div></div>';

  $('.IOACon',box).append(text);

  //Set the enable checkboxes
  var flag = EditObj.Signals[id].EnStates;

  $('.IOEn input[type=checkbox]',box)[0].checked = flag & 0x80; //Free / Green
  $('.IOEn input[type=checkbox]',box)[1].checked = flag & 0x40; //Ready to stop / Orange
  $('.IOEn input[type=checkbox]',box)[2].checked = flag & 0x20; //Stop / Red
  $('.IOEn input[type=checkbox]',box)[3].checked = flag & 0x10; //Slow-free / Flashing Green / Yellow
  $('.IOEn input[type=checkbox]',box)[4].checked = flag & 0x8;  //Yard / Flashing Orange / DarkRed
  //$('.IOEn input[type=checkbox]',box)[5].checked = flag & 0x4;
  //$('.IOEn input[type=checkbox]',box)[0].checked = flag & 0x40;
  //$('.IOEn input[type=checkbox]',box)[0].checked = flag & 0x40;



  for(var i = 0;i<EditObj.Signals[id].IOAddress.length;i++){
    text = '<div class="IOAddr';

    if(j == 0){
      j++;
      text += " first";
    }

    flag = EditObj.Signals[id].States[i];

    text += '"><div><input type="text" value="'+EditObj.Signals[id].IOAddress[i]+'"/></div>'+
      '<span><input type="checkbox"';
    if(flag & 0x80){text += ' checked'}
    text += '/></span><span><input type="checkbox"';
    if(flag & 0x40){text += ' checked'}
    text += '/></span><span><input type="checkbox"';
    if(flag & 0x20){text += ' checked'}
    text += '/></span><span><input type="checkbox"';
    if(flag & 0x10){text += ' checked'}
    text += '/></span><span><input type="checkbox"';
    if(flag & 0x8){text += ' checked'}
    text += '/></span></div>';

    $('.IOACon .IOAddr.Add',box).before(text);
    $('.IOACon',box).css("width",parseInt($('.IOACon',box).css("width").slice(0,-2))+40+"px");
  }
}

function SaveEditSignal(evt){

  box = $(evt.target.parentNode);


  nr = parseInt($('.Addr',box).val());
  console.log(nr);
  if(typeof $('.Addr',box)[0].disabled === 'undefined' || $('.Addr',box)[0].disabled == false){
    if((typeof EditObj.Signals[nr] === 'undefined') == false){
      alert("Address is already in use");
      return false;
    }else{
      var ENsts;
      $.each($('.IOEn input[type=checkbox]',box),function(index,element){
        if(element.checked == true){
          ENsts |= (1 << (7-i))
        }else{
          ENsts &= ~(1 << (7-i))
        }
        i++;
      });

      var IOAddr = [],IOout = [];
      $.each($('.IO_Wrapper input[type=text]',box),function(index,element){
        var val = element.value;

        if(val != "" && !isNaN(val)){
          IOAddr.push(parseInt(val));
        }else{
          alert("IO Address should be a number!!");
          return false;
        }
      });

      $.each($('.IOAddr:not(.Add)',box),function(index,element){
        IOout.push(0);
        $.each($('input[type=checkbox]',$(element)),function(index2,element2){
          if(element2.checked == true){
            IOout[index] |= (1 << (7-index2))
          }else{
            IOout[index] &= ~(1 << (7-index2))
          }
        });
      });

      addSignalTable(["CSig",$('.Name',box).val(),$('.type input[type=radio]:checked',box)[0].value,IOAddr,IOAddr.length,ENsts,IOout,"E","E"]);
      /*if((typeof EditObj.Blocks[nr+1] === 'undefined') == false){
        $('.IO_table_wrapper.blocks .row'+(nr-1)).after($('.IO_table_wrapper.blocks .row'+nr));
      }*/
    }
  }

  //Type
  EditObj.Signals[nr].Type = $('.type input[type=radio]:checked',box)[0].value;
  $('#TableSignal .row'+nr+' div:nth-child(1) img').attr("src","./img/Signals/"+EditObj.Signals[nr].Type+".svg");

  var i = 0;

  //Enabled States
  $.each($('.IOEn input[type=checkbox]',box),function(index,element){
    if(element.checked == true){
      EditObj.Signals[nr].EnStates |= (1 << (7-i))
    }else{
      EditObj.Signals[nr].EnStates &= ~(1 << (7-i))
    }
    i++;
  });

  i = 0;

  //IO Addresses
  $.each($('.IO_Wrapper input[type=text]',box),function(index,element){
    var val = element.value;

    if(val == ""){
      EditObj.Signals[nr].IOAddress.splice(index,1);
      EditObj.Signals[nr].States.splice(index,1);
      element.parentNode.parentNode.remove();
      $('.IOACon',box).css("width",parseInt($('.IOACon',box).css("width").slice(0,-2))-40+"px");
      i++;
    }else if(!isNaN(val)){
      EditObj.Signals[nr].IOAddress[index-i] = parseInt(val);
    }else{
      alert("IO Address should be a number!!");
      return false;
    }
  });

  $('#TableSignal .row'+nr+' div:nth-child(5)').html(EditObj.Signals[nr].IOAddress.toString());

  //Output States
  $.each($('.IOAddr:not(.Add)',box),function(index,element){
    $.each($('input[type=checkbox]',$(element)),function(index2,element2){
      if(element2.checked == true){
        EditObj.Signals[nr].States[index] |= (1 << (7-index2))
      }else{
        EditObj.Signals[nr].States[index] &= ~(1 << (7-index2))
      }
    });
  });

  return true;
}

function DeleteSignal(evt){
  box = $(evt.target.parentNode);
  id = $('.Addr',box).val();
  if (confirm("You're gonna delete Signal "+id+"\nAre you sure??") == false) {
    return false;
  }

  $('.row'+id,'.IO_table_wrapper.signals').remove();

  delete EditObj.Signals[id];
  return true;
}


function editStation(evt){
  $('#EModulesBox .Station').css('display','block');
  $('#EModulesBox').css('display','block');

  var box = $('#EModulesBox .Station');

  var id = parseInt($(evt.target.parentNode.parentNode).attr("class").split(" ")[1].slice(3)) - 1;

  console.log(EditObj.Stations[id]);

  $('.Addr',box)[0].disabled = true;

  $('.Addr',box).val(id);
  $('.Name',box).val(EditObj.Stations[id].Name);

  if((t = EditObj.Stations[id].Type) == 0){
    $('.Type',box).val("None");
  }else if(t == 1){
    $('.Type',box).val("Passengers");
  }else if(t == 2){
    $('.Type',box).val("Cargo");
  }else if(t == 3){
    $('.Type',box).val("Both");
  }

  $('.StationBlocks',box).empty();

  string = ""
  $.each(EditObj.Stations[id].Blocks,function(i,v){
    if(i != 0){
      string += "<br/>";
    }
    string += '<input type="number" value="'+v+'"/>';
  })
  $('.StationBlocks',box).append(string+'<img src="./img/plus.png">');

  $('.StationBlocks img',box).click(box,addStationBlock);
}

function addStationBlock(evt){
	$('.StationBlocks img',evt.data[0]).before('<input type="number"/>');
}

function SaveEditStation(evt){
  box = $(evt.target.parentNode);


  nr = parseInt($('.Addr',box).val());
  console.log(nr);
  if(typeof $('.Addr',box)[0].disabled === 'undefined' || $('.Addr',box)[0].disabled == false){
    if((typeof EditObj.Stations[nr] === 'undefined') == false){
      alert("Address is already in use");
      return false;
    }else{
      /*
      var ENsts;
      $.each($('.IOEn input[type=checkbox]',box),function(index,element){
        if(element.checked == true){
          ENsts |= (1 << (7-i))
        }else{
          ENsts &= ~(1 << (7-i))
        }
        i++;
      });

      var IOAddr = [],IOout = [];
      $.each($('.IO_Wrapper input[type=text]',box),function(index,element){
        var val = element.value;

        if(val != "" && !isNaN(val)){
          IOAddr.push(parseInt(val));
        }else{
          alert("IO Address should be a number!!");
          return false;
        }
      });

      $.each($('.IOAddr:not(.Add)',box),function(index,element){
        IOout.push(0);
        $.each($('input[type=checkbox]',$(element)),function(index2,element2){
          if(element2.checked == true){
            IOout[index] |= (1 << (7-index2))
          }else{
            IOout[index] &= ~(1 << (7-index2))
          }
        });
      });

      addSignalTable(["CSig",$('.Name',box).val(),$('.type input[type=radio]:checked',box)[0].value,IOAddr,IOAddr.length,ENsts,IOout,"E","E"]);
      /*if((typeof EditObj.Blocks[nr+1] === 'undefined') == false){
        $('.IO_table_wrapper.blocks .row'+(nr-1)).after($('.IO_table_wrapper.blocks .row'+nr));
      }*/
      console.log("Create New");
    }
  }

  //Name
  EditObj.Stations[nr].Name = $('.Name',box).val();
  //Type
  if((t = $('.Type',box).val()) == "None"){
    EditObj.Stations[nr].Type = 0;
  }else if(t == "Passengers"){
    EditObj.Stations[nr].Type = 1;
  }else if(t == "Cargo"){
    EditObj.Stations[nr].Type = 2;
  }else if(t == "Both"){
    EditObj.Stations[nr].Type = 3;
  }

  var i = 0;

  //Enabled States
  EditObj.Stations[0].Blocks = [];
  string = "";
  $.each($('.StationBlocks input',box),function(index,element){
    if(!isNaN(element.value) && element.value != ""){
      if(i != 0){
        string += " ";
      }
      string += element.value;
      i++;
      EditObj.Stations[0].Blocks.push(element.value);
    }
  });

  $('#TableStation .row'+(nr+1)+' div:nth-child(1)').html(EditObj.Stations[nr].Name);
  $('#TableStation .row'+(nr+1)+' div:nth-child(2)').html($('.Type',box).val());
  $('#TableStation .row'+(nr+1)+' div:nth-child(3)').html(i);
  $('#TableStation .row'+(nr+1)+' div:nth-child(4)').html(string);

  return true;
}

function DeleteStation(evt){
  box = $(evt.target.parentNode);
  id = parseInt($('.Addr',box).val());
  if (confirm("You're gonna delete Station "+(id+1)+"\nAre you sure??") == false) {
    return false;
  }

  $('.row'+(id+1),'.IO_table_wrapper.stations').remove();

  delete EditObj.Stations[id];
  return true;
}
