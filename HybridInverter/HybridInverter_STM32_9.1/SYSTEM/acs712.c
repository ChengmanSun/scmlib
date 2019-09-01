/*
********************************************************************************
*                              COPYRIGHT NOTICE
*                             Copyright (c) 2016
*                             All rights reserved
*
*  @FileName       : acs712.c
*  @Author         : scm 351721714@qq.com
*  @Create         : 2017/11/16 14:09:42
*  @Last Modified  : 2018/03/02 10:12:05
********************************************************************************
*/

#include "stm32f10x_dma.h"
#include "acs712.h"
#include "adc.h"
#include "debug.h"

uint16_t adc_dma_buff[6] = {0};

void ACS712_Init(void)
{
    adc_init(ADC1, 6, ENABLE, DISABLE);
    adc_regularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_55Cycles5);
    adc_regularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_55Cycles5);
    adc_regularChannelConfig(ADC1, ADC_Channel_8, 3, ADC_SampleTime_55Cycles5);
    adc_regularChannelConfig(ADC1, ADC_Channel_1, 4, ADC_SampleTime_55Cycles5);
    adc_regularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);
    adc_regularChannelConfig(ADC1, ADC_Channel_6, 6, ADC_SampleTime_55Cycles5);
    adc_regularDMAConfig(ADC1, adc_dma_buff, sizeof(adc_dma_buff)/sizeof(adc_dma_buff[0]));
}

void ACS712_Capture(BatteryInfo_t *info)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET) //wait DMA transfer finish.
        ;
    DMA_ClearFlag(DMA1_FLAG_TC1);

    //(3.3f * ((float)adc_dma_buff[VoltageIndex] / (0xfff+1))) * 11 * 1000      -----���۹�ʽ
    //��ص�ѹ
    info->voltage = adc_dma_buff[VoltageIndex] * (3.28f * 11 * 1000 / (0xfff+1));
    //Vout��������ѹ�� = 2.5��0A��׼�� + 0.1����ȷ��Ϊ100mV/A��*Ip(������       -----���۹�ʽ
    //Vout��������ѹ�� = 1.595��0A��׼�� + 0.1����ȷ��Ϊ100mV/A��*Ip(������      -----ʵ�ʹ�ʽ
    //������
    info->charge_current = (3280.0f*adc_dma_buff[ChargeIndex]/(4096*0.1f) - 1595/0.1f)*1.584f;
    //�ŵ����
    info->discharge_current = (3280.0f*adc_dma_buff[DischargeIndex]/(4096*0.1f) - 1595/0.1f)*1.584f;

    //ǰ���Ƹ˷�����ѹ
    info->front_pushrod_voltage = adc_dma_buff[FrontPushrodIndex] * (3.28f  / (0xfff+1));
    //����Ƹ˷�����ѹ
    info->rear_pushrod_voltage = adc_dma_buff[RearPushrodIndex] * (3.28f / (0xfff+1));
    //�������Ƹ˷�����ѹ
    info->protect_pushrod_voltage = adc_dma_buff[ProtectPushrodIndex] * (3.28f / (0xfff+1));
}
