select MAX(CAST(n_controlled_sufficiently as INT) + has_indirect_reg_control) as "Number of Registers Sufficiently Controlled", MIN(CAST(n_control_flow_changes as INT)) as "Number of Controlled Flow Changes", name, pc
-- controlled_sufficiently, controlled_sufficiently_indirect, has_indirect_reg_control
from
	(select *, case when
	(
	controlled_sufficiently_indirect like "%rax%"
	OR controlled_sufficiently_indirect like "%rbx%"
	OR controlled_sufficiently_indirect like "%rsi%"
	OR controlled_sufficiently_indirect like "%rdx%"
	OR controlled_sufficiently_indirect like "%rcx%"
	OR controlled_sufficiently_indirect like "%r8%"
	OR controlled_sufficiently_indirect like "%r9%"
	OR controlled_sufficiently_indirect like "%r10%"
	OR controlled_sufficiently_indirect like "%r11%"
	OR controlled_sufficiently_indirect like "%r12%"
	OR controlled_sufficiently_indirect like "%r13%"
	OR controlled_sufficiently_indirect like "%r14%"
	OR controlled_sufficiently_indirect like "%r15%"
	) then 1 else 0 end as "has_indirect_reg_control"
	from victims
	)

where (CAST(n_controlled_sufficiently as INT) > 0 or has_indirect_reg_control == 1)
group by pc,n_controlled_sufficiently
ORDER by CAST(n_controlled_sufficiently as INT) DESC
