if [ -z "$1" ]; then
    echo "no proxyEvent file was provided. usage $1 <path <source_file.cpp>"
    exit 1
fi

SRC_PROXYEVENT_NAME=$1

cd ../modelCompile

echo "[COMPILE SCRIPT] sim file is: $SRC_PROXYEVENT_NAME @ dir: $(pwd)"

#if [ -z "$(ls -A build)" ]; then
#  echo "no need to delete old file"
#else
#  echo "now delete old file if there are"
#  #rm build/*    # for debug mode
#fi

echo -n "[COMPILE SCRIPT] compiling....."
sleep 1
time g++ -fPIC -fopt-info-all -O2  -shared -o build/$SRC_PROXYEVENT_NAME.so -I ../src 2> optec.txt \
generated/$SRC_PROXYEVENT_NAME.cpp \
../src/model/simIntf/base/proxyEventBase.cpp \
../src/util/fileWriter/fileWriterBase.cpp \
../src/sim/simResWriter/simResWriter.cpp
echo  "\r[COMPILE SCRIPT] compiled"