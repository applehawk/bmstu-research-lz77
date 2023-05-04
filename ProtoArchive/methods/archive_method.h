#ifndef __ARCHIVE_METHOD__H__
#define __ARCHIVE_METHOD__H__

#include <QBuffer>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDataStream>

enum MethodType
{
  LZX = 0,
  LZ_SUFFIX,
  ZLib,
};

#define DEFAULT_WNDBITS 15
#define DEFAULT_BLOCKBITS 6
#define DEFAULT_MINMATCH 3

//Класс параметров для методов архивации
//Т.к. для каждого из методов параметры специфичны
//они в данном классе не конкретизируются по назначению

//Жестко фиксируются возможные ключи(тоесть параметры) в конструкторе
class MethodParams
{
private:
	QMap<QString, qint32> m_int_params;
	QMap<QString, float> m_float_params;
public:
	bool isExist(QString& paramName)
	{
		return 
			m_float_params.contains(paramName) || 
			m_int_params.contains(paramName);
	}
	qint32 intParam(const QString& name)
	{
		return (m_int_params.contains(name) ? m_int_params[name] : 0);
	}
	float floatParam(const QString& name)
	{
		return (m_float_params.contains(name) ? m_float_params[name] : 0.f);
	}
	void setParam(const QString& name, qint32 val)
	{
		//if(m_int_params.contains(name))
			m_int_params[name] = val;
	}
	void setParam(const QString& name, float val)
	{
		//if(m_float_params.contains(name))
			m_float_params[name] = val;
	}

	//Получить список целочисленных параметров
	QStringList paramsIntList()
	{	return m_int_params.uniqueKeys(); }

	QStringList paramsFloatList()
	{	return m_float_params.uniqueKeys(); }

	QStringList paramsNameList()
	{
		return paramsIntList() + paramsFloatList();
	}

};

  enum CompressionMode
  {
    Blocking = 0,
    Streaming,
  };

  enum CompressionFunction
  {
    Not = 0,
    OnlyCompression,
    OnlyDecompression,
    CompressionAndDecompression,

    OnlyC = OnlyCompression,
    OnlyD = OnlyDecompression,
    CandD = CompressionAndDecompression,
  };

//Базовый класс метода архивирования
//включает операции по сжатию потоков данных, и распаковке
//операции сжатия файлов, и распаковки
class ArchiveMethod
{
private:
  MethodType m_type;
  MethodParams m_params;
	CompressionFunction m_func;
protected:
	virtual void _InitMethod( CompressionFunction function ) = 0;
	virtual void _DestroyMethod() = 0;

  //Методы, необходимые для записи специфичных для каждого метода сжатия параметров
  virtual void _WriteParams(QDataStream& out) = 0;
  virtual int _ReadParams(QDataStream& in) = 0;
	//Сжимает одну порцию данных в памяти
	//Входные параметры: исходный буффер, размер блок, и буффер для записи результата
	//Основное требование: размер packBuffer'а должен быть больше sourceBuffer
	virtual quint32 _CompressedBlock(QByteArray& sourceBuffer, QByteArray& packBuffer) = 0;
	//Распаковывает весь блок целиком, возращает размер распакованного блока
	virtual quint32 _UncompressedBlock(QByteArray& packBuffer, QByteArray& dstBuffer) = 0;

	//Потоковое сжатие("на ходу" получает данные из входного потока, сжимает их, и отправляет в выходного поток)
	virtual quint32 _CompressedStream(QDataStream& input, QDataStream& output, quint32 size) = 0;
	virtual quint32 _UncompressedStream(QDataStream& input, QDataStream& output, quint32 size) = 0;
	
	//Установить стандартные параметры
	virtual void _SetDefaultParams() = 0;
	bool bInit;
public:
  ArchiveMethod(MethodType type);
	virtual ~ArchiveMethod();
	//Общий интерфейс
	quint32 compress(QByteArray& src, QByteArray& dst) 
		{	return _CompressedBlock(src,dst); }
	quint32 uncompress(QByteArray& src, QByteArray& dst) 
		{	return _UncompressedBlock(src,dst); }

	quint32 compressStream(QDataStream& input, QDataStream& output, quint32 size)	
		{	return _CompressedStream(input, output, size); }
	quint32 uncompressStream(QDataStream& input, QDataStream& output, quint32 size)	
		{	return _UncompressedStream(input, output, size); }

	//Доступные методя для управления параметрами метода архивации
	void setDefaultParams() { _SetDefaultParams(); }
	MethodParams& params() { return m_params; }

  MethodType type() const { return m_type; }

	bool isInit() { return bInit; }
	CompressionFunction func() { return m_func; }

	void reset( CompressionFunction function )
	{
		if(isInit())
			destroy();
		init(function);
	}

	void init( CompressionFunction function )
	{
		_InitMethod(function);
		m_func = function;
		bInit = true;
	}

	void destroy()
	{
		_DestroyMethod();
		m_func = Not;
		bInit = false;
	}
	

	//сжимает данные, разбивая их на блоки
	//последний блок имеет остаточный размер
	//dataSize - размер всех данных
	//blockSize - размер одного из блоков(последний может не соответстовать, если исходный dataSize не кратен blockSize)
	//возращает кол-во записанных блоков

	//Все сжатые блоки содержат свой размер спереди
	//Выделяет память под размер блока+под результат(размер блока*2)+память используемая алгоритмом
	//ВОзращает размер сжатых данных, отрицательное число - ошибка
	//в dataSize возращает реальный объем обработанных данных
	qint64 compressDataBlocks(QDataStream& input, QDataStream& output, qint64& dataSize, qint32 blockSize, qint32& write_blocks); //аналог старых compressIStream
	//blocks - кол-во блоков для распаковки
	//Распаковываем определенное кол-во блоков входного потока данных(размеры блоков уже содержатся во входном потоке)
	//в blocks возращает реальное кол-во считанных блоков
	//return объем распакованных данных
	qint64 uncompressDataBlocks(QDataStream& input, QDataStream& output, qint32& blocks);
	

	//Вспомогательная штука(врятли понадобиться?)
	//Сжимает файл и записывает в файл архива)
	//Возращает размер сжатых данных
	qint64 compressFile(const QString& qsSourceFile, const QString& qsArchFile, CompressionMode mode = Blocking);
	//Распаковывает файл
	//Возращает размер распакованных данных
	qint64 uncompressArchFile(const QString& qsArchFile, const QString& qsDestFile);

	//Упаковка и распаковка файлов из архива.
	qint64 compressFiles(const QStringList& files, const QString& qsArchFile, CompressionMode mode = Blocking);
	qint64 uncompressFiles(const QString& qsArchFile, const QString& destDir);
};

#endif
