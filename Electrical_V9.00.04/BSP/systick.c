#include <stdio.h>
#include "systick.h"

static uint32_t face_ms, face_us, max_ms, max_us;
static SYSTICK_CALLBACK SysTickCallback;

/*
*******************************************************************************
*                       SysTick_Handler
* Description : System Ticker ���жϺ���
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)     : none
*******************************************************************************
*/
#if 1
void SysTick_Handler(void)
{
    if(SysTickCallback.pfun != NULL)
        SysTickCallback.pfun(SysTickCallback.arg);
}
#endif

/*
********************************************************************************
*                               init
* Description : ��ʼ������face��max
* 
* Arguments   : face_ms����ǰSysTickʱ��Դ����ʱ1ms��Ҫ�ļ���ֵ
*               face_us����ǰSysTickʱ��Դ����ʱ1us��Ҫ�ļ���ֵ
*               max_ms: ��ǰSysTickʱ��Դ��һ�μ�������ʱ����������
*               max_us: ��ǰSysTickʱ��Դ��һ�μ�������ʱ�����΢����
*               
* Return      : none
* 
* Note(s)     : none
********************************************************************************
*/
void systick_init(void)
{
    RCC_ClocksTypeDef rcc_clocks;

    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);    //���óɲ���Ƶ����SysTick��ʱ����ʱ��Ƶ�ʵ���HCLK

    //face_ms = SystemCoreClock / 1000;
    //face_us = SystemCoreClock / 1000000;
    RCC_GetClocksFreq(&rcc_clocks);
    face_ms = rcc_clocks.HCLK_Frequency / 1000;         //systick��ʱ1��������Ҫ�ļ���ֵ
    face_us = rcc_clocks.HCLK_Frequency / 1000000;      //systick��ʱ1΢������Ҫ�ļ���ֵ
    max_ms = 0xffffff / face_ms;                        //һ�μ����������ʱ99����
    max_us = 0xffffff / face_us;                        //һ�μ����������ʱ792΢��
}

/*
********************************************************************************
*                               setCLKSource 
*
* Description : ����SysTickʱ��ԴΪHCLK����HCLK��8��Ƶ��
*
* Arguments   : SysTick_CLKSourceȡֵΪ��SysTick_CLKSource_HCLK(ϵͳʱ��) ��
*               SysTick_CLKSource_HCLK_Div8(ϵͳʱ��8��Ƶ)��
*
* Returns     : none
*
* Note(s)     : Ĭ�������SysTickʱ��ԴΪHCLK��8��Ƶ��
********************************************************************************
*/
void systick_setCLKSource(uint32_t SysTick_CLKSource)
{
    /* param_assert(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource)); */

    RCC_ClocksTypeDef rcc_clocks;

    SysTick_CLKSourceConfig(SysTick_CLKSource); 

    RCC_GetClocksFreq(&rcc_clocks);
    if(SysTick_CLKSource == SysTick_CLKSource_HCLK)
        face_us = rcc_clocks.HCLK_Frequency / 1000000;
    else
        face_us = rcc_clocks.HCLK_Frequency / 8 / 1000000;

    face_ms = face_us * 1000;
    max_us = 0xffffff / face_us;
    max_ms = 0xffffff / face_ms;
}

/*
********************************************************************************
*                               delay_ms_private 
*
* Description : ������ʱ������
*
* Arguments   : nms ��ʱʱ�䣬��λ���롣
*
* Returns     : none
*
* Note(s)     : SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:nms*face_ms <= 0xffffff
*               ��48M�����²���ƵʱmFace_ms==48000������nms<=349
*               ��48M������8��ƵʱmFace_ms==6000������nms<=2796
*               ��72M�����²���ƵʱmFace_ms==72000������nms<=233
*               ��72M������8��ƵʱmFace_ms==9000������nms<=1864
*               ��168M�����²���Ƶʱface_ms==168000������nms<=99
*               ��168M������8��Ƶʱface_ms==21000������nms<=792
********************************************************************************
*/
static void systick_delay_ms_private(uint16_t nms)
{
    //param_assert(nms <= max_ms);

    uint32_t temp;
    SysTick->LOAD = nms * face_ms - 1;          //����ʱ��
    SysTick->VAL = 0x00;                        //��ռ�����
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   //��ʼ����
    do{
        temp = SysTick->CTRL;
    }while((temp&0x01) && !(temp&(1<<16)));     //�ȴ�ʱ�䵽��
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //�رռ�����
    SysTick->VAL = 0x00;                        //��ռ�����
}

/*
********************************************************************************
*                               setTimeOut_ms 
*
* Description : ��SysTick������ʱ��ʹ�ã�ÿnms����һ���ж�
*
* Arguments   : nms:��ʱ�ĺ�������
*               callback:�ص�������
*               �ص������Ĳ�����
*
* Returns     : none
*
* Note(s)     : SysTick->LOADΪ24λ�Ĵ������������ʱΪ��nms*face_ms <= 0xffffff
*               ��48M�����²���ƵʱmFace_ms==48000������nms<=349
*               ��48M������8��ƵʱmFace_ms==6000������nms<=2796
*               ��72M�����²���ƵʱmFace_ms==72000������nms<=233
*               ��72M������8��ƵʱmFace_ms==9000������nms<=1864
*               ��168M�����²���Ƶʱface_ms==168000������nms<=99
*               ��168M������8��Ƶʱface_ms==21000������nms<=792
********************************************************************************
*/
void systick_setTimeOut_ms(uint16_t nms, void (*pfun)(void *), void *arg)
{
    //param_assert(nms <= max_ms);

    SysTick->LOAD = face_ms * nms - 1;
    SysTickCallback.pfun = pfun;
    SysTickCallback.arg = arg;

    systick_ITConfig(ENABLE);
}

