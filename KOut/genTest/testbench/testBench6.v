`timescale 1ns/1ns

module tb;

parameter CLK_PEROID = 10;
parameter AMT_SIM_CLK = 100;

reg rst;
wire[7: 0] a;
wire[7: 0] b;
wire[7: 0] c;
wire[7: 0] plusReg;
wire[7: 0] minsReg;
wire[7: 0] bwAndReg;
wire[7: 0] bwOrReg;
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
.plusReg(plusReg),
.minsReg(minsReg),
.bwAndReg(bwAndReg),
.bwOrReg(bwOrReg),
.clk(clk)
);

integer amtError = 0;

integer cycle;
initial begin
    $dumpfile("output/6.vcd");
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
    rst = 0;
    #(CLK_PEROID)
    if (plusReg != 48) begin $display("plusReg expect 48"); amtError = amtError + 1;end
    #(CLK_PEROID)
    if (a != 36)       begin $display("a expect 36"); amtError = amtError + 1; end
    if (b != 36)       begin $display("b expect 36"); amtError = amtError + 1; end
    if (c != 36)       begin $display("c expect 36"); amtError = amtError + 1; end
    if (plusReg != 0)  begin $display("plusReg expect 0"); amtError = amtError + 1; end
    if (minsReg != 24) begin $display("minsReg expect 24"); amtError = amtError + 1; end
    if (bwAndReg != 4) begin $display("bwAndReg expect 4"); amtError = amtError + 1; end
    if (bwOrReg != 44) begin $display("bwOrReg expect 44"); amtError = amtError + 1; end
    #(CLK_PEROID)
    if (a != 36)       begin $display("c3 a expect 36");       amtError = amtError + 1; end
    if (b != 36)       begin $display("c3 b expect 36");       amtError = amtError + 1; end
    if (c != 36)       begin $display("c3 c expect 36");       amtError = amtError + 1; end
    if (plusReg != 0)  begin $display("c3 plusReg expect 0");  amtError = amtError + 1; end
    if (minsReg != 24) begin $display("c3 minsReg expect 24"); amtError = amtError + 1; end
    if (bwAndReg != 4) begin $display("c3 bwAndReg expect 4"); amtError = amtError + 1; end
    if (bwOrReg != 44) begin $display("c3 bwOrReg expect 44"); amtError = amtError + 1; end

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