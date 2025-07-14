#!/bin/bash
DIR=`dirname $0`

sqlite3 < $DIR/queries/join-all.sql
