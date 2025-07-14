select MIN(CAST(n_instr as INT)) as "Number of instructions"
from tfps_victims

where exploitable = 'True'
and CAST(n_instr as INT) <= 27 and CAST(n_victims_name as INT) > 0
group by pc
