# KATHRYN

Kathryn is a brand new Hardware Design Framework embedded in C++ and Python(next release) to **abstract** control logic, **reduce** manual hardware routing, and **automate** hardware resource management at ***Cycle-accurate level in Userland***.

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

### Quick Start (from clone to runnable)

Two smoke tests are provided:

| test | params file | what it does |
|---|---|---|
| **test sim** (framework regression) | `params/smParams` | runs the auto-sim regression suite of the framework itself |
| **test cpu** (Kride vs RIDECORE) | `params/krideRideCxxParams` | co-simulates the Kride OoO RISC-V CPU against the RIDECORE reference (Verilator), cycle-by-cycle compare, over 10 C++ workloads (`Fibo`, `Tarai`, `Cprime`, `Acker`, `Hanoi`, `Matmul`, `Sort3`, `Stencil`, `Stirling`, `Komachi`), and reports the **simulation time of each workload** |

#### 0. prerequisites

- Linux (tested on Ubuntu), `git`, `cmake >= 3.22`, `make`
- `g++` with C++17 support — needed **both** to build Kathryn **and at runtime**: the hybrid simulator JIT-generates C++, compiles it to a `.so` (`modelCompile/startGen.sh`), and `dlopen`s it (`buildSimMode = gcr`)
- **Verilator** — only for *test cpu*; headers are expected at `/usr/share/verilator/include`, which is where the distro package puts them:

``` bash
sudo apt install build-essential cmake git verilator
```

#### 1. clone the source code

``` bash
git clone https://github.com/Tanawin1701d/Kathryn.git
cd Kathryn
# the RIDECORE reference CPU is a git submodule (needed for *test cpu* only)
git submodule update --init
```

#### 2. make sure VCD and profiler recording are DISABLED

Before running the tests, check `src/params/simParam.cpp` — the two policies must be:

``` cpp
const MODULE_VCD_REC_POL PARAM_VCD_REC_POL      = MDE_REC_SKIP;
const MODULE_FLOW_PERF_PARAM PARAM_PERF_REC_POL = MFP_OFF;
```

`MDE_REC_SKIP` disables VCD waveform recording and `MFP_OFF` disables the ZEP flow profiler. This is the default of the repository. Keep them disabled for the tests: recording slows the simulation down dramatically, produces huge waveform/profile files, and would pollute the per-workload simulation-time measurement. (Re-enable them only when you actually want to debug a waveform/profile, then rebuild.)

#### 3. edit the params files to your local path

The `prefix` key in each params file is an **absolute** output path — change it to your clone location (do not forget the trailing `/`):

- `params/smParams` → `prefix = /path/to/Kathryn/KOut/simpleTest/`
- `params/krideRideCxxParams` → `prefix = /path/to/Kathryn/KOut/krideRideCxx/`

The workload images (`asm.out`) for *test cpu* are already checked in under `KOut/krideRideCxx/<workload>/`. Create the *test sim* output folder if it does not exist:

``` bash
mkdir -p KOut/simpleTest
```

#### 4. build

``` bash
mkdir -p build && cd build
# BUILD_RIDECORE=ON is required for *test cpu*; it also runs *test sim* fine.
# If you do not have Verilator and only want *test sim*, use -DBUILD_RIDECORE=OFF.
cmake -DBUILD_RIDECORE=ON ..
make -j
```

#### 5. run test sim (framework regression)

``` bash
./Kathryn ../params/smParams
```

Expected: a cocotb-style summary table where every auto-sim test case passes. (The first run takes longer — the simulator generates and compiles the `.so`.)

#### 6. run test cpu (Kride vs RIDECORE co-simulation)

``` bash
./Kathryn ../params/krideRideCxxParams
```

This simulates the Kathryn-built Kride CPU and the Verilated RIDECORE side by side, comparing the full pipeline state every cycle. Expected: `compare pass` for every workload and finally `all tests passes`. Note this takes a while (~25 min for all 10 workloads, dominated by `Komachi`) — the co-simulation runs both cores and diffs the whole pipeline state every cycle.

At the end it prints a **per-workload simulation time** table and saves the same data to `KOut/krideRideCxx/simTime.csv`:

```
[O3 RISC-V CMP] per workload simulation time
workload          cycles    kathryn(s)   ridecore(s)     wall(s)    kathryn(c/s)   ridecore(c/s)
Fibo                3352        1.9400        0.0776      2.0618            1728           43221
Tarai             242444      190.2168        8.0266    203.1320            1275           30205
Cprime            173018      106.8811        4.1940    114.0521            1619           41254
...
TOTAL            2231367     1411.5612       57.4350   1505.8834            1581           38850
```

Columns: simulated `cycles`, time spent in the **Kathryn** cycle function, time spent in the **Verilated RIDECORE** cycle function, whole-workload `wall` time (init + sim + per-cycle state compare), and simulated cycles/second for each side. Per-cycle timing is measured around each side's `doWorkloadCycle()` separately, so the two columns are directly comparable; `wall` additionally includes the co-simulation compare overhead, which is *not* part of either simulator.

#### 7. (optional) run the blink example

Blink example helps you to understand how to create a simple design with Kathryn — it is a standalone minimal design + sim with its own `main`.

``` bash
# 1. uncomment blinkSample.cpp in add_executable in CMakeLists.txt and comment out main.cpp
# 2. rebuild (step 4)
# 3. change the output directory (vcdFile/profFile) variable in params/blinkParams
# 4. run it
./Kathryn ../params/blinkParams
```

### Curious to dive deeper?
- Visit the official [Kathryn website](https://www.kathryn-tools.org/) for more details.
