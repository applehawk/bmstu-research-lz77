#ifndef __STATUS_UPDATER_LZX_H__
#define __STATUS_UPDATER_LZX_H__

#include "..\lzx\include\types_lzx.h"

enum eLZX_STATE
{
	lzxInit = 0, //флаг инициализации алгоритма сжатия/распаковки
	lzxCompression,//флаг сжатия - показывает, что сейчас идет процесс сжатия
	lzxDecompression,//флаг распаковки
	lzxFinished,//флаг завершения - означает, что все доступные блоки были сжаты
	lzxFail,//флаг ошибки - ошибка при сжатии
	lzxCancel,//флаг отмены(посылается инициатором процесса)
};

class StatusUpdater
{
private:
	eLZX_STATE back_flag;//обратный флаг
public:
	void cancelProcess() { back_flag = lzxCancel; }
	bool isCancel() { return back_flag == lzxCancel; }
        virtual void UpdateStatus(uint64 in_block_bytes, uint64 out_block_bytes, uint16 state_flag) = 0;
};

#endif
