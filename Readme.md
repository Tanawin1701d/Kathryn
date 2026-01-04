# KATHRYN

Kathryn is a brand new Hardware Design Framework embedded in C++ and Python(next release) to **abstract** control logic, **reduce** manual hardware routing, and **automate** hardware resource management at Cycle-accurate level in Userland.

Moreover, Kathryn **provides** effortless profiling, and **boost** design space exploration and increase simulation productivity.

READER MIGHT THINK THAT WE ARE ALTERNATIVE SYSTEM-C OR PIPELINE-C. IN FACT, WE COMPLETELY DIFFERENT WITH THEM AT ALL.

## offered features
##### 1. Hybrid Design Flow (HDF)
-  the new way to abstract hardware control logic, manage hardware parallelism through Hybrid Design Block (HDB) such as (```seq```, ```par```, ```[c/cdo/s]while```, ```[c/s/z]if```, ```pip/zync```, ```ztate```, ```[sy/sc]Wait```) ***while maintaining cycle-accurate level at USERLAND.***
##### 2. Decentralize Control
- the new way to manage hardware resouce value update without centralizing the control logic
##### 3. Hardware Aggregator
- our feature-rich approach to manage group of hardware resource such as (```SlotMeta```, ```[Reg/Wire]Slot```, ```Table```, ```MemTable```)
##### 4. 100% cycle determinism at userland.

##### 5. Zero Effort Cycle Spent Profiler (ZEP)
- the automatic tracker tracks how Hardware design spent time to each block
##### 6. Hybrid simulator (HS)
- unify dynamically built designers' model and optimized simulation engine
- Co-simulatable and high Performance simulator.
##### 7. Fully facilited by programming language(C++) as a generator/preprocess.

##### 8. Kride (RISC-V Out-of-Order Superscalar CPU)
- Kathryn provides Out-of-Order superscalar CPU based on [RIDE CORE](https://github.com/ridecore/ridecore)
- Currently, I am testing similarity test with RIDE CORE!

##### 9. Unlike HLS, Kathryn is hardware generation framework at full cycle-accurate level like Chisel, Pymtl, Pyrtl

### example code 1 (HDF)
- ```cpp showLineNumbers copy
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
- The update opertor ``` <<=``` will be treated as Cycle-Considered Element (CCE). Desigers can model the system at cycle-accurate level via these CCE and Hybrid design block (HDB). As a result, at line 11, the hardware takes 1 cycle and ```cdowhile``` (line 13-16, and 17-20) takes >1 cycles. The ```par``` block at line 37-46 enables both cdowhile blocks running parallelly.
- the state machine look like This


### example code 2 (HDF complex pipeline management)
- ```cpp showLineNumbers filename="main.cpp"  {10} copy
    PipMeta fetchChan;
    PipMeta decodeChan;
    pip(fetchChan){
        seq{
            myFetch <<= do_somthing(src);     ///// takes 1 cycle
            cwhile(x < 10){                   ///// takes at least 10 cycles
                zyncc(decodeChan){            ///// takes at least 1 cycle
                    fetchResult <<= myFetch;
                    x <<= x + 1;
                }
            }
        }
    }

    pip(decodeChan){
       decodeResult <<= fetchResult;   /////// takes  1 cycle
       syWait(10);                     /////// takes 10 cycles
    }

    seq{
        syWait(10);
        decodeChan.holdMaster();
        decodeChan.killSlave();
    }
  ```

- this example demonstrates complex cycle-accurate pipeline management in Kathryn, the system synchronize fetchChannel and decodeChannel together through ```pip``` and ```zync``` block
- Additionally, we support  various type of ```zync``` block such as ```zyncc(channel, condition)```. The ```zyncc``` block will be conducted if only ```condition``` is valid.
- The channel can be stalled or killed using command in line 22 - 23
### Curious to dive deeper?
- Visit the official [Kathryn website](https://www.kathryn-tools.org/) for more details.
