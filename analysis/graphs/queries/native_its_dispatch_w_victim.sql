select MIN(CAST(n_instr as INT)) as "Number of instructions"
from tfps_victims
join dir_addresses_reachable on dir_addresses_reachable.address==dir_orig_address
where
CAST(n_instr as INT) <= 80 and
(target_bit_length == 32 OR target_bit_length == 10 OR (target_bit_length == 12 and tag_match_33$24_23$14 == "True"))
and exploitable = 'True'

-- == query dependent conditions
and CAST(n_victims_name as INT) > 0
-- and reachable=="True"
-- and tag_match_33$24_23$14 == "True"
-- ==
group by pc
