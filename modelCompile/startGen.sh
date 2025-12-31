if [ -z "$1" ]; then
    echo "no proxyEvent file was provided. usage $1 <path <source_file.cpp>"
    exit 1
fi

if [ -z "$2" ]; then
    echo "no project path file was provided. usage $2 <path <folderPath>>"
fi

if [ -z "$3" ]; then
  echo "no optimization level provided. usagea $3 <-O3>"
fi

SRC_PROXYEVENT_NAME=$1
SRC_PRJ_PATH=$2
SRC_OPT_LEVEL=$3

cd $SRC_PRJ_PATH/modelCompile

echo "[COMPILE SCRIPT] sim file is: $SRC_PROXYEVENT_NAME @ dir: $(pwd)"
echo "[COMPILE SCRIPT] project path is: $SRC_PRJ_PATH"

#if [ -z "$(ls -A build)" ]; then
#  echo "no need to delete old file"
#else
#  echo "now delete old file if there are"
#  #rm build/*    # for debug mode
#fi


# 2> optec.txt -fopt-info-all

echo -n "[COMPILE SCRIPT] compiling....."
sleep 1
time g++ -fPIC  -shared -o build/$SRC_PROXYEVENT_NAME.so $SRC_OPT_LEVEL -I ../src \
generated/$SRC_PROXYEVENT_NAME.cpp \
../src/sim/modelSimEngine/base/proxyEventBase.cpp \
../src/util/fileWriter/fileWriterBase.cpp \
../src/sim/simResWriter/simResWriter.cpp
echo  "\r[COMPILE SCRIPT] compiled"