#ifndef EEPROM_H
#define EEPROM_H

#include "stdint.h"
#include "iic.h"

#define DEVICE_ADDRESS      0x50
#define EEPROM_PAGE_COUNT   32
#define EEPROM_PAGE_SIZE    8

void eeprom_init(void);
//��Ȼһ���ֽڵĵ�ַ�ܱ�ʾ256���ֽڣ�����size����Ҫ��uint16_t����ʾ���������һ��Ҫ��ȡ256���ֽ�ʱ
//�����������һ��uint8_t��˵256ʮ�����Ʊ�ʾΪ0xff�����Ƕ������������߶���ֽ����ϵı�����˵��256
//ʮ�����Ʊ�ʾΪ0x00000100����ô�����������������ֽ����ϵı�������size������ֵ256ʱ�ᵼ��size��ֵΪ0
void eeprom_read(uint16_t addr, uint8_t *data, uint16_t size);
void eeprom_write(uint16_t addr, uint8_t *data, uint16_t size);

#endif

