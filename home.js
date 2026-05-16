/**
 * home.js — Home page logic for PowerWatch Kenya
 * Loads stats and live feed on page load
 */

document.addEventListener('DOMContentLoaded', async () => {
  await loadStats();
  await loadLiveFeed();
  updateSystemStatus(true);
});

/** Load summary statistics into hero stat cards */
async function loadStats() {
  try {
    const stats = await apiStats();
    document.querySelector('#stat-total  .stat-num').textContent = stats.totalReports;
    document.querySelector('#stat-active .stat-num').textContent = stats.activeOutages;
    document.querySelector('#stat-resolved .stat-num').textContent = stats.resolvedCount;
    document.querySelector('#stat-avg .stat-num').textContent = stats.avgDuration.toFixed(1);
  } catch(e) {
    console.error('Stats load failed', e);
  }
}

/** Load the 5 most recent reports into the live feed */
async function loadLiveFeed() {
  const feedEl = document.getElementById('liveFeed');
  try {
    const reports = await apiList();
    feedEl.innerHTML = '';

    const latest = reports.slice(0, 5);
    if (latest.length === 0) {
      feedEl.innerHTML = '<div class="feed-loading">No reports yet. Be the first to report!</div>';
      return;
    }

    latest.forEach(r => {
      const item = document.createElement('div');
      item.className = 'feed-item';
      item.innerHTML = `
        <div class="feed-id">#${r.id}</div>
        <div>
          <div class="feed-loc">${escHtml(r.location)}</div>
          <div class="feed-by">Reported by ${escHtml(r.reportedBy)}</div>
        </div>
        <div class="feed-time">${escHtml(r.startTime)}</div>
        <div>${statusBadge(r.status)}</div>
      `;
      feedEl.appendChild(item);
    });
  } catch(e) {
    feedEl.innerHTML = '<div class="feed-loading">Failed to load reports.</div>';
  }
}

/** Update the navbar online/offline indicator */
function updateSystemStatus(online) {
  const dot  = document.getElementById('systemStatus');
  const text = document.getElementById('systemStatusText');
  if (online) {
    dot.classList.add('online');
    text.textContent = 'System Online';
  } else {
    dot.classList.add('offline');
    text.textContent = 'Offline';
  }
}

function escHtml(str) {
  const d = document.createElement('div');
  d.textContent = str;
  return d.innerHTML;
}
