// profreader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	if(!argv[1])
		return -1;

	FILE* tsFile = fopen(argv[1], "rt");
	char s[255];

	strcpy(s,argv[1]);
	strcat(s,"_c.txt");
	FILE* outFileC = fopen(s, "w+t");
	strcpy(s,argv[1]);
	strcat(s,"_d.txt");
	FILE*	outFileD = fopen(s, "w+t");

	__int64 time;
	int dTime;
	int processId;
	char methodName[128];
	fscanf(tsFile, "%d %d %d %s", &time, &dTime, &processId, methodName);
	if(strcmp(methodName, "*****************start*****************") == 0)
	{
		printf("Begin analyze...\n");
	} else {
		printf("Bad file\n");
		return 0;
	}

	int wndBits =0;
	int hashBits;
	int line;
	char methodNameEnd[128];

	int averTimeC = 0;
	int c=0;
	int averTimeD = 0;
	int d=0;
	while(!feof(tsFile))
	{

		fscanf(tsFile, "%d %d %d %s %d", &time, &dTime, &processId, methodName, &line);
		if(strcmp(methodName,"*****************stop*****************")==0)
		{
			if(wndBits != 0) {
				fprintf(outFileC, "%s\t%d\t%d\t%d\n", "compressIBlock_HASH", wndBits, hashBits, (int)(double(averTimeC)/double(c)));
				averTimeC = 0;
				c = 0;

				fprintf(outFileD, "%s\t%d\t%d\t%d\n","decompressIBlock_HASH", wndBits, hashBits, (int)(double(averTimeD)/double(d)));
				averTimeD = 0;
				d = 0;
			}
			break;
		}
		if(strcmp(methodName, "WndBits") == 0) //Идет описание параметров сжатия
		{
			if(wndBits != 0) {
			fprintf(outFileC, "%s\t%d\t%d\t%d\n", "compressIBlock_HASH", wndBits, hashBits, (int)(double(averTimeC)/double(c)));
			averTimeC = 0;
			c = 0;

			fprintf(outFileD, "%s\t%d\t%d\t%d\n","decompressIBlock_HASH", wndBits, hashBits, (int)(double(averTimeD)/double(d)));
			averTimeD = 0;
			d = 0;
			}
	
			wndBits = line;
			fscanf(tsFile, "%s %d", methodName, &hashBits);
		} else 
		{
			fscanf(tsFile, "%d %d %d %s %d", &time, &dTime, &processId, methodNameEnd, &line);
			if(strcmp(methodName, "compressIBlock_HASH")==0)
			{
				averTimeC += dTime;
				c+=1;
			} else 
				if(strcmp(methodName, "decompressIBlock_HASH")==0)
				{
					averTimeD += dTime;
					d+=1;
				} 
		}
	}
	fclose(outFileD);
	fclose(outFileC);
	fclose(tsFile);
	return 0;
}

