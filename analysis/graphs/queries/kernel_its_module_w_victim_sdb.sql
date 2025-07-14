select MIN(CAST(n_instr as INT)) as "Number of instructions"
from gadgets_victims

where
-- CAST(n_instr as INT) <= 80 and
transmitter=="TransmitterType.SECRET_DEP_BRANCH"

-- == query dependent conditions
and CAST(n_victims_name as INT) > 0
-- and reachable=="True"
-- and tag_match_29$22_21$14 == "True"
-- ==

and
(
( transmitter = "TransmitterType.SECRET_DEP_BRANCH"
AND
(
	cmp_value_control = "ControlType.CONTROLLED"
		and
		(secret_address_requirements_indirect_regs != cmp_value_requirements_indirect_regs
		and secret_address_requirements_direct_regs != cmp_value_requirements_direct_regs
		and secret_address_requirements_regs != cmp_value_requirements_regs)

	OR
	(
	base_control == "ControlType.CONTROLLED" and
		(base_control_type == "BaseControlType.BASE_INDEPENDENT_FROM_SECRET" or base_control_type == "BaseControlType.COMPLEX_TRANSMISSION")
)))
)


group by pc
