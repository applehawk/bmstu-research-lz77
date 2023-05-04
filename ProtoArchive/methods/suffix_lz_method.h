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
	//Распаковывает весь блок целиком, возращает размер распакованного блока
	quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer);

	//Установить стандартные параметры
	void _SetDefaultParams();

	//Потоковое сжатие("на ходу" получает данные из входного потока, сжимает их, и отправляет в выходного поток)
	quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size);
	quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size);
};
