select MIN(CAST(n_instr as INT)) as "Number of instructions"
from tfps

where exploitable = 'True'
and CAST(n_instr as INT) <= 27
group by pc
