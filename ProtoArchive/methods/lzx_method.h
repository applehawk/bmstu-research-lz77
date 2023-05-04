#include "archive_method.h"
#include "..\lzx\include\lzx.h"

//Не многопоточный класс(для каждого потока нужен свой экземпляр!)
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
	//Распаковывает весь блок целиком, возращает размер распакованного блока
	quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer);

	//Установить стандартные параметры
	void _SetDefaultParams();

	//Потоковое сжатие("на ходу" получает данные из входного потока, сжимает их, и отправляет в выходного поток)
	quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
	quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
};
