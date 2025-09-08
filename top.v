module top (
    input  wire        CLOCK_50,    // 50 MHz clock
    input  wire [3:0]  KEY,         // Push buttons
    output wire [9:0]  LEDR,        // Red LEDs
    output wire [6:0]  HEX0,        // Seven segment displays
    output wire [6:0]  HEX1,
    output wire [6:0]  HEX2,
    output wire [6:0]  HEX3,
    output wire [6:0]  HEX4,
    output wire [6:0]  HEX5,

    // SDRAM interface (connected to physical SDRAM chip)
    output wire [12:0] DRAM_ADDR,
    output wire [1:0]  DRAM_BA,
    output wire        DRAM_CAS_N,
    output wire        DRAM_CKE,
    output wire        DRAM_CS_N,
    inout  wire [15:0] DRAM_DQ,
    output wire        DRAM_LDQM,
    output wire        DRAM_RAS_N,
    output wire        DRAM_UDQM,
    output wire        DRAM_WE_N,
    output wire        DRAM_CLK
);

    wire pll_locked;

    binduhw3 u0 (
        .pll_ref_clk_clk               (CLOCK_50),                   // Clock input to PLL
        .pll_ref_reset_reset   (~KEY[0]),                    // Active-low reset

        // I/O
        .keys_external_connection_export          (KEY),
        .leds_external_connection_export      (LEDR),
        .hex0_export          (HEX0),
        .hex1_export          (HEX1),
        .hex2_export          (HEX2),
        .hex3_export          (HEX3),
        .hex4_export          (HEX4),
        .hex5_export          (HEX5),

        // SDRAM connections
        .sdram_wire_addr      (DRAM_ADDR),
        .sdram_wire_ba        (DRAM_BA),
        .sdram_wire_cas_n     (DRAM_CAS_N),
        .sdram_wire_cke       (DRAM_CKE),
        .sdram_wire_cs_n      (DRAM_CS_N),
        .sdram_wire_dq        (DRAM_DQ),
        .sdram_wire_dqm       ({DRAM_UDQM, DRAM_LDQM}),
        .sdram_wire_ras_n     (DRAM_RAS_N),
        .sdram_wire_we_n      (DRAM_WE_N),
        .pll_sdram_clk_clk    (DRAM_CLK),
       // .sdram_pll_locked_export (pll_locked)
    );

endmodule