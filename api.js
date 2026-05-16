/**
 * api.js — Shared API client for PowerWatch Kenya
 * Communicates with the C++ CGI backend via fetch()
 */

// ============================================================
// Configuration — update CGI path if deploying to a web server
// ============================================================
const API_BASE = '../backend/outage_api.cgi';   // Production CGI path
const USE_MOCK  = true;   // Set false when running real CGI server

// ============================================================
// MOCK DATA — mirrors C++ backend output for local development
// When USE_MOCK=true, API calls use this data instead of CGI
// ============================================================
const MOCK_REPORTS = [
  { id:1,  location:"Nairobi CBD", reportedBy:"John Kamau",    startTime:"2024-11-01 08:30", duration:3.5,  status:"resolved"  },
  { id:2,  location:"Westlands",   reportedBy:"Mary Wanjiku",  startTime:"2024-11-03 14:00", duration:2.0,  status:"resolved"  },
  { id:3,  location:"Nairobi CBD", reportedBy:"Peter Otieno",  startTime:"2024-11-05 09:15", duration:4.0,  status:"resolved"  },
  { id:4,  location:"Kisumu",      reportedBy:"Alice Achieng", startTime:"2024-11-06 11:00", duration:1.5,  status:"resolved"  },
  { id:5,  location:"Nairobi CBD", reportedBy:"Samuel Njoroge",startTime:"2024-11-08 07:45", duration:5.0,  status:"resolved"  },
  { id:6,  location:"Westlands",   reportedBy:"Grace Muthoni", startTime:"2024-11-10 16:30", duration:3.0,  status:"resolved"  },
  { id:7,  location:"Mombasa",     reportedBy:"Hassan Omar",   startTime:"2024-11-11 13:00", duration:2.5,  status:"resolved"  },
  { id:8,  location:"Nairobi CBD", reportedBy:"Faith Karimi",  startTime:"2024-11-13 08:00", duration:6.0,  status:"resolved"  },
  { id:9,  location:"Kisumu",      reportedBy:"Tom Ochieng",   startTime:"2024-11-14 10:30", duration:1.0,  status:"resolved"  },
  { id:10, location:"Westlands",   reportedBy:"Diana Chebet",  startTime:"2024-11-15 15:00", duration:2.0,  status:"resolved"  },
  { id:11, location:"Nairobi CBD", reportedBy:"Mike Gitonga",  startTime:"2024-11-16 09:00", duration:3.0,  status:"active"    },
  { id:12, location:"Mombasa",     reportedBy:"Fatuma Ali",    startTime:"2024-11-17 11:45", duration:0.0,  status:"active"    },
  { id:13, location:"Nakuru",      reportedBy:"James Ndegwa",  startTime:"2024-11-17 14:00", duration:0.0,  status:"active"    },
];

let _mockReports = [...MOCK_REPORTS];
let _nextId = 14;

// ============================================================
// MOCK PREDICTION LOGIC
// Mirrors the C++ SimplePrediction::predictOutageJSON()
// ============================================================
function mockPredict(location) {
  const filtered = _mockReports.filter(r => r.location === location);
  const freq = filtered.length;
  const resolved = filtered.filter(r => r.status === 'resolved');
  const avgDuration = resolved.length
    ? resolved.reduce((s,r) => s + r.duration, 0) / resolved.length
    : 0;

  let riskLevel, message, smsAlert;
  if (freq >= 5) {
    riskLevel = 'HIGH';
    message   = 'Frequent outages detected. High probability of another soon.';
    smsAlert  = 'ALERT: Possible outage in your area. Prepare backup power.';
  } else if (freq >= 2) {
    riskLevel = 'MEDIUM';
    message   = 'Occasional outages on record. Stay alert.';
    smsAlert  = 'Notice: Moderate outage risk in your area.';
  } else if (freq === 1) {
    riskLevel = 'LOW';
    message   = 'Only one prior outage recorded. Low risk.';
    smsAlert  = 'Info: Low outage risk in your area.';
  } else {
    riskLevel = 'NONE';
    message   = 'No outages recorded for this location.';
    smsAlert  = 'No outage risk detected.';
  }
  return { location, frequency: freq, avgDuration, riskLevel, message, smsAlert };
}

