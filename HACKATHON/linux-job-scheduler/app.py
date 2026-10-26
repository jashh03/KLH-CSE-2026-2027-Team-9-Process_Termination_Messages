from flask import Flask, render_template, request, jsonify
import subprocess
import threading
import time
import json
import os
from datetime import datetime

app = Flask(__name__)
DATA_FILE = "jobs.json"
LOG_FILE = "scheduler.log"
lock = threading.Lock()
jobs = []
next_id = 1

def save_jobs():
    with open(DATA_FILE, "w") as f:
        json.dump(jobs, f, indent=2)

def load_jobs():
    global jobs, next_id
    if os.path.exists(DATA_FILE):
        try:
            with open(DATA_FILE) as f:
                jobs = json.load(f)
            next_id = max([j["id"] for j in jobs], default=0) + 1
        except (ValueError, OSError):
            jobs = []

def log_event(job, message, status="INFO"):
    line = f"[{datetime.now().isoformat(timespec='seconds')}] Job {job['id']} | {status} | {message}\n"
    with open(LOG_FILE, "a") as f:
        f.write(line)

def execute_job(job):
    try:
        result = subprocess.run(
            ["./job_runner", job["command"]],
            capture_output=True,
            text=True,
            timeout=35
        )
        output = (result.stdout + result.stderr).strip()
        status = "SUCCESS" if result.returncode == 0 else "FAILED"
        log_event(
            job,
            f'{job["command"]} | {output} | exit={result.returncode}',
            status
        )
        with lock:
            job["last_status"] = status
            job["last_output"] = output[:2000]
            if job["interval"] == 0:
                job["completed"] = True
            else:
                job["next_run"] = time.time() + job["interval"]
            save_jobs()
    except subprocess.TimeoutExpired:
        log_event(job, "Command timed out", "FAILED")
        with lock:
            job["last_status"] = "FAILED"
            job["last_output"] = "Timed out"
            if job["interval"] == 0:
                job["completed"] = True
            else:
                job["next_run"] = time.time() + job["interval"]
            save_jobs()
    except OSError as e:
        log_event(job, str(e), "FAILED")
        with lock:
            job["last_status"] = "FAILED"
            job["last_output"] = str(e)
            if job["interval"] == 0:
                job["completed"] = True
            else:
                job["next_run"] = time.time() + job["interval"]
            save_jobs()

def scheduler_loop():
    while True:
        due = []
        now = time.time()
        with lock:
            for job in jobs:
                if job["enabled"] and not job["completed"] and job["next_run"] <= now and not job.get("running", False):
                    job["running"] = True
                    due.append(job)
        for job in due:
            try:
                execute_job(job)
            finally:
                with lock:
                    job["running"] = False
                    save_jobs()
        time.sleep(1)

@app.get("/")
def index():
    return render_template("index.html")

@app.get("/api/jobs")
def get_jobs():
    with lock:
        result = []
        for j in jobs:
            item = dict(j)
            item.pop("running", None)
            result.append(item)
    return jsonify(result)

@app.post("/api/jobs")
def add_job():
    global next_id
    data = request.get_json(silent=True) or {}
    command = str(data.get("command", "")).strip()
    try:
        delay = int(data.get("delay", 10))
        interval = int(data.get("interval", 0))
    except (ValueError, TypeError):
        return jsonify(error="Invalid delay or interval"), 400

    if not command or len(command) > 200 or "\n" in command:
        return jsonify(error="Enter a valid single-line command"), 400
    if delay < 1 or delay > 31536000 or interval not in (0, 10, 60, 3600, 86400):
        return jsonify(error="Invalid delay or repeat interval"), 400

    with lock:
        if len(jobs) >= 100:
            return jsonify(error="Job limit reached"), 400
        job = {
            "id": next_id, "command": command,
            "delay": delay, "interval": interval,
            "next_run": time.time() + delay,
            "enabled": True, "completed": False,
            "last_status": "PENDING", "last_output": ""
        }
        next_id += 1
        jobs.append(job)
        save_jobs()
    log_event(job, "Job scheduled")
    return jsonify(job), 201

@app.post("/api/jobs/<int:job_id>/<action>")
def job_action(job_id, action):
    with lock:
        job = next((j for j in jobs if j["id"] == job_id), None)
        if not job:
            return jsonify(error="Job not found"), 404

        if action == "pause":
            job["enabled"] = False
        elif action == "resume":
            if job["completed"]:
                return jsonify(error="Completed jobs cannot be resumed; create a new job"), 400
            job["enabled"] = True
            if job["next_run"] <= time.time():
                job["next_run"] = time.time() + 1
        elif action == "delete":
            if job.get("running"):
                return jsonify(error="Wait for the running job to finish"), 409
            jobs.remove(job)
        elif action == "run":
            if job.get("running"):
                return jsonify(error="Job is already running"), 409
            job["running"] = True
            threading.Thread(target=manual_run, args=(job,), daemon=True).start()
        else:
            return jsonify(error="Unknown action"), 400

        save_jobs()
    log_event(job, f"Action: {action}")
    return jsonify(ok=True)

def manual_run(job):
    try:
        execute_job(job)
    finally:
        with lock:
            job["running"] = False
            save_jobs()

@app.get("/api/logs")
def get_logs():
    if not os.path.exists(LOG_FILE):
        return jsonify(lines=[])
    with open(LOG_FILE) as f:
        return jsonify(lines=f.readlines()[-100:])

if __name__ == "__main__":
    load_jobs()
    threading.Thread(target=scheduler_loop, daemon=True).start()
    print("TaskFlow running at http://127.0.0.1:5000")
    app.run(host="127.0.0.1", port=5000, debug=False)
