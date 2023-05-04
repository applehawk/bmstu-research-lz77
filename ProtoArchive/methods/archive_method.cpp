#include "archive_method.h"

#include <QFile>
#include <QDir>

#include <QtDebug>

#include "string.h"

#define BLOCK_SIZE 0x19000 //100�����
#define HEADER_NAME "PA_10" //ProtoArhivator 1.0
#define HEADER_NAME_LEN 5

//���������� � ����, ������ ����� ��������� >> � << QDataStream
struct FileInfo_Block
{
	FileInfo_Block() {}
	FileInfo_Block(const QString& fName, quint32 attributes, quint64 srcSize, quint32 nblocks, quint64 packedSize)
	{
		fileName = fName;
		attribs = attributes;
		blocks = nblocks;
		sourceSize = srcSize;
		compressedSize = packedSize;
	}
  FileInfo_Block( const FileInfo_Block& copy)
  {
    fileName = copy.fileName;
    attribs = copy.attribs;
    blocks = copy.blocks;
    sourceSize = copy.sourceSize;
    compressedSize = copy.compressedSize;
  }
	QString fileName; //��� �����
  quint32 attribs; //��������� �����
  qint32 blocks; //���-�� ������ �� ����, ���� -1 �� �������������� ��������� ������
  qint64 sourceSize; //�������� ������
  qint64	compressedSize; //������ ������
};

inline qint32 sizeof_infoblock(const FileInfo_Block& block)
{
  //�������, �� �� Qt 4.5.2 ���� ���� ��������� � ������� 4 ����� �� ����� �����
  //��� ��������� ��������� ������ ������?
  return (block.fileName.size()/*+2 //Qt 4.5.2//*/)*2+
		sizeof(FileInfo_Block)-sizeof(QString);
}

QDataStream& operator << (QDataStream & s, const FileInfo_Block& block)
{
	//default BigEndian
  //������ ����� �� ���������� ? (��������� � ���������� ������ Qt?)
	s << block.fileName; //���������� � ������� ��� �����
	s << block.attribs;
	s << block.blocks;
	s << block.sourceSize;
	s << block.compressedSize;
	return s;
}

QDataStream& operator >> (QDataStream & s, FileInfo_Block& block)
{
	s >> block.fileName;
	s >> block.attribs;
	s >> block.blocks;
	s >> block.sourceSize;
	s >> block.compressedSize;
	return s;
}

ArchiveMethod::ArchiveMethod(MethodType type) : bInit(false), m_type(type)
{
}
ArchiveMethod::~ArchiveMethod() 
{
}

//��� ������ ����� �������� ���� ������ �������(������������� ����������� � ��������� �������� ������ ������ "<<")
qint64 ArchiveMethod::compressDataBlocks(QDataStream& input, QDataStream& output, qint64& dataSize, qint32 blockSize, qint32& write_blocks)
{
	if(!(func() == OnlyC || func() == CandD))
  {
    qDebug() << "Error Compression Function, compressDataBlocks";
		return -1;
	}

	QByteArray buf;
	QByteArray dstbuf;
	
	buf.resize(blockSize);
	qint64 nPackedSize = 0;
	qint64 nReaded = 0;
	output << blockSize;
	while( !input.atEnd() && (nReaded < dataSize) )
	{
		quint32 readedBytes = input.readRawData(buf.data(), buf.size());

		buf.resize(readedBytes);
		nReaded += readedBytes;

		quint32 compressBlockSize = compress(buf, dstbuf);
    nPackedSize += compressBlockSize;
		++write_blocks;
		output.writeRawData((const char*)&compressBlockSize, sizeof(compressBlockSize));
		output.writeRawData(dstbuf.data(), compressBlockSize);
	}
	dataSize = nReaded;
	return nPackedSize;
}
//blocks - ���-�� ������ ��� ����������
qint64 ArchiveMethod::uncompressDataBlocks(QDataStream& input, QDataStream& output, qint32& blocks)
{
	if(!(func() == OnlyD || func() == CandD))
	{
		qDebug() << "Error Decompression Function, uncompressDataBlocks";
		return -1;
	}

	QByteArray buf;
	QByteArray dstbuf;
	//��� ������� ����� ������ ������� ��������� ������(���� ��������� �����)

	qint32 nUnpackedSize = 0;
	qint32 nBlocks = 0;
	qint32 blockSize;
	input >> blockSize;
  dstbuf.resize(blockSize); //�������� ������, ��������������, ��� ������������� ���� ������������ � ���� ������(����� ������!)
	while( !input.atEnd() && (nBlocks++ < blocks) )
	{
		quint32 compressBlockSize;
		input.readRawData((char*)&compressBlockSize, sizeof(compressBlockSize));
		if(static_cast<quint32>(buf.size()) < compressBlockSize)
			buf.resize(compressBlockSize);

		input.readRawData(buf.data(), compressBlockSize);
		quint32 uncompressBlockSize = uncompress(buf, dstbuf);
		output.writeRawData(dstbuf.data(), uncompressBlockSize);
		nUnpackedSize += uncompressBlockSize;
	}
	blocks = nBlocks;

	return nUnpackedSize;
}


