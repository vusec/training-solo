select "rax" as register, count(DISTINCT PC) as total
from victims
where rax_controlled_sufficiently = "True"

UNION ALL

select "rbx" as register, count(DISTINCT PC) as total
from victims
where rbx_controlled_sufficiently = "True"

UNION ALL

select "rcx" as register, count(DISTINCT PC) as total
from victims
where rcx_controlled_sufficiently = "True"

UNION ALL

select "rdx" as register, count(DISTINCT PC) as total
from victims
where rdx_controlled_sufficiently = "True"

UNION ALL

select "rsi" as register, count(DISTINCT PC) as total
from victims
where rsi_controlled_sufficiently = "True"

UNION ALL

select "rdi" as register, count(DISTINCT PC) as total
from victims
where rdi_controlled_sufficiently = "True"

UNION ALL

select "r8" as register, count(DISTINCT PC) as total
from victims
where r8_controlled_sufficiently = "True"

UNION ALL

select "r9" as register, count(DISTINCT PC) as total
from victims
where r9_controlled_sufficiently = "True"

UNION ALL

select "r10" as register, count(DISTINCT PC) as total
from victims
where r10_controlled_sufficiently = "True"

UNION ALL

select "r11" as register, count(DISTINCT PC) as total
from victims
where r11_controlled_sufficiently = "True"

UNION ALL

select "r12" as register, count(DISTINCT PC) as total
from victims
where r12_controlled_sufficiently = "True"

UNION ALL

select "r13" as register, count(DISTINCT PC) as total
from victims
where r13_controlled_sufficiently = "True"

UNION ALL

select "r14" as register, count(DISTINCT PC) as total
from victims
where r14_controlled_sufficiently = "True"

UNION ALL

select "r15" as register, count(DISTINCT PC) as total
from victims
where r15_controlled_sufficiently = "True"
-- group by rsi_controlled_sufficiently
-- order by total DESC

-- "rax", "rbx", "rcx", "rdx", "rsi",
--              "rdi", "rbp", "rsp", "r8" , "r9",
--              "r10", "r11", "r12", "r13", "r14", "r15"
