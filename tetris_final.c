#include "tetris.h"

static struct sigaction act, oact;

int main() {
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));
	createRankList();
	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_EXIT: exit=1; break;
		case MENU_REC:	
			recommendedPlay();
			break;
		case MENU_RANK:
			rank();
			break;
		default: break;
		}

	}

	endwin();
	system("clear");
	writeRankFile();
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;
	for(i=0; i<BLOCK_NUM; i++)
		nextBlock[i]=rand()%7;

	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	recRoot=(RecNode*)malloc(sizeof(RecNode));
	recRoot->lv=-1;
	recRoot->score=0;
	
	for(i=0; i<HEIGHT; i++) 
		for(j=0; j<WIDTH; j++) 
			recRoot->f[i][j]=field[i][j];	
		
	createTree(recRoot);
	recommend(recRoot, 0);

	DrawOutline();
	DrawField();
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void createTree(RecNode* root) {	
	int i;
	RecNode **child=root->c;

	for(i=0; i<CHILDREN_MAX; i++){
		child[i]=(RecNode*)malloc(sizeof(RecNode));
		child[i]->lv=root->lv+1;
		child[i]->parent=root;
		if(child[i]->lv < DEPTH)
			createTree(child[i]);
	}

}


void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);
	
	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	move(10, WIDTH+10);
	DrawBox(10, WIDTH+10, 4, 8);	
	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(17, WIDTH+10);
	printw("SCORE");
	DrawBox(18,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	case ' ':
		fall();
		break;
	default:
		break;
	}
	if(drawFlag) {
		DrawChange(field, command,nextBlock[0],blockRotate,blockY,blockX);
	}
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(19,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for(i=0; i<4; i++) {
		move(11+i, WIDTH+13);
		for(j=0; j<4; j++) {
			if(block[nextBlock[2]][0][i][j]==1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}
	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++) {
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	recommended_flag=0;
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	} while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu() {
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	int i, j, x, y;
	for(j=0; j<4; j++) {
		for(i=0; i<4; i++) {
			if(block[currentBlock][blockRotate][j][i]) {
				x=blockX+i;
				y=blockY+j;
				if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT) return 0;
				if(f[y][x]) return 0;	
			}
		}
	}
	return 1;
}

int where_shadow(int y, int x, int currentBlock, int blockRotate) {
	while(CheckToMove(field, currentBlock, blockRotate, y+1, x)) {
		y++;
	}
	return y;
}
void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	DrawRecommend(recRoot->recBlockY, recRoot->recBlockX, recRoot->curBlockID, recRoot->recBlockRotate);
	if(recommended_flag==0)	DrawShadow(y, x, blockID, blockRotate);
	DrawBlock(y, x, blockID, blockRotate, ' ');
	
}
void ErasePrevious(int currentBlock, int blockRotate, int blockY, int blockX) {
	int i,j;
	int shadow_y;
	shadow_y=where_shadow(blockY, blockX, currentBlock, blockRotate);
	for(j=0;j<4;j++) {
		for(i=0;i<4;i++){
			if(block[currentBlock][blockRotate][j][i]==1 && j+blockY>=0){
				move(blockY+j+1, blockX+i+1);
				printw(".");
				move(shadow_y+j+1, blockX+i+1);
				printw(".");
				
			}
		}
	}
	move(HEIGHT,WIDTH+10);	
}

void DrawChange(char f[HEIGHT][WIDTH], int command,int currentBlock,int blockRotate, int blockY, int blockX){
	int i, j;
	
	switch(command) {
		case KEY_UP:
			ErasePrevious(currentBlock, (blockRotate+3)%4, blockY, blockX);
			break;
		case KEY_DOWN:
			ErasePrevious(currentBlock, blockRotate, blockY-1, blockX);
			break;
		case KEY_RIGHT:
			ErasePrevious(currentBlock, blockRotate, blockY, blockX-1);
			break;
		case KEY_LEFT:
			ErasePrevious(currentBlock, blockRotate, blockY, blockX+1);
			break;
		default:
			break;
	}
//	recommendR=blockRotate;
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
	
}