//��������������� ����� 
//� ������ ������ ������ ��������� ���-�� ������
qint64 ArchiveMethod::compressFile(const QString& qsSourceFile, const QString& qsArchFile, CompressionMode mode)
{
	if(!(func() == OnlyC || func() == CandD))
	{
		qDebug() << "Error Compression Function, compressFile";
		return -1;
	}

	QFile srcFile(qsSourceFile);
	QFile archFile(qsArchFile);
	if (!srcFile.open(QIODevice::ReadOnly) || 
			!archFile.open(QIODevice::WriteOnly))
	{
		qDebug() << "Error open " << 	(srcFile.isOpen() ? "source file ":"") << (archFile.isOpen() ? "archive file":""); 
		return -1;
	}

	QDataStream input(&srcFile);
	QDataStream output(&archFile);

  //������� � ������ ��������� ������(��������, ������ ����, ������ ����)
  output << (qint16)mode;
  //������ ���������� ����������� � ����������� ���������(�������� ����������� ��� ������)
  _WriteParams(output);

	qint64 srcSize = srcFile.size();
	qint64 compressSize;

	if(mode == Blocking) 
	{
    qint32 write_blocks = 0;
		quint32 posNBlocks = output.device()->pos();
		output.writeRawData("",4);
		compressSize = compressDataBlocks(input, output, srcSize, BLOCK_SIZE, write_blocks);
		output.device()->seek(posNBlocks);
		output << write_blocks;
	}
	else if(mode == Streaming)
		compressSize = compressStream(input, output, srcFile.size());

	archFile.close();
	srcFile.close();

	return compressSize;
}
//������������� ����
qint64 ArchiveMethod::uncompressArchFile(const QString& qsArchFile, const QString& qsDestFile)
{
	if(!(func() == OnlyD || func() == CandD))
	{
		qDebug() << "Error Decompression Function, uncompressFiles";
		return -1;
	}
	QFile archFile(qsArchFile);
	QFile destFile(qsDestFile);
	if (!archFile.open(QIODevice::ReadOnly) || 
		!destFile.open(QIODevice::WriteOnly))
	{
		qDebug() << "Error open " << 
			(!archFile.isOpen() ? QString("archive file \"")+archFile.fileName():QString::null) << 
			(!destFile.isOpen() ? QString("dest file \"")+destFile.fileName():QString::null); 
    destFile.close();
    archFile.close();
		return -1;
	}

	QDataStream input(&archFile);
	QDataStream output(&destFile);

	qint64 uncompressSize = 0;
  qint16 mode;
  input >> mode;
  //������� ������� ��������� �������������� ��� ������� ������ ���������
  if(_ReadParams(input) != 0)
  {
    qDebug() << "Error uncompress file, method type not corrected for this file!";
    destFile.close();
    archFile.close();
    return -1;
  }
  //����������� ������ �����,
  //�.�. ��������� ����� ���������, ������������� ����� ������������� �����
  reset( func() );

	if(mode == Blocking)
	{
		qint32 write_blocks;
		input >> write_blocks;
		if(write_blocks > 0)
		uncompressSize = uncompressDataBlocks(input, output, write_blocks);
	} else if(mode == Streaming)
		uncompressSize = uncompressStream(input, output, archFile.size());

	destFile.close();
	archFile.close();

	return uncompressSize;
}


