module top(
    input clk100,
    input [1:0] btn,
    output [3:0] led,
    output ftdi_rxd,
    input ftdi_txd,
);

wire clk_25mhz;

attosoc soc(
    .clk(clk_25mhz),
    .reset_n(btn[0]),
    .led(led),
    .uart_tx(ftdi_rxd),
    .uart_rx(ftdi_txd)
);

SB_PLL40_PAD #(
    .FEEDBACK_PATH ("SIMPLE"),
    .DIVR (4'b0000),
    .DIVF (7'b0000111),
    .DIVQ (3'b101),
    .FILTER_RANGE (3'b101)
) uut (
    .RESETB         (1'b1),
    .BYPASS         (1'b0),
    .PACKAGEPIN     (clk100),
    .PLLOUTGLOBAL   (clk_25mhz)
);

endmodule
