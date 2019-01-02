#ifndef __SPICC_H__
#define __SPICC_H__


#define SPICC_FIFO_SIZE 16

#define SPICC_REG_RXDATA (0<<2)
#define SPICC_REG_TXDATA (1<<2)
#define SPICC_REG_CON    (2<<2)
#define SPICC_REG_INT    (3<<2)
#define SPICC_REG_DMA    (4<<2)
#define SPICC_REG_STA    (5<<2)
#define SPICC_REG_PERIOD (6<<2)
#define SPICC_REG_TEST   (7<<2)
#define SPICC_REG_DRADDR (8<<2)
#define SPICC_REG_DWADDR (9<<2)

#define CON_ENABLE bits_desc(SPICC_REG_CON, 0, 1)
#define CON_MODE bits_desc(SPICC_REG_CON, 1, 1)
#define CON_XCH bits_desc(SPICC_REG_CON, 2, 1)
#define CON_SMC bits_desc(SPICC_REG_CON, 3, 1)
#define CON_CLK_POL bits_desc(SPICC_REG_CON, 4, 1)
#define CON_CLK_PHA bits_desc(SPICC_REG_CON, 5, 1)
#define CON_SS_CTL bits_desc(SPICC_REG_CON, 6, 1)
#define CON_SS_POL bits_desc(SPICC_REG_CON, 7, 1)
#define CON_DRCTL bits_desc(SPICC_REG_CON, 8, 2)
#define CON_CHIP_SELECT bits_desc(SPICC_REG_CON, 12, 2)
#define CON_DATA_RATE_DIV bits_desc(SPICC_REG_CON, 16, 3)
#define CON_BITS_PER_WORD bits_desc(SPICC_REG_CON, 19, 6)
#define CON_BURST_LEN bits_desc(SPICC_REG_CON, 25, 7)

#define INT_TX_EMPTY_EN bits_desc(SPICC_REG_INT, 0, 1)
#define INT_TX_HALF_EN bits_desc(SPICC_REG_INT, 1, 1)
#define INT_TX_FULL_EN bits_desc(SPICC_REG_INT, 2, 1)
#define INT_RX_READY_EN bits_desc(SPICC_REG_INT, 3, 1)
#define INT_RX_HALF_EN bits_desc(SPICC_REG_INT, 4, 1)
#define INT_RX_FULL_EN bits_desc(SPICC_REG_INT, 5, 1)
#define INT_RX_OF_EN bits_desc(SPICC_REG_INT, 6, 1)
#define INT_XFER_COM_EN bits_desc(SPICC_REG_INT, 7, 1)

#define DMA_EN bits_desc(SPICC_REG_DMA, 0, 1)
#define DMA_TX_FIFO_TH bits_desc(SPICC_REG_DMA, 1, 5)
#define DMA_RX_FIFO_TH bits_desc(SPICC_REG_DMA, 6, 5)
#define DMA_NUM_RD_BURST bits_desc(SPICC_REG_DMA, 11, 4)
#define DMA_NUM_WR_BURST bits_desc(SPICC_REG_DMA, 15, 4)
#define DMA_URGENT bits_desc(SPICC_REG_DMA, 19, 1)
#define DMA_THREAD_ID bits_desc(SPICC_REG_DMA, 20, 6)
#define DMA_BURST_NUM bits_desc(SPICC_REG_DMA, 26, 6)

#define STA_TX_EMPTY bits_desc(SPICC_REG_STA, 0, 1)
#define STA_TX_HALF bits_desc(SPICC_REG_STA, 1, 1)
#define STA_TX_FULL bits_desc(SPICC_REG_STA, 2, 1)
#define STA_RX_READY bits_desc(SPICC_REG_STA, 3, 1)
#define STA_RX_HALF bits_desc(SPICC_REG_STA, 4, 1)
#define STA_RX_FULL bits_desc(SPICC_REG_STA, 5, 1)
#define STA_RX_OF bits_desc(SPICC_REG_STA, 6, 1)
#define STA_XFER_COM bits_desc(SPICC_REG_STA, 7, 1)

#define CLK_FREE_EN bits_desc(SPICC_REG_TEST, 24, 1)

struct spicc_platform_data {
	int device_id;
	struct spicc_regs __iomem *regs;
	struct pinctrl *pinctrl;
	struct clk *clk;
	int num_chipselect;
	int *cs_gpios;
};

#endif

