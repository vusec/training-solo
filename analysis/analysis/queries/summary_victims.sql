select "All victims" as "Description", count(DISTINCT pc) as total_unique_pc, count(DISTINCT pc_symbol) as total_unique_symbol, count(DISTINCT address_symbol) as total_unique_entry
from victims
where n_controlled > 0

UNION ALL

select "# registers controlled >= 1" as "Description", count(DISTINCT pc) as total_unique_pc, count(DISTINCT pc_symbol) as total_unique_symbol, count(DISTINCT address_symbol) as total_unique_entry
from victims
where n_controlled > 0

UNION All

select "# registers sufficient controlled >= 1", count(DISTINCT pc) as total_unique_pc, count(DISTINCT pc_symbol) as total_unique_symbol, count(DISTINCT address_symbol) as total_unique_entry
from victims
where n_controlled_sufficiently > 0
