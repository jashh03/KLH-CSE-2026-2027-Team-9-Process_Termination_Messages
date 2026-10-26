#!/bin/bash
cd "$HOME/linux-job-scheduler" || exit 1
source .venv/bin/activate
python app.py
