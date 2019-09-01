#include "eeprom.h"

_IIC_TypeDef iic;

void eeprom_init(void)
{
    iic_init(&iic, GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7);
}
    
void eeprom_read(uint16_t addr, uint8_t *data, uint16_t size)
{
#if EEPROM_PAGE_SIZE * EEPROM_PAGE_COUNT <= 256
    //EEPROM����С�ڵ���256���ֽ�ʱ�Ķ���ʽ
    iic_read_mem(&iic, DEVICE_ADDRESS, addr, data, size);
#else
    //EEPROM��������256���ֽ�ʱ�Ķ���ʽ
    iic_read_mem_addr16bit(&iic, DEVICE_ADDRESS, addr, data, size);
#endif
}

//���addr��ҳ�е�ƫ����0����length���ֵΪEEPROM_PAGE_SIZE
//���addr��ҳ�е�ƫ����1����length���ֵΪEEPROM_PAGE_SIZE-1
//��������
static void eeprom_pageWrite(uint16_t addr, uint8_t *data, uint16_t size)
{
#if EEPROM_PAGE_SIZE * EEPROM_PAGE_COUNT <= 256
    //EEPROM����С�ڵ���256���ֽ�ʱ��д��ʽ
    iic_write_mem(&iic, DEVICE_ADDRESS, addr, data, size);
#else
    //EEPROM��������256���ֽ�ʱ��д��ʽ
    iic_write_mem_addr16bit(&iic, DEVICE_ADDRESS, addr, data, size);
#endif
}

void eeprom_write(uint16_t addr, uint8_t *data, uint16_t size)
{
    int byte2write = 0, indexInPage = 0;
    
    while(size > 0)
    {
        indexInPage = addr % EEPROM_PAGE_SIZE;
        byte2write = EEPROM_PAGE_SIZE - indexInPage;
        if(byte2write > size) byte2write = size;
        
        eeprom_pageWrite(addr, data, byte2write);
        iic_delay_us(10000);    //ÿдһҳ��ʱ10ms���ȴ�����ɹ�������д��һҳ
        
        addr += byte2write;
        data += byte2write;
        size -= byte2write;
    }
}
