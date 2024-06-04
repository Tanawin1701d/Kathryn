cd ../src/modelCompile
g++ -fPIC -shared -o build/simClient.so -I ../../src \
proxyEvent.cpp \
proxyEventBase.cpp \
../util/fileWriter/fileWriterBase.cpp \
../sim/simResWriter/simResWriter.cpp
