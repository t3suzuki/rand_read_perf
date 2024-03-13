for i_th in {1,2,4,8}
do
    make N_TH=${i_th} URAND=1
    sudo ./my 2
    sudo ./my 3
    make N_TH=${i_th} URAND=0 THETA=0.7
    sudo ./my 2
    sudo ./my 3
    make N_TH=${i_th} URAND=0 THETA=0.9
    sudo ./my 2
    sudo ./my 3
    make N_TH=${i_th} URAND=0 THETA=0.99
    sudo ./my 2
    sudo ./my 3
done