qint64 ArchiveMethod::compressFiles(const QStringList& files, const QString& qsArchFile, CompressionMode mode)
{
	if(!(func() == OnlyC || func() == CandD))
	{
		qDebug() << "Error Compression Function, compressFiles";
		return -1;
	}

	QFile archFile(qsArchFile);
	if (!archFile.open(QIODevice::WriteOnly))
	{
		qDebug() << "Error open archive file"; 
		return -1;
	}
	QDataStream ostream(&archFile);
	QIODevice* outDev = ostream.device();

	//���������� ���������
	/* PA_10|���-�� ������|������� ���� � ��������|...������ ������, ������...|.|...|����������� ����� �����| 
	*/
	qint64 apos = outDev->pos();
	ostream.writeRawData(HEADER_NAME,strlen(HEADER_NAME));
  ostream << (qint32)files.count();
  //������ ����� ��������� ������ ������
  ostream << (qint16)mode;
  _WriteParams(ostream);

	qint32 headerSize = outDev->pos();
	qint32 tableSize = 0;
	//������� ������ ������
	FileInfo_Block* fileInfoTable = new FileInfo_Block[files.count()];
	
	QStringList::const_iterator iter, end = files.end();
	int k = 0;
	for(iter = files.begin(); iter != end; ++iter, ++k)
	{
    const QString& currFilePath = (*iter);
    qint64 fileSize = QFileInfo(currFilePath).size();
		//���������� ������ ��������� ����� ����, � ����� ������ ����� � ��� �����������
		fileInfoTable[k] =  FileInfo_Block(
      QFileInfo(currFilePath).fileName(), 0, fileSize, 0, 0);
		
		tableSize += sizeof_infoblock(fileInfoTable[k]);
	}
	k=0;
	ostream.writeRawData("",tableSize);
	qint64 totalCompressionSize = 0;
	//���������� ����� ��� �������
	for(iter = files.begin(); iter != end; ++iter, ++k)
	{
		const QString& currFilePath = (*iter);
		//���� ������ ����� �������, ������ � ��� �� ������(������ � ��� ����� ��� ���������)
		if(fileInfoTable[k].sourceSize != 0)
		{
			QFile currFile( currFilePath );
			if (!currFile.open(QIODevice::ReadOnly))
			{
				qDebug() << "Error open sourcce file " << currFilePath; 
				return -1;
			}
			QDataStream istream(&currFile);
			int blocks = 0;
			qint64 currFileSize = currFile.size();
      //���-�� ������ � ����� ��������� LZX, ��� ������������ �������������(���� ��� ���������� ������ ����������������, � ����� ���������������� prev?
      reset(func()); //������ ����� ��� ��������� �������������, ��� �������!
      qint64 compressedSize = compressDataBlocks(istream, ostream, currFileSize, BLOCK_SIZE, blocks);
      fileInfoTable[k].compressedSize = compressedSize;
			fileInfoTable[k].blocks = blocks;

			totalCompressionSize += fileInfoTable[k].compressedSize;

			currFile.close();
		}
	}
  outDev->seek(headerSize);
/*
  QByteArray infoTable(tableSize,0);

  //������� ������� ���������� � ������
  QByteArray compressedInfoTable;
  QBuffer buffInfoTable(&infoTable);
  buffInfoTable.open(QIODevice::WriteOnly);
  QDataStream dsInfoTable( &buffInfoTable );*/
  for(int i = 0; i < k; ++i)
    //dsInfoTable << fileInfoTable[i];
    ostream << fileInfoTable[i];
  /*buffInfoTable.close();
  quint32 compressedSize = compress(infoTable, compressedInfoTable);
  //��� ������
  QByteArray cInfoTable(compressedSize,0);
  cInfoTable.fromRawData(compressedInfoTable.data(), compressedSize);
  //���������� ������� �������� � ������ ������*/
  //ostream << tableSize; //������ ������������� �������
  //qint32 ctableSize = cInfoTable.size();
  //ostream << ctableSize;
  //ostream.writeRawData(cInfoTable.data(), ctableSize);

delete [] fileInfoTable;
	archFile.close();
	return totalCompressionSize;
}

