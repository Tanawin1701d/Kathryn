`timescale 1ns/1ns

module tb;

parameter CLK_PEROID = 10;
parameter AMT_SIM_CLK = 100;

reg rst;
reg clk;
wire[7: 0] a;
wire[7: 0] b;
wire[7: 0] c;


initial begin
    clk = 0;
    forever #(CLK_PEROID/2) clk = ~clk;
end


top top_md(
    .rst(rst),
    .a(a),
    .b(b),
    .c(c),
    .clk(clk)

);

integer cycle;
initial begin
    $dumpfile("output/5.vcd");
    $dumpvars(0, rst);
    $dumpvars(0, a);
    $dumpvars(0, b);
    $dumpvars(0, c);
    $dumpvars(0, clk);
    $dumpvars(0, top_md.SR_ST304_startNode);
    $dumpvars(0, top_md.MODULE307.SR_ST313_seqStateReg_312_0);
    $dumpvars(0, top_md.MODULE307.SR_ST317_seqStateReg_312_1);
    $dumpvars(0, top_md.MODULE307.SR_ST321_seqStateReg_312_2);
    rst = 1;
    #(CLK_PEROID);
    rst = 0;
    for (cycle = 0; cycle < 99; cycle = cycle + 1)begin
        #(CLK_PEROID);
    end
    $finish;
end

endmodule


// module  top(
// input wire[0: 0] rst,
// output wire[7: 0] a,
// output wire[7: 0] b,
// output wire[7: 0] c,
// input wire clk
// );
// endmodule