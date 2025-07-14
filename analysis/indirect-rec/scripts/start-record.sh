#!/bin/bash
set -e

echo "" > /sys/kernel/debug/tracing/kprobe_events


# Instal kprobes
echo "[+] Installing kprobes"

cat kprobe_events | while read p
do
    echo $p >> /sys/kernel/debug/tracing/kprobe_events 2>/dev/null || true
done

echo " - Total kprobes:" `wc -l kprobe_events`
echo " - Installed kprobes:" `wc -l /sys/kernel/debug/tracing/kprobe_events`

echo "[+] Installing kprobes ret"

cat kprobe_events_ret | while read p
do
    echo $p >> /sys/kernel/debug/tracing/kprobe_events 2>/dev/null || true
done

echo " - Total kprobes ret:" `wc -l kprobe_events_ret`
echo " - Installed kprobes:" `wc -l /sys/kernel/debug/tracing/kprobe_events`

# Enable kprobe histograms
echo "[+] Enabling kprobes histograms"

for file in /sys/kernel/debug/tracing/events/kprobes/ind_*/trigger
do
    # echo "hist:keys=__probe_ip.hex,target.hex,common_stacktrace:vals=1" > $file
    echo "hist:keys=__probe_ip.hex,target.hex:vals=1" > $file
done

for file in /sys/kernel/debug/tracing/events/kprobes/ret_*/trigger
do
    # echo "hist:keys=__probe_ip.hex,target.hex,common_stacktrace:vals=1" > $file
    echo "hist:keys=__probe_ip.hex,call.hex:vals=1" > $file
done


# Enable tracing
echo "[+] Enabling tracing"
echo 1 > /sys/kernel/debug/tracing/events/kprobes/enable

# Execute corpus
echo "[+] Executing courpus"

time timeout 30m ./syz-execprog -procs `nproc` -sandbox 'setuid' -disable '' ci-upstream-kasan-gce-root-corpus.db || true

# Disable tracing

echo "[+] Disabling tracing"

echo 0 > /sys/kernel/debug/tracing/events/kprobes/enable

# Aggegrate logs
echo "[+] Aggegrate logs"

cat /sys/kernel/debug/tracing/events/kprobes/*/hist > kprobe_hist_full
cat /sys/kernel/debug/tracing/events/kprobes/*/hist | grep "target:" > kprobe_hist
cat /sys/kernel/debug/tracing/events/kprobes/*/hist | grep "call:" > kprobe_hist_ret

echo "[+] Number of log lines: `wc -l kprobe_hist`"

touch magic

echo "[+] Done"
