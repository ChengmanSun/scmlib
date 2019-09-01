/*
********************************************************************************
*                              COPYRIGHT NOTICE
*                             Copyright (c) 2016
*                             All rights reserved
*
*  @fileName       : update.c
*  @author         : scm 351721714@qq.com
*  @create         : 2019/01/31 23:27:19
********************************************************************************
*/

#include <stdio.h>
#include <string.h>
#include "update.h"
#include "ff.h"
#include "md5.h"
#include "usart.h"
#include "byteorder.h"

//ͨ�Ŵ�����
#define ERR_STORAGE     (1<<0)  //�洢������
#define ERR_HARD_FAULT  (1<<1)  //�豸Ӳ������
#define ERR_TASK        (1<<2)  //�豸�������
#define ERR_SEQUENCE    (1<<3)  //�������
#define ERR_MD5         (1<<4)  //�ļ���MD5У�����

enum {
    UPDATE_CMD_START = 0,       //��ʼ��������
    UPDATE_CMD_END = 1,         //������������
    UPDATE_CMD_RESET = 2        //��λ����
};

enum {
    DATA_TYPE_FILENAME = 0,     //�������ͣ��ļ���
    DATA_TYPE_CONTENTS = 1,     //�������ͣ��ļ�����
    DATA_TYPE_MD5 = 2           //�������ͣ�MD5
};

FIL g_hFile = {0};                     //�ļ����
const char g_tempName[] = "temp.dat";  //��ʱ�ļ���
char g_finalName[32] = {0};            //�����ļ���
char *g_pFinalName = NULL;             //���ΪNULL��ʹ��Ĭ���ļ������棬������g_finalName�ļ�������

int g_fileSize = 0;                    //�ļ��Ĵ�С
int g_dataOffset = 0;                  //�ڴ����ļ����ݵ�ƫ��λ��
int g_transmitTimeout = 0;             //�ļ����ݴ��䳬ʱ����
uint8_t g_MD5[16] = {0};               //������λ��������ļ���MD5У��ֵ

static inline void update_send(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, data, len, 10);
}

uint8_t update_checksum(uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;
    for(int i = 0; i < len; ++i)
    {
        sum ^= data[i];
    }
    return sum;
}

//ɾ�������ļ����ݵ��ļ����������δ���
static void update_clear_data(void)
{
    f_close(&g_hFile);
    f_unlink(g_tempName);
    
    g_pFinalName = NULL;
    g_fileSize = 0;
    g_dataOffset = 0;
    g_transmitTimeout = 0;
}

static int update_md5_check(FIL *f)
{
    mbedtls_md5_context ctx;
    uint8_t md5[16], buff[32];
    uint32_t br;
    
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts(&ctx);
    f_rewind(f);
    while(f_read(f, buff, sizeof(buff), &br) == FR_OK && br > 0)
    {
        mbedtls_md5_update(&ctx, buff, br);
    }
    mbedtls_md5_finish(&ctx, md5);
    mbedtls_md5_free(&ctx);
    return memcmp(g_MD5, md5, sizeof(md5)) != 0;
}

