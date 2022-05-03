#include "uart.h"

static bool isErrorOcure = FALSE;
static uint16_t UartErrFlags = 0;
static UartBuffer_t UartBuffer;
static UartEvents_t *UartEvents = NULL;
static uint8_t uart_buffer[UART_BUFF_SIZE];

/*!
 \brief Initialize the USART unit in DMA mode
 \param [IN] br - UART boudrate
 \param [IN] events - Pointer to a structure containing callbacks
 */
void initUart(uint32_t br, UartEvents_t *events)
{
    dma_parameter_struct dma_init_struct;
    
	UartEvents = events;
	UartBuffer.data = uart_buffer;
    
    /* Enable peripheral clocks */
    rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_DMA);
    
	/* Configure USART0 GPIO ports(PA.9, PA.10) */
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
    
    /* Configure USART0 */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, br);
    /* Enable USART transmission and reception */
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_dma_receive_config(USART0, USART_DENR_ENABLE);
    usart_dma_transmit_config(USART0, USART_DENT_ENABLE);
    usart_receiver_timeout_enable(USART0);
	usart_receiver_timeout_config(USART0, 20);
    usart_enable(USART0);
    /* Enable USART interrupts */
	nvic_irq_enable(USART0_IRQn, 0, 0);
	usart_interrupt_enable(USART0, USART_INT_RT);
    usart_interrupt_enable(USART0, USART_INT_ERR);
    
    /* Configure DMA channel 2 (USART0 RX) */
    dma_deinit(DMA_CH2);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)UartBuffer.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = UART_BUFF_SIZE;
    dma_init_struct.periph_addr = USART0_RDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA_CH2, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA_CH2);
    /* enable DMA channel2 */
    dma_channel_enable(DMA_CH2);
    /* Configure DMA channel 1 (USART0 TX) */
    dma_deinit(DMA_CH1);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_addr = USART0_TDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA_CH1, &dma_init_struct);
    /* Configure DMA mode */
    dma_circulation_disable(DMA_CH1);
    /* Configure NVIC */
    nvic_irq_enable(DMA_Channel1_2_IRQn, 0, 0);
}

/*!
 \brief Start tranfer data from memory to peripheral
 \param [IN] Transferred data size
 */
void startDmaMemToPeriph(uint32_t m_addr, uint32_t size)
{
    DMA_CHMADDR(DMA_CH1) = m_addr;
    DMA_CHCNT(DMA_CH1) = size;
    /* Enable DMA transfer complete interrupt */
    dma_interrupt_enable(DMA_CH1, DMA_CHXCTL_FTFIE);
    /* Enable DMA channel_1 */
    dma_channel_enable(DMA_CH1);
}

/*!
 \brief DMA transmit complete interrupt
*/
void DMA_Channel1_2_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA_CH1, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(DMA_CH1, DMA_INT_FLAG_G);
        dma_interrupt_disable(DMA_CH1,DMA_CHXCTL_FTFIE);
        dma_channel_disable(DMA_CH1);  // stop DMA 
    }
}

/*!
 \brief USART0 interrupt handler in DMA mode
 */
void USART0_IRQHandler(void)
{
	/* USART error inerrupts */
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_FERR)) {
        UartErrFlags |= UART_FERR_FLAG; // frame error
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_FERR);
    }
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_NERR)) {
        UartErrFlags |= UART_NERR_FLAG;// noise error
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_NERR);
    }
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_ORERR)) {
        UartErrFlags |= UART_ORERR_FLAG; // overrun error
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR);
    }
    if ((UartErrFlags != UART_NO_ERROR) || isErrorOcure) {
        isErrorOcure = (UartErrFlags == UART_NO_ERROR) ? false : true;
        UartEvents->uartError(UartErrFlags); // Notifies app about uart errors event
        UartErrFlags = 0;
    }
    
    /* USART timeout interrupt (Received all data block) */
	if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RT)) {
        // Clear interrupt flag
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RT);
        
        // Number of data received
        UartBuffer.len = UART_BUFF_SIZE - (dma_transfer_number_get(DMA_CH2));
        
        // Disable DMA and reconfigure
        dma_channel_disable(DMA_CH2);
        dma_transfer_number_config(DMA_CH2, UART_BUFF_SIZE);
        dma_channel_enable(DMA_CH2);
		
        // Notifies application about uart_rt event
        if ((UartEvents != NULL) && UartEvents->uartRxBuff != NULL) {
			UartEvents->uartRxBuff(UartBuffer.data, UartBuffer.len);
		}
	}
}
//eof
