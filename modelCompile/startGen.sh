if [ -z "$1" ]; then
    echo "no proxyEvent file was provided. usage $1 <path <source_file.cpp>"
fi

SRC_PROXYEVENT_NAME=$1
echo "sim file is $SRC_PROXYEVENT_NAME"

echo "changing directory ...."
cd ../modelCompile
echo "current path is $(pwd)"

if [ -z "$(ls -A build)" ]; then
  echo "no need to delete old file"
else
  echo "now delete old file if there are"
  #rm build/*    # for debug mode
fi

echo "now start compiling....."
g++  -fPIC -shared -o build/simClient.so -I ../src \
generated/$SRC_PROXYEVENT_NAME \
../src/model/simIntf/base/proxyEventBase.cpp \
../src/util/fileWriter/fileWriterBase.cpp \
../src/sim/simResWriter/simResWriter.cpp
echo "finish modelCompile"