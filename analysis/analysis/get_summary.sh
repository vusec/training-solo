OWN_DIR=`dirname "$0"`
echo $OWN_DIR

# --
echo "==========================================================="
echo "Found victims"

sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/summary_victims.sql

# --
echo "==========================================================="
echo "Found exploitable gadgets per type and CPU"
sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/exploitable_summary.sql

# --
echo "==========================================================="
echo "Found exploitable gadgets per type and CPU with a matching victim"

sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/exploitable_summary_with_victim.sql


# --
echo "==========================================================="
echo "Found exploitable gadgets per type and CPU with a matching victim, n_instr <= 30"

sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/exploitable_summary_with_victim_max_30_insts.sql


# --
echo "==========================================================="
echo "Exploitable TFPs"

sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/exploitable_tfp.sql

# --
echo "==========================================================="
echo "Exploitable Gadgets"

sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/exploitable_trans.sql

# --
echo "==========================================================="
echo "Exploitable Secret Dependent Branch"

sqlite3 gadgets.db --cmd  ".mode table" < ${OWN_DIR}/queries/exploitable_sdb.sql

