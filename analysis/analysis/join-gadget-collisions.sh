CURRENT_DIR=`dirname $0`

echo "[-] Joining found gadgets with collisions file"
sqlite3 < ${CURRENT_DIR}/queries/join_gadget_collisions.sql
