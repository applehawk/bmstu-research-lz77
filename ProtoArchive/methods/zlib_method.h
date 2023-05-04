#ifndef __ZLIB_METHOD_H__
#define __ZLIB_METHOD_H__

#include "archive_method.h"

//�� ������������� �����(��� ������� ������ ����� ���� ���������!)
class ZLIB_Method : public ArchiveMethod
{
public:
	ZLIB_Method();
	~ZLIB_Method();
protected:
	void _Init();
	quint32 _CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer);
	//������������� ���� ���� �������, ��������� ������ �������������� �����
	quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer);

	//���������� ����������� ���������
	void _SetDefaultParams();

	//��������� ������("�� ����" �������� ������ �� �������� ������, ������� ��, � ���������� � ��������� �����)
	quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
	quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
};

#endif