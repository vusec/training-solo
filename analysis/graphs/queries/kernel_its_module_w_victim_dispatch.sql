select MIN(CAST(n_instr as INT)) as "Number of instructions"
from tfps_victims
where
-- CAST(n_instr as INT) <= 80 and
exploitable = 'True'

-- == query dependent conditions
and CAST(n_victims_name as INT) > 0
-- and reachable=="True"
-- and tag_match_33$24_23$14 == "True"
-- ==
group by pc
-- order by CAST(n_instr as INT) DESC