void BlockDown(int sig) {
	int i, j;
	if(CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)) {
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}
	else {
		if(blockY!=-1) {
			for(i=0; i<4; i++) {
				for(j=0; j<4; j++) {
					if(block[recRoot->curBlockID][recRoot->recBlockRotate][i][j]) {
						move(recRoot->recBlockY+i+1, recRoot->recBlockX+j+1);
						printw(".");
					}					
				}
			}
			score+=AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
			score+=DeleteLine(field);
			for(i=0; i<BLOCK_NUM-1; i++)
				nextBlock[i]=nextBlock[i+1];
			nextBlock[BLOCK_NUM-1]=rand()%7;
			blockY=-1;

			blockX=WIDTH/2-2;
			blockRotate=0;

			recRoot->curBlockID=nextBlock[0];
			recRoot->recBlockY=blockY;
			recRoot->recBlockX=blockX;
			recRoot->recBlockRotate=blockRotate;
			recRoot->score=0;
			recRoot->holecount=0;

			for(i=0; i<HEIGHT; i++)
				for(j=0; j<WIDTH; j++)
					recRoot->f[i][j]=field[i][j];
			recommend(recRoot, 0);

		
			DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
			DrawNextBlock(nextBlock);
			//���� ������ ��� �ϴ���?? 
			PrintScore(score);
		}
		else {
			gameOver=1;
			DrawField();
		}

	}
	timed_out=0;
}



int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX) {
	
	int i, j, touched=0;
	for(j=0; j<4; j++) {
		for(i=0; i<4; i++) {
			if(block[currentBlock][blockRotate][j][i]) {
				if(f[blockY+j+1][blockX+i]==1) touched++;
				if(blockY+j>=HEIGHT-1) touched++;
				if(blockY+j>=0 && blockY+j <HEIGHT && blockX+i>=0 && blockX+i<WIDTH) {
					f[blockY+j][blockX+i]=1;
				}
				
			}
		}
	}
	return 10*touched;
}

int DeleteLine(char f[HEIGHT][WIDTH]) {
	int i, j, k, flag, cnt, result;
	cnt=0;
	for(j=0; j<HEIGHT; j++) {
		flag=1;
		for(i=0; i<WIDTH; i++) {
			if(!f[j][i]) { 
				flag=0;
				break; 
			}
		}
		if(flag) {
			cnt++;
			for(k=j-1; k>=0; k--) {
				for(i=0; i<WIDTH; i++) {
					f[k+1][i]=f[k][i];
				}
			}
		}
	}
	result=cnt*cnt*100;
	DrawField();
	return result;
	// user code
}

void DrawShadow(int y, int x, int blockID,int blockRotate) {
	while(CheckToMove(field, blockID, blockRotate, y+1, x)) {
		y++;
	}
	DrawBlock(y, x, blockID, blockRotate, '/');

	// user code
}

void createRankList() {
	// user code
	int i, score;
	FILE *fp=fopen("rank.txt", "r");
	char name[NAMELEN];
	fscanf(fp, "%d", &ranknum);
	for(i=0; i<ranknum; i++) {
		fscanf(fp, "%s%d", name, &score);
		node* new=NULL;
		new=(node*)malloc(sizeof(node));
		strcpy(new->name, name);
		new->score=score;
		new->link=NULL;
		if(!head) {
			head=new;
		}			
		else {
			node* search=head;
			while(search->link!=NULL)
				search=search->link;
			search->link=new;
		}
	}
	fclose(fp);
}
	

