//
// Created by tanawin on 6/5/2567.
//


const int AMT_DATA = 4;

int a[AMT_DATA] = {4,3,2,1};
int b[AMT_DATA];




void sort(){


    ///////// section 1
    int  batchSize = 2;
    int* abSrc     = a;
    int* abDes     = b;


    /////////// main loop
    while (batchSize <=  AMT_DATA){

        int  mainStart = 0;
        int  mainStop  = mainStart + batchSize;
        int  batchhalf = (batchSize >> 1);
        int  mid       = mainStart + batchhalf;


        ///////// iterate trough each batch size
        //////////iterate loop
        while (mainStart < AMT_DATA){

            int i = mainStart;
            int j = mid;
            int resIdx = mainStart;

            //////////////// compare loop

            while (i < mid || j < mainStop){

                if ( (i<mid) && (j==mainStop) ){
                    abDes[resIdx] = abSrc[i];
                    i++;
                }else if ( (j<mainStop) && (i== mid) ){
                    abDes[resIdx] = abSrc[j];
                    j++;
                }else if ( abSrc[i] < abSrc[j] ){
                    abDes[resIdx] = abSrc[i];
                    i++;
                }else{
                    abDes[resIdx] = abSrc[j];
                    j++;
                }
                resIdx++;

            }


            mainStart = mainStop;
            mainStop  = mainStart + batchSize;
            mid       = mainStart  +  (batchSize >> 1);
        }
        std::swap(abSrc, abDes);
        batchSize = batchSize << 1;
    }

}