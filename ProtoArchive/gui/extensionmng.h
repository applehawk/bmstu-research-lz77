#ifndef __ARCHPROJ_EXTENSION_MNG__
#define __ARCHPROJ_EXTENSION_MNG__

#include <QList>
#include <QString>
#include <QStringList>
#include <QLibrary>
#include <QThread>

#include "..\methods\archive_method.h"

class BitsStream;
class ArchiveMethod;

struct CompressInfo
{
	char* fmt; //расширение формата
	size_t source_total_size; //искодный, общий размер
	size_t compressed_total_size; //конечный, общий размер
	int total_files;
};

class StatusDlg;

enum eSTATUS_STATE
{
	STAT_Init = 1,
	STAT_Compression,
	STAT_Decompression,
	STAT_Finished,
	STAT_Canceled,

	STAT_Fail = -1,
};
class ArchStatus : public QObject
{
Q_OBJECT
private:
	//Кол-во обработанных байтов текущего файла
	volatile int m_nProcessedBytes;
	QString m_fileName;
	volatile int m_fileSize;
	//Флаг состояния(сжатие,распаковка,финиш)
  volatile int m_state;
signals:
  void changeStatus();
public:
	ArchStatus()
	{
		setCurrentFile(QString(),0);
	}

	void setCurrentFile(const QString& fileName, int fileSize)
	{
		m_fileName = fileName;
		m_fileSize = fileSize;
	}
	void set(int nProcessedBytes)
	{
		m_nProcessedBytes = nProcessedBytes;
	}

	int processedBytes() { return m_nProcessedBytes; }

	int fileSize() { return m_fileSize; }
	const QString& fileName() { return m_fileName; }

	void setFlag(eSTATUS_STATE state_flag) { m_state = state_flag; }

	bool isFail() { return m_state == STAT_Fail; }
	bool isInit() { return m_state == STAT_Init; }
	bool isFinished() { return m_state == STAT_Finished; }
	bool isCompression() { return m_state == STAT_Compression; }
	bool isDecompression() { return m_state == STAT_Decompression; }
	bool isCanceled() { return m_state == STAT_Canceled; }
};

class ArchThread : public QThread
{
	Q_OBJECT
signals:
	//Позволяет получать статус выполнения потока
	void changeStatus(ArchStatus* status);
protected:
  //Метод архивации
  ArchiveMethod* m_archMethod;
	ArchStatus m_status;
	QStringList m_filesList;
	//Имя файла архива в который сжимаются данные,
	//или из которого происходит распаковка
	QString m_archFile;
	QString m_destDir;
private:
	//Булевская переменная определяющая тип процесса(сжатия или распаковка)
	bool m_bCompression;
public:
	//Запуск потока на сжатие данных(передаются соответсвующие параметры)
	void startCompressProcess(const QStringList& filesList, const QString& destFile);
	//Запуск потока на распаковку
	void startDecompressProcess(const QString& sourceFile, const QString& destDir);
  ArchiveMethod* archMethod() { return m_archMethod; }
protected:
	//Методы выполняющиеся в потоке
	virtual void compressRun() = 0;
	virtual void decompressRun() = 0;
	void run();
};

class ArchFormat
{
public:
  ArchFormat() {}
  virtual ~ArchFormat() {}
	/*
	Методы вызываются когда динамически добавляется новый формат
	*/
	virtual void load() {};
	virtual void unload() {};

	//Создает поток для работы с данным форматом архива
  virtual ArchThread* createArchThread(MethodType type) = 0;

	virtual void formats_support(QStringList& fmts) = 0;
};

class ExtensionMng
{
private:
	QLibrary m_loader;
	QList<ArchFormat*> m_methods;
	QString m_string;
public:
	ExtensionMng();
	~ExtensionMng();

	int formats_list(const QStringList& outFormats);

	void load_extension(const QString& file);
	void add_extension(ArchFormat* extFmt);
	void load_extensions_dir(const QString& dir);

	void unload_extensions();

	//Ищет по формату файла (*.APF и т.д.)
	ArchFormat* findFormatClass(const QString& fmtName);

	//Возращает поток для сжатия или распаковки
	//если NULL, то запрашиваемый формат не поддерживается
  ArchThread* createArchThread(const QString& fmtName, MethodType type);
};

#endif
