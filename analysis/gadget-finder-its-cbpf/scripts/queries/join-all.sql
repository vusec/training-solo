.mode csv
.separator ;

-- Import analysis results
.import all-gadgets-reasoned.csv gadgets
.import all-tfps-reasoned.csv tfps

-- Import lists generated from the target
.import lists/its_cbpf_targets.csv its_cbpf


-- .import gadget_collisions.csv gadget_collisions
-- .import tfp_collisions.csv tfp_collisions

.import tfp_its_cbpf_victim.csv tfps_victims
.import gadget_its_cbpf_victim.csv gadgets_victims

.import lists/reachable_functions.csv reachable_functions

.import ../out_victims/all-tfps-reasoned.csv victims


-- DROP TABLE gadgets;
-- DROP TABLE tfps;
-- DROp TABLE collisions;

.save gadgets.db
