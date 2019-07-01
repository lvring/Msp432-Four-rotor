#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.7
*** History: 1.0 �������޸���tivaware
             2.0 ΪRTE������������
*****************************************************************************/
#if RTE_USE_RINGBUF == 1
#define RB_INDH(rb)                ((rb)->head & ((rb)->count - 1))
#define RB_INDT(rb)                ((rb)->tail & ((rb)->count - 1))
/*************************************************
*** Args:   
			*RingBuff �������ζ���
			*buffer ʵ��ռ�õ��ڴ�ռ�
			itemSize �����洢��Ԫ��С
			count ��������
*** Function: ��ʼ��һ�����ζ���
*************************************************/
int RTE_RingQuene_Init(RTE_RingQuene_t *RingBuff, void *buffer, int itemSize, int count)
{
	RingBuff->data = buffer;
	RingBuff->count = count;
	RingBuff->itemSz = itemSize;
	RingBuff->head = RingBuff->tail = 0;
	return 1;
}
/*************************************************
*** Args:   
			*RingBuff �������ζ���
      *data ����������
*** Function: ���ζ���β�����һ������
*************************************************/
int RTE_RingQuene_Insert(RTE_RingQuene_t *RingBuff, const void *data)
{
	uint8_t *ptr = RingBuff->data;
	/* We cannot insert when queue is full */
	if (RTE_RingQuene_IsFull(RingBuff))
		return 0;
	ptr += RB_INDH(RingBuff) * RingBuff->itemSz;
	memcpy(ptr, data, RingBuff->itemSz);
	RingBuff->head++;
	return 1;
}
/*************************************************
*** Args:   
			*RingBuff �������ζ���
      *data �����������׵�ַ
		   num ���������ݸ���
*** Function: ���ζ���β��������ɸ�����
*************************************************/
int RTE_RingQuene_InsertMult(RTE_RingQuene_t *RingBuff, const void *data, int num)
{
	uint8_t *ptr = RingBuff->data;
	int cnt1, cnt2;
	/* We cannot insert when queue is full */
	if (RTE_RingQuene_IsFull(RingBuff))
		return 0;
	/* Calculate the segment lengths */
	cnt1 = cnt2 = RTE_RingQuene_GetFree(RingBuff);
	if (RB_INDH(RingBuff) + cnt1 >= RingBuff->count)
		cnt1 = RingBuff->count - RB_INDH(RingBuff);
	cnt2 -= cnt1;
	cnt1 = RTE_MATH_MIN(cnt1, num);
	num -= cnt1;
	cnt2 = RTE_MATH_MIN(cnt2, num);
	num -= cnt2;
	/* Write segment 1 */
	ptr += RB_INDH(RingBuff) * RingBuff->itemSz;
	memcpy(ptr, data, cnt1 * RingBuff->itemSz);
	RingBuff->head += cnt1;
	/* Write segment 2 */
	ptr = (uint8_t *) RingBuff->data + RB_INDH(RingBuff) * RingBuff->itemSz;
	data = (const uint8_t *) data + cnt1 * RingBuff->itemSz;
	memcpy(ptr, data, cnt2 * RingBuff->itemSz);
	RingBuff->head += cnt2;
	return cnt1 + cnt2;
}
/*************************************************
*** Args:   
			*RingBuff �������ζ���
      *data ������ݵĵ�ַ
*** Function: �ӻ��ζ���ͷ��ȡ��һ������
*************************************************/
int RTE_RingQuene_Pop(RTE_RingQuene_t *RingBuff, void *data)
{
	uint8_t *ptr = RingBuff->data;
	/* We cannot pop when queue is empty */
	if (RTE_RingQuene_IsEmpty(RingBuff))
		return 0;
	ptr += RB_INDT(RingBuff) * RingBuff->itemSz;
	memcpy(data, ptr, RingBuff->itemSz);
	RingBuff->tail++;
	return 1;
}
/*************************************************
*** Args:   
			*RingBuff �������ζ���
      *data ������ݵĵ�ַ
			num ȥ�����ݸ���
*** Function: �ӻ��ζ���ͷ��ȡ��һ������
*************************************************/
int RTE_RingQuene_PopMult(RTE_RingQuene_t *RingBuff, void *data, int num)
{
	uint8_t *ptr = RingBuff->data;
	int cnt1, cnt2;
	/* We cannot insert when queue is empty */
	if (RTE_RingQuene_IsEmpty(RingBuff))
		return 0;
	/* Calculate the segment lengths */
	cnt1 = cnt2 = RTE_RingQuene_GetCount(RingBuff);
	if (RB_INDT(RingBuff) + cnt1 >= RingBuff->count)
		cnt1 = RingBuff->count - RB_INDT(RingBuff);
	cnt2 -= cnt1;
	cnt1 = RTE_MATH_MIN(cnt1, num);
	num -= cnt1;
	cnt2 = RTE_MATH_MIN(cnt2, num);
	num -= cnt2;
	/* Write segment 1 */
	ptr += RB_INDT(RingBuff) * RingBuff->itemSz;
	memcpy(data, ptr, cnt1 * RingBuff->itemSz);
	RingBuff->tail += cnt1;
	/* Write segment 2 */
	ptr = (uint8_t *) RingBuff->data + RB_INDT(RingBuff) * RingBuff->itemSz;
	data = (uint8_t *) data + cnt1 * RingBuff->itemSz;
	memcpy(data, ptr, cnt2 * RingBuff->itemSz);
	RingBuff->tail += cnt2;
	return cnt1 + cnt2;
}
/*************************************************
*** Args:   
			*MessageQuene ��������Ϣ����
      Size ��Ϣ���д�С
*** Function: ��ʼ��һ����Ϣ����
*************************************************/
void RTE_MessageQuene_Init(RTE_MessageQuene_t *MessageQuene, uint16_t Size)
{
	MessageQuene->QueneBuffer = (uint8_t *)RTE_BGetz(MEM_RTE,Size);
	RTE_AssertParam(MessageQuene->QueneBuffer);
	RTE_AssertParam(RTE_RingQuene_Init(&MessageQuene->RingBuff,MessageQuene->QueneBuffer,sizeof(uint8_t),Size));
}
/*************************************************
*** Args:   
			*MessageQuene ��������Ϣ����
      *Data �����������׵�ַ
			DataSize ���������ݴ�С
*** Function: ����Ϣ�����м���һ������
*************************************************/
RTE_MessageQuene_Err_e RTE_MessageQuene_In(RTE_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t DataSize)
{
	uint8_t SizeHigh = (DataSize>>8)&0xFF;
	if(RTE_RingQuene_Insert(&MessageQuene->RingBuff,&SizeHigh))
	{
		uint8_t SizeLow = DataSize&0xFF;
		if(RTE_RingQuene_Insert(&MessageQuene->RingBuff,&SizeLow))
		{
			if(RTE_RingQuene_InsertMult(&MessageQuene->RingBuff,Data,DataSize) == DataSize)
				return MSG_NO_ERR;
		}
	}
	return MSG_EN_FULL;
}
/*************************************************
*** Args:   
			*MessageQuene ��������Ϣ����
      *Data ������ݿռ��׵�ַ
			*DataSize ������ݴ�С������ַ
*** Function: ����Ϣ������ȡ��һ������
*************************************************/
RTE_MessageQuene_Err_e RTE_MessageQuene_Out(RTE_MessageQuene_t *MessageQuene, uint8_t *Data,uint16_t *DataSize)
{
	uint8_t SizeHigh = 0x00;
	uint8_t SizeLow = 0x00;
	if(RTE_RingQuene_Pop(&MessageQuene->RingBuff,&SizeHigh))
	{
		if(RTE_RingQuene_Pop(&MessageQuene->RingBuff,&SizeLow))
		{
			*DataSize = (uint16_t)(SizeHigh<<8)|SizeLow;
			if(RTE_RingQuene_PopMult(&MessageQuene->RingBuff,Data,*DataSize) == *DataSize)
			{
				uint16_t AnotherFrameSize=0;
				if(Data[*DataSize-1] == 0xAA&& Data[*DataSize-2] == 0x55)
				{
					RTE_MessageQuene_Out(MessageQuene,Data+*DataSize-2,&AnotherFrameSize);
					*DataSize = AnotherFrameSize + *DataSize-2;
				}
				return MSG_NO_ERR;
			}
		}
	}
	return MSG_DE_EMPTY;
}
#endif
