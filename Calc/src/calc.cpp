/*
 * Copyright (C) 2009 Artem Danilenko <adanilenko@proxyconn.kiev.ua>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#include "inkview.h"
#include <math.h>

#define BUF_SIZE 255
#define Zones 2


#define log_printf(x) stderr = iv_fopen(SDCARDDIR"/log.txt","a"); fprintf(stderr,"%i" ,x); iv_fclose(stderr);
//#define log_printf(x) fprintf(stderr, "%s ",x);


ifont *Mono35;
short last[BUF_SIZE][Zones],act[Zones]={0,0},actpos[Zones]={0,-1};
unsigned int section,function,length[Zones]={160,24};
int oldOrientation;
char s[Zones][BUF_SIZE]={"_",""};
extern const ibitmap keyboard,mcursor,scursor,clr,ok;
short zone;
bool written=0;

const char* WRONG_BRACKETS_ORDER="Wrong brackets order: ";
const char* UNKNOWN_FUNCTION="Unknown function: ";
const char* DIVISION_BY_ZERO="Division by zero";
const char* LOGARITHM_DOMAIN="Argument outside the domain of logarithm";
const char* ARCCOSINE_DOMAIN="Argument outside the domain of arccosine";
const char* ARCSINE_DOMAIN="Argument outside the domain of arcsine";
const char* ROOT_DOMAIN="Argument outside the domain of square root";
const char* EMPTY_ARGUMENT="Empty argument";

//int main_handler(int type, int par1, int par2);

double ans=0,X=0;
char ERROR_MSG[BUF_SIZE]="";
char ar[6]="+-*/^";
const double EPS=0.0001,PI=acos(-1),E=exp(1);
bool error=false;
struct node{
	node *left,*right;
	char key[BUF_SIZE];
	node(char k[BUF_SIZE])
	{
		left=NULL;
		right=NULL;
		strcpy (key,k);
	}
};
//______________________________________________________________________________

