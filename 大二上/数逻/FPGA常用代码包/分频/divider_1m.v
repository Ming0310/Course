`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/05/08 14:02:44
// Design Name: 
// Module Name: divider_1m
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


module divider_1m(
    input clk_i,
    output reg clk_1m_o
    );
reg [15:0] counter = 1'b0;
always @(posedge clk_i) begin  
    if(counter == 'b0)begin
        clk_1m_o <= 1'b0;
        counter <= 'b1; 
    end else if(counter < 16'b1100001101010000)begin
        clk_1m_o <= clk_1m_o;
        counter <= counter + 'b1;
    end else begin
        clk_1m_o <= !clk_1m_o;
        counter <= 'b1;
    end
end
endmodule
