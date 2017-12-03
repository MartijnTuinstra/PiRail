int pathFinding(struct adr Begin, struct adr End, struct Sw_PATH *(OUT_Sw_Nodes)[MAX_ROUTE]){
	struct adr NAdr,SNAdr,Adr = Begin;
	struct Sw_A_PATH * Sw_Nodes[MAX_ROUTE] = {NULL};
	int nr_switches = 0;
	struct Sw_A_PATH * Prev_PATH_Node = NULL;
	struct Sw_A_PATH * Curr_PATH_Node = NULL;

	char list_of_M[MAX_Modules] = {0};

	_Bool init = TRUE;
	_Bool found = FALSE;

	int a = 0;

	printf("S\t%i:%i:%i\ttype:%c\n",Adr.M,Adr.B,Adr.S,Adr.type);
	//printf("\n%i\n",i);
	int direct = blocks[Adr.M][Adr.B][Adr.S]->dir;
	char prev_dir = direct;
	Next_Adr:{};

	if(Adr.type == 'e'){
		goto Next_Switch;
	}

	char dir = blocks[Adr.M][Adr.B][Adr.S]->dir;

	if(prev_dir == 0 && dir == 1){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	else if(prev_dir == 1 && dir == 0){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}
	else if(prev_dir == 129 && dir == 0){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	else if(prev_dir == 128 && dir == 1){
		prev_dir ^= 0x80;	//Reverse
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}
	else if((prev_dir >> 7) == 1){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
		}else{
			NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
		}
	}
	else if(dir == 0 || dir == 2 || dir == 5){
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->NAdr;
	}
	else{
		NAdr = blocks[Adr.M][Adr.B][Adr.S]->PAdr;
	}

	prev_dir = (prev_dir & 0xC0) + (dir & 0xF);

	Next_Switch:{};
	//printf("\n");

	//printf("%i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);

	if(NAdr.M == End.M && NAdr.B == End.B && NAdr.S == End.S){
		//printf("\n");
		FOUND:{}

		//Set all switches in try mode to Successfull
		//and register the module number
		for(int i = 0;i<MAX_ROUTE;i++){
			if(Sw_Nodes[i]){
				for(int j = 0;j<10;j++){
					if(Sw_Nodes[i]->suc[j] == 1){
						Sw_Nodes[i]->suc[j] = 2;
					}
				}
				_Bool A = FALSE;
				for(int k = 0;k<MAX_Modules;k++){
					if(list_of_M[k] == 0 || list_of_M[k] == Sw_Nodes[i]->adr.M){
						list_of_M[k] = Sw_Nodes[i]->adr.M;
						break;
					}
				}
			}else{
				break;
			}
		}
		found = TRUE;

		for(int i = 0;i<MAX_ROUTE;i++){
			//printf("i:%i\t",i);
			if(Sw_Nodes[i] != NULL){
				//printf("Adr.type = %c\t",Sw_Nodes[i]->adr.type);
				if(Sw_Nodes[i]->adr.type == 'S' && (Sw_Nodes[i]->suc[0] == 0 || Sw_Nodes[i]->suc[1] == 0)){
					Curr_PATH_Node = Sw_Nodes[i];
          NAdr = Adr = Curr_PATH_Node->adr;
          dir = prev_dir = Curr_PATH_Node->dir;
					printf("Retrying %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
					usleep(1000);
					goto Next_Switch;
				}else if((Sw_Nodes[i]->adr.type == 'm' || Sw_Nodes[i]->adr.type == 'M')){
					printf("Test Moduls\n");
					/*
					for(int j = 0;j<Moduls[Sw_Nodes[i]->adr.M][Sw_Nodes[i]->adr.B][Sw_Nodes[i]->adr.S]->length;j++){
						if(Sw_Nodes[i]->suc[j] == 0){
							Curr_PATH_Node = Sw_Nodes[i];
							NAdr = Adr = Curr_PATH_Node->adr;
							dir = prev_dir = Curr_PATH_Node->dir;
							printf("Retrying %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
							usleep(1000);
							goto Next_Switch;
						}
					}
					*/
				}
			}else{
				break;
			}
		}
		printf("FOUND the route\n");
		goto DONE;
		return 1;//blocks[Adr.M][Adr.B][Adr.S];
	}
	usleep(2000);
	//printf("i:%i",i);

	//printf("prev dir:%i\tdir:%i\t",prev_dir,dir);
	//printf("%i:%i:%i\t=>\t%i:%i:%i\ttype:%c\t",Adr.M,Adr.B,Adr.S,NAdr.M,NAdr.B,NAdr.S,NAdr.type);

	if(NAdr.type == 'e' || (NAdr.type == 'R') && blocks[NAdr.M][NAdr.B][NAdr.S]->oneWay == 1 && (char)(dir + (prev_dir & 0x80)) != blocks[NAdr.M][NAdr.B][NAdr.S]->dir){ //End of rails
    if(NAdr.type == 'e'){
	    //printf("End of rails returning to Current PATH Node\n");
    }else{
      //printf("Wrong Way!!\n");
    }
		goto FAIL;
	}
	else if(NAdr.M == Begin.M && NAdr.B == Begin.B && NAdr.S == Begin.S){ //Back at starting Node??
		//printf("Back at starting Node\n");
		goto FAIL;
	}
	else if(NAdr.type == 'R'){ //If rail
		Adr = NAdr;
		goto Next_Adr;
	}
	else if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'm' || NAdr.type == 'M'){ //Next node is a (MS) Switch
		//printf("\ni:%i\tC:%i:%i:%i type:%c\n",i,NAdr.M,NAdr.B,NAdr.S,NAdr.type);
		if(blocks[NAdr.M][NAdr.B][0] != NULL){
			a++;
		}
		if(NAdr.type == 'S'){
			//printf("N%i %i:%i:%i\n",nr_switches++,NAdr.M,NAdr.B,NAdr.S);
			goto New_S_Node;
			Ret_S_Node:{}

			if(Curr_PATH_Node != NULL){
				if(Curr_PATH_Node->suc[0] == 1){
					//printf("Straight\t");
					SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Str;
				}else if(Curr_PATH_Node->suc[1] == 1){
					//printf("Diverging\t");
					SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->Div;
				}
			}
		}
		else if(NAdr.type == 's'){
			SNAdr = Switch[NAdr.M][NAdr.B][NAdr.S]->App;
      //printf("SNAdr: %i:%i:%i\n",SNAdr.M,SNAdr.B,SNAdr.S);
		}
		else if(NAdr.type == 'M'){
			goto New_M_Node;
			Ret_M_Node:{}
			//printf("m\n");
			if(Curr_PATH_Node != NULL){
				for(int i = 0;i<Moduls[NAdr.M][NAdr.B][NAdr.S]->length;i++){
					if(Curr_PATH_Node->suc[i] == 1){
						//printf("\tState: %i\t",i);
						SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->mAdr[i];
						break;
					}
				}
			}
		}
		else if(NAdr.type == 'm'){
			goto New_M_Node;
			Ret_m_Node:{}
			//printf("m\n");
			if(Curr_PATH_Node != NULL){
				for(int i = 0;i<Moduls[NAdr.M][NAdr.B][NAdr.S]->length;i++){
					if(Curr_PATH_Node->suc[i] == 1){
						//printf("\tState: %i\t",i);
						SNAdr = Moduls[NAdr.M][NAdr.B][NAdr.S]->MAdr[i];
					}
				}
			}
		}
		//printf("SN %i:%i:%i%c\tN %i:%i:%i\ti:%i\n",SNAdr.M,SNAdr.B,SNAdr.S,SNAdr.type,NAdr.M,NAdr.B,NAdr.S,i);
		if(SNAdr.type == 'S' || SNAdr.type == 's'){
			NAdr = SNAdr;
			goto Next_Switch;
		}else if(SNAdr.type == 'M' || SNAdr.type == 'm'){
			NAdr = SNAdr;
			goto Next_Switch;

		}else{
			Adr = SNAdr;
      //printf("prev dir:%i\t%i:%i:%i\t=>\t%i:%i:%i\ttype:%c\t",prev_dir,NAdr.M,NAdr.B,NAdr.S,Adr.M,Adr.B,Adr.S,Adr.type);
			NAdr = Adr;
			goto Next_Switch;
		}
	}else{
		printf("\n\n!!!!!!!!!UNKOWN RAIL TYPE!!!!!\n\n");
	}
  printf("Return 0\n");
	return 0;

	FAIL:{
		if(Curr_PATH_Node != NULL){
			//printf("FAIL %i:%i:%i:\t",Curr_PATH_Node->adr.M,Curr_PATH_Node->adr.B,Curr_PATH_Node->adr.S);
      if(Curr_PATH_Node->adr.type == 'S'){
        if(Curr_PATH_Node->suc[0] == 1){
          Curr_PATH_Node->suc[0] = -1;
	        Curr_PATH_Node->suc[1] = 1;
					NAdr = Curr_PATH_Node->adr;
					dir = prev_dir = Curr_PATH_Node->dir;
					//printf("[%i][%i]\n",Curr_PATH_Node->suc[0],Curr_PATH_Node->suc[1]);
					if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
						//printf("Next_Switch\n");
						goto Next_Switch;
					}else{
						goto Next_Adr;
					}
        }else if(Curr_PATH_Node->suc[1] == 1){
          Curr_PATH_Node->suc[1] = -1;
					//printf("[%i][%i]\n",Curr_PATH_Node->suc[0],Curr_PATH_Node->suc[1]);
        }else if(Curr_PATH_Node->suc[0] == 2 || Curr_PATH_Node->suc[1] == 2){
					if(!Curr_PATH_Node->Prev){
						goto NOT_FOUND;
					}else if(found == TRUE){
						goto FOUND;
					}
				}
      }
			else{
  			for(int i = 0;i<10;i++){
  				if(Curr_PATH_Node->suc[i] == 1){
  					Curr_PATH_Node->suc[i] = -1;
						if((i+1) < Moduls[Curr_PATH_Node->adr.M][Curr_PATH_Node->adr.B][Curr_PATH_Node->adr.S]->length){
	  					Curr_PATH_Node->suc[i+1] = 1;
						}
  					NAdr = Curr_PATH_Node->adr;
  					dir = prev_dir = Curr_PATH_Node->dir;
						//printf("[%i]",Curr_PATH_Node->suc[i]);
  					if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
  						//printf("\nNext_Switch\n");
  						goto Next_Switch;
  					}else{
							//printf("\n");
  						goto Next_Adr;
  					}
  				}
  			}
      }
			Curr_PATH_Node = Curr_PATH_Node->Prev;
			goto FAIL;
		}
	}

	printf("Return FAIL\n");
	return 0;

	New_S_Node:{
		//printf("NSN\t");
		if((Curr_PATH_Node &&	!Adr_Comp(Curr_PATH_Node->adr,NAdr)) ||	!Curr_PATH_Node){
			if(Sw_Nodes[0] != NULL && Adr_Comp(Sw_Nodes[0]->adr,NAdr)){ //Back to first discovered turnout??
				//printf("Back to start!!\n");

				if(Curr_PATH_Node != NULL){
					for(int i = 0;i<10;i++){
						if(Curr_PATH_Node->suc[i] == 1){
							Curr_PATH_Node->suc[i] = -1;
							NAdr = Curr_PATH_Node->adr;
							dir = Curr_PATH_Node->dir;
							prev_dir = dir;
							if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
								goto Next_Switch;
							}else{
								goto Next_Adr;
							}
						}
					}
				}
			}
			else if(found == TRUE){
				//printf("Check if between borders\t");
				for(int i = 0;i<MAX_Modules;i++){
					if(list_of_M[i] == NAdr.M){
						break;
					}
					//Oustide
					if((i+1) == MAX_Modules){
						if(Curr_PATH_Node != NULL){
							for(int i = 0;i<10;i++){
								if(Curr_PATH_Node->suc[i] == 1){
									Curr_PATH_Node->suc[i] = -1;
									NAdr = Curr_PATH_Node->adr;
									dir = Curr_PATH_Node->dir;
									prev_dir = dir;
									if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
										goto Next_Switch;
									}else{
										goto Next_Adr;
									}
								}
							}
							if(found == TRUE){
								goto FOUND;
							}
							Curr_PATH_Node = Curr_PATH_Node->Prev;
						}
					}
				}
				//printf("Check if allready known??\n");
				for(int i = 0;i<MAX_ROUTE;i++){
					if(Sw_Nodes[i] != NULL && Adr_Comp(Sw_Nodes[i]->adr,NAdr)){
						//printf("Allready known Node\n");
						if(Sw_Nodes[i]->adr.type == 'S'){
							//printf("It a switch\t");
							if(Sw_Nodes[i]->suc[0] == -1 && Sw_Nodes[i]->suc[1] == -1){
								//printf("Both have no route\t");
								goto FAIL;
							}
						}
						goto FOUND;
						break;
					}
				}
			}

			//printf("Save PATH Node\tDir=%i\t",dir);

			struct Sw_A_PATH * Z = (struct Sw_A_PATH *)malloc(sizeof(struct Sw_A_PATH));

			Z->adr = NAdr;
			Z->length = 2;
			memset(Z->suc,0,10);
			Z->suc[0] = 1;
			Z->dir = dir + (prev_dir & 0x80);
			Z->Prev = Curr_PATH_Node;

			for(int i = 0;i<MAX_ROUTE;i++){
				if(Sw_Nodes[i] == NULL){
					//printf("i:%i\t",i);
					Sw_Nodes[i] = Z;
					//printf("%i:%i:%i:%c",Sw_Nodes[i]->adr.M,Sw_Nodes[i]->adr.B,Sw_Nodes[i]->adr.S,Sw_Nodes[i]->adr.type);
					break;
				}else{
					//printf("No\t%i:%i:%i:%c\n",Sw_Nodes[i]->adr.M,Sw_Nodes[i]->adr.B,Sw_Nodes[i]->adr.S,Sw_Nodes[i]->adr.type);
				}
			}
			//printf("\n");
			Curr_PATH_Node = Z;
		}
		else if(Curr_PATH_Node != NULL && Adr_Comp(Curr_PATH_Node->adr,NAdr)){ //If current Node exists
			//printf("Success [%i][%i]",Curr_PATH_Node->suc[0],Curr_PATH_Node->suc[1]);
			if(Curr_PATH_Node->suc[0] != 0 && Curr_PATH_Node->suc[0] != 1){
				if(Curr_PATH_Node->suc[1] == 0){
					//printf("\tTry next");
					Curr_PATH_Node->suc[1] = 1;
				}else if(Curr_PATH_Node->suc[1] == -1){
					goto FAIL;
				}
			}
		}
		goto Ret_S_Node;
	}

	printf("Return New_S_Node\n");
	return 0;

	New_M_Node:{
		//printf("NMN\t");
		if((Curr_PATH_Node && !Adr_Comp(Curr_PATH_Node->adr,NAdr)) || !Curr_PATH_Node){
			if(Sw_Nodes[0] != NULL && Adr_Comp(Sw_Nodes[0]->adr,NAdr)){ //Back to first discovered turnout??
				//printf("Back to start!!\n");

				if(Curr_PATH_Node != NULL){
					for(int i = 0;i<10;i++){
						if(Curr_PATH_Node->suc[i] == 1){
							Curr_PATH_Node->suc[i] = -1;
							NAdr = Curr_PATH_Node->adr;
							dir = Curr_PATH_Node->dir;
							prev_dir = dir;
							if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
								goto Next_Switch;
							}else{
								goto Next_Adr;
							}
						}
					}
				}
			}
			else if(found == TRUE){
				//printf("Check if between borders\n");
				for(int i = 0;i<MAX_Modules;i++){
					if(list_of_M[i] == NAdr.M){
						break;
					}
					//Oustide
					if((i+1) == MAX_Modules){
						if(Curr_PATH_Node != NULL){
							for(int i = 0;i<10;i++){
								if(Curr_PATH_Node->suc[i] == 1){
									Curr_PATH_Node->suc[i] = -1;
									NAdr = Curr_PATH_Node->adr;
									dir = Curr_PATH_Node->dir;
									prev_dir = dir;
									if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
										printf("Next_Switch\n");
										goto Next_Switch;
									}else{
										goto Next_Adr;
									}
								}
							}
							if(found == TRUE){
								goto FOUND;
							}
							Curr_PATH_Node = Curr_PATH_Node->Prev;
						}
					}
				}
			}
			//printf("Check if allready known??\n");
			for(int i = 0;i<MAX_ROUTE;i++){
				if(Sw_Nodes[i] != NULL && Adr_Comp(Sw_Nodes[i]->adr,NAdr)){
					printf("Allready known Node\n");
					if(Sw_Nodes[i]->adr.type == 'S'){
						printf("It a switch\t");
						if(Sw_Nodes[i]->suc[0] == -1 && Sw_Nodes[i]->suc[1] == -1){
							printf("Both have no route\t");
							goto FAIL;
						}
					}else if(Sw_Nodes[i]->adr.type == 'M' || Sw_Nodes[i]->adr.type == 'm'){
						printf("It a MS switch\t");
						_Bool NoSucc = TRUE;
						for(int i = 0;i<Moduls[Sw_Nodes[i]->adr.M][Sw_Nodes[i]->adr.B][Sw_Nodes[i]->adr.S]->length;i++){
							if(Sw_Nodes[i]->suc[i] == 2){
								NoSucc = FALSE;
								break;
							}
						}
						if(NoSucc == TRUE){
							printf("No route\t");
							goto FAIL;
						}
					}
					goto FOUND;
					break;
				}
			}

			//printf("Save PATH Node\t");

			struct Sw_A_PATH * Z = (struct Sw_A_PATH *)malloc(sizeof(struct Sw_A_PATH));

			Z->adr = NAdr;
			Z->length = 2;
			memset(Z->suc,0,10);
			Z->suc[0] = 1;
			Z->dir = prev_dir;
			Z->Prev = Curr_PATH_Node;

			for(int i = 0;i<MAX_ROUTE;i++){
				if(Sw_Nodes[i] == NULL){
					printf("%i",i);
					Sw_Nodes[i] = Z;
					break;
				}
			}
			//printf("\n");
			Curr_PATH_Node = Z;
		}
		else if(Curr_PATH_Node != NULL && Adr_Comp(Curr_PATH_Node->adr,NAdr)){ //If current Node exists
			//printf("Success [%i][%i]",Curr_PATH_Node->suc[0],Curr_PATH_Node->suc[1]);
			for(int i = 0;i<Moduls[NAdr.M][NAdr.B][NAdr.S]->length;i++){
				if(Curr_PATH_Node->suc[i] == 0){
					//printf("\tTry next");
					Curr_PATH_Node->suc[i] = 1;
				}else if(Curr_PATH_Node->suc[i] == -1 && (i+1) == Moduls[NAdr.M][NAdr.B][NAdr.S]->length){
					//printf("\tAll failed");
					Curr_PATH_Node = Curr_PATH_Node->Prev;
					NAdr = Curr_PATH_Node->adr;
					if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
						goto Next_Switch;
					}else{
						goto Next_Adr;
					}
				}
			}
		}
		if(NAdr.type == 'M'){
			goto Ret_M_Node;
		}else{
			goto Ret_m_Node;
		}
	}

	printf("Return New_S_Node\n");
	return 0;

	NOT_FOUND:{}


	printf("Return NOT_FOUND\n");
	return 0;

	DONE:{}

	printf("All Switches:\n");

	int k = 0;

	for(int i = 0;i<MAX_ROUTE;i++){
		if(Sw_Nodes[i] != NULL){
			_Bool A = FALSE;
			for(int j = 0;j<10;j++){
				if(Sw_Nodes[i]->suc[j] == 2){
					A = TRUE;
				}
			}
			if(A){
				printf("\n%i\t%i:%i:%i:%c  \t",k,Sw_Nodes[i]->adr.M,Sw_Nodes[i]->adr.B,Sw_Nodes[i]->adr.S,Sw_Nodes[i]->adr.type);
				for(int j = 0;j<10;j++){
					if(Sw_Nodes[i]->suc[j] == 0){
						break;
					}
					printf("%d\t",Sw_Nodes[i]->suc[j]);
				}
				struct Sw_PATH * Z = (struct Sw_PATH *)malloc(sizeof(struct Sw_PATH));
				Z->adr = Sw_Nodes[i]->adr;
				for(int l = 0;l<10;l++){
					Z->suc[l] = Sw_Nodes[i]->suc[l];
				}
				*OUT_Sw_Nodes++ = Z;
			}
		}else{
			break;
		}
	}

	return 1;
}
