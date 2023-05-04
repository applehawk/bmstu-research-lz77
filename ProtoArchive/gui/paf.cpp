#include "paf.h"
#include "statusdlg.h"

#include <QStringList>
#include <QTemporaryFile>
#include <QThread>
#include <QFileInfo>

#include "..\methods\archive_method.h"
#include "..\methods\lzx_method.h"
#include "..\methods\suffix_lz_method.h"

PAF_Thread::PAF_Thread( MethodType type )
{
  //Создаем в зависимости от типа алгоритма(не забываем указать тип!!!)
  switch( type )
  {
    case LZX:
      {
        m_archMethod = new LZX_Method();
        break;
      }
    case LZ_SUFFIX:
      {
        m_archMethod = new Suffix_LZ_Method();
        break;
      }
    default:
      m_archMethod = new LZX_Method();
  };

  m_archMethod->setDefaultParams();
}
PAF_Thread::~PAF_Thread()
{
  delete m_archMethod;
}

qint64 PAF_Thread::compressFiles(ArchiveMethod* method, const QStringList& files, const QString& qsArchFile,
                                ArchStatus* status, CompressionMode mode)
{
}
qint64 PAF_Thread::uncompressFiles(ArchiveMethod* method, const QString& qsArchFile, const QString& destDir,
                       ArchStatus* status)
{
}

void PAF_Thread::compressRun()
{
  m_archMethod->init(OnlyC);
  //Работает!!m_archMethod->compressFile(m_filesList[0], m_archFile, ArchiveMethod::Blocking);
  //compressFiles( m_filesList, m_archFile, ArchiveMethod::Blocking)
  m_archMethod->compressFiles( m_filesList, m_archFile, Blocking);
  m_archMethod->destroy();
}

void PAF_Thread::decompressRun()
{
  m_archMethod->init(OnlyD);
  //Работает!!m_archMethod->uncompressArchFile(m_archFile, m_destDir + "\\" + QFileInfo(m_archFile).baseName() + ".res");
  m_archMethod->uncompressFiles(m_archFile, m_destDir);
  m_archMethod->destroy();
}

PAF_ArchFormat::PAF_ArchFormat( PAF_Version version )
{
  m_version = version;
}

void PAF_ArchFormat::load() {}
void PAF_ArchFormat::unload() {}

ArchThread* PAF_ArchFormat::createArchThread( MethodType type )
{
  return new PAF_Thread( type );
}

void PAF_ArchFormat::formats_support(QStringList& fmts)
{
  fmts << "paf";
}
