#ifndef __ZLIB_METHOD_H__
#define __ZLIB_METHOD_H__

#include "archive_method.h"

//Не многопоточный класс(для каждого потока нужен свой экземпляр!)
class ZLIB_Method : public ArchiveMethod
{
public:
	ZLIB_Method();
	~ZLIB_Method();
protected:
	void _Init();
	quint32 _CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer);
	//Распаковывает весь блок целиком, возращает размер распакованного блока
	quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer);

	//Установить стандартные параметры
	void _SetDefaultParams();

	//Потоковое сжатие("на ходу" получает данные из входного потока, сжимает их, и отправляет в выходного поток)
	quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
	quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) {return 0;}
};

#endif