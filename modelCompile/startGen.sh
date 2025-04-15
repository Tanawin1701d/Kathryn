if [ -z "$1" ]; then
    echo "no proxyEvent file was provided. usage $1 <path <source_file.cpp>"
    exit 1
fi

if [ -z "$2" ]; then
    echo "no project path file was provided. usage $2 <path <folderPath>>"
    exit 1
fi

if [ -z "$3" ]; then
    echo "no optimization level provided. using $3 <number of optimization -O0, -O1, -O2, -O3>"
    exit 1
fi

SRC_PROXYEVENT_NAME=$1
SRC_PRJ_PATH=$2
OPT_FLAG=$3

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
time g++ -fPIC $OPT_FLAG  -shared -o build/$SRC_PROXYEVENT_NAME.so -I ../src \
generated/$SRC_PROXYEVENT_NAME.cpp \
../src/sim/modelSimEngine/base/proxyEventBase.cpp \
../src/util/fileWriter/fileWriterBase.cpp \
../src/sim/simResWriter/simResWriter.cpp
echo  "\r[COMPILE SCRIPT] compiled"

GEN_CPP="generated/$SRC_PROXYEVENT_NAME.cpp"
SO_FILE="build/$SRC_PROXYEVENT_NAME.so"

echo "[CPP FILE SIZE] $SRC_PROXYEVENT_NAME.cpp: $(stat --format="%s bytes" "$GEN_CPP")"
echo "[SO FILE SIZE]  $SRC_PROXYEVENT_NAME.cpp: $(stat --format="%s bytes" "$SO_FILE")"