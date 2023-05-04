#ifndef __ARCHIVE_METHOD__H__
#define __ARCHIVE_METHOD__H__

#include <QBuffer>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDataStream>

enum MethodType
{
  LZX = 0,
  LZ_SUFFIX,
  ZLib,
};

#define DEFAULT_WNDBITS 15
#define DEFAULT_BLOCKBITS 6
#define DEFAULT_MINMATCH 3

//����� ���������� ��� ������� ���������
//�.�. ��� ������� �� ������� ��������� ����������
//��� � ������ ������ �� ���������������� �� ����������

//������ ����������� ��������� �����(������ ���������) � ������������
class MethodParams
{
private:
	QMap<QString, qint32> m_int_params;
	QMap<QString, float> m_float_params;
public:
	bool isExist(QString& paramName)
	{
		return 
			m_float_params.contains(paramName) || 
			m_int_params.contains(paramName);
	}
	qint32 intParam(const QString& name)
	{
		return (m_int_params.contains(name) ? m_int_params[name] : 0);
	}
	float floatParam(const QString& name)
	{
		return (m_float_params.contains(name) ? m_float_params[name] : 0.f);
	}
	void setParam(const QString& name, qint32 val)
	{
		//if(m_int_params.contains(name))
			m_int_params[name] = val;
	}
	void setParam(const QString& name, float val)
	{
		//if(m_float_params.contains(name))
			m_float_params[name] = val;
	}

	//�������� ������ ������������� ����������
	QStringList paramsIntList()
	{	return m_int_params.uniqueKeys(); }

	QStringList paramsFloatList()
	{	return m_float_params.uniqueKeys(); }

	QStringList paramsNameList()
	{
		return paramsIntList() + paramsFloatList();
	}

};

  enum CompressionMode
  {
    Blocking = 0,
    Streaming,
  };

  enum CompressionFunction
  {
    Not = 0,
    OnlyCompression,
    OnlyDecompression,
    CompressionAndDecompression,

    OnlyC = OnlyCompression,
    OnlyD = OnlyDecompression,
    CandD = CompressionAndDecompression,
  };

//������� ����� ������ �������������
//�������� �������� �� ������ ������� ������, � ����������
//�������� ������ ������, � ����������
class ArchiveMethod
{
private:
  MethodType m_type;
  MethodParams m_params;
	CompressionFunction m_func;
protected:
	virtual void _InitMethod( CompressionFunction function ) = 0;
	virtual void _DestroyMethod() = 0;

  //������, ����������� ��� ������ ����������� ��� ������� ������ ������ ����������
  virtual void _WriteParams(QDataStream& out) = 0;
  virtual int _ReadParams(QDataStream& in) = 0;
	//������� ���� ������ ������ � ������
	//������� ���������: �������� ������, ������ ����, � ������ ��� ������ ����������
	//�������� ����������: ������ packBuffer'� ������ ���� ������ sourceBuffer
	virtual quint32 _CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer) = 0;
	//������������� ���� ���� �������, ��������� ������ �������������� �����
	virtual quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer) = 0;

	//��������� ������("�� ����" �������� ������ �� �������� ������, ������� ��, � ���������� � ��������� �����)
	virtual quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size) = 0;
	virtual quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) = 0;
	
	//���������� ����������� ���������
	virtual void _SetDefaultParams() = 0;
	bool bInit;
public:
  ArchiveMethod(MethodType type);
	virtual ~ArchiveMethod();
	//����� ���������
	quint32 compress(QByteArray& src, QByteArray& dst) 
		{	return _CompressedBlock(src,dst); }
	quint32 uncompress(QByteArray& src, QByteArray& dst) 
		{	return _UncompressedBlock(src,dst); }

	quint32 compressStream(QDataStream& input, QDataStream& output, quint32 size)	
		{	return _CompressedStream(input, output, size); }
	quint32 uncompressStream(QDataStream& input, QDataStream& output, quint32 size)	
		{	return _UncompressedStream(input, output, size); }

	//��������� ������ ��� ���������� ����������� ������ ���������
	void setDefaultParams() { _SetDefaultParams(); }
	MethodParams& params() { return m_params; }

  MethodType type() const { return m_type; }

	bool isInit() { return bInit; }
	CompressionFunction func() { return m_func; }

	void reset( CompressionFunction function )
	{
		if(isInit())
			destroy();
		init(function);
	}

	void init( CompressionFunction function )
	{
		_InitMethod(function);
		m_func = function;
		bInit = true;
	}

	void destroy()
	{
		_DestroyMethod();
		m_func = Not;
		bInit = false;
	}
	

	//������� ������, �������� �� �� �����
	//��������� ���� ����� ���������� ������
	//dataSize - ������ ���� ������
	//blockSize - ������ ������ �� ������(��������� ����� �� ��������������, ���� �������� dataSize �� ������ blockSize)
	//��������� ���-�� ���������� ������

	//��� ������ ����� �������� ���� ������ �������
	//�������� ������ ��� ������ �����+��� ���������(������ �����*2)+������ ������������ ����������
	//��������� ������ ������ ������, ������������� ����� - ������
	//� dataSize ��������� �������� ����� ������������ ������
	qint64 compressDataBlocks(QDataStream& input, QDataStream& output, qint64& dataSize, qint32 blockSize, qint32& write_blocks); //������ ������ compressIStream
	//blocks - ���-�� ������ ��� ����������
	//������������� ������������ ���-�� ������ �������� ������ ������(������� ������ ��� ���������� �� ������� ������)
	//� blocks ��������� �������� ���-�� ��������� ������
	//return ����� ������������� ������
	qint64 uncompressDataBlocks(QDataStream& input, QDataStream& output, qint32& blocks);
	

	//��������������� �����(������ ������������?)
	//������� ���� � ���������� � ���� ������)
	//��������� ������ ������ ������
	qint64 compressFile(const QString& qsSourceFile, const QString& qsArchFile, CompressionMode mode = Blocking);
	//������������� ����
	//��������� ������ ������������� ������
	qint64 uncompressArchFile(const QString& qsArchFile, const QString& qsDestFile);

	//�������� � ���������� ������ �� ������.
	qint64 compressFiles(const QStringList& files, const QString& qsArchFile, CompressionMode mode = Blocking);
	qint64 uncompressFiles(const QString& qsArchFile, const QString& destDir);
};

#endif
