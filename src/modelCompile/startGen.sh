echo "changing directory ...."
cd ../src/modelCompile
echo "current path is $(pwd)"
if [ -z "$(ls -A build)" ]; then
  echo "no need to delete old file"
else
  echo "now delete old file if there are"
  rm build/*
fi
echo "now start compiling....."
g++  -fPIC -shared -o build/simClient.so -I ../../src \
proxyEvent.cpp \
proxyEventBase.cpp \
../util/fileWriter/fileWriterBase.cpp \
../sim/simResWriter/simResWriter.cpp
echo "finish modelCompile"