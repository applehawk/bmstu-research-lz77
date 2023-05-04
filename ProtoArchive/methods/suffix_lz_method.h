#include "archive_method.h"

class Suffix_LZ_Method : public ArchiveMethod
{
public:
	Suffix_LZ_Method();
	~Suffix_LZ_Method();
protected:
	void _InitMethod( CompressionFunction function );
	void _DestroyMethod();

  void _WriteParams(QDataStream& out);
  int _ReadParams(QDataStream& in);

	quint32 _CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer);
	//������������� ���� ���� �������, ��������� ������ �������������� �����
	quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer);

	//���������� ����������� ���������
	void _SetDefaultParams();

	//��������� ������("�� ����" �������� ������ �� �������� ������, ������� ��, � ���������� � ��������� �����)
	quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size);
	quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size);
};
