#ifndef __LZX_H__
#define __LZX_H__

#include "types_lzx.h"

//����� WND_BITS 4 ������� ����� 8\
//������ �������� 16 ���(�.�. put_bits ������ 16 ��� �� ��� �� �������)

//�������� ����� ���...(��� ������) ����������� ����������� ���� � �����
#define LZHX_BLOCK_BITS 6 //������ ����� ��� ������� 2-��
#define LZHX_WND_BITS 15 //log2(w_size) - ������ ���� ��� ������� 2-��
#define LZHX_MIN_MATCH 3 //����������� ������ �������, ������� ����� �������� �� ���
#define LZHX_MAX_DEPTH 32 //������������ ������� ������ � ���-������� "-1" - ������������)


#define LZHX_HASH_BITS 10
#define LZHX_HASH_SIZE 0x10000
#define LZHX_HASH_MASK 0xFFFF//2^(8*3) - ��� ������� ������������ � ������(LZHX_HASH_SIZE-1)


struct lzx_hstate
{
	/*
	* ���������� ������� � �������� ������ ���������
	* � ��������� ������������ ������ � ��������� move_window, 
	* � � �������� ������ �� ����� ������(����� ������� ����� ���� ���������)
	*/
	uint8* input_buffer;//������� �����, ������� ��������������
	uint8* pi_buffer;//������� ������ ��� ������ �� ������� ������
	uint32 ib_avail_bytes;//�������� ���� ��� ���������

	uint8* output_buffer;//�������� �����
	uint8* po_buffer;//������� ������ ��� ������ � �������� ������
	uint32 ob_avail_bytes;//�������� ���� ��� ������

	//���������� ��� ������/������ �� ������ ���
	uint8 bits_out; //������� ��� ������������� ��� ������
	uint8 bits_in; //������� ���� ��������
	uint32 data_in; //������� ���������
	uint32 data_out; 
	/*
	* ��������� ����������� ��������� ��������� � ��������� ������
	*/
	//������ ����������� � ���������� �����
	uint8 block_bits;//log2(block_maxsize) - ������ ����� ���������� ��� ������� 2-��
	uint8 w_bits; //log2(w_size) - ������ ���� ��� ������� 2-��
	//�� ����������� ������
	uint8 hash_bits;//log2(hash_size) - ������ ���-�������� ��� ������� 2-��
	uint32 hash_size;
	uint32 hash_mask;
	uint32 hash_shift;

	uint32 min_match; //����������� ������ �������, ������� ����� �������� �� ���
	uint32 max_depth; //������������ ������� ������ � ���-������� "-1" - ������������)
	/*
	* ���������� ��������������� ��� ���������� ����������
	*/
	uint8* window;//���� ��������� ��� ��������� �������

	uint32 block_start;//������ ������ ��� ������ � �������
	uint32 block_maxsize;//������������ ����� ���������

	//����� count_queue ����� w_size �� ���� ��������� �����������
	uint32 count_queue; //���-�� ��������� � �������
	uint32 front_queue;//������ �������
	uint32 back_queue; //����� �������(������ ��������� �������)

	//������� ���������� �������� ��-�� ������� �����(����������)
	uint32 match_pos; //������� ����������
	uint32 match_len; //����� ����������

	uint32 w_size; //������ ����������� ����(������� � �����)
	uint32 w_mask; //wsize-1

	uint32* prev; //��������� ����������� ������ � ����������� ���-�������
	uint32* head; //���-�������
	uint32* depth; //������� ���� ���-�������(����������� ����� prev ������)
};

//������� ������������� ���������� ���������� LZHX
//������� ������ ������������ ����������� �����������
void initCompression_HASH(lzx_hstate* state, uint8 window_bit_size = LZHX_WND_BITS, uint8 block_bit_size = LZHX_BLOCK_BITS,
													uint8 hash_bit_size = LZHX_HASH_BITS, uint32 min_match = LZHX_MIN_MATCH,
													uint32 max_depth = LZHX_MAX_DEPTH);
//������� ������������� ���������� ���������� LZHX
//������� ������ ���������� ��� ����� �� 0 �� 7
//������� ����� ������� �������� ��������������� ��������� ����������
void initCompressionLevel_HASH(lzx_hstate* state, uint8 levelCompression);

//������� ���� ������� �����, ���������� ������������� ����� ��� �������� ������ � output_buffer
//��� �������� ���������� ��������� ������ ������ ������
int compressIBlock_HASH(lzx_hstate* state);
void destroyCompression_HASH(lzx_hstate* state);

void initDecompression_HASH(lzx_hstate* state, uint8 window_bit_size = LZHX_WND_BITS, uint8 block_bit_size = LZHX_BLOCK_BITS);
//��������� ���������� ���� ������(� initDecompress_HASH)
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