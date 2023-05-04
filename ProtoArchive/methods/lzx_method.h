#include "archive_method.h"
#include "..\lzx\include\lzx.h"

//�� ������������� �����(��� ������� ������ ����� ���� ���������!)
class LZX_Method : public ArchiveMethod
{
public:
  LZX_Method();
	~LZX_Method();
private:
	lzx_hstate hstate_compress;
	lzx_hstate hstate_decompress;
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
	quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
	quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
};
