#!/bin/bash
# Execute a subprocess in a network namespace
# Copyright  Huawei Technologies Co., Ltd. 2010-2020. All rights reserved.

set -e

readonly NETNS="ns-$(mktemp -u XXXXXX)"

setup() {
	ip netns add "${NETNS}"
	ip -netns "${NETNS}" link set lo up
}

cleanup() {
	ip netns del "${NETNS}"
}

trap cleanup EXIT
setup

"$@"
exit "$?"
