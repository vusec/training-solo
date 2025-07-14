select MIN(CAST(n_instr as INT)) as "Number of instructions"
from gadgets_victims

where
CAST(n_instr as INT) <= 30 and CAST(msb_difference as INT) <= 9 and
(
(exploitable = 'True' AND base_has_indirect_secret_dependency = 'False')
OR

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


UNION ALL

select MIN(CAST(n_instr as INT)) as "All Gadgets"
from tfps_victims
where exploitable = 'True'
and CAST(n_instr as INT) <= 30 and CAST(msb_difference as INT) <= 9
group by pc

