module slave (
    input  wire        clk      ,   
    input  wire        rst_n    ,
    output reg         a_ready  ,
    input  wire        a_valid  ,  
    input  wire [3:0]  a_opcode ,
    input  wire [3:0]  a_mask   ,
    input  wire [3:0]  a_address,
    input  wire [31:0] a_data   ,
    input  wire        d_ready  ,
    output reg         d_valid  ,
    output reg  [3:0]  d_opcode ,
    output reg  [31:0] d_data   ,
    output reg         reg_wr   ,
    output reg         reg_rd   ,
    output reg  [3:0]  reg_byte ,
    output reg  [3:0]  reg_addr ,
    output reg  [31:0] reg_wdata,
    input  wire [31:0] reg_rdata
);


endmodule