//�ļ���������֡����
void update_Cmd_process(uint8_t *recData, uint16_t len)
{
    Update_Cmd_Req *req;
    Update_Cmd_Ack ack;
    uint8_t ackflag = 0;

    g_transmitTimeout = 0;                              //���䳬ʱ��������
    
    req = (Update_Cmd_Req *)recData;
    memset(&ack, 0, sizeof(ack));
    
    if(req->command == UPDATE_CMD_START)                //��ʼ�����ļ�
    {
//        ����豸״̬
//        if()
//        {
//            ackflag |= ERR_HARD_FAULT;
//            ackflag |= ERR_TASK;
//        }
        /*else*/ if(f_open(&g_hFile, g_tempName, FA_CREATE_ALWAYS|FA_READ|FA_WRITE) == FR_OK) //������ʱ�ļ�
        {
            g_fileSize = ntohl(req->firmwareSize);      //�ļ��Ĵ�С
            g_dataOffset = 0;                           //��0ƫ�ƿ�ʼ�����ļ�����
        }
        else                                            //��������ļ�ʧ��
        {
            update_clear_data();                        //�������
            ackflag |= ERR_STORAGE;                     //��֪��λ�����洢����
        }
    }
    else if(req->command == UPDATE_CMD_END)             //����ļ�����
    {
        if(update_md5_check(&g_hFile) != 0)
        {
            update_clear_data();                        //�������
            ackflag |= ERR_MD5;                         //��֪��λ����MD5У�����        
        }
        
        if(ackflag == 0 && f_close(&g_hFile) != FR_OK)  //�����ļ�
        {
            update_clear_data();                        //�������
            ackflag |= ERR_STORAGE;                     //��֪��λ�����洢����
        }

        if(ackflag == 0 && g_pFinalName != NULL)        //����������ļ���
        {
            f_unlink(g_pFinalName);                     //ɾ���Ѿ����ڵ��ļ��������޷���������ͬ���ļ�
            if(f_rename(g_tempName, g_pFinalName) != FR_OK)
            {
                update_clear_data();                    //�������
                ackflag |= ERR_STORAGE;                 //��֪��λ�����洢����
            }
        }
        
        g_pFinalName = NULL;
        g_fileSize = 0;
        g_dataOffset = 0;
        g_transmitTimeout = 0;
    }
    else if(req->command == UPDATE_CMD_RESET)           //��λ��Ƭ����ִ�й̼�����
    {
        //��λ����Ȼ���bootloader������ִ����д�̼�
//        __set_FAULTMASK(1);                             //�ر������ж�
//        NVIC_SystemReset();                             //�����λ
        
        //����bootloader������ִ����д�̼���bootloader������0x8000000��ַ����
//        typedef void (*BOOTLOADER)(void);
//        BOOTLOADER bootloader = *(BOOTLOADER *)(0x8000000+4);
        void(*bootloader)(void) = *((void(**)(void))(0x8000000+4));
        SCB->VTOR = 0x8000000;
        __ASM {MSR MSP, *(volatile uint32_t*)(0x8000000)}
        bootloader();
    }
    else                                                //���ݴ���ʧ��������ǰ���������
    {
        update_clear_data();
    }
    
    ack.head = UPDATE_FRAME_HEAD;
    ack.type = req->type + 1;
    ack.length = sizeof(ack) - 5;
    memcpy(&ack.ID, &req->ID, sizeof(ack.ID));
    ack.acknowledge = ackflag;
    ack.reserve = 0;
    ack.checkSum = update_checksum(((uint8_t *)&ack), sizeof(ack)-2);
    ack.tail = UPDATE_FRAME_TAIL;
    update_send((uint8_t *)&ack, sizeof(ack));
}

//�ļ����ݴ���֡����
void update_Transmit_process(uint8_t *recData, uint16_t len)
{
    Update_Transmit_Req *req;
    Update_Transmit_Ack ack;
    uint32_t offset;
    uint8_t type, count, ackflag = 0;
    UINT bw;
    
    g_transmitTimeout = 0;                      //���䳬ʱ��������
    
    req = (Update_Transmit_Req *)recData;
    type = req->dataType;                       //����֡������������ͣ�0���ļ��� 1���ļ����� 2��MD5У��ֵ��
    count = req->sendCnt;                       //���δ�����ļ������ֽ���
    offset = ntohl(req->firmwareIndex);         //���δ���ĵ�һ���������ļ��е�ƫ��ֵ
    
    if(type == DATA_TYPE_FILENAME)              //�����ļ���
    {
        memcpy(g_finalName, &req->data, count);
        g_pFinalName = g_finalName;
    }
    else if(type == DATA_TYPE_CONTENTS)         //�����ļ�����
    {
        if((g_dataOffset != offset)             //���ƫ��ֵ�Ƿ�Ե���
            || (offset + count > g_fileSize)    //�����յ��ļ������Ƿ��Ѿ�����ԭ��Լ������ֵ
            || (count + 24 != len))             //��鴫����ļ����ݺ�֡�����Ƿ�Ե��ϣ���������Խ��
        {
            update_clear_data();
            ackflag |= ERR_SEQUENCE;            //�������
            goto out;
        }
    
        if(f_write(&g_hFile, &req->data, count, &bw) != FR_OK)
        {
            update_clear_data();
            ackflag |= ERR_STORAGE;             //�洢����
        }
    
        //���δ�������ݴ洢���쳣���ȴ����պ�������ݡ�
        if(ackflag == 0)
        {
            g_dataOffset += count;
        }
    }
    else if(type == DATA_TYPE_MD5)              //�����ļ�MD5У��ֵ
    {
        if(count != sizeof(g_MD5))
            ackflag |= ERR_MD5;
        else
            memcpy(g_MD5, &req->data, sizeof(g_MD5));
    }

out:    
    ack.head = UPDATE_FRAME_HEAD;
    ack.type = req->type + 1;
    ack.length = sizeof(ack) - 5;
    memcpy(&ack.ID, &req->ID, sizeof(ack.ID));
    ack.acknowledge = ackflag;
    ack.nextOffset = htonl(g_dataOffset);
    ack.reserve = 0;
    ack.checkSum = update_checksum((uint8_t *)&ack, sizeof(ack)-2);
    ack.tail = UPDATE_FRAME_TAIL;
    update_send((uint8_t *)&ack, sizeof(ack));
}

//���ݴ����жϼ�⡣��10ms����20ms�����е������������
void update_timeout_check(void)
{
    if(g_fileSize != 0 && g_transmitTimeout++ > 500)
    {
        update_clear_data();
    }
}
