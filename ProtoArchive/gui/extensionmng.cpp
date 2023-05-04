#include "extensionmng.h"
#include <QMessageBox>

//Запуск потока на сжатие данных(передаются соответсвующие параметры)
void ArchThread::startCompressProcess(const QStringList& filesList, const QString& destFile)
{
	m_filesList = filesList;
	m_archFile = destFile;
	m_bCompression = true;
	start(NormalPriority);
}
//Запуск потока на распаковку
void ArchThread::startDecompressProcess(const QString& sourceFile, const QString& destDir)
{
	m_archFile = sourceFile;
	m_destDir = destDir;
	m_bCompression = false;
	start(NormalPriority);
}
void ArchThread::run()
{
	if(m_bCompression)
		compressRun();
	else
		decompressRun();
}

ExtensionMng::ExtensionMng(){}
ExtensionMng::~ExtensionMng(){}

int ExtensionMng::formats_list(const QStringList& outFormats)
{
	return 0;
}

void ExtensionMng::unload_extensions()
{
	ArchFormat* format=NULL;
	QList<ArchFormat*>::iterator iter,end=m_methods.end();
	for(iter = m_methods.begin(); iter != end; ++iter)
	{
		format = (*iter);
		format->unload();
		delete format;
	}
}

void ExtensionMng::load_extension(const QString& file)
{
//*TODO*//
}

void ExtensionMng::load_extensions_dir(const QString& dir)
{
//*TODO//
}

void ExtensionMng::add_extension(ArchFormat* extFmt)
{
	if(!extFmt)
		return;
	m_methods.push_back(extFmt);
}


ArchFormat* ExtensionMng::findFormatClass(const QString& fmtName)
{
	//Ищем в менеджере расширений поддержку переданного формата
	ArchFormat* format = NULL;
	QList<ArchFormat*>::iterator iter,end=m_methods.end();
	for(iter = m_methods.begin(); iter != end; ++iter)
	{
		format = (*iter);
		QStringList fmts;
		format->formats_support(fmts);
    if(fmts.contains(fmtName.toLower()))
			return format;
	}
	return NULL;
}


ArchThread* ExtensionMng::createArchThread(const QString& fmtName, MethodType type)
{
	ArchFormat* format = findFormatClass(fmtName);
	if(format)
	{
    return format->createArchThread(type);
	}
	return NULL;
}