void rank() {
	clear();
	// user codeI
	int n;
	int default_x=0;
	int default_y=ranknum-1;
	int x, y;
	int input;
	char name[NAMELEN];
	int flag=0;
	move(0, 0);
	printw("1. list ranks form X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	
	input=wgetch(stdscr);
	move(3, 0);
	switch(input) {
	case '1':
		printw("X: ");
		x=wgetch(stdscr);
		if(x=='\n') {
			x=default_x;
			printw("\n");
		}
		else {
			printw("%c",x);
			x=x-'0'-1;
			printw("\n");
		}
		printw("Y: ");
		y=wgetch(stdscr);
		if(y=='\n') {
			y=default_y;
			printw("\n");
		}
		else {
			printw("%c", y);
			y=y-'0'-1;
			printw("\n");
		}
		printRanksfromXtoY(x, y);
		rank();
		break;
	case '2':
		echo();
		printw("Input your name: ");
		getstr(name);
		printRanksChosenName(name);
		noecho();
		 break;
	case '3':
		echo();
		printw("Input the rank: ");
		scanw("%d", &n);
		DeleteRank(n);
		noecho();
		 break;
	case 'q':
		flag=1;
		 break;
	case 'Q':
		flag=1;
		 break;
	defaut:
		break;
	}
}
void printRanksChosenName(char* name) {
	move(5, 5);
	printw("name");
	move(5, 20);
	printw("|");
	move(5, 23);
	printw("score\n");
	printw("----------------------------\n");
	node* temp=head;
	int flag=0;
	while(temp) {
		if(!strcmp(temp->name, name)) {
			printw("%-16s| %-10d\n", temp->name, temp->score);
			flag++;
		}
		temp=temp->link;
	}
	if(!flag) printw("search failure: no rank in the list\n");
	wgetch(stdscr);
	return;
}
void DeleteRank(int n) {
	node* previous=head;
	node* temp=head;
	int cnt=0;
	if(n<1 || n>ranknum) {
		printw("search failure: no rank in the list\n");
		return;
	}
	while(temp) {
		if(cnt==n-1) {
			if(!head->link && previous==head) {
				free(head);
				head=0;
			}
			else if (head->link && previous==head) {
				free(previous);
				head=head->link;	
			}
			else {
				previous->link=temp->link;
				free(temp);
			}
		ranknum--;
		printw("result: the rank deleted\n");
		break;
		}
		cnt++;
		previous=temp;
		temp=temp->link;	
	}
	wgetch(stdscr);	
	return;		
}

void printRanksfromXtoY(int x, int y) {
	move(5, 5);
	printw("name");
	move(5, 20);
	printw("|");
	move(5, 23);
	printw("score\n");
	printw("----------------------------\n");
	node* temp=head;
	int flag=0;
	if(temp==NULL || x>y || x<0 || y<0 || x>=ranknum || y>=ranknum){
		printw("search failure: no rank in the list");
		wgetch(stdscr);
		return;
	}
	while(temp) {
		if(x<=flag && flag<=y)
			printw("%16s | %4d\n", temp->name, temp->score);
		temp=temp->link;
		flag++;
	}
	wgetch(stdscr);
	return;
}	

void writeRankFile() {
	FILE* fp=fopen("rank.txt","w");
	node* temp=head;
	fprintf(fp, "%d\n", ranknum);
	while(temp) {
		fprintf(fp, "%s %d\n", temp->name, temp->score);
		temp=temp->link;
	}	// user code
	fclose(fp);
}

void newRank(int score) {
	echo();
	clear();
	move(0, 0);
	printw("your name: ");
	char name[NAMELEN]={0,};
	getstr(name);
	node* temp=head;
	node* search;
	node* new=(node*)malloc(sizeof(node));
	strcpy(new->name, name);
	new->link=NULL;
	new->score=score;
	node* linked=head;
	if(head==NULL) {
		head=new;
	}
	else {
		if(temp->link!=NULL) {
			while(temp->link!=NULL) {
				if(temp->score < new->score) {
					if(temp==head) {
						new->link=temp;
						head=new;
						break;
					}
					else {
						new->link=temp;
						linked->link=new;
						break;
					}
				}
				linked=temp;
				temp=temp->link;
			
	
			}
		}	
		else {
			if(temp->score < new->score) {
				new->link=temp;
				head=new;
			}
			else {
				temp->link=new;
			}
		}
	}
	ranknum++;
	noecho();
	
	// user code
}


void DrawRecommend(int y, int x, int blockID, int blockRotate) {
	// user code
	recommendY=y;
	recommendX=x;
	recommendR=blockRotate;
	DrawBlock(y, x, blockID, blockRotate, 'R');

}

int recommend(RecNode *root, int level) {
	int max=0; 
	int i, j, k, candidate_index=0;
	int tempy, tempx;
	int tempmax, tempscore;
	RecNode** child=root->c;	
	if(level>DEPTH-1) return 0;
	for(i=0; i<NUM_OF_ROTATE; i++) {
		if(nextBlock[level]==4 && i>0) break;
		else if((nextBlock[level]==0||nextBlock[level]==5||nextBlock[level]==6) && i>1) break;
		for(tempx=-3; tempx<WIDTH; tempx++) {
			tempy=blockY;
			tempscore=0;
			if(CheckToMove(root->f, nextBlock[level], i, tempy, tempx)) {
				while(CheckToMove(root->f, nextBlock[level], i, tempy+1, tempx)) tempy++;
				for(j=0; j<HEIGHT; j++) {
					for(k=0; k<WIDTH; k++) {
						child[candidate_index]->f[j][k]=root->f[j][k];
							
					}
				}
				child[candidate_index]->curBlockID=nextBlock[level];
				child[candidate_index]->recBlockX=tempx;
				child[candidate_index]->recBlockY=tempy;
				child[candidate_index]->recBlockRotate=i;

				
				
				child[candidate_index]->score+=2*child[candidate_index]->recBlockY*(HEIGHT-child[candidate_index]->recBlockY);
				child[candidate_index]->score=root->score+AddBlockToField(child[candidate_index]->f, nextBlock[level], i, tempy, tempx);
				
				child[candidate_index]->score+=2*DeleteLine(child[candidate_index]->f);
				 
				
				if(level!=DEPTH-1) {
					tempmax=recommend(child[candidate_index], level+1);
					if(max<tempmax) {
						max=tempmax;
						if(!level) {
							recRoot->curBlockID=nextBlock[level];
							recRoot->recBlockX=tempx;
							recRoot->recBlockY=tempy;
							recRoot->recBlockRotate=i;
						}		
					}
					else if(max==tempmax) {
						if(tempy>recRoot->recBlockY) {
							max=tempmax;
							if(!level) {
								recRoot->curBlockID=nextBlock[level];
								recRoot->recBlockX=tempx;
								recRoot->recBlockY=tempy;
								recRoot->recBlockRotate=i;
							}
						}
					}
					
				}
				else {
					child[candidate_index]->holecount=0;		
					for(i=child[candidate_index]->recBlockY+1; i<HEIGHT; i++) {
						for(j=child[candidate_index]->recBlockX; j<child[candidate_index]->recBlockX+4; j++) {
							if(j>=WIDTH) break;
							if(!child[candidate_index]->f[i][j]) child[candidate_index]->holecount++;
						}
					
				}
				//	child[candidate_index]->score-=child[candidate_index]->holecount*100;
					max=child[candidate_index]->score;
				
				}
			candidate_index++;
			}
		}
	}
	// 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수

	// user code

	return max;
}
void BlockDownREC(int sig) {

	int i, j;
        if(recRoot->recBlockY!=-1) {
			/*
        	for(i=0; i<4; i++) {
               	for(j=0; j<4; j++) {
                      	if(block[recRoot->curBlockID][recRoot->recBlockRotate][i][j]) {
                             	move(recRoot->recBlockY+i+1, recRoot->recBlockX+j+1);
                                printw(".");
						}
                }
			}
       */
                score+=AddBlockToField(field, recRoot->curBlockID, recRoot->recBlockRotate, recRoot->recBlockY, recRoot->recBlockX);
				score+=DeleteLine(field);
                for(i=0; i<BLOCK_NUM-1; i++)
                       nextBlock[i]=nextBlock[i+1];
                nextBlock[BLOCK_NUM-1]=rand()%7;
              	blockY=-1;

                blockX=WIDTH/2-2;
                blockRotate=0;

                recRoot->curBlockID=nextBlock[0];
                recRoot->recBlockY=blockY;
                recRoot->recBlockX=blockX;
                recRoot->recBlockRotate=blockRotate;
                recRoot->score=0;

                for(i=0; i<HEIGHT; i++)
                        for(j=0; j<WIDTH; j++)
                                recRoot->f[i][j]=field[i][j];
                recommend(recRoot, 0);


                DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
                DrawNextBlock(nextBlock);
                        //���� ������ ��� �ϴ���??
                PrintScore(score);
       }
       else {
               gameOver=1;
               DrawField();
       }
timed_out=0;
	
}

void recommendedPlay() {
	int command;
	recommended_flag =1;
	clear();
	act.sa_handler = BlockDownREC;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(command==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();

}

void fall() {
	while(CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX))
		blockY++;
	DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	BlockDown(-1);
	
}
