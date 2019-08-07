#include <rtthread.h>
#include <rtdevice.h>

#include "dw1000_usr.h"
#include "drv_usr.h"
#include "board.h"
#include "drv_spi.h"

// SPI_HandleTypeDef hspi1;
// static void MX_SPI1_Init(unsigned int rate)
// {

//     /* USER CODE BEGIN SPI1_Init 0 */

//     /* USER CODE END SPI1_Init 0 */

//     /* USER CODE BEGIN SPI1_Init 1 */

//     /* USER CODE END SPI1_Init 1 */
//     /* SPI1 parameter configuration*/
//     hspi1.Instance = SPI1;
//     hspi1.Init.Mode = SPI_MODE_MASTER;
//     hspi1.Init.Direction = SPI_DIRECTION_2LINES;
//     hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
//     hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
//     hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
//     hspi1.Init.NSS = SPI_NSS_SOFT;
//     hspi1.Init.BaudRatePrescaler = rate;//SPI_BAUDRATEPRESCALER_8;
//     hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
//     hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
//     hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//     hspi1.Init.CRCPolynomial = 10;
//     if (HAL_SPI_Init(&hspi1) != HAL_OK)
//     {
//         Error_Handler();
//     }
//     /* USER CODE BEGIN SPI1_Init 2 */

//     /* USER CODE END SPI1_Init 2 */

// }

static int rt_hw_spi_DW1000_init(void)
{
    __HAL_RCC_GPIOF_CLK_ENABLE();
    rt_hw_spi_device_attach("spi2", "spi20", GPIOF, GPIO_PIN_7);

    // if (RT_NULL == rt_sfud_flash_probe("W25Q128", "spi10"))
    // {
    //     return -RT_ERROR;
    // };

    return RT_EOK;
}
/* ???????? */
// INIT_COMPONENT_EXPORT(rt_hw_spi_DW1000_init);

void process_deca_irq(void *pvParameters);

void DW1000_gpio_init(void)
{
    // rt_pin_mode(W25Q128_CS, PIN_MODE_OUTPUT); //FLASH CS
    // rt_pin_write(W25Q128_CS, PIN_HIGH);       //À­¸ßflash ½ûÖ¹flash²Ù×÷

    // rt_pin_mode(DW1000_CS, PIN_MODE_OUTPUT);  //DW1000 SPI CS
    rt_pin_mode(DW1000_RST, PIN_MODE_OUTPUT); //DW1000 RESET

    rt_pin_mode(DW1000_IRQ, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(DW1000_IRQ, PIN_IRQ_MODE_RISING_FALLING, dw1000_irq_isr_handler, (void *)"callbackargs");
    rt_pin_irq_enable(DW1000_IRQ, PIN_IRQ_ENABLE);

    // rt_pin_irq_enable(DW1000_IRQ, PIN_IRQ_DISABLE);
}
void DW1000_init(void)
{
    DW1000_gpio_init();
    rt_hw_spi_DW1000_init();
    // HAL_SPI_MspDeInit(&hspi1);
    // MX_SPI1_Init(SPI_BAUDRATEPRESCALER_32);
}
//-----------------------------------
void spi_write(uint8_t *pData, uint16_t Size)
{
    rt_spi_send(rt_spi_device, &cmd, sizeof(cmd));
    HAL_SPI_Transmit(&hspi1, pData, Size, 0xffff);
}
void spi_read(uint8_t *pData, uint16_t Size)
{
    HAL_SPI_Receive(&hspi1, pData, Size, 0xffff);
}
void DW1000_enableirq(void)
{
    rt_pin_irq_enable(DW1000_IRQ, PIN_IRQ_ENABLE);
}
void DW1000_disableirq(void)
{
    rt_pin_irq_enable(DW1000_IRQ, PIN_IRQ_DISABLE);
}
/*
#define SPI_BAUDRATEPRESCALER_2         0x00000000U
#define SPI_BAUDRATEPRESCALER_4         0x00000008U
#define SPI_BAUDRATEPRESCALER_8         0x00000010U
#define SPI_BAUDRATEPRESCALER_16        0x00000018U
#define SPI_BAUDRATEPRESCALER_32        0x00000020U
#define SPI_BAUDRATEPRESCALER_64        0x00000028U
#define SPI_BAUDRATEPRESCALER_128       0x00000030U
#define SPI_BAUDRATEPRESCALER_256       0x00000038U
*/
//speed:SPI_BAUDRATEPRESCALER_2,SPI_BAUDRATEPRESCALER_4....
void spi_speed_set(int speed)
{
    HAL_SPI_MspDeInit(&hspi1);
    MX_SPI1_Init(speed);
    HAL_SPI_MspInit(&hspi1);
}