qint64 ArchiveMethod::uncompressFiles(const QString& qsArchFile, const QString& destDir)
{
	if(!(func() == OnlyD || func() == CandD))
	{
		qDebug() << "Error Decompression Function, uncompressFiles";
		return -1;
	}
	QFile archFile(qsArchFile);
  if (!archFile.open(QIODevice::ReadOnly))
	{
		qDebug() << "Error open archive file"; 
		return -1;
	}
	QDataStream istream(&archFile);
	QString currFilePath;
  //����������... ���, ����� �������� ��������� � �������!

	//������ ������ 3 ����� - ��� �������
	char headName[HEADER_NAME_LEN];
	istream.readRawData(headName, HEADER_NAME_LEN);
	if(memcmp(headName,HEADER_NAME, HEADER_NAME_LEN)!=0)
	{	
		qDebug() << "Header name not valid!, current header name is " << HEADER_NAME;
		return -1;
	}
  //������ ���-�� ������
  qint32 filesCount;
  istream >> filesCount;
  //����� ���������
  qint16 mode;
  istream >> mode;
  //������� ������� ��������� �������������� ��� ������� ������ ���������
  if(_ReadParams(istream) != 0)
  {
    qDebug() << "Error uncompress file, method type not corrected for this file!";
    archFile.close();
    return -1;
  }
  //������������ �����
  reset( func() );

	QVector<FileInfo_Block> filesInfoTable;
	filesInfoTable.resize(filesCount);
  //������ ������ ������� ����(��� ������ ������������� ��� �� ��������, ��� � ��� ������ ���� ������ ������)
  qint32 tableSize;
  qint32 ctableSize;
  //istream >> tableSize;
  //istream >> ctableSize;
/*
  QByteArray compressedInfoTable(ctableSize,0);
  istream.readRawData(compressedInfoTable.data(), ctableSize);
  qint32 cSize = compressedInfoTable.size();
  QByteArray infoTable(tableSize,0);
  quint32 unSize = uncompress(compressedInfoTable, infoTable); //realloc ����� ������ ! resize �� �������
  QBuffer buffInfoTable(&infoTable);
  if(!buffInfoTable.open(QIODevice::ReadOnly))
  {
    qDebug() << "error open buffer info table";
    return -1;
  }
  QDataStream dsInfoTable(&buffInfoTable);*/

  //������ ������� ���� � ������
	for(int i=0; i<filesCount; ++i)
    istream >> filesInfoTable[i];
    //  dsInfoTable >> filesInfoTable[i];
  //buffInfoTable.close();

	qint64 totalUncompressedSize = 0;
	for(int i=0; i<filesCount; ++i)
	{
		QFile outFile;
		FileInfo_Block& refBlock = filesInfoTable[i];
		outFile.setFileName( QDir::toNativeSeparators(destDir + QString("\\") + refBlock.fileName) );
		
		outFile.open(QIODevice::WriteOnly);
		QDataStream ostream(&outFile);
		qint32 blocks = refBlock.blocks;
		totalUncompressedSize += uncompressDataBlocks(istream, ostream, blocks);
		outFile.close();
	}
	archFile.close();

	return totalUncompressedSize;
}
