#include "..\methods\archive_method.h"
#include "..\methods\lzx_method.h"
#include "..\methods\suffix_lz_method.h"

#include <QDir>
#include <QTextStream>
#include <QAxObject>

#include "timer.h"

#include <iostream>
#include "qaxobject.h"

#include "dtimestamp.h"

#define S_NATIVE(x) QDir::toNativeSeparators(x)


void testpack_all_indir(ArchiveMethod* method, const QStringList& dirPaths, const QString& packDirName, const QString& unpackDirName);

void info()
{
	printf("protoarchive [dirname] method[suffix|lzx](Default LZX)\n");
	/*printf("protoarchive -s [srcfile] -d [dstfile] (-p|-u) -method:(lzx|suffix) -wb [window bits] -bb [block bits] (-hb)[hash_bits] -mm [minmatch] -lf [logfile]\n"
			"-s Source File\n"
			"-d Dest File\n"
			"-p Packing process, -u Unpacking process\n"
			"-method:(lzx|suffix) select current compression method\n"
			"-wb WindowBits(WndSize = 2^WindowBits)\n"
			"-bb BlockBits\n"
			"-hb HashBits\n"
			"-mm MinMatch\n"
			"-lf Log file for logging packing process\n");*/
}

void params_process(int argc, char* argv[])
{
	if(argc< 2) {
		printf("not valid parameters\n");
		info();
		return;
	}
	ArchiveMethod* method;
	QString dirName(argv[1]);
	QString methodName;
	if(argc > 2)
		methodName = QString(argv[2]);
	else 
		methodName = QString("lzx");
	if(methodName == QString("lzx"))
		method = new LZX_Method();
	if(methodName == QString("suffix"))
		method = new Suffix_LZ_Method();

	method->setDefaultParams();
	//testpack_all_indir(method, dirName, "arhives", "unpacks");	
}

void compare_2files_print1Diff(const QString& file1Path, const QString& file2Path)
{
	QFile file1(file1Path);
	QFile file2(file2Path);

	QDataStream strmFile1(&file1);
	QDataStream strmFile2(&file2);

	if(file1.size() != file2.size())
		std::cout << "Not equal size" << std::endl;
	while(!file1.atEnd() && !file2.atEnd())
	{
		quint32 d1,d2;
		strmFile1 >> d1;
		strmFile2 >> d2;

		if(d1 != d2)
		{
			std::cout << QString("File1: %1, File2: %2 Begin at pos: %3, Find Difference: F1:%4 and F2:%5\n")
				.arg(file1.fileName(), file2.fileName())
				.arg(file1.pos()-8)
				.arg(d1)
				.arg(d2).toAscii().data() << std::endl;
			break;
		}
	}
	std::cout << QString("The two files are identical!\n").toAscii().data() << std::endl;
}

int test_file(ArchiveMethod* method, const QString& fileName, const QString& packDirName, const QString& unpackDirName)
{
	QFileInfo info(fileName);
	QString dirPath = S_NATIVE(info.absolutePath()+"/");
	QDir(dirPath).mkdir(packDirName);
	QDir(dirPath).mkdir(unpackDirName);
	QString srcFilePath = S_NATIVE(dirPath+info.baseName()+(info.suffix().isEmpty()?"":"."+info.suffix()));
	QString archFilePath = S_NATIVE(dirPath+"\\"+packDirName+"\\"+info.baseName()+".lzdat");
	QString dstFilePath = S_NATIVE(dirPath+"\\"+unpackDirName+"\\"+info.baseName()+"_ulz"+(info.suffix().isEmpty()?"":"."+info.suffix()));

	std::cout << QString("--Begin test file: %1--").arg(info.fileName()).toAscii().data() << std::endl;
	
	method->reset(ArchiveMethod::OnlyC);

	if( method->compressFile(srcFilePath, archFilePath) < 0)
		return -1;

	method->reset(ArchiveMethod::OnlyD);
	if(method->uncompressArchFile(archFilePath, dstFilePath) < 0)
		return -1;

	compare_2files_print1Diff(srcFilePath, dstFilePath);
	return 0;
}

void testpack_all_indir(ArchiveMethod* method, const QStringList& dirPaths, const QString& packDirName, const QString& unpackDirName)
{
	for(quint32 j=0; j<dirPaths.count(); ++j)
	{
		QDir dir(dirPaths[j]);
		if(!dir.exists()) {
			std::cout << QString("%1, Not exist dir!").arg(dirPaths[j]).toAscii().data() << std::endl;
			return;
		}
		dir.setFilter(QDir::Files);

		//добавить возможность рекурсивного обхода папки
		for(quint32 i=0; i<dir.count(); ++i)
			if(test_file(method, dir.absoluteFilePath(dir[i]), packDirName, unpackDirName) < 0)
				break;
	}

}

void range_test(ArchiveMethod* method, const QStringList& dirPaths, const QString& packDirName, const QString& unpackDirName,
								qint32 minWndBits, qint32 maxWndBits, qint32 minHashBits, qint32 maxHashBits)
{
	MethodParams& params = method->params();

	params.setParam("MinMatch",4);
	params.setParam("MaxDepth",100);
	params.setParam("BlockBits", 5);

	for(qint32 wndBits = minWndBits; wndBits <= maxWndBits; ++wndBits)
	{
		for(qint32 hashBits = minHashBits; hashBits <= maxHashBits; ++hashBits)
		{
			params.setParam("WindowBits",wndBits);
			params.setParam("HashBits", hashBits);
			_DTimeStamp.Write("WndBits %d HashBits %d", params.intParam("WindowBits"), params.intParam("HashBits"));
			std::cout << "Begin_compression_test WndBits " << params.intParam("WindowBits")
				<< "HashBits " << params.intParam("HashBits") << std::endl;
			testpack_all_indir(method, dirPaths, packDirName, unpackDirName);
		}
	}
}

int main(int argc, char* argv[])
{
	ArchiveMethod* method = new LZX_Method();
	ArchiveMethod* suff_method = new Suffix_LZ_Method();
	suff_method->setDefaultParams();

	MethodParams& params = method->params();
	method->setDefaultParams();
	
	QStringList dirPaths;
  dirPaths << S_NATIVE("..\\test"); //<< S_NATIVE("..\\test\\VYCCT") << S_NATIVE("..\\test\\CalgCC");
	//Устанавливаем параметры диапазона тестирования
  range_test(method, dirPaths, "archives", "unpacks", 16, 16, 11, 11);
	//testpack_all_indir(method, S_NATIVE("..\\test\\artest26"),  "archives_suff", "unpacks_suff");
	//testpack_all_indir(method, S_NATIVE("..\\test\\CalgCC"), "archives", "unpacks");
  getchar();

	method->destroy();
	suff_method->destroy();
	delete method;
	delete suff_method;
	return 0;
}

