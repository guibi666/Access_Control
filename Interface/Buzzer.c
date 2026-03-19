#include "Buzzer.h"

void Buzzer_Init(void)
{
    MX_GPIO_Init();
}

void Buzzer_Work(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port,Buzzer_Pin,GPIO_PIN_RESET);
}

void Buzzer_NoWork(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port,Buzzer_Pin,GPIO_PIN_SET);
}
