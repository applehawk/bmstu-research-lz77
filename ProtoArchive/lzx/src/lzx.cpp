/*
*
*  Библиотека сжатия LZX (08.11.09 - 0.2 Stable)
*  History: LZX (22.09.09 - 0.1)
*  Автор: Василенко Владислав
*
*/

#include "lzx.h"

#include "..\..\console\dtimestamp.h"

#include <memory>


// UPDATE_HASH макрос вычисления хэш-функции
#define UPDATE_HASH(h,c) { (h) = (((h)<<state->hash_shift)^(c)) & state->hash_mask; }
#define INDEX(x) ((x) & state->w_mask)

#define LZHX_CHARFLAG 0
#define LZHX_PHRASEFLAG 1

//Работа с хэш-таблицами и хэш-цепочками(в insert_string)
inline uint32 hash_func(lzx_hstate* state, uint32 pos)
{
	uint32 h = 0;
	uint8* pWnd = state->window;

	UPDATE_HASH(h,pWnd[INDEX(pos)]);
	UPDATE_HASH(h,pWnd[INDEX(pos+1)]);
	UPDATE_HASH(h,pWnd[INDEX(pos+2)]);
	
	return h;
}

//удаляет данные о наборе символов в back_queue
//если они затираются
inline void remove_hash_back(lzx_hstate* state)
{
	if(state->count_queue < state->w_size)
		++state->count_queue;
	else 
	{
		int h = hash_func(state, state->back_queue);//back_queue и front_queue равны
		if(state->depth[h]>0)
			--state->depth[h];
	}
}

//добавление в циклическую очередь нового символа
inline void insert_queue_hash(uint8 c, lzx_hstate* state) {
	//т.к. затирается некоторый символ на позиции back_queue (когда движется окно)'
	//необходимо данные о нем удалить из хэш-таблицы
	remove_hash_back(state);

	if(state->back_queue > state->w_size)
	{
		printf("Critical Error!, state->back_queue > state->w_size\n");
	}
	state->window[ state->back_queue ] = c; //добавляем на свободное место(обозначено как back_queue)
	state->back_queue = INDEX(state->back_queue+1);//новое свободное место
}

inline void insert_string(lzx_hstate* state, int spos)
{
	int h = hash_func(state,spos);

	//if тупость... но решает проблему зацикливания списка(помоему, это можно сделать лучше)
	//если хэш-индекс указывает на удаленый символ соответвующий добавленному
	state->prev[ spos & state->w_mask ] =state->head[h];
	//++state->depth[ h ]; //увеличиваем глубину хэш цепочки
	state->head[ h ] = spos; //записываем в хэш-таблицу новую позицию
	++state->depth[ h ];
}

//При записи все байты инвертируются
inline void put_bits(lzx_hstate* state, int data, int nbits)
{
	int bit;
	data <<= (32-nbits);
	for(; nbits>0; nbits--)
	{
		state->data_out <<= 1;
		bit = (data & 0x80000000) ? 1:0;
		state->data_out += bit;
		state->bits_out++;
		data <<= 1;
		if(state->bits_out == 8)
		{
			if(state->ob_avail_bytes == 0)
			{
				printf("Critical error!, ob_avail_bytes == 0");
			}
			state->bits_out = 0;
			*(state->po_buffer++)=(uint8)state->data_out;
			--state->ob_avail_bytes;
		}
	}
}

inline int get_bits(lzx_hstate* state, int nbits)
{
	int bit,data = 0;
	for(; nbits>0; nbits--)
	{
		if(state->bits_in==8)
		{
			if(state->ib_avail_bytes == 0)
			{
				printf("Critical error!, ib_avail_bytes == 0");
			}
			state->bits_in=0;
			state->data_in = *(state->pi_buffer++);
			--state->ib_avail_bytes;
		}
		bit = (state->data_in & 0x80) ? 1:0;
		data <<= 1;
		data += bit;
		state->bits_in++;
		state->data_in <<= 1;
	}
	return data;
}

void initCompressionLevel_HASH(lzx_hstate* state, uint8* ibuffer, uint32 ibsize, uint8* obuffer, uint32 obsize,
															 uint8 levelCompression)
{

}

void initCompression_HASH(lzx_hstate* state, uint8 window_bit_size, uint8 block_bit_size, uint8 hash_bit_size, uint32 min_match,
													uint32 max_depth)
{
	state->w_bits =  window_bit_size;
	state->w_size = (1<<state->w_bits);
	state->w_mask = state->w_size-1;

	state->bits_out = 0;
	state->data_out = 0;

	state->min_match = min_match;
	state->max_depth = max_depth;

	state->hash_bits = hash_bit_size;
	state->hash_size = (1<<state->hash_bits);
	state->hash_mask = state->hash_size-1;
	state->hash_shift =  ((state->hash_bits+state->min_match-1)/state->min_match);

	state->window = new uint8[state->w_size];
	state->prev = new uint32[state->w_size];
	state->head = new uint32[state->hash_size];
	state->depth = new uint32[state->hash_size];

	memset(state->depth,0,state->hash_size * sizeof(uint32));

	state->match_pos = 0;
	state->match_len = 0;

	state->block_bits = block_bit_size; //размер в битах (до 15 символов)

	//минус 1 делаем для того, чтобы длина совпадения могла быть равна размеру блока(а кол-во бит ее описывающих мог-ло представить эту длинну)
	//block_bits битов могут задать длинны от 0..1<<block_bits-1
	state->block_maxsize = (1<<state->block_bits)-1;

	state->front_queue = 0;
	state->back_queue = 0;
	state->count_queue = 0;
	state->block_start = 0;
}

