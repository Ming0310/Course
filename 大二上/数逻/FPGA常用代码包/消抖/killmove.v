`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/05/08 13:59:05
// Design Name: 
// Module Name: killmove
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module killmove(
    input clk_1m_i,
    input rst_i,
    input s_i,
    output reg s_o
    );
    localparam TIME_20MS = 20;       // just for test

    reg key_cnt;
    reg [20:0] cnt;

    always @(posedge clk_1m_i or posedge rst_i) begin
        if(rst_i == 1'b1)
            key_cnt <= 0;
        else if(key_cnt == 0 && s_o != s_i)
            key_cnt <= 1;
        else if(cnt == TIME_20MS - 1)
            key_cnt <= 0;
    end

    always @(posedge clk_1m_i or posedge rst_i) begin
        if(rst_i == 1'b1)
            cnt <= 0;
        else if(key_cnt)
            cnt <= cnt + 1'b1;
        else
            cnt <= 0;
    end

    always @(posedge clk_1m_i or posedge rst_i) begin
        if(rst_i == 1'b1)
            s_o <= 1'b0;
        else if(cnt == TIME_20MS - 1)
            s_o <= s_i;
    end
endmodule
