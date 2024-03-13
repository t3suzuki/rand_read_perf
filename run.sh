for i_th in {1,2,4}
do
    make N_TH=${i_th} URNAD=1
    sudo ./a.out 2
    sudo ./a.out 3
    make N_TH=${i_th} URNAD=0 THETA=0.9
    sudo ./a.out 2
    sudo ./a.out 3
    make N_TH=${i_th} URNAD=0 THETA=0.99
    sudo ./a.out 2
    sudo ./a.out 3
done