bool Parser(char str[BUF_SIZE], node *&root);
//»щет в строке str любой символ из строки s который не стоит внутри круглых
//скобок и разбивает эту строку на три
//______________________________________________________________________________
bool Search(char str[BUF_SIZE],char s1[10], node *&root)
{
	unsigned int i=0;
	int c=0;
	char s[10]="";
	strcpy(s,s1);

	while (((i<strlen(str))&&(!((c==0)&&(i!=0)&&((strrchr(s,str[i]))!=NULL)))))
		{
			if (str[i]=='(')
				c++;
			if (str[i]==')')
				c--;
			i++;
		}

	if ((i!=strlen(str))&&(strrchr(ar,str[i-1])==NULL)&&(strrchr(ar,str[i+1])==NULL))
	{
		char str1[BUF_SIZE]="",str2[BUF_SIZE]="",str3[BUF_SIZE]="";
		strncpy(str1,str,i);
		str2[0]=str[i];
		for (unsigned int j = i; j<=strlen(str); j++)
			str3[j-i-1]=str[j];
		root = new node(str2);
		Parser(str1,root->left);
		Parser(str3,root->right);
		return true;
	}
	return false;
}
//______________________________________________________________________________
//ѕровер€ет начинаетс€ ли строка str со строки s, если да, то разбивает эту
//строку на две
bool SearchF(char str[BUF_SIZE],char s1[], node *&root)
{
	char s[10]="";
	strcpy(s,s1);
	if ((strncmp (str,s,strlen(s)) == 0)&&(str[strlen(str)-1]==')'))
	{
		char str1[BUF_SIZE]="";
		s[strlen(s)-1]='\0';
		for (unsigned int j = strlen(s)+1; j<strlen(str)-1; j++)
			str1[j-strlen(s)-1]=str[j];
		root = new node(s);
		Parser(str1,root->left);
		return true;
	}
	return false;
}
//______________________________________________________________________________
bool Parser(char str[BUF_SIZE], node *&root)
{
	unsigned int i=0,j=0;
	int c=0;
// ѕровер€ет на пустую строку
	if (strcmp(str,"")==0)
	{
		strcpy(ERROR_MSG,EMPTY_ARGUMENT);
		error=true;
		return !error;
	}
// ѕровер€ет что кол-во открытых скобок равно кол-ву закрытых

	for (i = 0; i < strlen(str); i++)
	{
		if (str[i]==')') c--;
		else if (str[i]=='(') c++;
		if (c<0) break;
	}
	if (c!=0)
	{
		error=true;
		strcpy(ERROR_MSG,WRONG_BRACKETS_ORDER);
		strncat (ERROR_MSG,str,100);
		if (strlen(str)>100) strcat (ERROR_MSG,"...");
		return false;
	}
// ѕереводит строку типа (***) в *** и запускает парсер от него
	i=1;
	if (str[0]=='(')
	{
		while ((i<strlen(str))&&!((str[i]==')')&&(c==0)))
		{
			if ((str[i]==')')&&(i!=strlen(str)-1)&&(c==0)) break;
			if (str[i]==')') c--;
			if (str[i]=='(') c++;
			i++;
		}
		if (i==strlen(str)-1)
		{
			for (j=0; j <= strlen(str)-2; j++)
				str[j]=str[j+1];
			str[strlen(str)-2]='\0';
			Parser(str,root);
			return !error;
		}
	}
//ѕровер€ет не €вле€тс€ ли строка числом

	char *End;
     strtod(str,&End);
     if (strlen(End)==0)
     {
          root = new node(str);
          return !error;
     }
//ѕровер€ет не €вл€етс€ ли строка просто иксом или аns или Pi или е
	if ((strcmp(str,"X")==0)||(strcmp(str,"ans")==0)||(strcmp(str,"Pi")==0)||(strcmp(str,"e")==0)) 
	{
		root = new node(str);
		return !error;
	}
//ѕровер€ет на унарный минус

	if (str[0]=='-')
	{
		root = new node("-");
		for (j=0; j < strlen(str); j++)
			str[j]=str[j+1];
		Parser(str,root->right);
		return !error;
	}

	if (Search(str,"+-",root)) return !error;
	if (Search(str,"*/",root)) return !error;
	if (Search(str,"^",root)) return !error;

	if (SearchF(str,"sin(",root)) return !error;
	if (SearchF(str,"cos(",root)) return !error;
	if (SearchF(str,"exp(",root)) return !error;
	if (SearchF(str,"ln(",root)) return !error;
	if (SearchF(str,"log(",root)) return !error;
	if (SearchF(str,"tan(",root)) return !error;
	if (SearchF(str,"ctan(",root)) return !error;
	if (SearchF(str,"acos(",root)) return !error;
	if (SearchF(str,"asin(",root)) return !error;
	if (SearchF(str,"atan(",root)) return !error;
	if (SearchF(str,"actan(",root)) return !error;
	if (SearchF(str,"cosh(",root)) return !error;
	if (SearchF(str,"sinh(",root)) return !error;
	if (SearchF(str,"tanh(",root)) return !error;
	if (SearchF(str,"ctanh(",root)) return !error;
	if (SearchF(str,"sqrt(",root)) return !error;

	strcpy(ERROR_MSG,UNKNOWN_FUNCTION);
	strncat (ERROR_MSG,str,100);
	if (strlen(str)>100) strcat (ERROR_MSG,"...");
	error=true;
	return !error;
}
//______________________________________________________________________________
bool Check(double a,const char* MSG)
{
	if (a<EPS) 
	{
		error=true;
		strcpy(ERROR_MSG,MSG);
		return false;
	}
	return true;
}

