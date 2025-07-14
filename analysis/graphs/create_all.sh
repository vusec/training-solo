
#Figure 5
./fig5_create_victim_branches.sh $1

#Figure 7
./fig7_create_ind_module_cdf.sh $1

# Figure 8
./fig8_create_its_cbpf_cdf.sh $1

# Figure 10
./fig10_create_its_native_cdf.sh $1

# Figure 6 + 9
./fig6_9_create_collision_stats.sh $1
