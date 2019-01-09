#!/usr/bin/env bash
###############################################
#  Run keosd
#  use wallet located in ./wallet
#
#  ./run-keosd.sh & + press Ctrl+C  to run in detached mode
###############################################
config_dir=$(pwd)/config/
wallet_dir=$(pwd)/wallet/
echo $config_dir
echo $wallet_dir

wallet_host="127.0.0.1"
wallet_port="3000"

NODEHOST="127.0.0.1"
NODEPORT="8888"

keosd --unlock-timeout=40000 --config-dir=$config_dir --wallet-dir=$wallet_dir --http-server-address=$wallet_host:$wallet_port