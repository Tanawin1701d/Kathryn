# KATHRYN

Kathryn is the Hardware Generation and Fast Simulation framework with 
Hybrid design flow embedded in C++ and Python(next release)
to reduce Hardware design workload, 
enhance parallelization control, 
boost design space exploration, 
and increase simulation productivity.


## offered features
- Hybrid Design Flow (HDF)
    -  the new way to control and synchronize the Hardware Control Flow.
- Zero Effort Cycle Spent Profiler (ZEP)
    -  the automatic tracker tracks how Hardware design spent time to doing something
- High Performance Simulation
- Co-simulatable and high Performance simulator.
- Fully facilited by programming language(C++) as a generator/preprocess.
- Relax Schematic model.
- 100% cycle accurate at userland.
- Unlike HLS, Kathryn is hardware generation framework at full cycle-accurate level like Chisel, Pymtl, Pyrtl

## example code 1
- ```cpp copy
  class ExampleModule: public Module{
  public:
      mWire(i, 32);
      mReg(a, 32);mReg(b, 32);
      mReg(c, 32);mReg(d, 32);
  
      ExampleModule(int x): Module(){ i.asInputGlob(); d.asOutputGlob();}
  
      void flow() override{
          seq{ /// all sub element run [seq]uentialy
              a <<= i;
              par{ /// all sub element run parallelly
                  cdowhile(a < 8){ /// do loop
                      a <<= a + 1;
                      c <<= c + 1;
                  }
                  cdowhile(b < 8){ /// do loop
                      b <<= b + 1;
                      d <<= d + 1;
                  }
              }
              d <<= c + d;
          }
      }
  };
  ```
- the state machine look like This

import Image from 'next/image from website'

## example code 2 (pipeline example)
- ```cpp filename="main.cpp"  {10} copy
  void flow() override{
        pipWrap{
            pipBlk{ a <= a + 1;}
            pipBlk{ b <= a;    }
            pipBlk{
                c <<= b;
                cif(c == 5){
                syWait(6) //// wait for 6 cycle
                } ////- The block will be automatically 
                  ////  SYNCHRONIZED with other block without 
                  //// overhead cycle!!!!!
            }
            pipBlk{ d <= c; }
        }
  }
  ```
  
## setup, test, and start your first Model!
- link to our webpage