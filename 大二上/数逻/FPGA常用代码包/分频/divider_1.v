`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2020/11/12 10:04:11
// Design Name: 
// Module Name: divider_2
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


module divider_1(
    input clk_i,
    output reg clk_o
    );
reg [25:0] counter = 'b0;
always @(posedge clk_i) begin
    if(counter == 'b0)begin
        clk_o <= 1'b0;
        counter <= 'b1; 
    end else if(counter < 'b10111110101111000010000000)begin
        clk_o <= clk_o;
        counter <= counter + 'b1;
    end else begin
        clk_o <= !clk_o;
        counter <= 'b1;
    end
end
endmodule
