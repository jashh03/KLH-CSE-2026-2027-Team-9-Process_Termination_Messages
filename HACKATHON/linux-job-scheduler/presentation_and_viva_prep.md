# 🎓 TaskFlow — Linux Job Scheduler: Presentation & Viva Preparation

---

## 1. Project Overview (Elevator Pitch)

> **TaskFlow** is a web-based Linux job scheduling system that lets users **schedule, execute, pause, resume, and monitor** Linux shell commands through a browser dashboard. It combines a **C-based process execution engine** with a **Python Flask backend** and a responsive **HTML/CSS/JS frontend**, demonstrating real-world OS concepts like **process creation (`fork`/`exec`)**, **concurrency (threads + locks)**, and **inter-process communication (subprocess pipes)**.

### One-Liner for Presentation
*"We built a full-stack job scheduler that uses a C program as its execution engine — leveraging `fork()` and `exec()` system calls — orchestrated by a Python Flask server with a real-time web dashboard."*

---

## 2. Architecture Diagram

```mermaid
graph TD
    A["🌐 Browser (HTML/CSS/JS)"] -->|REST API calls| B["🐍 Flask Backend (app.py)"]
    B -->|subprocess.run()| C["⚙️ C Engine (job_runner)"]
    C -->|fork() + exec()| D["🐧 Linux Shell (/bin/sh)"]
    D -->|executes| E["📋 User Command (e.g. /usr/bin/date)"]
    B -->|read/write| F["📁 jobs.json (Persistence)"]
    B -->|append| G["📄 scheduler.log (Audit Trail)"]
    B -->|background thread| H["🔄 Scheduler Loop (polling every 1s)"]
    H -->|triggers| C
```

---

## 3. File-by-File Breakdown

### 3.1 `scheduler.c` → Compiled to `job_runner`

| Aspect | Detail |
|--------|--------|
| **Purpose** | Low-level process execution engine |
| **System Calls Used** | `fork()`, `execl()`, `waitpid()`, `_exit()` |
| **How it works** | Receives a shell command as `argv[1]` → forks a child process → child calls `execl("/bin/sh", "sh", "-c", command)` → parent waits with `waitpid()` → reports exit code |
| **Error Handling** | Checks `fork()` failure, `exec()` failure, `EINTR` during `waitpid()`, abnormal termination via `WIFEXITED`/`WEXITSTATUS` |
| **Compilation** | `gcc -o job_runner scheduler.c` |

**Key OS Concepts Demonstrated:**
- **Process Creation** — `fork()` creates an identical child process
- **Process Replacement** — `execl()` replaces child's memory image with `/bin/sh`
- **Process Synchronization** — `waitpid()` blocks parent until child exits
- **Exit Status Retrieval** — `WIFEXITED()` and `WEXITSTATUS()` macros

### 3.2 `app.py` — Flask Backend

| Aspect | Detail |
|--------|--------|
| **Framework** | Flask (Python micro-framework) |
| **Concurrency Model** | `threading.Thread` for scheduler loop + manual job runs; `threading.Lock` for shared data |
| **Persistence** | `jobs.json` (JSON file, written on every state change) |
| **Logging** | `scheduler.log` (append-only text log with timestamps) |
| **Job Execution** | Calls `./job_runner <command>` via `subprocess.run()` with 35-second timeout |
| **Job Limit** | Max 100 jobs |

**API Endpoints:**

| Method | Endpoint | Purpose |
|--------|----------|---------|
| `GET` | `/` | Serve the dashboard HTML |
| `GET` | `/api/jobs` | List all jobs (JSON) |
| `POST` | `/api/jobs` | Create a new scheduled job |
| `POST` | `/api/jobs/<id>/<action>` | Perform action: `pause`, `resume`, `delete`, `run` |
| `GET` | `/api/logs` | Get last 100 log lines |

**Scheduler Loop Logic:**
1. Runs in a daemon thread, loops every **1 second**
2. Acquires the lock, finds jobs where `enabled=True`, `completed=False`, and `next_run <= now`
3. Marks them as `running=True`, releases lock
4. Executes each job sequentially via the C engine
5. Updates status (`SUCCESS`/`FAILED`), resets `running=False`, recalculates `next_run` for recurring jobs

### 3.3 `index.html` — Dashboard UI

