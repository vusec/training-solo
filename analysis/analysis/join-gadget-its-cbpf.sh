CURRENT_DIR=`dirname $0`

echo "[-] Joining found gadgets with cbpf its file"
sqlite3 < ${CURRENT_DIR}/queries/join_gadget_its_cbpf.sql
