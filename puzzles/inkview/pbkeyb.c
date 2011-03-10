
ifont *keybfont;

struct keyb_s {
  int x,y;
  char start;
  int num;
  int pos;
} keyb;

void pbkeyb(int x, int y, char start, int num) {
int i,al=0,hx,hy;
char sym[2];

  x-=25*((num%3)?(num/3)+1:(num/3))/2;

  keybfont = OpenFont("LiberationMono", 25, 0);
  SetFont(keybfont, BLACK);

  DrawRect(x, y, ((num%3)?(num/3)+1:(num/3))*25+2, 94, BLACK);
  x+=5; y+=5;
  hy=y;
  keyb.x=x; keyb.y=y;
  keyb.start=start;
  keyb.num=num;
  keyb.pos=keyb.num/2+abs((keyb.num/2)%3-2);
  for (i=0; i<num; i++) {
    hx=25*(i/3)+x;
    if ((!al)&&(start+i>57)) {al=1;start+=7;}
    sprintf(sym, "%c", start+i);
    DrawString(hx, hy, sym);
    DrawRect(hx-2, hy-2, 21, 28, BLACK);
    hy=(hy<y+60)?hy+30:y;
  }
  PartialUpdate(x-5, y-5, ((num%3)?(num/3)+1:(num/3))*25+2, 94);
}

char pbkeyb_sel(int dir) {
char last,inp;
int lr,cr;
  last=keyb.pos;
  lr=(keyb.pos%3)?(keyb.pos%3):3;
  switch (dir) {           
    case 0: InvertArea(keyb.x+25*((last-1)/3)-2, keyb.y+(lr-1)*30-2, 21, 28);
            PartialUpdate(keyb.x+25*((last-1)/3)-2, keyb.y+(lr-1)*30-2, 21, 28);
            return 0;
    case 1: if (keyb.pos > 3) keyb.pos-=3; break;
    case 2: if (keyb.pos < keyb.num-2) keyb.pos+=3; break;
    case 3: if (keyb.pos > 1) keyb.pos-=1; break;
    case 4: if (keyb.pos < keyb.num) keyb.pos+=1; break;
    case 5: inp=keyb.pos-1+keyb.start;
            return(inp <= 57)?(inp):(inp+7);
            break;
  }
  cr=(keyb.pos%3)?(keyb.pos%3):3;

  InvertArea(keyb.x+25*((last-1)/3)-2, keyb.y+(lr-1)*30-2, 21, 28);
  PartialUpdate(keyb.x+25*((last-1)/3)-2, keyb.y+(lr-1)*30-2, 21, 28);
  InvertArea(keyb.x+25*((keyb.pos-1)/3)-2, keyb.y+(cr-1)*30-2, 21, 28);
  PartialUpdate(keyb.x+25*((keyb.pos-1)/3)-2, keyb.y+(cr-1)*30-2, 21, 28);
  return 0;
}