- Sidebar navigation (Dashboard / Scheduled Jobs / Execution Logs)
- **Stats cards**: Total Jobs, Active, Completed, Failed
- **Job creation form**: Command input, delay (seconds), repeat interval dropdown
- **Jobs table**: ID, Command, Status, Next Run, Actions (Run Now / Pause / Resume / Delete)
- **Log viewer**: Scrollable `<pre>` block showing recent scheduler activity

### 3.4 `script.js` — Frontend Logic

- **Auto-refresh**: Jobs poll every 3 seconds, logs every 5 seconds
- **XSS prevention**: `esc()` function escapes HTML special characters
- **Action delegation**: Event delegation on the job table for button clicks
- **Error handling**: Displays backend errors in the UI

### 3.5 `style.css` — Dark Theme Styling

- Dark gradient background (`#090d18`)
- Glassmorphism-inspired panels
- Purple accent colors (`#8772f5`, `#a99bff`)
- Responsive: 3 breakpoints (desktop → tablet → mobile)

### 3.6 `start.sh` — Startup Script

```bash
#!/bin/bash
cd "$HOME/linux-job-scheduler" || exit 1
source .venv/bin/activate
python app.py
```

---

## 4. Key OS Concepts Covered

### 4.1 Process Management
| Concept | Where Used | Explanation |
|---------|-----------|-------------|
| `fork()` | `scheduler.c` | Creates a child process; returns 0 to child, child PID to parent, -1 on error |
| `exec()` | `scheduler.c` | Replaces the current process image with a new program (`/bin/sh`) |
| `waitpid()` | `scheduler.c` | Parent blocks until child terminates; handles `EINTR` (interrupted system call) |
| `_exit()` | `scheduler.c` | Terminates child immediately without flushing stdio buffers (important after failed exec) |

### 4.2 Concurrency & Synchronization
| Concept | Where Used | Explanation |
|---------|-----------|-------------|
| **Threading** | `app.py` | Scheduler loop runs in a separate daemon thread |
| **Mutex Lock** | `app.py` | `threading.Lock()` protects the shared `jobs` list from race conditions |
| **Critical Section** | `app.py` | Every access to `jobs` is wrapped in `with lock:` |
| **Daemon Threads** | `app.py` | Background threads auto-terminate when main process exits |

### 4.3 Inter-Process Communication
| Concept | Where Used | Explanation |
|---------|-----------|-------------|
| **Pipes** | `app.py` → `job_runner` | `subprocess.run(capture_output=True)` creates pipes for stdout/stderr |
| **Exit Codes** | `scheduler.c` | Communicates success/failure back to Python via return code |
| **Command-line Args** | `scheduler.c` | Python passes the command string as `argv[1]` |

### 4.4 File System
| Concept | Where Used | Explanation |
|---------|-----------|-------------|
| **File I/O** | `jobs.json`, `scheduler.log` | Persistent storage and audit logging |
| **Append Mode** | `scheduler.log` | Log file opened with `"a"` mode — atomic appends |

---

## 5. How to Present (Suggested Flow — 10 minutes)

### Slide 1: Title & Introduction (1 min)
- Project name: **TaskFlow — Linux Job Scheduler**
- Team members, subject (Operating Systems)
- One-line summary

### Slide 2: Problem Statement (1 min)
- *"Linux has `cron` and `at` for scheduling, but they lack a real-time visual interface. We built a web-based alternative that demonstrates core OS concepts."*

### Slide 3: Architecture (2 min)
- Show the architecture diagram above
- Highlight the **3-tier design**: Browser → Flask → C Engine → Linux Kernel

### Slide 4: C Execution Engine — Deep Dive (2 min)
- Walk through `fork()` → `exec()` → `waitpid()` flow
- Show the code with annotations
- Emphasize: *"This is the same mechanism the Linux shell itself uses to run commands"*

### Slide 5: Backend & Concurrency (2 min)
- Explain the scheduler loop (polling model)
- Show how `threading.Lock` prevents race conditions
- Mention input validation (command length, allowed intervals)

### Slide 6: Live Demo (2 min)
- Schedule a `/usr/bin/date` job with 5-second delay
- Show it transition from PENDING → SUCCESS
- Pause/Resume a recurring job
- Show the execution logs updating in real-time

