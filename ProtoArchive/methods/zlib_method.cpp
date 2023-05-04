#include "zlib_method.h"

void ZLIB_Method::_Init()
{
}
quint32 ZLIB_Method::_CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer)
{
	return 0;

}
//Распаковывает весь блок целиком, возращает размер распакованного блока
quint32 ZLIB_Method::_UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer)
{
	return 0;
}

//Установить стандартные параметры
void ZLIB_Method::_SetDefaultParams()
{
}