double Calculate(double x, node *&root)
{
	if ((root->key[0]=='+')&&(strlen(root->key)==1)) return (Calculate(x,root->left)+Calculate(x,root->right));
	if ((root->key[0]=='-')&&(strlen(root->key)==1)&&(root->left==NULL)) return (-Calculate(x,root->right));
	if ((root->key[0]=='-')&&(strlen(root->key)==1)) return (Calculate(x,root->left)-Calculate(x,root->right));
	if (root->key[0]=='*') return (Calculate(x,root->left)*Calculate(x,root->right));
	if (root->key[0]=='/')
	{
		if (Check(fabs(Calculate(x,root->right)),DIVISION_BY_ZERO)) return (Calculate(x,root->left)/Calculate(x,root->right));
		else return 0;
	}
	
	if (root->key[0]=='^') return (pow(Calculate(x,root->left),Calculate(x,root->right)));
	if (root->key[0]=='X') return x;
	if (strcmp(root->key,"cosh")==0) return cosh(Calculate(x,root->left));
	if (strcmp(root->key,"sinh")==0) return PI/2-atan(Calculate(x,root->left));
	if (strcmp(root->key,"sin")==0) return sin(Calculate(x,root->left));
	if (strcmp(root->key,"cos")==0) return cos(Calculate(x,root->left));
	if (strcmp(root->key,"exp")==0) return exp(Calculate(x,root->left));

	if (strcmp(root->key,"ln")==0)
	{
		if (Check(Calculate(x,root->left),LOGARITHM_DOMAIN)) return log(Calculate(x,root->left));
		else return 0;
	}
	if (strcmp(root->key,"log")==0)
	{
		if (Check(Calculate(x,root->left),LOGARITHM_DOMAIN)) return log10(Calculate(x,root->left));
		else return 0;
	}
	double a=0;
	if (strcmp(root->key,"tan")==0)
	{
		if (Check(fabs(cos(Calculate(x,root->left))),DIVISION_BY_ZERO)) return tan(Calculate(x,root->left));
		else return 0;
	}
	if (strcmp(root->key,"ctan")==0)
	{
		if (Check(fabs(sin(Calculate(x,root->left))),DIVISION_BY_ZERO)) return -tan(Calculate(x+PI/2,root->left));
		else return 0;
	}
	if (strcmp(root->key,"acos")==0)
	{
		a=Calculate(x,root->left);
		if ((a<-1)||(a>1))
		{
			error=true;
			strcpy(ERROR_MSG,ARCCOSINE_DOMAIN);
			return 0;
		}
		else return acos(a);
	}

	if (strcmp(root->key,"asin")==0)
	{
		a=Calculate(x,root->left);
		if ((a<-1)||(a>1))
		{
			error=true;
			strcpy(ERROR_MSG,ARCSINE_DOMAIN);
			return 0;
		}
		else return asin(a);
	}

	if (strcmp(root->key,"atan")==0) return atan(Calculate(x,root->left));
	if (strcmp(root->key,"actan")==0) return PI/2-atan(Calculate(x,root->left));
	if (strcmp(root->key,"tanh")==0) return tanh(Calculate(x,root->left));
	if (strcmp(root->key,"ctanh")==0)
	{
		if (Check(fabs(sinh(Calculate(x,root->left))),DIVISION_BY_ZERO)) return 1/tanh(a);
		else return 0;
	}

	if (strcmp(root->key,"sqrt")==0)
	{
		a=Calculate(x,root->left);
		if (a<0)
		{
			error=true;
			strcpy(ERROR_MSG,ROOT_DOMAIN);
			return 0;
		}
		else return pow(a,0.5);
	}
	if (strcmp(root->key,"ans")==0)
		return ans;
	if (strcmp(root->key,"Pi")==0)
		return PI;
	if (strcmp(root->key,"e")==0)
		return E;
	if (strcmp(root->key,"X")==0)
		return X;
	return strtod(root->key,NULL);
}

void DeleteRoot(node *&root)
{
	if (root->left!=NULL) DeleteRoot(root->left);
	if (root->right!=NULL) DeleteRoot(root->right);
	delete root;
}