// ============================================================
// MOCK STATS
// ============================================================
function mockStats() {
  const active   = _mockReports.filter(r => r.status !== 'resolved').length;
  const resolved = _mockReports.filter(r => r.status === 'resolved').length;
  const totalDur = _mockReports.filter(r => r.status === 'resolved').reduce((s,r) => s+r.duration, 0);
  const avgDuration = resolved ? (totalDur / resolved) : 0;
  return { totalReports: _mockReports.length, activeOutages: active, resolvedCount: resolved, avgDuration };
}

// ============================================================
// API FUNCTIONS
// Each function tries the real CGI backend first.
// Falls back to mock data if USE_MOCK=true or CGI unavailable.
// ============================================================

/** Fetch all outage reports */
async function apiList() {
  if (USE_MOCK) return [..._mockReports];
  try {
    const res  = await fetch(`${API_BASE}?action=list`);
    return await res.json();
  } catch(e) {
    console.warn('CGI unavailable, using mock data', e);
    return [..._mockReports];
  }
}

/** Get summary statistics */
async function apiStats() {
  if (USE_MOCK) return mockStats();
  try {
    const res = await fetch(`${API_BASE}?action=stats`);
    return await res.json();
  } catch(e) {
    return mockStats();
  }
}

/** Add a new outage report */
async function apiAdd(name, location, startTime, userType = 'resident', duration = 0) {
  if (USE_MOCK) {
    const newReport = {
      id: _nextId++,
      location, reportedBy: name, startTime, duration: parseFloat(duration),
      status: parseFloat(duration) > 0 ? 'resolved' : 'active'
    };
    _mockReports.unshift(newReport);
    return { success: true, message: 'Report submitted successfully.', totalReports: _mockReports.length };
  }
  try {
    const params = new URLSearchParams({ action:'add', name, location, startTime, userType, duration });
    const res = await fetch(`${API_BASE}?${params}`);
    return await res.json();
  } catch(e) {
    console.error('Add failed', e);
    return { success: false, message: String(e) };
  }
}

/** Run prediction for a location */
async function apiPredict(location) {
  if (USE_MOCK) return mockPredict(location);
  try {
    const res = await fetch(`${API_BASE}?action=predict&location=${encodeURIComponent(location)}`);
    return await res.json();
  } catch(e) {
    return mockPredict(location);
  }
}

/** Resolve a report */
async function apiResolve(id, duration) {
  if (USE_MOCK) {
    const r = _mockReports.find(x => x.id === parseInt(id));
    if (r) { r.status = 'resolved'; r.duration = parseFloat(duration); }
    return { success: true, message: 'Report resolved.' };
  }
  try {
    const params = new URLSearchParams({ action:'resolve', id, duration });
    const res = await fetch(`${API_BASE}?${params}`);
    return await res.json();
  } catch(e) {
    return { success: false, message: String(e) };
  }
}

// ============================================================
// UTILITY HELPERS
// ============================================================

/** Return badge HTML for a status string */
function statusBadge(status) {
  const map = {
    active:    '<span class="badge badge-active">Active</span>',
    resolved:  '<span class="badge badge-resolved">Resolved</span>',
    confirmed: '<span class="badge badge-confirmed">Confirmed</span>',
  };
  return map[status] || `<span class="badge">${status}</span>`;
}

/** Show a toast notification */
function showToast(msg, type = 'info') {
  let container = document.getElementById('toast-container');
  if (!container) {
    container = document.createElement('div');
    container.id = 'toast-container';
    document.body.appendChild(container);
  }
  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  toast.textContent = msg;
  container.appendChild(toast);
  setTimeout(() => toast.remove(), 4000);
}

/** Get unique locations from reports */
function uniqueLocations(reports) {
  return [...new Set(reports.map(r => r.location))].sort();
}

/** Format duration nicely */
function fmtDuration(h) {
  if (!h) return '—';
  if (h < 1) return `${Math.round(h * 60)}m`;
  return `${h.toFixed(1)}h`;
}
