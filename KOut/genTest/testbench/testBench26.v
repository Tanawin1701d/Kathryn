`timescale 1ns/1ns

module tb;

parameter CLK_PEROID = 10;
parameter AMT_SIM_CLK = 100;

reg rst;
wire[7: 0] a;
wire[7: 0] b;
wire[7: 0] c;
wire[7: 0] d;
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
.d(d),
.clk(clk)
);

integer amtError = 0;
integer cycle;
initial begin
    $dumpfile("output/26.vcd");
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
    for (integer i = 2; i <= 8; i = i + 1) begin
        if (a !== i) begin
            $display("a = %0d, expected %0d", a, i);
            amtError = amtError + 1;
        end
        #(CLK_PEROID);  // move to next clock cycle
    end
    for (integer i = 10; i <= 16; i = i + 2) begin
        if (a !== i) begin
            $display("a = %0d, expected %0d", a, i);
            amtError = amtError + 1;
        end
        #(CLK_PEROID);  // move to next clock cycle
    end
    if (a !== 24) begin
        $display("a = %0d, expected %0d", a, 24);
        amtError = amtError + 1;
    end
    #(CLK_PEROID);  // move to next clock cycle
    if (a !== 48) begin
        $display("a = %0d, expected %0d", a, 48);
        amtError = amtError + 1;
    end
    #(CLK_PEROID);  // move to next clock cycle

    //  #(3*CLK_PEROID)
    // if (a != 1) begin $display("a expect 1"); amtError = amtError + 1; end
    // if (b != 0) begin $display("b expect 0"); amtError = amtError + 1; end
    // if (c != 0) begin $display("c expect 0"); amtError = amtError + 1; end
    // if (d != 0) begin $display("d expect 0"); amtError = amtError + 1; end
    // #(CLK_PEROID)
    // if (a != 1) begin $display("a expect 1"); amtError = amtError + 1; end
    // if (b != 2) begin $display("b expect 2"); amtError = amtError + 1; end
    // if (c != 0) begin $display("c expect 0"); amtError = amtError + 1; end
    // if (d != 0) begin $display("d expect 0"); amtError = amtError + 1; end
    // #(CLK_PEROID)
    // if (a != 1) begin $display("a expect 1"); amtError = amtError + 1; end
    // if (b != 2) begin $display("b expect 2"); amtError = amtError + 1; end
    // if (c != 3) begin $display("c expect 3"); amtError = amtError + 1; end
    // if (d != 4) begin $display("d expect 4"); amtError = amtError + 1; end
    
    // #(39*CLK_PEROID)
    // if (a != 48) begin $display("a expect 48"); amtError = amtError + 1; end
    // if (b != 39) begin $display("b expect 39"); amtError = amtError + 1; end
    // if (c != 38) begin $display("c expect 38"); amtError = amtError + 1; end
    // if (d !=  0) begin $display("d expect  0"); amtError = amtError + 1; end

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