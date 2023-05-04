#include "lzx_method.h"

#include <QtDebug>

LZX_Method::LZX_Method() : ArchiveMethod(LZX) {
}

LZX_Method::~LZX_Method()
{
	_DestroyMethod();
}

void LZX_Method::_InitMethod( CompressionFunction function )
{
	if(isInit()) {
		printf("Error!!!");
		return ;
	}
	if(params().intParam("BlockBits") >= params().intParam("WindowBits"))
	{
		qDebug() << "Invalid values!, BlockBits " << params().intParam("BlockBits")
						 << " WindowBits " << params().intParam("BlockBits");

		params().setParam("BlockBits", LZHX_BLOCK_BITS);
		params().setParam("WindowBits", LZHX_WND_BITS);
	}

	if(function == OnlyC || function == CandD)
		initCompression_HASH(&hstate_compress, 
			params().intParam("WindowBits"), 
			params().intParam("BlockBits"), 
			params().intParam("HashBits"), 
			params().intParam("MinMatch"),
			params().intParam("MaxDepth"));
	if(function == OnlyD || function == CandD)
		initDecompression_HASH(&hstate_decompress,
			params().intParam("WindowBits"),
			params().intParam("BlockBits"));
}

void LZX_Method::_DestroyMethod()
{
	if(func() == OnlyC || func() == CandD)
		destroyCompression_HASH(&hstate_compress);
	if(func() == OnlyD || func() == CandD)
		destroyDecompression_HASH(&hstate_decompress);
}

void LZX_Method::_WriteParams(QDataStream& out)
{
  out << (qint16)type();
  out << params().intParam("BlockBits");
  out << params().intParam("WindowBits");
  //��������� ��������� ����� �� ���������, ��� �� ����� ��� ����������
  //������ ��� ��������� ����������
  out << params().intParam("HashBits");
  out << params().intParam("MinMatch");
  out << params().intParam("MaxDepth");
}
int LZX_Method::_ReadParams(QDataStream& in)
{
  qint16 atype;
  qint32 param;
  in >> atype;
  if(atype != type())
    return -1;
  in >> param;
  params().setParam("BlockBits", param);
  in >> param;
  params().setParam("WindowBits", param);
  //���.���������
  in >> param;
  params().setParam("HashBits", param);
  in >> param;
  params().setParam("MinMatch", param);
  in >> param;
  params().setParam("MaxDepth", param);
  return 0;
}

quint32 LZX_Method::_CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer)
{
	if(packBuffer.size() < sourceBuffer.size() << 1)
		packBuffer.resize(sourceBuffer.size() << 1);
	setIOBuffers_HASH(&hstate_compress, (uint8*)sourceBuffer.data(), sourceBuffer.size(), (uint8*)packBuffer.data(), packBuffer.size());
	char* data = packBuffer.data();
	quint32 packedSize = compressIBlock_HASH(&hstate_compress);
	return packedSize;//packBuffer.resize(packedSize);
}
//������������� ���� ���� �������, ��������� ������ �������������� �����
quint32 LZX_Method::_UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer)
{
  //�� ����� ��������� �������������� ������....(���������, ���� ��������� �������� ������� ������)
  //���� ������� ������ �������(�������� ������ ������������� ������ ������ ��� � 2 ����, �� ����� ������!)
  if(dstBuffer.size() < packBuffer.size() << 1)
    dstBuffer.resize(packBuffer.size() << 1);
	setIOBuffers_HASH(&hstate_decompress, (uint8*)packBuffer.data(), packBuffer.size(), (uint8*)dstBuffer.data(), dstBuffer.size());
	quint32 unpackSize = decompressIBlock_HASH(&hstate_decompress);
	return unpackSize;
}


//���������� ����������� ���������
void LZX_Method::_SetDefaultParams()
{
	params().setParam("WindowBits", LZHX_WND_BITS);
	params().setParam("HashBits", LZHX_HASH_BITS);
	params().setParam("BlockBits", LZHX_BLOCK_BITS);
	params().setParam("MinMatch", LZHX_MIN_MATCH);
	params().setParam("MaxDepth", LZHX_MAX_DEPTH);
}
