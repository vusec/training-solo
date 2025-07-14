select MIN(CAST(n_instr as INT)) as "Number of instructions"
from gadgets

where (CAST(n_instr as INT) <= 27 and (transmitter=="TransmitterType.LOAD" or transmitter == "TransmitterType.STORE" or transmitter = "TransmitterType.CODE_LOAD"))
and exploitable = 'True' AND base_has_indirect_secret_dependency = 'False'
group by pc
