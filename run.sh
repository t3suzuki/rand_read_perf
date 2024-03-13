for i_th in 8
do
    #make N_TH=${i_th} URAND=1
    #cd build; cmake .. -DN_TH=${i_th} -DURAND=1 -DTHETA=0; make -j 8; cd -
    #sudo numactl --membind=0 ./my 2
    #sudo numactl --membind=0 ./my 3
    #sudo numactl --membind=0 ./build/my 4
    make N_TH=${i_th} URAND=0 THETA=0.3
    cd build; cmake .. -DN_TH=${i_th} -DURAND=0 -DTHETA=0.3; make -j 8; cd -
    #sudo numactl --membind=0 ./my 2
    sudo numactl --membind=0 ./my 3
    sudo numactl --membind=0 ./build/my 4
    make N_TH=${i_th} URAND=0 THETA=0.5
    cd build; cmake .. -DN_TH=${i_th} -DURAND=0 -DTHETA=0.5; make -j 8; cd -
    #sudo numactl --membind=0 ./my 2
    sudo numactl --membind=0 ./my 3
    sudo numactl --membind=0 ./build/my 4
    make N_TH=${i_th} URAND=0 THETA=0.7
    cd build; cmake .. -DN_TH=${i_th} -DURAND=0 -DTHETA=0.7; make -j 8; cd -
    #sudo numactl --membind=0 ./my 2
    sudo numactl --membind=0 ./my 3
    sudo numactl --membind=0 ./build/my 4
    make N_TH=${i_th} URAND=0 THETA=0.9
    cd build; cmake .. -DN_TH=${i_th} -DURAND=0 -DTHETA=0.9; make -j 8; cd -
    #sudo numactl --membind=0 ./my 2
    sudo numactl --membind=0 ./my 3
    sudo numactl --membind=0 ./build/my 4
    make N_TH=${i_th} URAND=0 THETA=0.99
    cd build; cmake .. -DN_TH=${i_th} -DURAND=0 -DTHETA=0.99; make -j 8; cd -
    #sudo numactl --membind=0 ./my 2
    sudo numactl --membind=0 ./my 3
    sudo numactl --membind=0 ./build/my 4
done
