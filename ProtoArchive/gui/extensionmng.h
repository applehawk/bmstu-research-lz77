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
	char* fmt; //���������� �������
	size_t source_total_size; //��������, ����� ������
	size_t compressed_total_size; //��������, ����� ������
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
	//���-�� ������������ ������ �������� �����
	volatile int m_nProcessedBytes;
	QString m_fileName;
	volatile int m_fileSize;
	//���� ���������(������,����������,�����)
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
	//��������� �������� ������ ���������� ������
	void changeStatus(ArchStatus* status);
protected:
  //����� ���������
  ArchiveMethod* m_archMethod;
	ArchStatus m_status;
	QStringList m_filesList;
	//��� ����� ������ � ������� ��������� ������,
	//��� �� �������� ���������� ����������
	QString m_archFile;
	QString m_destDir;
private:
	//��������� ���������� ������������ ��� ��������(������ ��� ����������)
	bool m_bCompression;
public:
	//������ ������ �� ������ ������(���������� �������������� ���������)
	void startCompressProcess(const QStringList& filesList, const QString& destFile);
	//������ ������ �� ����������
	void startDecompressProcess(const QString& sourceFile, const QString& destDir);
  ArchiveMethod* archMethod() { return m_archMethod; }
protected:
	//������ ������������� � ������
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
	������ ���������� ����� ����������� ����������� ����� ������
	*/
	virtual void load() {};
	virtual void unload() {};

	//������� ����� ��� ������ � ������ �������� ������
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

	//���� �� ������� ����� (*.APF � �.�.)
	ArchFormat* findFormatClass(const QString& fmtName);

	//��������� ����� ��� ������ ��� ����������
	//���� NULL, �� ������������� ������ �� ��������������
  ArchThread* createArchThread(const QString& fmtName, MethodType type);
};

#endif
