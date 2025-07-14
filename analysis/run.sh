#!/bin/bash
set -e

start_run () {
    VERSION=$1
    FLAVOUR=$2
    if [ -z "$FLAVOUR" ]; then
        FLAVOUR=generic
    fi

    echo "======================"
    # echo Running version $VERSION $FLAVOUR
    # LINUX_DEB_URL=`apt download linux-image-${VERSION}-${FLAVOUR} --print-uris | awk '{print $1}' | tr -d "'"`
    # LINUX_DDEB_URL=`apt download linux-image-unsigned-${VERSION}-${FLAVOUR}-dbgsym --print-uris | awk '{print $1}' | tr -d "'"`
    # LINUX_MODULES_URL=`apt download linux-modules-${VERSION}-${FLAVOUR} --print-uris | awk '{print $1}' | tr -d "'"`

    echo "Running pre-configured version Ubuntu 6.8.0-38 22.04, uncomment for custom kernel"
    LINUX_DEB_URL=http://nl.archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.8/linux-image-6.8.0-38-generic_6.8.0-38.38%7e22.04.1_amd64.deb
    LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.8/linux-image-unsigned-6.8.0-38-generic-dbgsym_6.8.0-38.38%7e22.04.1_amd64.ddeb
    LINUX_MODULES_URL=http://nl.archive.ubuntu.com/ubuntu/pool/main/l/linux-hwe-6.8/linux-modules-6.8.0-38-generic_6.8.0-38.38%7e22.04.1_amd64.deb


    if [ -z "$LINUX_DDEB_URL" ] || [ -z "$LINUX_DDEB_URL" ]; then
        echo "Could not find package version!"
        echo $VERSION >> fail.txt
        return 0
    fi


    export LINUX_VERSION=$VERSION
    export LINUX_FLAVOUR=$FLAVOUR
    export LINUX_DEB_URL=$LINUX_DEB_URL
    export LINUX_DDEB_URL=$LINUX_DDEB_URL
    export LINUX_MODULES_URL=$LINUX_MODULES_URL


    echo $LINUX_DEB_URL
    echo $LINUX_DDEB_URL
    echo $LINUX_MODULES_URL

    # Indirect target analysis, just for gadget/reachability analysis later
    ./rebuild-indirector.sh linux${VERSION}-${FLAVOUR}

    # Do collisions calculations, both ind-ind and ITS (dir-ind)
    # Figure 6 + 9. Used for gadget analysis later
    ./rebuild-collisions.sh linux${VERSION}-${FLAVOUR}

    # Victim analysis, Figure 5. Used for gadget analysis later
    # Gadget analysis. Figure 10
    ./rebuild-main.sh linux${VERSION}-${FLAVOUR}

    # Do gadget analysis for ITS cBPF exploit. Figure 8
    ./rebuild-its-cbpf.sh linux${VERSION}-${FLAVOUR}

    # Analyze gadgets at kernel module indirect branch targets. Figure 7.
    # To run this analysis: please build the fine-ibt kernel manually
    # and copy images files ($DIR_WORK/images/vmlinuz, $DIR_WORK/images/vmlinux, $DIR_WORK/images/vmlinux-dbg)
    # To enable fine-ibt in kernel config:
    #   scripts/config -e CONFIG_FINEIBT
    #   scripts/config -e CONFIG_CFI_CLANG

    # ./rebuild-module-targets.sh linux${VERSION}-${FLAVOUR}-fineibt

}


start_run $1
