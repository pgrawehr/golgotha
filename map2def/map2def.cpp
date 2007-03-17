// map2def.cpp : Definiert den Einsprungpunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

char *desc;
char *infile;
int inhandle;
char *outfile;
int outhandle;
const char *head="LIBRARY i4_core\nEXPORTS\n";

bool validexport(char *name,char *lib)
{
	if (lib[0]=='<')
	{
		return false;
	}
	if (strstr(lib,":")!=0)
	{
		return false;
	}
	if (name[0]=='_' && name[1]=='_') //something like __CT2PAD
	{
		return false;
	}
	if (strstr(name,"??_E")!=0 && strstr(name,"@@UAEPAXI@Z")!=0)
	{
		return false;
	}                //do not export vector deleting destructors
	return true;
}

void convert(void)
{
	char *buf;
	char *index,*endofsection;
	long indofs;
	char dummy[200],field[500],libmodule[500];
	long size=0;
	long symbols=0,skipped=0;
	long r;
	//char usuallyanf;
	char rvabase[30];
	_lseek(inhandle,0,SEEK_END);
	size=_tell(inhandle);
	_lseek(inhandle,0,SEEK_SET);
	buf=new char[size+1];
	_read(inhandle,buf,size);
	buf[size]=0;
	_write(outhandle,head,strlen(head));
	index=buf;
	//r=sscanf(buf,"\t\t\t\t\n%n",&index);
	r=1;
	index=strstr(buf," Address");
	if (index==0)
	{
		printf("Error: The Map file is invalid and doesn't contain a publics by value list\n");
		return;
	}
	while ((*index)!='\n') index++;

	index+=2; //we still point at the '\n'
	endofsection=strstr(buf,"entry point")-2;
	while (r&&(index<endofsection))
	{

		sscanf(index,"%s %[^ ] %s%n",dummy,field,rvabase,&indofs);
		indofs+=4; //skip : a space, an 'f', a space, an 'i' or another space and a fifth space
		sscanf(index+indofs,"%s\n",libmodule);
		//while ((*index)!='\t' && (*index!=0)) index++;
		//index++;
		//while ((*index)!='\t' && (*index!=0)) index++;
		//index++;

		//sscanf(index,"%d\t%d\t%d\t%f\t%f\t%f",
		//	&from,&to,&lanes,&length,&cap,&freespd);
		while ((*index)!='\n' && (*index!=0)) index++;

		index++;

		if (validexport(field,libmodule))
		{
			//printf("Symbol name: %s Library: %s\n",field,libmodule);
			symbols++;
			int cskip=0;
			if (field[0]=='_')
			{
				cskip=1;
				;        //skip of leading _ for C identifiers
			}
			_write(outhandle,field+cskip,strlen(field));
			_write(outhandle,"\n",1);
		}
		else
		{
			skipped++;
		}
	}
	printf("Wrote %d symbols, skipped %d.\n",symbols,skipped);
	delete [] buf;
	return;

}

int main(int argc, char *argv[])
{
	printf("map2def MAP to DEF file converter.\n");
	if (argc<3)
	{
		printf("Usage: map2def infile.map outfile.def [description]\n");
		return 1;
	}
	if (argc==4)
	{
		desc=argv[3];
	}
	else
	{
		desc="i4 application library dll";
	}
	infile=argv[1];
	outfile=argv[2];
	inhandle= _open(infile,_O_TEXT|_O_RDONLY);
	if (inhandle==-1)
	{
		printf("Error opening input file.\n");
		return 2;
	}
	outhandle= _open(outfile,_O_CREAT|_O_TEXT|_O_WRONLY|_O_TRUNC,_S_IREAD|_S_IWRITE);
	if (outhandle==-1)
	{
		printf("Error opening output file for writing.\n");
		return 3;
	}

	convert();
	_close(inhandle);
	_close(outhandle);
	printf("Done\n");
	return 0;
}