void destroyCompression_HASH(lzx_hstate* state)
{
	delete [] state->window;
	delete [] state->prev;
	delete [] state->head;
	delete [] state->depth;
}

void initDecompression_HASH(lzx_hstate* state, uint8 window_bit_size, uint8 block_bit_size)
{
	state->w_bits = window_bit_size;//(1<<match_pos_bits)-1;
	state->w_size = (1<<state->w_bits);
	state->w_mask = (1<<state->w_bits)-1;

	state->block_start = 0;
	state->block_bits = block_bit_size;//(1<<match_len_bits)-1;
	state->block_maxsize = (1<<state->block_bits)-1;

	state->front_queue = 0;
	state->back_queue = 0;
	state->count_queue = 0;
	state->block_start = 0;

	state->window = new uint8[state->w_size];
	state->prev = NULL;//не используется
	state->head = NULL;//не используется
	state->depth = NULL;//не используется

	state->match_pos = 0;
	state->match_len = 0;
}

void destroyDecompression_HASH(lzx_hstate* state)
{
	delete [] state->window;
}

//i - найденная позиция в словаре
//k - позиция образца(буфера)
//возращает величину совпадаения образца и фразы в словаре
inline uint32 match_len(lzx_hstate* state, uint32 i, uint32 k)
{
	uint32 l = 0;

	int back_dx = 0;
	int i_dx = 0;
	back_dx = INDEX(state->back_queue-state->block_start+state->w_size);
	i_dx = INDEX(i - state->block_start);

	//если i лежит в буфере возращаем 0
	if ((0 <= i_dx) && (i_dx < back_dx))
		return 0;

	//Совпадение не больше длинны самого буфера
	while( back_dx-- &&
		state->window[INDEX(i+l)] == state->window[INDEX(k+l)] ) {
			++l;
	}
	return l;
}

void find_match(lzx_hstate* state)
{
	uint8* pWnd = state->window;
	uint32* pPrev = state->prev;
	//Получаем позицию образца как сумма начала блока и разности текущего размера очерди и максимальной длинны подстроки
	int buffpos = state->block_start;
	//Хэшируем первые 3 символа образца
	int h = hash_func(state, buffpos);
	uint32 x = state->head[h];
	int max_matchlen = 0;
	uint32 matchpos = 0;

	//символы образца
	uint8 c0 = pWnd[INDEX(buffpos)];
	uint8 c1 = pWnd[INDEX(buffpos+1)];
	uint8 c2 = pWnd[INDEX(buffpos+2)];

	//на одну глубину мы уже спустились(решает проблему проверки head[h] на незаданное значение
  uint32 d = state->depth[h];//для ограничения глубины,( можно убрать, но при большой глубине ошибка
	if(d > state->max_depth)
		d = state->max_depth;

	while(d > 0) {
		//Ищем в хэш цепочке искомый фрагмент строки
		//сравниваем с последнего символы
		if(
			(pWnd[INDEX(x+2)] == c2) &&
			(pWnd[INDEX(x+1)] == c1) &&
			(pWnd[INDEX(x+0)] == c0))
		{
			int l = match_len(state, x, buffpos);
			//l становится больше MAXBLOCKSIZE (напр 32)
			if(l >= max_matchlen) {
				max_matchlen = l;
				matchpos = INDEX(state->block_start - x);
			}
		}
		//Если не достигли конца хэш-цепочки, найден совпадающий фрагмент, фиксируем его
		--d;
    if(d>0) { //При некооректном изменении глубины хэш-цепочек возможен выход за x
      if(x >= state->w_size)
      {
        printf("x error in find_match\n");
        d = 0;
      } else
        x = pPrev[x];
    }
	}

	state->match_pos = matchpos;
	state->match_len = max_matchlen;
}

//двигаем окно(фактически дописываем в конец очереди новый символ из входного буфера)
inline void move_window(lzx_hstate* state)
{
	//добавляем в окно новый символ
	//при вызове учитываем, чтобы ib_avail_bytes > 0 было
	//циклически можно на n символов сдвигать
	insert_queue_hash(*(state->pi_buffer++), state);
	state->block_start = INDEX(state->block_start+1);
	//буфер заполнен(должен быть в init функции), двигаем его позицию
	--state->ib_avail_bytes;
	//новый хэш добавляем
	insert_string(state, (state->block_start-1)%state->w_size);
}

