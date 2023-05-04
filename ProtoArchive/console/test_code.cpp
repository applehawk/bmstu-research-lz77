#include "timer.h"
#include "lzss.h"
#include "bitsstream.h"
#include <stdio.h>
void compression_test(char* testName, char* srcName, char* dstName, char* unpackName)
{
	printf("Begin %s\n",testName);
	BitsStream istream(srcName);
	BitsStream ostream(dstName);
	istream.open(BitsStream::ReadOnly);
	ostream.open(BitsStream::WriteOnly);
	Timer t1;
	t1.reset();

	lzx_state lzstate;
	initLZ77(&lzstate, &istream, &ostream);
	compressLZ77(&lzstate);
	destroyLZ77(&lzstate);
	float time = t1.cpuTime();
	ostream.close();
	ostream.open(BitsStream::ReadOnly);
	printf("%s Result\n\tCompressing Time: %f\n\t", testName, time);
	printf("Source Size: %d \n\tResult size: %d \n", istream.fileSize(), ostream.fileSize());
	float coef = ((float)ostream.fileSize()/(float)istream.fileSize())*100.f;
	printf("Compression coeficient: %.2f%%\n", coef);

	ostream.close();
	istream.close();

	istream.setFile(dstName);
	ostream.setFile(unpackName);
	istream.open(BitsStream::ReadOnly);
	ostream.open(BitsStream::WriteOnly);

	t1.reset();
	initLZ77(&lzstate, &istream, &ostream);
	decompressLZ77(&lzstate);
	printf("%s Decompressing Time: %f\n\n", testName, t1.cpuTime());
	destroyLZ77(&lzstate);
	istream.close();
	ostream.close();
}