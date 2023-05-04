#ifndef __LZX_H__
#define __LZX_H__

#include "types_lzx.h"

//Когда WND_BITS 4 выводит верно 8\
//Ставим максимум 16 бит(т.к. put_bits больше 16 бит за раз не выводит)

//смотреть вывод бит...(или чтение) некооректно обрабтывает ввод и вывод
#define LZHX_BLOCK_BITS 6 //Размер блока как степень 2-ки
#define LZHX_WND_BITS 15 //log2(w_size) - размер окна как степень 2-ки
#define LZHX_MIN_MATCH 3 //минимальный размер образца, который можно заменить на код
#define LZHX_MAX_DEPTH 32 //максимальная глубина поиска в хэш-цепочке "-1" - неограничено)


#define LZHX_HASH_BITS 10
#define LZHX_HASH_SIZE 0x10000
#define LZHX_HASH_MASK 0xFFFF//2^(8*3) - три символа записываются в индекс(LZHX_HASH_SIZE-1)


struct lzx_hstate
{
	/*
	* Переменные входных и выходных данных алгоритма
	* В алгоритме используется только в процедуре move_window, 
	* и в условиях выхода из цикла сжатия(когда входной буфер весь обработан)
	*/
	uint8* input_buffer;//входной буфер, который обрабатывается
	uint8* pi_buffer;//текущий символ для чтения во входном буфере
	uint32 ib_avail_bytes;//доступно байт для обработки

	uint8* output_buffer;//выходной буфер
	uint8* po_buffer;//текущий символ для записи в выходном буфере
	uint32 ob_avail_bytes;//доступно байт для записи

	//Переменные для записи/чтения на уровне бит
	uint8 bits_out; //сколько бит подготовленно для записи
	uint8 bits_in; //сколько было считанно
	uint32 data_in; //битовые хранилища
	uint32 data_out; 
	/*
	* Параметры позволяющие управлять скоростью и качеством сжатия
	*/
	//Данные привязанные к сжимаемому блоку
	uint8 block_bits;//log2(block_maxsize) - размер блока экземпляра как степень 2-ки
	uint8 w_bits; //log2(w_size) - размер окна как степень 2-ки
	//Не привязанные данные
	uint8 hash_bits;//log2(hash_size) - Размер хэш-табилицы как степень 2-ки
	uint32 hash_size;
	uint32 hash_mask;
	uint32 hash_shift;

	uint32 min_match; //минимальный размер образца, который можно заменить на код
	uint32 max_depth; //максимальная глубина поиска в хэш-цепочке "-1" - неограничено)
	/*
	* Переменные непосредственно для реализации компрессии
	*/
	uint8* window;//окно реализуем как кольцевую очередь

	uint32 block_start;//начало буфера для поиска в словаре
	uint32 block_maxsize;//максимальная длина подстроки

	//Когда count_queue равен w_size то окно считается заполненным
	uint32 count_queue; //кол-во элементов в очереди
	uint32 front_queue;//начало очереди
	uint32 back_queue; //конец очереди(первый свободный элемент)

	//позиция совпадения задается от-но позиции блока(экземпляра)
	uint32 match_pos; //позиция совпадения
	uint32 match_len; //длина совпадения

	uint32 w_size; //размер скользящего окна(словарь и буфер)
	uint32 w_mask; //wsize-1

	uint32* prev; //реализует односвязные списки с одинаковыми хэш-суммами
	uint32* head; //хэш-таблица
	uint32* depth; //глубина всех хэш-цепочек(реализуемых через prev список)
};

//Функция инициализации компрессии алгоритмом LZHX
//Уровень сжатия определяется переданными параметрами
void initCompression_HASH(lzx_hstate* state, uint8 window_bit_size = LZHX_WND_BITS, uint8 block_bit_size = LZHX_BLOCK_BITS,
													uint8 hash_bit_size = LZHX_HASH_BITS, uint32 min_match = LZHX_MIN_MATCH,
													uint32 max_depth = LZHX_MAX_DEPTH);
//Функция инициализации компрессии алгоритмом LZHX
//Уровень сжатия передается как число от 0 до 7
//функция самма выберет наиболее соответствующие параметры компрессии
void initCompressionLevel_HASH(lzx_hstate* state, uint8 levelCompression);

//Сжимает весь входной буфер, необходимо предусмотреть место для выходных данных в output_buffer
//при успешном выполнении возращает размер сжатых данных
int compressIBlock_HASH(lzx_hstate* state);
void destroyCompression_HASH(lzx_hstate* state);

void initDecompression_HASH(lzx_hstate* state, uint8 window_bit_size = LZHX_WND_BITS, uint8 block_bit_size = LZHX_BLOCK_BITS);
//Разжимает переданный блок данных(в initDecompress_HASH)
int decompressIBlock_HASH(lzx_hstate* state);
void destroyDecompression_HASH(lzx_hstate* state);

inline void setIOBuffers_HASH(lzx_hstate* state, uint8* ibuffer, uint32 ibsize, uint8* obuffer, uint32 obsize)
{
	state->input_buffer = ibuffer;
	state->output_buffer = obuffer;

	state->pi_buffer = ibuffer;
	state->po_buffer = obuffer;

	state->ib_avail_bytes = ibsize;
	state->ob_avail_bytes = obsize;
}

#endif