int decompressIBlock_HASH(lzx_hstate* state)
{
	DEBUG_MAKE_TIMESTAMP;
	uint8 c;
	uint32 match_pos;
	uint32 match_len;
	int complete_flag = 0;
	uint32 output_size = state->ob_avail_bytes;
	state->pi_buffer = state->input_buffer;
	state->po_buffer = state->output_buffer;
	state->bits_in = 8;
	state->data_in = 0;
	state->back_queue = 0;

	//Данные уже все считаны когда 0, но в буфере data_in еще имеются данные
	while(state->ib_avail_bytes >= 0)
	{
		//ASSERT(state->ob_avail_bytes != 0 && state->ob_avail_bytes <= output_size)
		if( get_bits(state,1) == LZHX_CHARFLAG)
		{
			c = get_bits(state,8);//считываем символ

			state->window[ state->back_queue ] = c;
			state->back_queue = INDEX(state->back_queue+1);
			//insert_queue(c, state);
			*(state->po_buffer++) = c;
			--state->ob_avail_bytes;
		} else {
			match_len = get_bits(state,state->block_bits);
			if(match_len > 20000)
			{
				printf("Warn, matchlen > 20000\n");
			}
			if(match_len != 0) 
			{
				match_pos = get_bits(state,state->w_bits);
				if(match_pos > state->w_size)
				{
					printf("Error!, Critical!\n");
				}
				int wpos = INDEX(state->back_queue - match_pos);
				while(match_len-- > 0)//& ob_avail_bytes > 0
				{
					//block_start двигается при добавлении нового символа, поэтому все верно
					c = state->window[ wpos ];
					*(state->po_buffer++) = c;
					--state->ob_avail_bytes;
					//insert_queue( state->window[ wpos ], state);

					state->window[ state->back_queue ] = c;
					state->back_queue = INDEX(state->back_queue+1);
					wpos = INDEX(wpos+1);
				}
			} else {
				DEBUG_MAKE_TIMESTAMP;
				//вернем размер распакованных данных
        //учтем ситуацию, когда ob_avail исчерпался вместе с ib_avail
        return output_size - state->ob_avail_bytes;
			}
		}
	}
	//неудачное завершение
	return -1;
}

int compressIBlock_HASH(lzx_hstate* state)
{
	DEBUG_MAKE_TIMESTAMP;
	state->bits_out = 0;
	state->data_out = 0;
	state->pi_buffer = state->input_buffer;
	state->po_buffer = state->output_buffer;

  //Не используем словарь с предыдущих файлов
  state->front_queue = 0;
  state->back_queue = 0;
  state->count_queue = 0;
  state->block_start = 0;

  memset(state->depth,0,state->hash_size * sizeof(uint32));

	uint32 output_size = state->ob_avail_bytes;
	//заполним блок экземпляра
	int bsize=state->block_maxsize;
	while(state->ib_avail_bytes > 0 && bsize--) {
		insert_queue_hash(*(state->pi_buffer++), state);
		--state->ib_avail_bytes;
	}

	while( state->ib_avail_bytes>0 )
	{
		find_match(state);
		if(state->match_len>=state->min_match)
		{
			put_bits(state, LZHX_PHRASEFLAG, 1);
			if(state->match_len == 0)
			{
				printf("Error, match_len = 0, then LZHX_PHRASEFLAG(not end block)\n");
			}
			put_bits(state, state->match_len, state->block_bits);
			put_bits(state, state->match_pos, state->w_bits);
		} else {
			put_bits(state, LZHX_CHARFLAG, 1);
			put_bits(state, state->window[INDEX(state->block_start)], 8);
			state->match_len = 1;//для одного сдвига окна
		}
		//сдвигаем окно на длинну совпадения(учитывая возможность опустошения буфера)
		while( state->ib_avail_bytes>0 && (state->match_len)>0) {
			move_window(state);
			--state->match_len;
		}
	}
	//обаботаем оставшиеся данные в буфере
	bsize = INDEX(state->back_queue-state->block_start+state->w_size);
	while(state->match_len > 0) {
		state->block_start = INDEX(state->block_start+1);
		--state->match_len;
		--bsize;
	}
	while(bsize)
	{
		find_match(state);

		if(state->match_len>=state->min_match)
		{
			put_bits(state, LZHX_PHRASEFLAG, 1);
			if(state->match_len == 0)
			{
				printf("Error, match_len = 0, then LZHX_PHRASEFLAG(not end block)\n");
			}
			put_bits(state, state->match_len, state->block_bits);
			put_bits(state, state->match_pos, state->w_bits);
		} else {
			put_bits(state, LZHX_CHARFLAG, 1);
			put_bits(state, state->window[INDEX(state->block_start)], 8);
			state->match_len = 1;//для одного сдвига окна
		}
		while(state->match_len > 0) {
			state->block_start = INDEX(state->block_start+1);
			--state->match_len;
			--bsize;
		}
	}

	//Завершаем блок(около 6 бит)
	put_bits(state, LZHX_PHRASEFLAG, 1);
	put_bits(state, 0, state->block_bits);

	if(state->bits_out == 8)
	{
		printf("aa");
	}
	if(state->bits_out)
		put_bits(state, 0, 8-state->bits_out);//дозаписываем оствшийся байт
	DEBUG_MAKE_TIMESTAMP;
	return (output_size - state->ob_avail_bytes);
}
