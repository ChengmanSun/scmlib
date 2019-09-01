/*
********************************************************************************
*                              COPYRIGHT NOTICE
*                             Copyright (c) 2016
*                             All rights reserved
*
*  @FileName       : batteryManage.h
*  @Author         : scm 351721714@qq.com
*  @Create         : 2017/11/20 17:11:42
*  @Last Modified  : 2018/07/18 18:57:24
********************************************************************************
*/

#ifndef BATTERYMANAGE_H
#define BATTERYMANAGE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    //��ص�ѹ
    float voltage;
    //��س�����
    float charge_current;
    //��طŵ����
    float discharge_current;
    //ǰ���Ƹ˷�����ѹ
    float front_pushrod_voltage;
    //����Ƹ˷�����ѹ
    float rear_pushrod_voltage;
    //�������Ƹ˷�����ѹ
    float protect_pushrod_voltage;
    //Ϊ�˼��ټ����������ʱ�ľ�����ʧ��energy���붨��Ϊ˫���ȸ�������
    //���ʣ�����
    double energy;
    //���ʣ������ٷֱ�
    uint8_t energy_percent;
    //����¶�
    float temperature;
    //����ڵ�ǰ�¶��µ�����
	float capacity;
    //���ͷŵ�ͨ���ĵ����־
    bool zeroset_flag;
    //���ͷŵ�ͨ�ŵĵ���ֵ
    float zeroset_value;
} BatteryInfo_t;

void battery_init(void);
void battery_energe_init(void);
void battery_zerosetting(void);
void battery_manage(void);
void battery_getInfo(BatteryInfo_t *info);
void battery_setTemperature(uint16_t temp);

#endif /* end of include guard: BATTERYMANAGE_H */