void SelectButton()
{
	DrawBitmap( 0, 409 , &keyboard);
	switch (section) {
		case 1:
		switch (function) {
			case 1:DrawBitmap(64,460,&scursor);break;
			case 2:DrawBitmap(111,460,&scursor);break;
			case 3:DrawBitmap(160,460,&scursor);break;
			case 4:DrawBitmap(208,460,&scursor);break;
			case 5:DrawBitmap(255,460,&scursor);break;
			case 6:DrawBitmap(305,460,&scursor);break;
			case 7:DrawBitmap(353,460,&scursor);break;
			case 8:DrawBitmap(401,460,&scursor);break;
			case 9:DrawBitmap(449,460,&scursor);break;
			case 10:DrawBitmap(497,460,&scursor);break;
		}
		break;

		case 2:
		switch (function) {
			case 1:DrawBitmap(10,527,&mcursor);break;
			case 2:DrawBitmap(10,569,&mcursor);break;
			case 3:DrawBitmap(10,610,&mcursor);break;
			case 4:DrawBitmap(10,653,&mcursor);break;
			case 5:DrawBitmap(123,527,&mcursor);break;
			case 6:DrawBitmap(123,569,&mcursor);break;
			case 7:DrawBitmap(123,610,&mcursor);break;
			case 8:DrawBitmap(123,653,&mcursor);break;
		}
		break;

		case 3:
		switch (function) {
			case 1:	DrawBitmap(242,527,&mcursor);break;
			case 2:	DrawBitmap(242,569,&mcursor);break;
			case 3:	DrawBitmap(242,610,&mcursor);break;
			case 4:	DrawBitmap(242,653,&mcursor);break;			
		}
		break;

		case 4:
		switch (function) {
			case 1:	DrawBitmap(373,527,&mcursor);break;
			case 2:	DrawBitmap(373,569,&mcursor);break;
			case 3:	DrawBitmap(373,610,&mcursor);break;
			case 4:	DrawBitmap(373,653,&mcursor);break;
			case 5:	DrawBitmap(482,527,&mcursor);break;
			case 6:	DrawBitmap(482,569,&mcursor);break;
			case 7:	DrawBitmap(482,610,&mcursor);break;
			case 8:	DrawBitmap(482,653,&mcursor);break;
		}
		break;

		case 5:
		switch (function) {
			case 1:DrawBitmap(112,722,&scursor);break;
			case 2:DrawBitmap(155,722,&scursor);break;
			case 3:DrawBitmap(196,722,&scursor);break;
			case 4:DrawBitmap(240,722,&scursor);break;
			case 5:DrawBitmap(284,722,&scursor);break;
			case 6:DrawBitmap(323,722,&scursor);break;
			case 7:DrawBitmap(364,722,&scursor);break;
			case 8:DrawBitmap(408,722,&scursor);break;
			case 9:DrawBitmap(445,722,&scursor);break;
		}
		break;	
		
		case 6:
		switch (function) {	
			case 1:DrawBitmap(187,770,&ok);break;
			case 2:DrawBitmap(298,770,&clr);break;
		}
		break;
	}
	
}

void DeleteLastWrite(int zon)
{
	if (last[0][zon]>0)
	{
		char s1[BUF_SIZE]="";
		actpos[zon]=actpos[zon]-last[act[zon]][zon];
		strcpy(s1,&s[zon][actpos[zon]+1]);
		s[zon][actpos[zon]]='\0';
		strcat(s[zon],"_");
		strcat(s[zon],&s1[last[act[zon]][zon]]);
		last[0][zon]--;
		act[zon]--;
		if (act[zon]!=last[0][zon])
		{
			for (int i=act[zon]+1;i<last[0][zon];i++)
				last[i][zon]=last[i+1][zon];
		}
	}
}

void WriteText()
{
	DrawTextRect(10, 10,590,400,s[0], ALIGN_LEFT | VALIGN_TOP);
	DrawTextRect(85, 413,515,20,s[1], ALIGN_LEFT | VALIGN_TOP);
	if(error)
	{
		DrawTextRect(4, 300,596,100,ERROR_MSG, ALIGN_LEFT | VALIGN_TOP);
		error=0;
	}
	if(written)
	{
		DeleteLastWrite(0);
		written=0;
	}
}

void write(char *str,int zon) 
{
	if (strlen(s[zon])+strlen(str)<=length[zon])
	{
		char s1[BUF_SIZE]="";
		strcpy(s1,&s[zon][actpos[zon]+1]);
		s[zon][actpos[zon]]='\0';
		strcat(s[zon],str);
		if (zone==zon) strcat(s[zon],"_");
		strcat(s[zon],s1);
		act[zon]++;
		actpos[zon]+=strlen(str);
		last[0][zon]++;
		if (act[zon]==last[0][zon])
		{
			last[last[0][zon]][zon]=strlen(str);
			if ((strlen(str)==0)&&(last[0][zon]!=0)) 
				last[last[0][zon]][zon]=9;
		}
		else
		{
			for (int i=last[0][zon];i>act[zon];i--)
				last[i][zon]=last[i-1][zon];
			last[act[zon]][zon]=strlen(str);
		}
	}
}
void DeleteCursor(int zone)
{
	char s1[BUF_SIZE]="";
	strcpy(s1,&s[zone][actpos[zone]+1]);
	s[zone][actpos[zone]]='\0';
	strcat(s[zone],s1);
	actpos[zone]--;
}
void AddCursor(int zone)
{
	strcat(s[zone],"_");
	actpos[zone]++;
}
double Calc() 
{
	node* root=NULL;
	DeleteCursor(zone);
	if (strcmp(s[1],"")!=0) 
	{
		Parser(s[1],root);
		if (!error)
		{
			double a=Calculate(0,root);
			if (!error)
				X=a;
		}
	}

	if (root!=NULL)
	{
		DeleteRoot(root);
		root=NULL;
	}
	act[0]=last[0][0];
	actpos[0]=strlen(s[0])-1;
	if (!error)Parser(s[0],root);
	if (!error)
	{
		double a=Calculate(X,root);
		if (root!=NULL)DeleteRoot(root);
		root=NULL;
		if (!error)
		{
			char s1[BUF_SIZE]="";
			char buf[BUF_SIZE]="";
			sprintf(buf, "%.8f", a);
			strcpy(s1,"=");
			strcat(s1,buf);
			AddCursor(zone);
			if(zone==1)actpos[0]++;
			write(s1,0);
			written=1;
			return a;
		}
	}
	AddCursor(zone);
	if (root!=NULL)DeleteRoot(root);
	return 0;
}