### Slide 7: Conclusion & Future Scope (1 min)
- Summary of OS concepts demonstrated
- Future improvements (see Section 7)

---

## 6. Viva Questions & Answers

### 🔵 Basic Level

**Q1: What is `fork()` and what does it return?**
> `fork()` is a system call that creates a new process by duplicating the calling process. It returns:
> - **0** to the child process
> - **Child's PID** (positive integer) to the parent process
> - **-1** on failure (no child is created)

**Q2: Why do you use `execl()` instead of `system()`?**
> `execl()` replaces the current process image, giving us fine-grained control — we can track the child PID with `waitpid()`, retrieve the exact exit status, and handle signals. `system()` internally does `fork + exec + waitpid` but hides error details and introduces security concerns (shell interpretation).

**Q3: What happens if `exec()` fails?**
> The child process continues executing from the line after `execl()`. In our code, we call `perror("exec")` and then `_exit(127)` to terminate the child. We use `_exit()` instead of `exit()` to avoid flushing the parent's stdio buffers.

**Q4: Why `_exit()` instead of `exit()` after a failed `exec()`?**
> `exit()` flushes all stdio buffers and runs `atexit()` handlers, which could corrupt the parent's buffered output. `_exit()` terminates immediately without cleanup — safe for a forked child.

**Q5: What does `waitpid()` do?**
> It suspends the calling process until the specified child terminates. It fills a `status` variable that can be inspected with macros like `WIFEXITED(status)` (did it exit normally?) and `WEXITSTATUS(status)` (what was the exit code?).

**Q6: What is the purpose of the `EINTR` check in `waitpid()`?**
> If a signal interrupts `waitpid()` before the child exits, it returns -1 and sets `errno` to `EINTR`. Our `while` loop retries `waitpid()` in that case, making the wait robust against signal interruptions.

---

### 🟡 Intermediate Level

**Q7: Why use a threading lock in app.py?**
> The `jobs` list is shared between the Flask request-handling threads and the background scheduler thread. Without a lock, concurrent reads/writes could cause race conditions — e.g., a job being modified while the scheduler iterates over the list, or two threads writing to `jobs.json` simultaneously.

**Q8: What is a daemon thread and why use it?**
> A daemon thread runs in the background and is automatically killed when the main program exits. The scheduler loop and manual-run threads are daemons so the server can shut down cleanly without waiting for them.

**Q9: How does `subprocess.run()` communicate with the C program?**
> Python creates a child process (internally using `fork`/`exec`), passes `argv[1]` as the command string, and uses **pipes** for `stdout` and `stderr` (`capture_output=True`). The return code is accessible via `result.returncode`.

**Q10: What happens if a job command takes too long?**
> `subprocess.run()` has a `timeout=35` parameter. If the C engine doesn't return within 35 seconds, Python raises `subprocess.TimeoutExpired`, the job is marked as `FAILED` with output `"Timed out"`, and the process is killed.

**Q11: How do you handle recurring vs one-time jobs?**
> Each job has an `interval` field. If `interval == 0`, it's a one-time job — after execution, `completed` is set to `True`. If `interval > 0`, the scheduler recalculates `next_run = time.time() + interval` after each execution, keeping the job in the queue.

**Q12: What input validation do you perform?**
> - Command: non-empty, max 200 chars, no newlines
> - Delay: 1 to 31,536,000 seconds (1 year max)
> - Interval: must be one of `{0, 10, 60, 3600, 86400}`
> - Max 100 total jobs
> - XSS prevention in the frontend via HTML escaping

**Q13: How does the frontend prevent XSS attacks?**
> The `esc()` function in `script.js` replaces `&`, `<`, `>`, `"`, and `'` with their HTML entities before inserting user-supplied data (like command strings) into the DOM.

---

### 🔴 Advanced Level

**Q14: What are the limitations of your polling-based scheduler?**
> The scheduler polls every 1 second, introducing up to 1-second delay. A more efficient approach would be an **event-driven model** using `threading.Event` with timeouts, or a priority queue sorted by `next_run` time (like `cron` uses internally). The current approach also executes due jobs **sequentially** within each polling cycle, which could cause delays if many jobs are due simultaneously.

