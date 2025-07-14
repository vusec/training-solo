.mode csv
.separator ;

-- Import analysis results
.import all-gadgets-reasoned.csv gadgets
.import all-tfps-reasoned.csv tfps

-- Import lists generated from the target
.import lists/collisions.csv collisions

-- Create merged tables (faster than views)

CREATE TABLE gadget_collisions AS SELECT * FROM(
	select * from collisions
	JOIN gadgets ON collisions.target=gadgets.address
);

CREATE TABLE tfp_collisions AS SELECT * FROM(
	select * from collisions
	JOIN tfps ON collisions.target=tfps.address
);


.headers on
.once gadget_collisions.csv
select * from gadget_collisions;

.once tfp_collisions.csv
select * from tfp_collisions;


-- DROP TABLE gadgets;
-- DROP TABLE tfps;
-- DROp TABLE collisions;

-- .save gadgets.db