/*
********************************************************************************
*                               setTimeOut_us 
*
* Description : ��SysTick������ʱ��ʹ�ã�ÿnus����һ���ж�
*
* Arguments   : nus:��ʱ��΢������
*               callback:�ص�������
*               �ص������Ĳ�����
*
* Returns     : none
*
* Note(s)     : SysTick->LOADΪ24λ�Ĵ������������ʱΪ��nus*face_us <= 0xffffff
*               ��48M�����²���ƵʱmFac_us==48������nus<=349525
*               ��48M������8��ƵʱmFac_us==6������nus<=2796202
*               ��72M�����²���ƵʱmFace_us==72,����nus<=233016
*               ��72M������8�ֲ�ʱmFace_us==9,����nus<=1864135
*               ��168M�����²���Ƶʱface_us==168,����nus<=99864
*               ��168M������8�ֲ�ʱface_us==21,����nus<=798915
*******************************************************************************
*/
void systick_setTimeOut_us(uint16_t nus, void (*pfun)(void *), void *arg)
{
    //param_assert(nus <= max_us);

    SysTick->LOAD = face_us * nus - 1;
    SysTickCallback.pfun = pfun;
    SysTickCallback.arg = arg;

    systick_ITConfig(ENABLE);
}

/*
********************************************************************************
*                               mDelay 
*
* Description : ������ʱ������
*
* Arguments   : nms��ʱ��������
*
* Returns     : none
*
* Note(s)     : ����ʱ�ǳ�����ʱ�䡣
********************************************************************************
*/
void systick_mDelay(uint32_t nms)
{
    if(nms <= max_ms)
    {
        systick_delay_ms_private((uint16_t)nms);
    }
    else
    {
        while(nms--)
            systick_delay_ms_private(1);
    }
}

/*
********************************************************************************
*                               uDelay 
*
* Description : ΢����ʱ������
*
* Arguments   : nus��ʱ΢������
*
* Returns     : none
*
* Note(s)     : SysTick->LOADΪ24λ�Ĵ��������������ʱΪ��nus*face_us <= 0xffffff
*               ��48M�����²���ƵʱmFac_us==48������nus<=349525
*               ��48M������8��ƵʱmFac_us==6������nus<=2796202
*               ��72M�����²���ƵʱmFace_us==72,����nus<=233016
*               ��72M������8�ֲ�ʱmFace_us==9,����nus<=1864135
*               ��168M�����²���Ƶʱface_us==168,����nus<=99864
*               ��168M������8�ֲ�ʱface_us==21,����nus<=798915
********************************************************************************
*/
void systick_uDelay(uint32_t nus)
{
    //param_assert(nus <= max_us);

    uint32_t temp;
    SysTick->LOAD = nus * face_us - 1;           //����ʱ��
    SysTick->VAL = 0x00;                        //��ռ�����
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   //��ʼ����
    do{
        temp = SysTick->CTRL;
    }while((temp&0x01) && !(temp&(1<<16)));     //�ȴ�ʱ�䵽��
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //�رռ�����
    SysTick->VAL = 0x00;                        //��ռ�����
}

/*
********************************************************************************
*                               setITConfig 
*
* Description : �������߹ر�SysTick�жϡ�
*
* Arguments   : NewState: ENABLE �����жϣ���SysTick����ֵΪ0ʱ����SysTick�жϡ�
*                         DISABLE �ر��жϡ�
*
* Returns     : none
*
* Note(s)     : none
********************************************************************************
*/
void systick_ITConfig(FunctionalState NewState)
{
    if(NewState != DISABLE)
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    else
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

/*
********************************************************************************
*                               setReload 
*
* Description : ����SysTick���Զ���װֵ��
*
* Arguments   : reloadVal
*
* Returns     : none
*
* Note(s)     : �Զ���װֵӦ��С��0xffffff��
********************************************************************************
*/
void systick_setReload(uint32_t reloadVal)
{
    //param_assert(reloadVal <= 0xffffff);

    SysTick->LOAD = reloadVal;
}

/*
********************************************************************************
*                               value 
*
* Description : ��ȡSysTick�ĵ�ǰ����ֵ��
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)     : �˺���������������ĳЩ����������ʱ�䡣ʹ�÷�����setReload(0),
*               Ȼ�����enable()��ʼ��ʱ����value()��������ֵ�����SysTickʱ��Դ
*               ����Ƶ����value()/168Ϊ�����ִ�е�us�������8��Ƶ��value()/21Ϊ��
*               ���ִ�е�us����
********************************************************************************
*/
uint32_t systick_getValue(void)
{
    return SysTick->VAL;
}

/*
********************************************************************************
*                               enable 
*
* Description : ����SysTick��ʱ����
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)     : �Լ���ֵ��0��Ȼ��Ӳ������Ԥװ�ؼĴ������ؼ���ֵ���еݼ�������
********************************************************************************
*/
void systick_enable(void)
{
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   
}

/*
********************************************************************************
*                               disable 
*
* Description : ֹͣSysTick�����������������ǰ�ļ���ֵ��
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)     : none
********************************************************************************
*/
void systick_disable(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


