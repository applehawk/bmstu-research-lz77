#ifndef __SUFFIX_METHOD__H__
#define __SUFFIX_METHOD__H__

#include "suffix_lz_method.h"
#include "../patl/lz77_suffix.hpp"


class IFunctor_BArray
{
public:
	IFunctor_BArray(QByteArray& pBytes) : m_nTotalBytes(0)
	{
		m_ibuf.setBuffer(&pBytes);
		m_ibuf.open(QBuffer::ReadOnly);
	}
	~IFunctor_BArray() 
	{
		m_ibuf.close();
	}
	bool operator!() const
	{		return !m_ibuf.atEnd();	}
	unsigned operator()(void *buf, unsigned size)
	{
		qint64 readed = m_ibuf.read(static_cast<char*>(buf), size);
		Q_ASSERT_X(readed > 0, __FILE__, "IFunctor_BArray, not readed from ibuffer");
		m_nTotalBytes += readed;
		return readed;
	}
	//¬сего считанных байт
	quint32 totalBytes() { return m_nTotalBytes; }
private:
	QBuffer m_ibuf;
	quint32 m_nTotalBytes;
};

class OFunctor_BArray
{
public:
	OFunctor_BArray(QByteArray& pBytes)  : m_nTotalBytes(0)
	{
		m_obuf.setBuffer(&pBytes);
		m_obuf.open(QBuffer::WriteOnly);
	}
	~OFunctor_BArray() 
	{
		m_obuf.close();
	}
	bool operator!() const
	{		return !m_obuf.atEnd();	}
	unsigned operator()(const void *buf, unsigned size)
	{		
		quint32 writed = m_obuf.write(static_cast<const char*>(buf), size);
		m_nTotalBytes += writed;
		return writed;
	}
	quint32 totalBytes() { return m_nTotalBytes; }
private:
	QBuffer m_obuf;
	quint32 m_nTotalBytes;
};

class IFunctor_DStream
{
public:
	IFunctor_DStream(QDataStream* pStream) : m_istream(pStream), m_nTotalBytes(0) {}
	~IFunctor_DStream() { }
	bool operator!() const
	{		return !m_istream->atEnd();	}
	unsigned operator()(void *buf, unsigned size)
	{		
		quint32 readed = m_istream->readRawData(static_cast<char*>(buf), size);
		m_nTotalBytes += readed;
		return readed;
	}
	quint32 totalBytes() { return m_nTotalBytes; }
private:
	QDataStream* m_istream;
	quint32 m_nTotalBytes;
};

class OFunctor_DStream
{
public:
	OFunctor_DStream(QDataStream* pStream) : m_ostream(pStream), m_nTotalBytes(0) {}
	~OFunctor_DStream() { }
	bool operator!() const
	{		return !m_ostream->atEnd();	}
	unsigned operator()(void *buf, unsigned size)
	{
		quint32 writed = m_ostream->writeRawData(static_cast<char*>(buf),size);
		m_nTotalBytes += writed;
		return writed;
	}
	quint32 totalBytes() { return m_nTotalBytes; }
private:
	QDataStream* m_ostream;
	quint32 m_nTotalBytes;
};

Suffix_LZ_Method::Suffix_LZ_Method() : ArchiveMethod(LZ_SUFFIX) {}
Suffix_LZ_Method::~Suffix_LZ_Method() {}

void Suffix_LZ_Method::_InitMethod( CompressionFunction function )
{
}
void Suffix_LZ_Method::_DestroyMethod()
{
}

void Suffix_LZ_Method::_WriteParams(QDataStream& out)
{
  out << (qint16)type();
  out << params().intParam("BufferBits");
  out << params().intParam("WindowBits");
  out << params().intParam("MinMatch");
}
int Suffix_LZ_Method::_ReadParams(QDataStream& in)
{
  qint16 atype;
  qint32 param;
  in >> atype;
  if(atype != type())
    return -1;
  in >> param;
  params().setParam("BufferBits", param);
  in >> param;
  params().setParam("WindowBits", param);
  in >> param;
  params().setParam("MinMatch", param);
  return 0;
}

quint32 Suffix_LZ_Method::_CompressedStream(QDataStream& input, QDataStream& output, quint32 size) 
{
	OFunctor_DStream ofunctor(&output);
  IFunctor_DStream ifunctor(&input);
  lzCompress(params().intParam("BufferBits"), params().intParam("WindowBits"), params().intParam("MinMatch"),
             ifunctor, ofunctor);
	return ofunctor.totalBytes();
}
quint32 Suffix_LZ_Method::_UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) 
{
	OFunctor_DStream ofunctor(&output);
  IFunctor_DStream ifunctor(&input);
  lzDecompress(ifunctor, ofunctor);
	return ofunctor.totalBytes();
}

quint32 Suffix_LZ_Method::_CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer)
{
	OFunctor_BArray ofunctor(packBuffer);
  IFunctor_BArray ifunctor(sourceBuffer);
	lzCompress(params().intParam("BufferBits"), params().intParam("WindowBits"),	params().intParam("MinMatch"),
    ifunctor, ofunctor);
	return ofunctor.totalBytes();
}

//–аспаковывает весь блок целиком, возращает размер распакованного блока
quint32 Suffix_LZ_Method::_UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer)
{
	OFunctor_BArray ofunctor(dstBuffer);
  IFunctor_BArray ifunctor(packBuffer);
  lzDecompress(ifunctor, ofunctor);
	return ofunctor.totalBytes();
}

//”становить стандартные параметры
void Suffix_LZ_Method::_SetDefaultParams()
{
	params().setParam("WindowBits", 16);
	params().setParam("BufferBits", 20);
	params().setParam("MinMatch", 4);
}

#endif
