const $ = id => document.getElementById(id);
const esc = s => String(s ?? "").replace(/[&<>"']/g, c =>
  ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));

async function api(url, options = {}) {
  const response = await fetch(url, {
    ...options,
    headers: {"Content-Type":"application/json", ...(options.headers || {})}
  });
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || "Request failed");
  return data;
}

function actionButton(id, action, label) {
  return `<button class="action secondary" data-id="${id}" data-action="${action}">${label}</button>`;
}

async function refreshJobs() {
  try {
    const jobs = await api("/api/jobs");
    $("total").textContent = jobs.length;
    $("active").textContent = jobs.filter(j => j.enabled && !j.completed).length;
    $("completed").textContent = jobs.filter(j => j.completed).length;
    $("failed").textContent = jobs.filter(j => j.last_status === "FAILED").length;

    $("jobTable").innerHTML = jobs.length ? jobs.map(j => `
      <tr>
        <td>${j.id}</td>
        <td title="${esc(j.command)}">${esc(j.command)}</td>
        <td class="status">${j.completed ? "COMPLETED" : !j.enabled ? "PAUSED" : j.running ? "RUNNING" : esc(j.last_status)}</td>
        <td>${j.completed ? "—" : new Date(j.next_run * 1000).toLocaleString()}</td>
        <td>
          ${actionButton(j.id, "run", "Run now")}
          ${actionButton(j.id, j.enabled ? "pause" : "resume", j.enabled ? "Pause" : "Resume")}
          ${actionButton(j.id, "delete", "Delete")}
        </td>
      </tr>`).join("") : '<tr><td colspan="5">No jobs yet. Create your first job above.</td></tr>';
  } catch (e) {
    $("message").textContent = "Backend unavailable: " + e.message;
  }
}

async function refreshLogs() {
  try {
    const data = await api("/api/logs");
    $("logOutput").textContent = data.lines.join("") || "No execution logs yet.";
  } catch (e) {
    $("logOutput").textContent = "Unable to load logs: " + e.message;
  }
}

$("jobForm").addEventListener("submit", async e => {
  e.preventDefault();
  $("message").textContent = "Scheduling...";
  try {
    await api("/api/jobs", {
      method: "POST",
      body: JSON.stringify({
        command: $("command").value.trim(),
        delay: Number($("delay").value),
        interval: Number($("interval").value)
      })
    });
    $("message").textContent = "Job scheduled successfully!";
    $("jobForm").reset();
    $("delay").value = 10;
    await refreshJobs();
  } catch (err) {
    $("message").textContent = err.message;
  }
});

$("jobTable").addEventListener("click", async e => {
  const button = e.target.closest("button[data-action]");
  if (!button) return;
  const {id, action} = button.dataset;
  if (action === "delete" && !confirm("Delete job " + id + "?")) return;
  try {
    await api(`/api/jobs/${id}/${action}`, {method:"POST"});
    await refreshJobs();
    await refreshLogs();
  } catch (err) {
    alert(err.message);
  }
});

$("refresh").addEventListener("click", refreshJobs);
$("refreshLogs").addEventListener("click", refreshLogs);
refreshJobs();
refreshLogs();
setInterval(refreshJobs, 3000);
setInterval(refreshLogs, 5000);
