`timescale 1ns/1ns

module tb;

parameter CLK_PEROID = 10;
parameter AMT_SIM_CLK = 100;

reg rst;
wire[7: 0] a;
wire[7: 0] b;
wire[7: 0] c;
reg clk;

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

integer amtError = 0;


integer cycle;
initial begin
    $dumpfile("output/7.vcd");
    $dumpvars(0, tb);
    // $dumpvars(0, a);
    // $dumpvars(0, b);
    // $dumpvars(0, c);
    // $dumpvars(0, clk);
    // $dumpvars(0, top_md.SR_ST304_startNode);
    // $dumpvars(0, top_md.MODULE307.SR_ST313_seqStateReg_312_0);
    // $dumpvars(0, top_md.MODULE307.SR_ST317_seqStateReg_312_1);
    // $dumpvars(0, top_md.MODULE307.SR_ST321_seqStateReg_312_2);
    rst = 1;
    #(CLK_PEROID);
    ///////// start node set
    rst = 0;
    #(CLK_PEROID)
    #(CLK_PEROID)
    if (a != 36)       begin $display("a expect 36"); amtError = amtError + 1; end
    if (b != 12)       begin $display("b expect 12"); amtError = amtError + 1; end
    #(CLK_PEROID)
    if (a != 48)       begin $display("c3 a expect 48");       amtError = amtError + 1; end
    if (b != 12)       begin $display("c3 b expect 12");       amtError = amtError + 1; end
    if (c != 48)       begin $display("c3 c expect 48");       amtError = amtError + 1; end

    if (amtError > 0) begin
        $display("FAIL THIS TEST CASE");
    end else begin
        $display("PASS");
    end
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