void clear()
{
	last[0][zone]=0;
	act[zone]=0;
	actpos[zone]=0;
	error=false;
	strcpy(s[zone],"_");
	write("",zone);
}


int main_handler(int type, int par1, int par2) 
{
	if (type == EVT_INIT) {
		oldOrientation = GetOrientation();
		SetOrientation(ROTATE0);
		Mono35 = OpenFont("LiberationMono", 35, 1);		
		//arial35 = OpenFont("DroidSans", 35, 1);
		SetFont(Mono35, BLACK);
		section=3;
		function=2;
		ClearScreen();
		SelectButton();
		PartialUpdateBW(0, 0, ScreenWidth(), ScreenHeight());
	}

		
	if (type == EVT_SHOW) {
	}

	if ((type == EVT_KEYRELEASE)&&(par2 == 0)) {
		switch (par1) {

			case KEY_OK:
			switch (section) {	
				case 1:
				switch (function) {
					case 1: write("1",zone);break;
					case 2:	write("2",zone);break;
					case 3:	write("3",zone);break;
					case 4: write("4",zone);break;
					case 5:	write("5",zone);break;
					case 6:	write("6",zone);break;
					case 7:	write("7",zone);break;
					case 8:	write("8",zone);break;
					case 9:	write("9",zone);break;
					case 10:write("0",zone);break;
				}
				break;

				case 2:
				switch (function) {
					case 1:write("asin(",zone);break;
					case 2:write("acos(",zone);break;
					case 3:write("atan(",zone);break;
					case 4:write("actan(",zone);break;
					case 5:write("sin(",zone);break;
					case 6:write("cos(",zone);break;
					case 7:write("tan(",zone);break;
					case 8:write("ctan(",zone);break;
				}
				break;

				case 3:
				switch (function) {
					case 1:write("sqrt(",zone);break;
					case 2:write("ln(",zone);break;
					case 3:write("log(",zone);break;
					case 4:write("exp(",zone);break;
				}
				break;

				case 4:
				switch (function) {
					case 1:write("sinh(",zone);break;
					case 2:write("cosh(",zone);break;
					case 3:write("tanh(",zone);break;
					case 4:write("ctanh(",zone);break;
					case 5:write("X",zone);break;
					case 6:write("e",zone);break;
					case 7:write("Pi",zone);break;
					case 8:write("ans",zone);break;
				}
				break;

				case 5:
				switch (function) {
					case 1:write("+",zone);break;
					case 2:write("-",zone);break;
					case 3:write(".",zone);break;
					case 4:DeleteLastWrite(zone);break;
					case 5:write(" ",zone);break;
					case 6:write("*",zone);break;
					case 7:write("/",zone);break;
					case 8:write("(",zone);break;
					case 9:write(")",zone);break;
				}
				break;
				
				case 6:
				switch (function) {
					case 1: ans=Calc();break;
					case 2: clear();break;
				}
				break;
			}
			break;

			case KEY_BACK:
				clear();
			break;

			case KEY_LEFT:
			switch (section) {
				case 1:
					function--;
					if (function ==0) 
						function=10;
				break;

				case 2:
					if (function<5)
					{
						section=4;
						function+=4;
					}
					else function-=4;
				break;

				case 3:
					section=2;
					function+=4;
				break;

				case 4:
					if (function<5)	
						section=3;
					else function-=4;
				break;

				case 5:
					function--;
					if (function ==0) 
						function=9;
					if (function ==5) 
						function--;
				break;	
				
				case 6:
					if (function ==1) 
						function=2;
					else function=1;
				break;
			}
			break;

			case KEY_RIGHT:
			switch (section) {
				case 1:
					function++;
					if (function ==11) 
						function=1;
				break;
		
				case 2:
					if (function>4)
					{
						section=3;
						function-=4;
					}
					else function+=4;
				break;
	
				case 3:
					section++;		
				break;

				case 4:
					if (function>4)
					{
						section=2;
						function-=4;
					}
					else function+=4;
				break;

				case 5:
					function++;
					if (function ==10) 
						function=1;
					if (function ==5) 
						function++;	
				break;	
				
				case 6:
					if (function ==1) 
						function=2;
					else function=1;
				break;
			}
			break;

			case KEY_UP:
			switch (section) {
				case 1:
					section=6;
					function=1;
				break;
					
				case 2:
					function--;
					if (function==0||function==4)
					{
						section=1;
						function=1;
					}
				break;

				case 3:
					function--;
					if (function==0) 
					{
						section=1;
						function=6;
					}
				break;

				case 4:
					function--;
					if (function==0||function==4)
					{
						section=1;
						function=10;
					}
				break;

				case 5:
					if (function<4) 
					{
						section=2;
						function=8;
					}
					else if (function<7) 
					{
						section=3;
						function=4;
					}
					else 
					{
						section=4;
						function=4;
					}
				break;	
				
				case 6:
					section=5;
					function=4;
				break;
			}
			break;

			case KEY_DOWN:
			switch (section) {
				case 1:
					if (function<5) 
					{
						section=2;
						function=5;
					}
					else if (function<7) 
					{
						section=3;
						function=1;
					}
					else 
					{
						section=4;
						function=1;
					}
				break;
	
				case 2:
					function++;
					if (function==5||function==9)
					{
						section=5;
						function=4;
					}
				break;

				case 3:
					function++;
					if (function==5) 
					{
						section=5;
						function=4;
					}
				break;

				case 4:
					function++;
					if (function==5||function==9)
					{
						section=5;
						function=4;
					}
				break;

				case 5:
					section=6;
					function=1;
				break;
				
				case 6:
					section=1;
					function=6;
				break;
			}
			break;

			case KEY_MUSIC:

			break;

			case KEY_MENU:
			
			break;

			case KEY_DELETE:

			break;

		}
	}
	
	if ((type == EVT_KEYREPEAT)&&(par2%2==0)) {
		switch (par1) {
			case KEY_OK:
				ans=Calc();
			break;

			case KEY_BACK:
				CloseApp();
			break;

			case KEY_LEFT:
				if (act[zone]!=0)
				{
					char s1[255]="",s2[255]="";
					strcpy(s1,&s[zone][actpos[zone]+1]);
					strncpy(s2,&s[zone][actpos[zone]-last[act[zone]][zone]],last[act[zone]][zone]);
					s[zone][actpos[zone]-last[act[zone]][zone]]='\0';
					strcat(s[zone],"_");
					strcat(s[zone],s2);
					strcat(s[zone],s1);
					actpos[zone]-=last[act[zone]][zone];
					act[zone]--;
					ClearScreen();
				}
			break;

			case KEY_RIGHT:
				SelectButton();
				if (act[zone]!=last[0][zone])
				{
					char s1[255]="",s2[255]="";
					strncpy(s1,&s[zone][actpos[zone]+1],last[act[zone]+1][zone]);
					strcpy(s2,&s[zone][actpos[zone]+last[act[zone]+1][zone]+1]);
					s[zone][actpos[zone]]='\0';				
					strcat(s[zone],s1);
					strcat(s[zone],"_");
					strcat(s[zone],s2);
					act[zone]++;
					actpos[zone]+=last[act[zone]][zone];
					ClearScreen();
				}
			break;

			case KEY_UP:
				DeleteCursor(zone);
				zone=0;
				AddCursor(zone);
			break;

			case KEY_DOWN:
				DeleteCursor(zone);
				zone=1;
				AddCursor(zone);
			break;

			case KEY_MUSIC:
			break;

			case KEY_MENU:
			break;

			case KEY_DELETE:
			break;
		}
	}
	if (type == EVT_EXIT) {
		SetOrientation(oldOrientation);
	}
	if (((type == EVT_KEYRELEASE)&&(par2 == 0))||((type == EVT_KEYREPEAT)&&(par2%2==0)))
	{
		ClearScreen();
		SelectButton();
		WriteText();
		PartialUpdateBW(0, 0, ScreenWidth(), ScreenHeight());
	}
	return 0;
}


int main(int argc, char **argv)
{
	InkViewMain(main_handler);
	return 0;
}

