var Stations = {
	// Platform state: 0 = free, 1 = stopped, 2 = leaving, 3 = arriving
	data: [{name:"Small Station",platforms:[{name:"Spoor 1",type:"P",parts:2,state:0},{name:"Spoor 2",type:"P",parts:2,state:1},{name:"Spoor 3",type:"P",parts:2,state:2},{name:"Spoor 4",type:"P",parts:2,state:3},{name:"Spoor 5",type:"P",parts:2,state:0}]},
		{name:"Small Yard",platforms:[{name:"Siding",type:"Y",parts:1,state:0},{name:"Mainline east",type:"C",parts:1,state:0},{name:"Mainline west",type:"C",parts:1,state:0}]}],
	selected: [],
	init: function(){
		$('#route .station').off("click");
		$('#route').empty();

		var content = "";

		$.each(this.data,function(i,v){

			content += '<div class="station_Box">' +
						'<div class="header">'+v.name+'</div><div class="scroller">';

			$.each(v.platforms,function(i2,v2){
				content += '<div class="station" id="St'+i+"-"+i2+'">'
				content += '<div class="station_type ';
				content += ((v2.type == "P")?"person":((v2.type == "C")?"cargo":"storage"))
				content += '"></div><div class="station_name">'+v2.name+'</div>' +
							'<div class="station_DCC_id">';
				content += '...';
				content += '</div>' +
							'<div class="station_state"><small>';
				content += ((v2.state == 1)?"Stopped":((v2.state == 2)?"Leaving":((v2.state == 3)?"Arriving":"")));
				content += '</small> ..:..</div>'
				content += (v2.parts == 1)?('<div class="length_1 lengthL lengthR"></div>'):('<div class="length_2 lengthL">a</div><div class="length_2 m"></div><div class="length_2 lengthR">b</div>');
				content += '</div>';
			});

			content += '</div></div>';
		});

		$('#route').html(content);

		$('#route .station').on("click",function(evt){Stations.click(evt);});
	},

	import: function(data){
		console.warn("Implement: station import");
	},

	select: function(ids){
		Sid = parseInt(ids[0]); // Station nr
		Pid = parseInt(ids[1]); // Platform nr

		if(!$('#route #St'+Sid+'-'+Pid+'.station').hasClass("active")){
			if(this.selected.length == 1){
				if(ids[2] != undefined){
					$('#route #St'+Sid+'-'+Pid+' .length_'+this.data[Sid].platforms[Pid].parts).toggleClass("active");

					this.selected.push({S:Sid,P:Pid,sP:ids[2]});

					if(this.selected.length == 2){
						$('#route .station.active').toggleClass("active");
						$('#route .station .active').toggleClass("active");
						Train.setRoute(this.selected[0].dcc,this.selected[0],this.selected[1]);
						this.selected = [];
					}
				}
				else if(this.data[Sid].platforms[Pid].parts > 1){
					$('#route #St'+Sid+'-'+Pid+' .length_'+this.data[Sid].platforms[Pid].parts).toggleClass("active");

				}
			}
			else if(this.data[Sid].platforms[Pid].state == 1){
				this.selected.push({S:Sid,P:Pid});
				$('#route #St'+Sid+'-'+Pid+'.station').toggleClass("active");
			}
			else{
				if(Train.selected.length >= 1){
					Train.setRoute(Train.data[Train.selected[0]].dcc,undefined,{S:Sid,P:Pid});
					$('#route #St'+Sid+'-'+Pid+'.station').toggleClass("active");
					setTimeout(function(){$('#route #St'+Sid+'-'+Pid+'.station').toggleClass("active");},500);
				}
			}
		}
	},
	click: function(evt){
		obj = evt.currentTarget.id.slice(2).split("-");
		if($(evt.target).hasClass("length_1") || $(evt.target).hasClass("length_2")){
			obj.push($(evt.target).text());
		}
		this.select(obj)
	}
}
