.mode csv
.separator ;

-- Import analysis results
.import all-gadgets-reasoned.csv gadgets
.import all-tfps-reasoned.csv tfps

-- Import lists generated from the target
.import lists/its_cbpf_targets.csv its_cbpf

-- Create merged tables (faster than views)

CREATE TABLE gadget_its_cbpf AS SELECT * FROM(
	select * from its_cbpf
	JOIN gadgets ON its_cbpf.target=gadgets.address
);

CREATE TABLE tfp_its_cbpf AS SELECT * FROM(
	select * from its_cbpf
	JOIN tfps ON its_cbpf.target=tfps.address
);


.headers on
.once gadget_its_cbpf.csv
select * from gadget_its_cbpf;

.once tfp_its_cbpf.csv
select * from tfp_its_cbpf;


-- DROP TABLE gadgets;
-- DROP TABLE tfps;
-- DROp TABLE collisions;

-- .save gadgets.db