**Q15: Is your `jobs.json` persistence thread-safe?**
> It's protected by the application-level `threading.Lock`, so only one thread writes at a time. However, it's not crash-safe — if the process dies mid-write, the JSON file could be corrupted. A more robust approach would be **write-to-temp + atomic rename**, or using SQLite with WAL mode.

**Q16: What is `_POSIX_C_SOURCE 200809L` at the top of scheduler.c?**
> It's a feature-test macro that tells the C standard library to expose POSIX.1-2008 functions and definitions. It ensures functions like `fork()`, `execl()`, and `waitpid()` are available with their full specifications.

**Q17: Could you replace the C engine with a pure Python solution?**
> Yes — Python's `subprocess` module or `os.fork()` could do the same thing. But the C engine demonstrates **direct system call usage**, gives finer control over the `fork`/`exec` lifecycle, and showcases the layered architecture (high-level orchestration in Python, low-level execution in C).

**Q18: How would you make this production-ready?**
> - Replace JSON file with a **database** (SQLite/PostgreSQL)
> - Use **atomic file writes** (write + rename) for crash safety
> - Add **authentication** (login system, API tokens)
> - **Sandbox commands** using `seccomp`, namespaces, or containers
> - Use a **process pool** for parallel job execution
> - Replace polling with an **event-driven scheduler** (min-heap priority queue)
> - Add **HTTPS** and **CSRF protection**

**Q19: What is the zombie process problem and does your code handle it?**
> A zombie process occurs when a child exits but the parent hasn't called `waitpid()` yet — the child's entry remains in the process table. Our code handles this correctly: the parent always calls `waitpid()` in a loop, ensuring the child's resources are reaped. If we used `WNOHANG` or forgot to wait, zombies could accumulate.

**Q20: What happens if someone schedules `rm -rf /` through the web UI?**
> Currently, there's no command sandboxing — it would attempt to run it (though it would fail without `--no-preserve-root` and `sudo`). In production, you'd need:
> - A **whitelist** of allowed commands
> - Run the C engine with **minimal privileges** (dedicated user)
> - Use **Linux namespaces/containers** to isolate execution
> - Implement **`seccomp` filters** to restrict system calls

---

## 7. Future Scope (Good to Mention)

| Feature | Description |
|---------|-------------|
| **Database Backend** | Replace `jobs.json` with SQLite for ACID guarantees |
| **Job Priority** | Assign priority levels, execute higher-priority jobs first |
| **Cron Expression Support** | Parse cron syntax (`*/5 * * * *`) instead of fixed intervals |
| **Email/Webhook Notifications** | Alert users when jobs fail |
| **Job Output History** | Store output of every execution, not just the last one |
| **User Authentication** | Login system with role-based access |
| **Container Isolation** | Run each job in a Docker container for security |
| **WebSocket Live Updates** | Replace polling with real-time push notifications |

---

## 8. Quick Reference — Technical Terms to Know

| Term | Definition |
|------|-----------|
| **Process** | An instance of a running program with its own memory space, PID, and resources |
| **Fork** | System call that creates a new process by duplicating the caller |
| **Exec** | System call family that replaces current process image with a new program |
| **PID** | Process Identifier — unique integer assigned by the kernel |
| **Zombie Process** | Terminated child whose exit status hasn't been collected by parent |
| **Orphan Process** | Child whose parent has terminated; adopted by `init` (PID 1) |
| **Race Condition** | Bug where behavior depends on the timing of thread execution |
| **Mutex** | Mutual exclusion lock — ensures only one thread accesses a resource |
| **Daemon** | Background process/thread that runs independently of user interaction |
| **Pipe** | Unidirectional IPC mechanism; one end writes, the other reads |
| **REST API** | Architectural style using HTTP methods (GET/POST/PUT/DELETE) for CRUD operations |
| **SIGCHLD** | Signal sent to parent when a child process terminates |

---

> [!TIP]
> **During the demo**, schedule a `/usr/bin/uptime` or `/usr/bin/df -h` command — these produce interesting output that makes the demo more engaging than just `/usr/bin/date`.

> [!IMPORTANT]
> **Key differentiator to emphasize**: Unlike `cron`, your project provides a **real-time web dashboard** and uses a **dedicated C engine** for process execution, demonstrating the full `fork`→`exec`→`wait` lifecycle that the Linux kernel uses internally.
