#ifndef DMA_MEM2MEM_H
#define DMA_MEM2MEM_H

#include "stm32f1xx.h"
#include <stdint.h>

void dma1_mem2mem_config(void);
void dma_transfer_start(uint32_t src_buff, uint32_t dest_buff, uint32_t len);

#endif /* end of include guard: DMA_MEM2MEM_H */
