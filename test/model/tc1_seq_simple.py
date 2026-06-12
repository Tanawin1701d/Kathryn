
from kathryn import *
from kathryn import emit_verilog


class tc1_seq_simple(Module):
    @init
    def com_declare(self):
        self.x = reg(8, "x")
        self.y = reg(8, "y")
        self.simple_val = val(8, 48, "simple_val")

        self.x.mark_output("my_x")
        self.y.mark_output("my_y")

    @flow
    def my_flow(self):

        with seq():
            self.x |= self.simple_val
            self.y |= self.x

def test_fun(output_folder: str):
    reset()
    module = tc1_seq_simple()
    build_model(module)
    emit_verilog(output_folder)

test_fun("/media/tanawin/tanawin1701e/project8/Kathryn2/test/.model_output/tc1_seq_simple")