select MIN(CAST(n_instr as INT)) as "Number of instructions"
from gadgets_victims
join dir_addresses_reachable on dir_addresses_reachable.address==dir_orig_address
where
CAST(n_instr as INT) <= 80 and
(transmitter=="TransmitterType.LOAD" or transmitter == "TransmitterType.STORE" or transmitter = "TransmitterType.CODE_LOAD")
and exploitable = 'True' AND base_has_indirect_secret_dependency = 'False'
and (target_bit_length == 32 OR target_bit_length == 10 OR (target_bit_length == 12 and tag_match_33$24_23$14 == "True"))

and CAST(n_victims_name as INT) > 0
-- and reachable=="True"
group by pc
