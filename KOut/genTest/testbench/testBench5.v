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

integer amtError = 0;
integer cycle;
initial begin
    $dumpfile("output/5.vcd");
    $dumpvars(0, tb);

    rst = 1;
    #(CLK_PEROID);
    rst = 0;
    #(CLK_PEROID);
    if (a !=  0) begin $display("a expect  0"); amtError = amtError + 1; end
    if (b !=  0) begin $display("b expect  0"); amtError = amtError + 1; end
    if (c !=  0) begin $display("c expect  0"); amtError = amtError + 1; end
    #(CLK_PEROID);
    if (a != 48) begin $display("a expect  48"); amtError = amtError + 1; end
    if (b !=  0) begin $display("b expect  0"); amtError = amtError + 1; end
    if (c !=  0) begin $display("c expect  0"); amtError = amtError + 1; end
    #(CLK_PEROID);
    if (a != 48) begin $display("a expect  48"); amtError = amtError + 1; end
    if (b != 48) begin $display("b expect  48"); amtError = amtError + 1; end
    if (c !=  0) begin $display("c expect  0"); amtError = amtError + 1; end
    #(CLK_PEROID);
    if (a != 48) begin $display("a expect  48"); amtError = amtError + 1; end
    if (b != 48) begin $display("b expect  48"); amtError = amtError + 1; end
    if (c != 48) begin $display("c expect  48"); amtError = amtError + 1; end

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