#include "extensionmng.h"
#include <QThread>
#include <QStringList>

#include "..\methods\archive_method.h"
//Набор классов описывающий формат архива PAF - ProtoArchiveFormat 1.0

class ArchiveMethod;
class StatusDlg;

enum PAF_Version
{
  PAF_1_0 = 0,
};

class PAF_Thread : public ArchThread
{
public:
	//Если указан диалог статуса, в него передается информация о ходе выполнения процесса
  PAF_Thread( MethodType type );
  ~PAF_Thread();

  //Упаковка и распаковка файлов из архива формата PAF

  //Возращает размер сжатых данных
  static qint64 compressFiles(ArchiveMethod* method, const QStringList& files, const QString& qsArchFile,
                              ArchStatus* status = NULL, CompressionMode mode = Blocking);
  //Возращает размер распакованных данных
  static qint64 uncompressFiles(ArchiveMethod* method, const QString& qsArchFile, const QString& destDir,
                                ArchStatus* status = NULL);
protected:
	//Методы выполняющиеся в потоке
	void compressRun();
  void decompressRun();
};

class PAF_ArchFormat : public ArchFormat
{
private:
  PAF_Version m_version;
public:
  PAF_ArchFormat( PAF_Version version = PAF_1_0 );
  ~PAF_ArchFormat() {}

	void load();
	void unload();

  ArchThread* createArchThread( MethodType type );

	void formats_support(QStringList& fmts);
};
