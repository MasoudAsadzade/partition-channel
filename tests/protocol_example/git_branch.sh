#!/usr/bin/env sh
# -*- coding: utf-8 -*-

cd "$(dirname "$0")"
git rev-parse --abbrev-ref HEAD | tr -d '\n'